#include "McRave.h"

using namespace std;
using namespace BWAPI;
using namespace UnitTypes;

namespace McRave
{
    UnitInfo::UnitInfo() {}

    void UnitInfo::setLastPositions()
    {
        lastPos = getPosition();
        lastTile = getTilePosition();
        lastWalk =  walkPosition;
    }

    void UnitInfo::verifyPaths()
    {
        if (lastTile != unit()->getTilePosition()) {
            BWEB::Path emptyPath;
            path = emptyPath;
        }
    }

    void UnitInfo::update()
    {
        auto t = unit()->getType();
        auto p = unit()->getPlayer();

        if (unit()->exists()) {

            setLastPositions();
            verifyPaths();

            // Points		
            position				= unit()->getPosition();
            destination				= Positions::Invalid;
            goal                    = Positions::Invalid;
            tilePosition			= Math::getTilePosition(unit());
            walkPosition			= Math::getWalkPosition(unit());

            // Stats
            type				    = t;
            player					= p;
            health					= unit()->getHitPoints() > 0 ? unit()->getHitPoints() : health;
            shields					= unit()->getShields() > 0 ? unit()->getShields() : shields;
            energy					= unit()->getEnergy();
            percentHealth			= t.maxHitPoints() > 0 ? double(health) / double(t.maxHitPoints()) : 0.0;
            percentShield			= t.maxShields() > 0 ? double(shields) / double(t.maxShields()) : 0.0;
            percentTotal			= t.maxHitPoints() + t.maxShields() > 0 ? double(health + shields) / double(t.maxHitPoints() + t.maxShields()) : 0.0;
            groundRange				= Math::groundRange(*this);
            groundDamage			= Math::groundDamage(*this);
            groundReach				= Math::groundReach(*this);
            airRange				= Math::airRange(*this);
            airReach				= Math::airReach(*this);
            airDamage				= Math::airDamage(*this);
            speed 					= Math::moveSpeed(*this);
            minStopFrame			= Math::stopAnimationFrames(t);
            visibleGroundStrength	= Math::visibleGroundStrength(*this);
            maxGroundStrength		= Math::maxGroundStrength(*this);
            visibleAirStrength		= Math::visibleAirStrength(*this);
            maxAirStrength			= Math::maxAirStrength(*this);
            priority				= Math::priority(*this);


            flying					= unit()->isFlying() || unit()->getType().isFlyer() || unit()->getOrder() == Orders::LiftingOff || unit()->getOrder() == Orders::BuildingLiftOff;

            simRadius               = 640.0;//walkPosition.isValid() ? clamp(flying && !mapBWEM.GetArea(walkPosition) ? 480.0 : (mapBWEM.GetArea(walkPosition) ? 640.0 - mapBWEM.GetArea(walkPosition)->MaxAltitude() : simRadius), 400.0, 640.0) : simRadius;

            // States
            lState					= LocalState::None;
            gState					= GlobalState::None;
            tState					= TransportState::None;

            // Frames
            remainingTrainFrame =   max(0, remainingTrainFrame - 1);
            lastAttackFrame			= (t != Protoss_Reaver && (unit()->isStartingAttack() || unit()->isRepairing())) ? Broodwar->getFrameCount() : lastAttackFrame;

            // BWAPI won't reveal isStartingAttack when hold position is executed if the unit can't use hold position, XIMP uses this on workers
            if (getPlayer() != Broodwar->self() && getType().isWorker()) {
                if (unit()->getGroundWeaponCooldown() == 1 || unit()->getAirWeaponCooldown() == 1)
                    lastAttackFrame = Broodwar->getFrameCount();
            }

            target = weak_ptr<UnitInfo>();
            updateTarget();
            updateHidden();
            updateStuckCheck();
        }


        if (getPlayer()->isEnemy(Broodwar->self()))
            updateThreatening();
    }

