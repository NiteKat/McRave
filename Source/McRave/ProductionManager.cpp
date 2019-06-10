#include "McRave.h"

using namespace BWAPI;
using namespace std;
using namespace UnitTypes;

namespace McRave::Production {

    namespace {
        // TODO: Template these 3? Hold the min/gas value only?
        map <Unit, UnitType> idleProduction;
        map <Unit, TechType> idleTech;
        map <Unit, UpgradeType> idleUpgrade;
        map <UnitType, int> trainedThisFrame;
        int reservedMineral, reservedGas;
        int idleFrame = 0;

        bool haveOrUpgrading(UpgradeType upgrade, int level) {
            return ((Broodwar->self()->isUpgrading(upgrade) && Broodwar->self()->getUpgradeLevel(upgrade) == level - 1) || Broodwar->self()->getUpgradeLevel(upgrade) >= level);
        }

        bool isAffordable(UnitType unit)
        {
            auto mineralReserve = int(!BuildOrder::isTechUnit(unit)) * reservedMineral;
            auto gasReserve = int(!BuildOrder::isTechUnit(unit)) * reservedGas;
            auto mineralAffordable = (Broodwar->self()->minerals() >= unit.mineralPrice() + Buildings::getQueuedMineral() + mineralReserve) || unit.mineralPrice() == 0;
            auto gasAffordable = (Broodwar->self()->gas() >= unit.gasPrice() + Buildings::getQueuedGas() + gasReserve) || unit.gasPrice() == 0;
            auto supplyAffordable = Players::getSupply(PlayerState::Self) + unit.supplyRequired() <= Broodwar->self()->supplyTotal();

            return mineralAffordable && gasAffordable && supplyAffordable;
        }

        bool isAffordable(TechType tech)
        {
            return Broodwar->self()->minerals() >= tech.mineralPrice() && Broodwar->self()->gas() >= tech.gasPrice();
        }

        bool isAffordable(UpgradeType upgrade)
        {
            return Broodwar->self()->minerals() >= upgrade.mineralPrice() && Broodwar->self()->gas() >= upgrade.gasPrice();
        }

        bool isCreateable(Unit building, UnitType unit)
        {
            if (!BuildOrder::isUnitUnlocked(unit))
                return false;

            switch (unit)
            {
                // Gateway Units
            case UnitTypes::Enum::Protoss_Zealot:
                return true;
            case UnitTypes::Enum::Protoss_Dragoon:
                return Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Cybernetics_Core) > 0;
            case UnitTypes::Enum::Protoss_Dark_Templar:
                return Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Templar_Archives) > 0;
            case UnitTypes::Enum::Protoss_High_Templar:
                return Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Templar_Archives) > 0;

