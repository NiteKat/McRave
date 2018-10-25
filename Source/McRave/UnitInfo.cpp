#include "UnitInfo.h"
#include "Util.h"
#include "UnitManager.h"
#include "TargetManager.h"
#include "UnitMath.h"

// DEAR GOD PLEASE FUCKING REMOVE THIS WHY DID I DO THIS
#define Targets TargetSingleton::Instance()
#define Units UnitSingleton::Instance()

namespace McRave
{
	UnitInfo::UnitInfo()
	{
		visibleGroundStrength = 0.0;
		visibleAirStrength = 0.0;
		maxGroundStrength = 0.0;
		maxAirStrength = 0.0;
		priority = 0.0;
		
		percentHealth = 0.0;
		percentShield = 0.0;
		percentTotal = 0.0;
		groundRange = 0.0;
		airRange = 0.0;
		groundDamage = 0.0;
		airDamage = 0.0;
		speed = 0.0;
		engageDist = 0.0;
		simValue = 0.0;
		simBonus = 1.0;

		lastAttackFrame = 0;
		lastVisibleFrame = 0;
		lastMoveFrame = 0;
		resourceHeldFrames = 0;
		remainingTrainFrame = 0;
		shields = 0;
		health = 0;
		minStopFrame = 0;
		energy = 0;

		killCount = 0;
		

		burrowed = false;

		thisUnit = nullptr;
		transport = nullptr;
		unitType = UnitTypes::None;
		buildingType = UnitTypes::None;
		player = nullptr;
		target = nullptr;
		resource = nullptr;
		role = Role::None;
		tState = TransportState::None;
		cState = CombatState::None;

		position = Positions::Invalid;
		engagePosition = Positions::Invalid;
		destination = Positions::Invalid;
		simPosition = Positions::Invalid;
		lastPos = Positions::Invalid;
		walkPosition = WalkPositions::Invalid;
		lastWalk = WalkPositions::Invalid;
		tilePosition = TilePositions::Invalid;
		lastTile = TilePositions::Invalid;
	}

	void UnitInfo::setLastPositions()
	{
		lastPos = this->getPosition();
		lastTile = this->getTilePosition();
		lastWalk =  this->getWalkPosition();
	}

	void UnitInfo::updateUnit()
	{
		auto t = this->unit()->getType();
		auto p = this->unit()->getPlayer();

		this->setLastPositions();

		// Update unit positions		
		position				= thisUnit->getPosition();
		destination				= Positions::None;
		tilePosition			= unit()->getTilePosition();
		walkPosition			= Math::getWalkPosition(thisUnit);

		// Update unit stats
		unitType				= t;
		player					= p;
		health					= thisUnit->getHitPoints();
		shields					= thisUnit->getShields();
		energy					= thisUnit->getEnergy();
		percentHealth			= t.maxHitPoints() > 0 ? double(health) / double(t.maxHitPoints()) : 1.0;
		percentShield			= t.maxShields() > 0 ? double(shields) / double(t.maxShields()) : 1.0;
		percentTotal			= t.maxHitPoints() + t.maxShields() > 0 ? double(health + shields) / double(t.maxHitPoints() + t.maxShields()) : 1.0;
		groundRange				= Math::groundRange(*this);
		airRange				= Math::airRange(*this);
		groundDamage			= Math::groundDamage(*this);
		airDamage				= Math::airDamage(*this);
		speed 					= Math::speed(*this);
		minStopFrame			= Math::getMinStopFrame(t);
		burrowed				= (thisUnit->isBurrowed() || thisUnit->getOrder() == Orders::Burrowing || thisUnit->getOrder() == Orders::VultureMine);

		// Update McRave stats
		visibleGroundStrength	= Math::getVisibleGroundStrength(*this);
		maxGroundStrength		= Math::getMaxGroundStrength(*this);
		visibleAirStrength		= Math::getVisibleAirStrength(*this);
		maxAirStrength			= Math::getMaxAirStrength(*this);
		priority				= Math::getPriority(*this);
		lastAttackFrame			= (t != UnitTypes::Protoss_Reaver && (thisUnit->isStartingAttack() || thisUnit->isRepairing())) ? Broodwar->getFrameCount() : lastAttackFrame;
		killCount				= unit()->getKillCount();
		simBonus				= 1.0;

		// Resource held frame
		if (thisUnit->isCarryingGas() || thisUnit->isCarryingMinerals())
			resourceHeldFrames = max(resourceHeldFrames, 0) + 1;
		else if (thisUnit->isGatheringGas() || thisUnit->isGatheringMinerals())
			resourceHeldFrames = min(resourceHeldFrames, 0) - 1;
		else
			resourceHeldFrames = 0;

		// Remaining train frame
		remainingTrainFrame = max(0, remainingTrainFrame - 1);

		this->updateTarget();
		this->updateStuckCheck();
	}

	void UnitInfo::updateTarget()
	{
		// Update my targets
		if (player && player == Broodwar->self()) {
			if (unitType == UnitTypes::Terran_Vulture_Spider_Mine) {
				auto mineTarget = unit()->getOrderTarget();

				if (Units.getEnemyUnits().find(mineTarget) != Units.getEnemyUnits().end())
					target = mineTarget != nullptr ? &Units.getEnemyUnits()[mineTarget] : nullptr;
				else
					target = nullptr;
			}
			else
				Targets.getTarget(*this);
		}

		// Assume enemy targets
		else if (player && player->isEnemy(Broodwar->self())) {

			if (unitType == UnitTypes::Terran_Vulture_Spider_Mine && Units.getMyUnits().find(thisUnit->getOrderTarget()) != Units.getMyUnits().end())
				target = &Units.getMyUnits()[thisUnit->getOrderTarget()];
			else if (unitType != UnitTypes::Terran_Vulture_Spider_Mine && thisUnit->getOrderTarget() && Units.getMyUnits().find(thisUnit->getOrderTarget()) != Units.getMyUnits().end())
				target = &Units.getMyUnits()[thisUnit->getOrderTarget()];
			else
				target = nullptr;
		}
	}

	void UnitInfo::updateStuckCheck() {
		if (player != Broodwar->self() || lastPos != position || !thisUnit->isMoving() || thisUnit->getLastCommand().getType() == UnitCommandTypes::Stop || lastAttackFrame == Broodwar->getFrameCount())
			lastMoveFrame = Broodwar->getFrameCount();
	}

	void UnitInfo::createDummy(UnitType t) {
		unitType				= t;
		player					= Broodwar->self();
		groundRange				= Math::groundRange(*this);
		airRange				= Math::airRange(*this);
		groundDamage			= Math::groundDamage(*this);
		airDamage				= Math::airDamage(*this);
		speed 					= Math::speed(*this);
	}
}