    void UnitInfo::updateTarget()
    {
        // Update my target
        if (getPlayer() == Broodwar->self()) {
            if (getType() == Terran_Vulture_Spider_Mine) {
                if (unit()->getOrderTarget())
                    target = Units::getUnitInfo(unit()->getOrderTarget());
            }
            else
                Targets::getTarget(*this);
        }

        // Assume enemy target
        else if (getPlayer()->isEnemy(Broodwar->self())) {
            if (unit()->getOrderTarget()) {
                auto &targetInfo = Units::getUnitInfo(unit()->getOrderTarget());
                if (targetInfo) {
                    target = targetInfo;
                    targetInfo->getTargetedBy().push_back(make_shared<UnitInfo>(*this));
                }
            }
            else
                target = weak_ptr<UnitInfo>();
        }
    }

    void UnitInfo::updateStuckCheck() {

        // Check if a worker is being blocked from mining or returning cargo
        if (unit()->isCarryingGas() || unit()->isCarryingMinerals())
            resourceHeldFrames = max(resourceHeldFrames, 0) + 1;
        else if (unit()->isGatheringGas() || unit()->isGatheringMinerals())
            resourceHeldFrames = min(resourceHeldFrames, 0) - 1;
        else
            resourceHeldFrames = 0;

        // Check if clipped between terrain or buildings
        if (getTilePosition().isValid() && BWEB::Map::isUsed(getTilePosition()) == None && Util::isWalkable(getTilePosition())) {

            vector<TilePosition> directions{ {1,0}, {-1,0}, {0, 1}, {0,-1} };
            bool trapped = true;

            // Check if the unit is stuck by terrain and buildings
            for (auto &tile : directions) {
                auto current = getTilePosition() + tile;
                if (BWEB::Map::isUsed(current) == None && Util::isWalkable(current))
                    trapped = false;
            }

            // Check if the unit intends to be here to build
            if (getBuildPosition().isValid()) {
                const auto center = Position(getBuildPosition()) + Position(getBuildType().tileWidth() * 16, getBuildType().tileHeight() * 16);
                if (center.getDistance(getPosition()) <= 160.0)
                    trapped = false;

                auto cnt = 0;
                for (auto &tile : directions) {
                    auto current = getTilePosition() + tile;
                    if (BWEB::Map::isUsed(current).isMineralField())
                        cnt++;
                }

                if (cnt >= 3)
                    trapped = true;                
            }

            if (trapped)
                lastTileMoveFrame = 0;
        }

        // Check if a unit hasn't moved in a while but is trying to
        if (getPlayer() != Broodwar->self() || getLastTile() != getTilePosition() || !unit()->isMoving() || unit()->getLastCommand().getType() == UnitCommandTypes::Stop || getLastAttackFrame() == Broodwar->getFrameCount())
            lastTileMoveFrame = Broodwar->getFrameCount();
    }

    void UnitInfo::updateHidden()
    {
        // Burrowed check for non spider mine type units or units we can see using the order for burrowing
        burrowed = (getType() != Terran_Vulture_Spider_Mine && unit()->isBurrowed()) || unit()->getOrder() == Orders::Burrowing;

        // If this is a spider mine and doesn't have a target, then it is an inactive mine and unable to attack
        if (getType() == Terran_Vulture_Spider_Mine && (!unit()->exists() || (!hasTarget() && unit()->getSecondaryOrder() == Orders::Cloak))) {
            burrowed = true;
            groundReach = getGroundRange();
        }

        // A unit is considered hidden if it is burrowed or cloaked and not under detection
        hidden = (burrowed || bwUnit->isCloaked())
            && (player->isEnemy(BWAPI::Broodwar->self()) ? !Actions::overlapsDetection(bwUnit, position, PlayerState::Self) : !Actions::overlapsDetection(bwUnit, position, PlayerState::Enemy));
    }