                // Robo Units
            case UnitTypes::Enum::Protoss_Shuttle:
                return true;
            case UnitTypes::Enum::Protoss_Reaver:
                return Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Robotics_Support_Bay) > 0;
            case UnitTypes::Enum::Protoss_Observer:
                return Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Observatory) > 0;

                // Stargate Units
            case UnitTypes::Enum::Protoss_Corsair:
                return true;
            case UnitTypes::Enum::Protoss_Scout:
                return true;
            case UnitTypes::Enum::Protoss_Carrier:
                return Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Fleet_Beacon) > 0;
            case UnitTypes::Enum::Protoss_Arbiter:
                return Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Arbiter_Tribunal) > 0;

                // Barracks Units
            case UnitTypes::Enum::Terran_Marine:
                return true;
            case UnitTypes::Enum::Terran_Firebat:
                return Broodwar->self()->completedUnitCount(UnitTypes::Terran_Academy) > 0;
            case UnitTypes::Enum::Terran_Medic:
                return Broodwar->self()->completedUnitCount(UnitTypes::Terran_Academy) > 0;
            case UnitTypes::Enum::Terran_Ghost:
                return Broodwar->self()->completedUnitCount(UnitTypes::Terran_Covert_Ops) > 0;
            case UnitTypes::Enum::Terran_Nuclear_Missile:
                return Broodwar->self()->completedUnitCount(UnitTypes::Terran_Covert_Ops) > 0;

                // Factory Units
            case UnitTypes::Enum::Terran_Vulture:
                return true;
            case UnitTypes::Enum::Terran_Siege_Tank_Tank_Mode:
                return building->getAddon() != nullptr ? true : false;
            case UnitTypes::Enum::Terran_Goliath:
                return (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Armory) > 0);

                // Starport Units
            case UnitTypes::Enum::Terran_Wraith:
                return true;
            case UnitTypes::Enum::Terran_Valkyrie:
                return (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Armory) > 0 && building->getAddon() != nullptr) ? true : false;
            case UnitTypes::Enum::Terran_Battlecruiser:
                return (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Physics_Lab) && building->getAddon() != nullptr) ? true : false;
            case UnitTypes::Enum::Terran_Science_Vessel:
                return (Broodwar->self()->completedUnitCount(UnitTypes::Terran_Science_Facility) > 0 && building->getAddon() != nullptr) ? true : false;
            case UnitTypes::Enum::Terran_Dropship:
                return building->getAddon() != nullptr ? true : false;

                // Zerg Units
            case UnitTypes::Enum::Zerg_Drone:
                return true;
            case UnitTypes::Enum::Zerg_Zergling:
                return (Broodwar->self()->completedUnitCount(UnitTypes::Zerg_Spawning_Pool) > 0);
            case UnitTypes::Enum::Zerg_Hydralisk:
                return (Broodwar->self()->completedUnitCount(UnitTypes::Zerg_Hydralisk_Den) > 0);
            case UnitTypes::Enum::Zerg_Mutalisk:
                return (Broodwar->self()->completedUnitCount(UnitTypes::Zerg_Spire) > 0);
            case UnitTypes::Enum::Zerg_Scourge:
                return (Broodwar->self()->completedUnitCount(UnitTypes::Zerg_Spire) > 0);
            case UnitTypes::Enum::Zerg_Ultralisk:
                return (Broodwar->self()->completedUnitCount(UnitTypes::Zerg_Ultralisk_Cavern) > 0);
            }
            return false;
        }

        bool isCreateable(UpgradeType upgrade)
        {
            auto geyserType = Broodwar->self()->getRace().getRefinery();
            if (upgrade.gasPrice() > 0 && com(geyserType) == 0)
                return false;

            // First upgrade check
            if (upgrade == BuildOrder::getFirstUpgrade() && Broodwar->self()->getUpgradeLevel(upgrade) == 0 && !Broodwar->self()->isUpgrading(upgrade))
                return true;

            // Upgrades that require a building
            if (upgrade == UpgradeTypes::Adrenal_Glands)
                return Broodwar->self()->completedUnitCount(Zerg_Hive);

            for (auto &unit : upgrade.whatUses()) {
                if (BuildOrder::isUnitUnlocked(unit) && Broodwar->self()->getUpgradeLevel(upgrade) != upgrade.maxRepeats() && !Broodwar->self()->isUpgrading(upgrade))
                    return true;
            }
            return false;
        }

        bool isCreateable(TechType tech)
        {
            // First tech check
            if (tech == BuildOrder::getFirstTech() && !Broodwar->self()->hasResearched(tech) && !Broodwar->self()->isResearching(tech))
                return true;

            // Tech that require a building
            if (tech == TechTypes::Lurker_Aspect)
                return Broodwar->self()->completedUnitCount(Zerg_Lair);

            for (auto &unit : tech.whatUses()) {
                if (BuildOrder::isUnitUnlocked(unit) && !Broodwar->self()->hasResearched(tech) && !Broodwar->self()->isResearching(tech))
                    return true;
            }
            return false;
        }

        bool isSuitable(UnitType unit)
        {
            if (unit.isWorker()) {
                if (Broodwar->self()->completedUnitCount(unit) < 90 && (!Resources::isMinSaturated() || !Resources::isGasSaturated()))
                    return true;
                else
                    return false;
            }

            if (unit.getRace() == Races::Zerg)
                return true;

            bool needReavers = false;
            bool needShuttles = false;

            // Determine whether we want reavers or shuttles;
            if (!Strategy::needDetection()) {
                if ((Terrain::isIslandMap() && vis(unit) < 2 * vis(UnitTypes::Protoss_Nexus))
                    || vis(UnitTypes::Protoss_Reaver) > (vis(UnitTypes::Protoss_Shuttle) * 2)
                    || vis(Protoss_High_Templar) > vis(Protoss_Shuttle) * 4
                    || (Players::vP() && vis(UnitTypes::Protoss_Shuttle) == 0 && com(UnitTypes::Protoss_Observatory) == 0))
                    needShuttles = true;
                if (!Terrain::isIslandMap() || (vis(UnitTypes::Protoss_Reaver) <= (vis(UnitTypes::Protoss_Shuttle) * 2)))
                    needReavers = true;
            }

            switch (unit)
            {
                // Gateway Units
            case Protoss_Zealot:
                return true;
            case Protoss_Dragoon:
                return true;
            case Protoss_Dark_Templar:
                return vis(unit) < 4;
            case Protoss_High_Templar:
                return vis(unit) < 6;

                // Robo Units
            case Protoss_Shuttle:
                return needShuttles;
            case Protoss_Reaver:
                return needReavers;
            case Protoss_Observer:
                return vis(unit) < 1 + (Players::getSupply(PlayerState::Self) / 100);

                // Stargate Units
            case Protoss_Corsair:
                return vis(unit) < (10 + (Terrain::isIslandMap() * 10));
            case Protoss_Scout:
                return true;
            case Protoss_Carrier:
                return true;
            case Protoss_Arbiter:
                return (vis(unit) < 10 && (Broodwar->self()->isUpgrading(UpgradeTypes::Khaydarin_Core) || Broodwar->self()->getUpgradeLevel(UpgradeTypes::Khaydarin_Core)));

                // Barracks Units
            case Terran_Marine:
                return true;
            case Terran_Firebat:
                return true;
            case Terran_Medic:
                return Broodwar->self()->completedUnitCount(unit) * 4 < Broodwar->self()->completedUnitCount(Terran_Marine);
            case Terran_Ghost:
                return BuildOrder::getCurrentBuild() == "TNukeMemes";
            case Terran_Nuclear_Missile:
                return BuildOrder::getCurrentBuild() == "TNukeMemes";

                // Factory Units
            case Terran_Vulture:
                return true;
            case Terran_Siege_Tank_Tank_Mode:
                return true;
            case Terran_Goliath:
                return true;

                // Starport Units
            case Terran_Wraith:
                return BuildOrder::getCurrentBuild() == "T2PortWraith";
            case Terran_Valkyrie:
                return BuildOrder::getCurrentBuild() == "T2PortWraith";
            case Terran_Battlecruiser:
                return true;
            case Terran_Science_Vessel:
                return vis(unit) < 6;
            case Terran_Dropship:
                return vis(unit) <= 0;
            }
            return false;
        }

        bool isSuitable(UpgradeType upgrade)
        {
            using namespace UpgradeTypes;

            // Allow first upgrade
            if (upgrade == BuildOrder::getFirstUpgrade() && !BuildOrder::firstReady())
                return true;

            // Don't upgrade anything in opener if nothing is chosen
            if (BuildOrder::getFirstUpgrade() == UpgradeTypes::None && BuildOrder::isOpener())
                return false;

            // If this is a specific unit upgrade, check if it's unlocked
            if (upgrade.whatUses().size() == 1) {
                for (auto &unit : upgrade.whatUses()) {
                    if (!BuildOrder::isUnitUnlocked(unit))
                        return false;
                }
            }

            // If this isn't the first upgrade and we don't have our first tech/upgrade
            if (upgrade != BuildOrder::getFirstUpgrade()) {
                if (BuildOrder::getFirstUpgrade() != UpgradeTypes::None && Broodwar->self()->getUpgradeLevel(BuildOrder::getFirstUpgrade()) <= 0 && !Broodwar->self()->isUpgrading(BuildOrder::getFirstUpgrade()))
                    return false;
                if (BuildOrder::getFirstTech() != TechTypes::None && !Broodwar->self()->hasResearched(BuildOrder::getFirstTech()) && !Broodwar->self()->isResearching(BuildOrder::getFirstTech()))
                    return false;
            }

            // If we're playing Protoss, check Protoss upgrades
            if (Broodwar->self()->getRace() == Races::Protoss) {
                switch (upgrade) {

                    // Energy upgrades
                case Khaydarin_Amulet:
                    return (vis(UnitTypes::Protoss_Assimilator) >= 4 && Broodwar->self()->hasResearched(TechTypes::Psionic_Storm) && Broodwar->self()->gas() >= 750);
                case Khaydarin_Core:
                    return true;

                    // Range upgrades
                case Singularity_Charge:
                    return vis(UnitTypes::Protoss_Dragoon) >= 1;

                    // Sight upgrades
                case Apial_Sensors:
                    return (Broodwar->self()->minerals() > 1500 && Broodwar->self()->gas() > 1000);
                case Sensor_Array:
                    return (Broodwar->self()->minerals() > 1500 && Broodwar->self()->gas() > 1000);

                    // Capacity upgrades
                case Carrier_Capacity:
                    return vis(Protoss_Carrier) >= 2;
                case Reaver_Capacity:
                    return (Broodwar->self()->minerals() > 1500 && Broodwar->self()->gas() > 1000);
                case Scarab_Damage:
                    return (Broodwar->self()->minerals() > 1500 && Broodwar->self()->gas() > 1000);

                    // Speed upgrades
                case Gravitic_Drive:
                    return vis(UnitTypes::Protoss_Shuttle) > 0;
                case Gravitic_Thrusters:
                    return vis(UnitTypes::Protoss_Scout) > 0;
                case Gravitic_Boosters:
                    return (Broodwar->self()->minerals() > 1500 && Broodwar->self()->gas() > 1000);
                case Leg_Enhancements:
                    return (vis(UnitTypes::Protoss_Nexus) >= 2);

                    // Ground unit upgrades
                case Protoss_Ground_Weapons:
                    return !Terrain::isIslandMap() && (Players::getSupply(PlayerState::Self) > 120 || Players::getRaceCount(Races::Zerg, PlayerState::Enemy) > 0);
                case Protoss_Ground_Armor:
                    return !Terrain::isIslandMap() && (Broodwar->self()->getUpgradeLevel(Protoss_Ground_Weapons) > Broodwar->self()->getUpgradeLevel(Protoss_Ground_Armor) || Broodwar->self()->isUpgrading(Protoss_Ground_Weapons));
                case Protoss_Plasma_Shields:
                    return haveOrUpgrading(Protoss_Ground_Weapons, 3) && haveOrUpgrading(Protoss_Ground_Armor, 3);

                    // Air unit upgrades
                case Protoss_Air_Weapons:
                    return (vis(UnitTypes::Protoss_Corsair) > 0 || vis(UnitTypes::Protoss_Scout) > 0 || (vis(Protoss_Stargate) > 0 && BuildOrder::isTechUnit(Protoss_Carrier) && Players::vT()));
                case Protoss_Air_Armor:
                    return Broodwar->self()->getUpgradeLevel(Protoss_Air_Weapons) > Broodwar->self()->getUpgradeLevel(Protoss_Air_Armor);
                }
            }

            else if (Broodwar->self()->getRace() == Races::Terran) {
                switch (upgrade) {

                    // Speed upgrades
                case Ion_Thrusters:
                    return true;

                    // Range upgrades
                case Charon_Boosters:
                    return Strategy::getUnitScore(UnitTypes::Terran_Goliath) > 1.00;
                case U_238_Shells:
                    return Broodwar->self()->hasResearched(TechTypes::Stim_Packs);

                    // Bio upgrades
                case Terran_Infantry_Weapons:
                    return true;// (BuildOrder::isBioBuild());
                case Terran_Infantry_Armor:
                    return (Broodwar->self()->getUpgradeLevel(Terran_Infantry_Weapons) > Broodwar->self()->getUpgradeLevel(Terran_Infantry_Armor) || Broodwar->self()->isUpgrading(Terran_Infantry_Weapons));

                    // Mech upgrades
                case Terran_Vehicle_Weapons:
                    return (Players::getStrength(PlayerState::Self).groundToGround > 20.0);
                case Terran_Vehicle_Plating:
                    return (Broodwar->self()->getUpgradeLevel(Terran_Vehicle_Weapons) > Broodwar->self()->getUpgradeLevel(Terran_Vehicle_Plating) || Broodwar->self()->isUpgrading(Terran_Vehicle_Weapons));
                case Terran_Ship_Weapons:
                    return (Players::getStrength(PlayerState::Self).airToAir > 20.0);
                case Terran_Ship_Plating:
                    return (Broodwar->self()->getUpgradeLevel(Terran_Ship_Weapons) > Broodwar->self()->getUpgradeLevel(Terran_Ship_Plating) || Broodwar->self()->isUpgrading(Terran_Ship_Weapons));
                }
            }

            else if (Broodwar->self()->getRace() == Races::Zerg) {
                switch (upgrade)
                {
                    // Speed upgrades
                case Metabolic_Boost:
                    return true;
                case Muscular_Augments:
                    return Broodwar->self()->getUpgradeLevel(Grooved_Spines);
                case Pneumatized_Carapace:
                    return !BuildOrder::isOpener();
                case Anabolic_Synthesis:
                    return true;

                    // Range upgrades
                case Grooved_Spines:
                    return true;

                    // Other upgrades
                case Chitinous_Plating:
                    return true;
                case Adrenal_Glands:
                    return true;

                    // Ground unit upgrades
                case Zerg_Melee_Attacks:
                    return (Players::getSupply(PlayerState::Self) > 120);
                case Zerg_Missile_Attacks:
                    return vis(Zerg_Hydralisk) >= 8 || vis(Zerg_Lurker) >= 4;
                case Zerg_Carapace:
                    return (Players::getSupply(PlayerState::Self) > 120);

                    // Air unit upgrades
                case Zerg_Flyer_Attacks:
                    return (Players::getSupply(PlayerState::Self) > 120);
                case Zerg_Flyer_Carapace:
                    return (Players::getSupply(PlayerState::Self) > 120);
                }
            }
            return false;
        }

        bool isSuitable(TechType tech)
        {
            using namespace TechTypes;

            // Allow first tech
            if (tech == BuildOrder::getFirstTech() && !BuildOrder::firstReady())
                return true;

            // If this is a specific unit tech, check if it's unlocked
            if (tech.whatUses().size() == 1) {
                for (auto &unit : tech.whatUses()) {
                    if (!BuildOrder::isUnitUnlocked(unit))
                        return false;
                }
            }

            // If this isn't the first tech and we don't have our first tech/upgrade
            if (tech != BuildOrder::getFirstTech()) {
                if (BuildOrder::getFirstUpgrade() != UpgradeTypes::None && Broodwar->self()->getUpgradeLevel(BuildOrder::getFirstUpgrade()) <= 0 && !Broodwar->self()->isUpgrading(BuildOrder::getFirstUpgrade()))
                    return false;
                if (BuildOrder::getFirstTech() != TechTypes::None && !Broodwar->self()->hasResearched(BuildOrder::getFirstTech()) && !Broodwar->self()->isResearching(BuildOrder::getFirstTech()))
                    return false;
            }

            if (Broodwar->self()->getRace() == Races::Protoss) {
                switch (tech) {
                case Psionic_Storm:
                    return true;
                case Stasis_Field:
                    return Broodwar->self()->getUpgradeLevel(UpgradeTypes::Khaydarin_Core) > 0;
                case Recall:
                    return (Broodwar->self()->minerals() > 1500 && Broodwar->self()->gas() > 1000);
                case Disruption_Web:
                    return (vis(UnitTypes::Protoss_Corsair) >= 10);
                }
            }

            else if (Broodwar->self()->getRace() == Races::Terran) {
                switch (tech) {
                case Stim_Packs:
                    return true;// BuildOrder::isBioBuild();
                case Spider_Mines:
                    return Broodwar->self()->getUpgradeLevel(UpgradeTypes::Ion_Thrusters) > 0 || Broodwar->self()->isUpgrading(UpgradeTypes::Ion_Thrusters);
                case Tank_Siege_Mode:
                    return Broodwar->self()->hasResearched(TechTypes::Spider_Mines) || Broodwar->self()->isResearching(TechTypes::Spider_Mines) || vis(UnitTypes::Terran_Siege_Tank_Tank_Mode) > 0;
                case Cloaking_Field:
                    return vis(UnitTypes::Terran_Wraith) >= 2;
                case Yamato_Gun:
                    return vis(UnitTypes::Terran_Battlecruiser) >= 0;
                case Personnel_Cloaking:
                    return vis(UnitTypes::Terran_Ghost) >= 2;
                }
            }

            else if (Broodwar->self()->getRace() == Races::Zerg) {
                switch (tech) {
                case Lurker_Aspect:
                    return true;
                }
            }
            return false;
        }

        void addon(UnitInfo& building)
        {
            for (auto &unit : building.getType().buildsWhat()) {
                if (unit.isAddon() && BuildOrder::buildCount(unit) > vis(unit))
                    building.unit()->buildAddon(unit);
            }
        }

        void produce(UnitInfo& building)
        {
            auto offset = 16;
            auto best = 0.0;
            auto bestType = UnitTypes::None;

            const auto scoreUnit = [&](UnitType type) {
                const auto mineralCost = Broodwar->self()->minerals() == 0 || type.mineralPrice() == 0 ? 0.5 : double(Broodwar->self()->minerals() - type.mineralPrice() - (!BuildOrder::isTechUnit(type) * reservedMineral) - Buildings::getQueuedMineral()) / double(Broodwar->self()->minerals());
                const auto gasCost = Broodwar->self()->gas() == 0 || type.gasPrice() == 0 ? 0.5 : double(Broodwar->self()->gas() - type.gasPrice() - (!BuildOrder::isTechUnit(type) * reservedGas) - Buildings::getQueuedGas()) / double(Broodwar->self()->gas());

                const auto resourceScore = clamp(gasCost * mineralCost, 0.01, 1.0);
                const auto strategyScore = clamp(Strategy::getUnitScore(type) / double(max(1, vis(type))), 0.01, 1.0);
                return resourceScore * strategyScore;
            };

            if (building.getType() == UnitTypes::Zerg_Larva && BuildOrder::buildCount(Zerg_Overlord) > vis(Zerg_Overlord) + trainedThisFrame[Zerg_Overlord]) {
                building.unit()->morph(Zerg_Overlord);
                trainedThisFrame[Zerg_Overlord]++;
                return;
            }

            if (building.getType() == UnitTypes::Zerg_Larva && Buildings::overlapsQueue(building, building.getTilePosition())) {
                building.unit()->stop();
                return;
            }

            for (auto &type : building.getType().buildsWhat()) {
                const auto value = scoreUnit(type);

                // If we teched to DTs, try to create as many as possible
                if (type == UnitTypes::Protoss_Dark_Templar && BuildOrder::getTechList().size() == 1 && isCreateable(building.unit(), type) && isSuitable(type)) {
                    best = DBL_MAX;
                    bestType = type;
                }
                else if (type == BuildOrder::getTechUnit() && isCreateable(building.unit(), type) && isSuitable(type) && vis(type) == 0 && isAffordable(type)) {
                    best = DBL_MAX;
                    bestType = type;
                }
                else if (type == UnitTypes::Protoss_Observer && isCreateable(building.unit(), type) && isSuitable(type) && vis(type) < Broodwar->self()->completedUnitCount(UnitTypes::Protoss_Nexus)) {
                    best = DBL_MAX;
                    bestType = type;
                }
                else if (value >= best && isCreateable(building.unit(), type) && isSuitable(type) && (isAffordable(bestType) || Broodwar->getFrameCount() < 8000)) {
                    best = value;
                    bestType = type;
                }
            }

            if (bestType != UnitTypes::None) {

                // If we can afford it, train it
                if (isAffordable(bestType)) {
                    building.unit()->train(bestType);
                    building.setRemainingTrainFrame(bestType.buildTime());
                    idleProduction.erase(building.unit());
                }

                // Else if this is a tech unit, add it to idle production
                else if (BuildOrder::getTechUnit() == bestType || BuildOrder::getTechList().find(bestType) != BuildOrder::getTechList().end()) {
                    if (Players::getSupply(PlayerState::Self) < 380)
                        idleFrame = Broodwar->getFrameCount();

                    idleProduction[building.unit()] = bestType;
                    reservedMineral += bestType.mineralPrice();
                    reservedGas += bestType.gasPrice();
                }

                // Else store a zero value idle
                else
                    idleProduction[building.unit()] = UnitTypes::None;
            }
        }

        void research(UnitInfo& building)
        {
            for (auto &research : building.getType().researchesWhat()) {
                if (isCreateable(research) && isSuitable(research)) {
                    if (isAffordable(research))
                        building.unit()->research(research), idleTech.erase(building.unit());
                    else
                        idleTech[building.unit()] = research;
                    reservedMineral += research.mineralPrice();
                    reservedGas += research.gasPrice();
                }
            }
        }

        void upgrade(UnitInfo& building)
        {
            for (auto &upgrade : building.getType().upgradesWhat()) {
                if (isCreateable(upgrade) && isSuitable(upgrade)) {
                    if (isAffordable(upgrade))
                        building.unit()->upgrade(upgrade), idleUpgrade.erase(building.unit());
                    else
                        idleUpgrade[building.unit()] = upgrade;
                    reservedMineral += upgrade.mineralPrice();
                    reservedGas += upgrade.gasPrice();
                }
            }
        }

        void updateReservedResources()
        {
            // Reserved minerals for idle buildings, tech and upgrades
            reservedMineral = 0;
            reservedGas = 0;

            for (auto &[_, type] : idleProduction) {
                if (BuildOrder::isTechUnit(type)) {
                    reservedMineral += type.mineralPrice();
                    reservedGas += type.gasPrice();
                }
            }

            for (auto &[_, tech] : idleTech) {
                reservedMineral += tech.mineralPrice();
                reservedGas += tech.gasPrice();
            }

            for (auto &[_, upgrade] : idleUpgrade) {
                reservedMineral += upgrade.mineralPrice();
                reservedGas += upgrade.gasPrice();
            }
        }

        void updateProduction()
        {
            trainedThisFrame.clear();

            for (auto &u : Units::getUnits(PlayerState::Self)) {
                UnitInfo &building = *u;

                if (!building.unit()
                    || building.getRole() != Role::Production
                    || !building.unit()->isCompleted()
                    || building.getRemainingTrainFrames() >= Broodwar->getLatencyFrames())
                    continue;

                // TODO: Combine into one - iterate all commands and return when true
                if (!building.getType().isResourceDepot() || building.getType().getRace() == Races::Zerg) {
                    idleProduction.erase(building.unit());
                    idleUpgrade.erase(building.unit());
                    idleTech.erase(building.unit());

                    addon(building);
                    produce(building);
                    research(building);
                    upgrade(building);
                }

                else {
                    for (auto &unit : building.getType().buildsWhat()) {
                        if (unit.isAddon() && !building.unit()->getAddon() && BuildOrder::buildCount(unit) > vis(unit)) {
                            building.unit()->buildAddon(unit);
                            continue;
                        }
                        if (!BuildOrder::isWorkerCut() && unit.isWorker() && Broodwar->self()->completedUnitCount(unit) < 75 && isAffordable(unit) && (!Resources::isGasSaturated() || !Resources::isMinSaturated())) {
                            building.unit()->train(unit);
                            building.setRemainingTrainFrame(unit.buildTime());
                        }
                    }
                }
            }
        }
    }

    void onFrame()
    {
        Visuals::startPerfTest();
        updateReservedResources();
        updateProduction();
        Visuals::endPerfTest("Production");
    }

    int getReservedMineral() { return reservedMineral; }
    int getReservedGas() { return reservedGas; }
    bool hasIdleProduction() { return int(idleProduction.size()) > 0; }
}