    void UnitInfo::updateThreatening()
    {
        // Determine how close it is to strategic locations
        auto choke = Terrain::isDefendNatural() ? BWEB::Map::getNaturalChoke() : BWEB::Map::getMainChoke();
        auto closestGeo = BWEB::Map::getClosestChokeTile(choke, getPosition());
        auto closestStation = Stations::getClosestStation(PlayerState::Self, getPosition());
        auto closestCannon = Util::getClosestUnit(getPosition(), PlayerState::Self, [&](auto &u) { return u.getType() == Protoss_Photon_Cannon; });

        // If the unit is close to stations, defenses or resources owned by us
        auto atHome = (Util::getTime() < Time(10, 0) || !closestStation) ? Terrain::isInAllyTerritory(getTilePosition()) : (closestStation && getPosition().getDistance(closestStation) < 640.0);
        auto atDefense = getPosition().getDistance(closestGeo) <= 96.0;
        auto atResources = getPosition().getDistance(Terrain::getMineralHoldPosition()) < 256.0;

        // If the unit attacked defenders, citizens or is building something
        auto attackedDefender = hasAttackedRecently() && hasTarget() && Terrain::isInAllyTerritory(getTarget().getTilePosition()) && getTarget().getType().canAttack();
        auto attackedWorkers = hasAttackedRecently() && hasTarget() && getTarget().getType().isWorker();
        auto attackedBuildings = hasAttackedRecently() && hasTarget() && getTarget().getType().isBuilding();
        auto constructing = unit()->exists() && (unit()->isConstructing() || unit()->getOrder() == Orders::ConstructingBuilding || unit()->getOrder() == Orders::PlaceBuilding);

        bool threateningThisFrame = false;

        // Building
        if (getType().isBuilding()) {
            threateningThisFrame = Buildings::overlapsQueue(*this, getTilePosition())
                || ((atDefense || atHome) && (airDamage > 0.0 || groundDamage > 0.0 || getType() == Protoss_Shield_Battery || getType().isRefinery()))
                || (atResources);
        }

        // Worker
        else if (getType().isWorker())
            threateningThisFrame = (atHome || atDefense) && (constructing || attackedWorkers || attackedDefender || attackedBuildings);

        else if (getType().isFlyer())
            threateningThisFrame = attackedDefender || attackedWorkers;

        // Unit
        else {
            if (Strategy::defendChoke())
                threateningThisFrame = (hasAttackedRecently() && (atDefense || atHome)) || attackedWorkers || attackedDefender || (atHome && threatening);
            else
                threateningThisFrame = (attackedWorkers || attackedBuildings || atResources);
        }

        //// Close to a shield battery
        //if (com(Protoss_Shield_Battery) > 0) {
        //    auto &battery = Util::getClosestUnit(getPosition(), PlayerState::Self, [&](auto &u) {
        //        return u.getType() == Protoss_Shield_Battery && u.unit()->isCompleted();
        //    });
        //    if (battery && getPosition().getDistance(battery->getPosition()) <= 128.0 && Terrain::isInAllyTerritory(getTilePosition()))
        //        threateningThisFrame = true;
        //}

        // If determined threatening this frame
        if (threateningThisFrame) {
            lastThreateningFrame = Broodwar->getFrameCount();
            threatening = true;
        }
        threatening = Broodwar->getFrameCount() - lastThreateningFrame < 24;
    }

    void UnitInfo::createDummy(UnitType t) {
        type				    = t;
        player					= Broodwar->self();
        groundRange				= Math::groundRange(*this);
        airRange		        = Math::airRange(*this);
        groundDamage			= Math::groundDamage(*this);
        airDamage				= Math::airDamage(*this);
        speed 					= Math::moveSpeed(*this);
    }

    bool UnitInfo::move(UnitCommandType command, Position here)
    {
        // Check if we need to wait a few frames before issuing a command due to stop frames
        auto frameSinceAttack = Broodwar->getFrameCount() - lastAttackFrame;
        auto cancelAttackRisk = frameSinceAttack <= minStopFrame - Broodwar->getLatencyFrames();

        if (cancelAttackRisk)
            return false;

        // Check if this is a new order
        const auto newOrder = [&]() {
            auto newOrderPosition = unit()->getOrderTargetPosition().getDistance(here) > 16;
            auto newOrderType = unit()->getOrder() != Orders::Move && unit()->getLastCommandFrame() + Broodwar->getLatencyFrames() > Broodwar->getFrameCount() + 50;
            return (newOrderPosition || newOrderType);
        };

        // Check if this is a new command
        const auto newCommand = [&]() {
            auto newCommandPosition = (unit()->getLastCommand().getType() != command || unit()->getLastCommand().getTargetPosition().getDistance(here) > 16);
            return newCommandPosition;
        };

        // Check if we should overshoot for halting distance
        if (getType().isFlyer()) {
            auto distance = getPosition().getApproxDistance(here);
            auto haltDistance = max({ distance, 32, getType().haltDistance() / 256 });

            if (distance > 0) {
                here = getPosition() - ((getPosition() - here) * haltDistance / distance);
                here = Util::clipLine(getPosition(), here);
            }
        }

        // Add action and grid movement
        if (command == UnitCommandTypes::Move && getPosition().getDistance(here) < 128.0) {
            Actions::addAction(unit(), here, getType(), PlayerState::Self);
            Grids::addMovement(WalkPosition(here), getType());
        }

        // If this is a new order or new command than what we're requesting, we can issue it
        if (newOrder() || newCommand()) {
            if (command == UnitCommandTypes::Move)
                unit()->move(here);
            else if (command == UnitCommandTypes::Stop)
                unit()->stop();
            return true;
        }
        return false;
    }

    bool UnitInfo::click(UnitCommandType command, UnitInfo& targetUnit)
    {
        // Check if we need to wait a few frames before issuing a command due to stop frames
        auto frameSinceAttack = Broodwar->getFrameCount() - lastAttackFrame;
        auto cancelAttackRisk = frameSinceAttack <= minStopFrame - Broodwar->getLatencyFrames();

        if (cancelAttackRisk)
            return false;

        // Check if this is a new order
        const auto newOrder = [&]() {
            auto newOrderTarget = unit()->getOrderTarget() && unit()->getOrderTarget() != targetUnit.unit();
            return newOrderTarget;
        };

        // Check if this is a new command
        const auto newCommand = [&]() {
            auto newCommandTarget = (unit()->getLastCommand().getType() != command || unit()->getLastCommand().getTarget() != targetUnit.unit());
            return newCommandTarget;
        };

        // Add action
        Actions::addAction(unit(), targetUnit.getPosition(), getType(), PlayerState::Self);

        // If this is a new order or new command than what we're requesting, we can issue it
        if (newOrder() || newCommand() || command == UnitCommandTypes::Right_Click_Unit) {
            if (command == UnitCommandTypes::Attack_Unit)
                unit()->attack(targetUnit.unit());
            else if (command == UnitCommandTypes::Right_Click_Unit)
                unit()->rightClick(targetUnit.unit());
            return true;
        }
        return false;
    }

    bool UnitInfo::canStartAttack()
    {
        if (!hasTarget()
            || (getGroundDamage() == 0 && getAirDamage() == 0)
            || isSpellcaster())
            return false;

        // Units that don't hover or fly have animation times to start and continue attacks
        auto cooldown = (getTarget().getType().isFlyer() ? unit()->getAirWeaponCooldown() : unit()->getGroundWeaponCooldown()) - Broodwar->getLatencyFrames();

        if (getType() == Protoss_Reaver)
            cooldown = lastAttackFrame - Broodwar->getFrameCount() + 60;

        auto cooldownReady = cooldown <= getEngDist() / (hasTransport() ? getTransport().getSpeed() : getSpeed());
        return cooldownReady;
    }

    bool UnitInfo::canStartGather()
    {
        const auto hasMineableResource = hasResource() && getResource().getResourceState() == ResourceState::Mineable;

        if (hasMineableResource && (isWithinGatherRange() || Grids::getAGroundCluster(getPosition()) > 0.0f) && getResource().unit()->exists() && unit()->getTarget() != getResource().unit())
            return true;
        if (!hasMineableResource && (unit()->isIdle() || unit()->getLastCommand().getType() != UnitCommandTypes::Gather))
            return true;
        if (Buildings::overlapsQueue(*this, getTilePosition()))
            return true;
        return false;
    }

    bool UnitInfo::canStartCast(TechType tech)
    {
        if (!hasTarget()
            || !getPlayer()->hasResearched(tech)
            || Actions::overlapsActions(unit(), getTarget().getPosition(), tech, PlayerState::Self, Util::getCastRadius(tech)))
            return false;

        auto energyNeeded = tech.energyCost() - energy;
        auto framesToEnergize = 17.856 * energyNeeded;
        auto spellReady = energy >= tech.energyCost();
        auto spellWillBeReady = framesToEnergize < getEngDist() / (hasTransport() ? getTransport().getSpeed() : getSpeed());

        if (!spellReady && !spellWillBeReady)
            return false;

        if (getType() == Protoss_High_Templar && getEngDist() >= 64.0)
            return true;

        auto ground = Grids::getEGroundCluster(getTarget().getPosition());
        auto air = Grids::getEAirCluster(getTarget().getPosition());

        if (ground + air >= Util::getCastLimit(tech) || (getType() == Protoss_High_Templar && getTarget().isHidden()))
            return true;
        return false;
    }

    bool UnitInfo::canAttackGround()
    {
        return getGroundDamage() > 0.0
            || getType() == Protoss_High_Templar
            || getType() == Protoss_Dark_Archon
            || getType() == Protoss_Carrier
            || getType() == Terran_Medic
            || getType() == Terran_Science_Vessel
            || getType() == Zerg_Defiler
            || getType() == Zerg_Queen;
    }

    bool UnitInfo::canAttackAir()
    {
        return getAirDamage() > 0.0
            || getType() == Protoss_High_Templar
            || getType() == Protoss_Dark_Archon
            || getType() == Protoss_Carrier
            || getType() == Terran_Science_Vessel
            || getType() == Zerg_Defiler
            || getType() == Zerg_Queen;
    }

    bool UnitInfo::isWithinReach(UnitInfo& unit)
    {
        auto sizes = (max(unit.getType().width(), unit.getType().height()) + max(getType().width(), getType().height())) / 2;
        auto dist = getPosition().getDistance(unit.getPosition()) - sizes - 32.0;
        return unit.getType().isFlyer() ? airReach >= dist : groundReach >= dist;
    }

    bool UnitInfo::isWithinRange(UnitInfo& unit)
    {
        auto sizes = (max(unit.getType().width(), unit.getType().height()) + max(getType().width(), getType().height())) / 2;
        auto dist = getPosition().getDistance(unit.getPosition()) - sizes - 32.0;
        return unit.getType().isFlyer() ? getAirRange() >= dist : getGroundRange() >= dist;
    }

    bool UnitInfo::isWithinBuildRange()
    {
        if (!getBuildPosition().isValid())
            return false;

        const auto center = Position(getBuildPosition()) + Position(getBuildType().tileWidth() * 16, getBuildType().tileHeight() * 16);
        const auto close = getPosition().getDistance(center) <= 160.0;
        const auto sameArea = mapBWEM.GetArea(getTilePosition()) == mapBWEM.GetArea(TilePosition(center));

        return close || sameArea;
    }

    bool UnitInfo::isWithinGatherRange()
    {
        if (!hasResource())
            return false;

        // Great Barrier Reef was causing issues with ground distance to the resource
        const auto pathPoint = Combat::getClosestRetreatPosition(getResource().getPosition());

        if (!pathPoint.isValid())
            return false;

        const auto close = BWEB::Map::getGroundDistance(pathPoint, getPosition()) <= 128.0 || getPosition().getDistance(pathPoint) < 64.0;
        const auto sameArea = mapBWEM.GetArea(getTilePosition()) == mapBWEM.GetArea(TilePosition(pathPoint));
        return close || sameArea;
    }

    bool UnitInfo::localEngage()
    {
        return ((!getType().isFlyer() && getTarget().isSiegeTank() && getTarget().getTargetedBy().size() >= 4 && ((isWithinRange(getTarget()) && getGroundRange() > 32.0) || (isWithinReach(getTarget()) && getGroundRange() <= 32.0)))
            || (isHidden() && !Actions::overlapsDetection(unit(), getEngagePosition(), PlayerState::Enemy))
            || (getType() == Protoss_Reaver && !unit()->isLoaded() && isWithinRange(getTarget()))
            || (getTarget().getType() == Terran_Vulture_Spider_Mine && !getTarget().isBurrowed())
            || (hasTransport() && !unit()->isLoaded() && getType() == Protoss_High_Templar && canStartCast(TechTypes::Psionic_Storm) && isWithinRange(getTarget()))
            || (hasTransport() && !unit()->isLoaded() && getType() == Protoss_Reaver && canStartAttack()) && isWithinRange(getTarget()));
    }

    bool UnitInfo::localRetreat()
    {
        return (getType() == Protoss_Zealot && Broodwar->self()->getUpgradeLevel(UpgradeTypes::Leg_Enhancements) == 0 && getTarget().getType() == Terran_Vulture)                 // ...unit is a slow Zealot attacking a Vulture
            || (isLightAir() && getType().maxShields() > 0 && getTarget().getType() == Zerg_Overlord && Grids::getEAirThreat(getEngagePosition()) * 5.0 > (double)getShields())   // ...unit is a low shield light air attacking a Overlord under threat greater than our shields
            || (getType() == Protoss_Corsair && getTarget().getType() == Zerg_Scourge && com(Protoss_Corsair) < 6)                                                                // ...unit is a Corsair attacking Scourge with less than 6 completed Corsairs
            || (getType() == Terran_Medic && unit()->getEnergy() <= TechTypes::Healing.energyCost())                                                                              // ...unit is a Medic with no energy
            || (getType() == Zerg_Mutalisk && Grids::getEAirThreat(getEngagePosition()) > 0.0 && getHealth() <= 30)                                                               // ...unit is a low HP Mutalisk attacking a target under air threat
            || (getType().maxShields() > 0 && getPercentShield() < LOW_SHIELD_PERCENT_LIMIT && !BuildOrder::isRush() && Broodwar->getFrameCount() < 10000)                        // ...unit is a low shield unit in the early stages of the game
            || (getType() == Terran_SCV && Broodwar->getFrameCount() > 12000);                                                                                                    // ...unit is an SCV outside of early game
    }

    bool UnitInfo::globalEngage()
    {
        return (getTarget().isThreatening() && !getTarget().isHidden())                                                                                                                           // ...target is threatening                    
            || (!getType().isWorker() && getGroundRange() > getTarget().getGroundRange() && Terrain::isInAllyTerritory(getTarget().getTilePosition()) && !getTarget().isHidden());                // ...unit can get free hits in our territory
    }

    bool UnitInfo::globalRetreat()
    {
        const auto targetReach = getType().isFlyer() ? getTarget().getAirReach() : getTarget().getGroundReach();

        return (getType().getRace() == Races::Zerg && !isHealthy())                                                                    // ...unit is a Zerg unit and is not healthy in early game
            || Grids::getESplash(getWalkPosition()) > 0                                                                                // ...unit is within splash radius of a Spider Mine or Scarab
            || (getTarget().isHidden() && getPosition().getDistance(getTarget().getPosition()) <= targetReach)                         // ...target is hidden and Unit is within target reach
            || getGlobalState() == GlobalState::Retreat;                                                                               // ...global state is retreating
    }
}