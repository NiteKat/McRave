#include "McRave.h"
#include "BuildOrder.h"
#include <fstream>

using namespace std;
using namespace BWAPI;
using namespace UnitTypes;
using namespace McRave::BuildOrder::All;

namespace McRave::BuildOrder
{
    namespace {

        void updateBuild()
        {
            // Set s for better build readability - TODO: better build order management
            s = Players::getSupply(PlayerState::Self);
            startCount = Broodwar->getStartLocations().size();
            buildQueue.clear();
            armyComposition.clear();

            // TODO: Check if we own a <race> unit - have a build order allowed PER race for FFA weirdness and maybe mind control shenanigans
            if (Broodwar->self()->getRace() == Races::Protoss) {
                Protoss::opener();
                Protoss::tech();
                Protoss::composition();
                Protoss::situational();
                Protoss::unlocks();
            }
            if (Broodwar->self()->getRace() == Races::Terran) {
                Terran::opener();
                Terran::tech();
                Terran::situational();
            }
            if (Broodwar->self()->getRace() == Races::Zerg) {
                Zerg::opener();
                Zerg::tech();
                Zerg::composition();
                Zerg::situational();
                Zerg::unlocks();
            }
        }
    }

    bool atPercent(UnitType t, double percent) {
        if (com(t) > 0)
            return true;

        // Estimate how long until a building finishes based on how far it is from the nearest worker
        auto closestBuilding = Util::getClosestUnit(BWEB::Map::getMainPosition(), PlayerState::Self, [&](auto &u) {
            return u.getType() == t;
        });
        auto closestWorker = Util::getClosestUnit(BWEB::Map::getMainPosition(), PlayerState::Self, [&](auto &u) {
            return u.getType() == Broodwar->self()->getRace().getWorker();
        });

        if (closestBuilding && closestWorker)
            return double(t.buildTime() - closestBuilding->unit()->getRemainingBuildTime()) / double(t.buildTime()) >= percent;
        return false;
    }

    bool atPercent(TechType t, double percent) {
        if (Broodwar->self()->hasResearched(t))
            return true;

        // Estimate how long until a building finishes based on how far it is from the nearest worker
        auto closestBuilding = Util::getClosestUnit(BWEB::Map::getMainPosition(), PlayerState::Self, [&](auto &u) {
            return u.getType() == t.whatResearches() && u.unit()->isResearching();
        });
        return closestBuilding != nullptr;
    }

    bool techComplete()
    {
        if (Strategy::needDetection() && techUnit == Protoss_Observer)
            return vis(Protoss_Robotics_Facility) > 0;

        // When 1 unit finishes
        if (techUnit == Protoss_Scout || techUnit == Protoss_Corsair || techUnit == Protoss_Reaver || techUnit == Protoss_Observer || techUnit == Terran_Science_Vessel)
            return com(techUnit) > 0;

        // When 2 units are visible
        if (techUnit == Protoss_High_Templar)
            return vis(techUnit) >= 1 && Broodwar->self()->hasResearched(TechTypes::Psionic_Storm);

        // When 2 units finish
        if (techUnit == Protoss_Dark_Templar)
            return com(techUnit) >= 2;

        // When timing attack finishes
        if (techUnit == Zerg_Mutalisk || techUnit == Zerg_Hydralisk)
            return total(techUnit) >= 12;
        if (techUnit == Zerg_Lurker)
            return Broodwar->self()->hasResearched(TechTypes::Lurker_Aspect);

        // When 1 unit is visible
        return vis(techUnit) > 0;
    }

    bool shouldExpand()
    {
        const auto baseType = Broodwar->self()->getRace().getResourceDepot();
        const auto hatchCount = vis(Zerg_Hatchery) + vis(Zerg_Lair) + vis(Zerg_Hive);
        const auto productionCap = int(round(1.5 * double(Stations::getMyStations().size())));

        if (Broodwar->self()->getRace() == Races::Zerg) {
            if (Resources::isMinSaturated() && Resources::isGasSaturated() && techSat && productionSat)
                return true;
            else if (techUnit == None && techSat && productionSat && Broodwar->self()->minerals() >= 600 && !saveLarva)
                return true;
        }
        else {
            if (com(baseType) >= 2 && Broodwar->self()->minerals() >= 400)
                return true;
            else if (techUnit == None && (Resources::isMinSaturated() || com(baseType) >= 3) && (techSat || com(baseType) >= 3) && productionSat)
                return true;
        }
        return false;
    }

    bool shouldAddProduction()
    {
        const auto baseType = Broodwar->self()->getRace().getResourceDepot();
        const auto hatchCount = com(Zerg_Hatchery) + com(Zerg_Lair) + com(Zerg_Hive);

        if (Broodwar->self()->getRace() == Races::Zerg)
            return !productionSat && ((techUnit == None && techSat && Broodwar->self()->minerals() >= 600 && !saveLarva) || (Broodwar->self()->minerals() >= 400 && vis(Zerg_Larva) < 3 && !saveLarva));
        else
            return !productionSat && ((techUnit == None && Broodwar->self()->minerals() >= 150 && (techSat || com(baseType) >= 3)) || Broodwar->self()->minerals() >= 300);
        return false;
    }

    bool shouldAddGas()
    {
        auto workerCount = com(Broodwar->self()->getRace().getWorker());
        auto refineryCount = vis(Broodwar->self()->getRace().getRefinery());
        auto workerUnlimitedGas = Players::ZvT() ? 40 : 60;

        if (Broodwar->self()->getRace() == Races::Zerg)
            return gasLimit >= Workers::getGasWorkers() && workerCount >= 10 && ((Broodwar->self()->minerals() > 600 && Broodwar->self()->gas() < 200 && Resources::isMinSaturated()) || productionSat || workerCount >= workerUnlimitedGas || Players::ZvZ());
        else
            return (Broodwar->self()->gas() < 300 || workerCount >= 30) && ((refineryCount != 1 && Broodwar->self()->getRace() != Races::Zerg) || workerCount >= 30 || Broodwar->self()->minerals() > 600 || Resources::isMinSaturated());
    }

    double getCompositionPercentage(UnitType unit)
    {
        auto ptr = armyComposition.find(unit);
        if (ptr != armyComposition.end())
            return ptr->second;
        return 0.0;
    }

    int buildCount(UnitType unit)
    {
        auto ptr = buildQueue.find(unit);
        if (ptr != buildQueue.end())
            return ptr->second;
        return 0;
    }

    bool firstReady()
    {
        if (firstTech != TechTypes::None && Broodwar->self()->hasResearched(firstTech))
            return true;
        else if (firstUpgrade != UpgradeTypes::None && Broodwar->self()->getUpgradeLevel(firstUpgrade) > 0)
            return true;
        else if (firstTech == TechTypes::None && firstUpgrade == UpgradeTypes::None)
            return true;
        return false;
    }

    bool unlockReady(UnitType type) {
        bool ready = false;

        // P
        if (type == Protoss_High_Templar || type == Protoss_Dark_Templar || type == Protoss_Archon || type == Protoss_Dark_Archon)
            ready = com(Protoss_Citadel_of_Adun) > 0;
        if (type == Protoss_Corsair || type == Protoss_Scout)
            ready = com(Protoss_Stargate) > 0;
        if (type == Protoss_Reaver || type == Protoss_Observer)
            ready = com(Protoss_Robotics_Facility) > 0;
        if (type == Protoss_Carrier)
            ready = com(Protoss_Fleet_Beacon) > 0;
        if (type == Protoss_Arbiter)
            ready = com(Protoss_Arbiter_Tribunal) > 0;

        // Z
        if (type == Zerg_Mutalisk)
            ready = com(Zerg_Spire) > 0;
        if (type == Zerg_Hydralisk)
            ready = com(Zerg_Hydralisk_Den) > 0;
        if (type == Zerg_Lurker)
            ready = com(Zerg_Hydralisk_Den) > 0 && com(Zerg_Lair) > 0;

        return ready;
    }

    void getNewTech()
    {
        // If we already have a tech choice based on build, only try to unlock it and nothing else for now
        if (firstUnit != None && !isTechUnit(firstUnit)) {
            if (unlockReady(firstUnit)) {
                techUnit = firstUnit;
                getTech = false;
                techList.insert(techUnit);
                unlockedType.insert(techUnit);
            }
            return;
        }

        if (getTech) {

            // If we already chose a tech unit
            if (techUnit != None) {
                getTech = false;
                techList.insert(techUnit);
                unlockedType.insert(techUnit);
                return;
            }

            // Select next tech based on the order
            for (auto &type : techOrder) {
                if (!isTechUnit(type)) {
                    techUnit = type;
                    getTech = false;
                    techList.insert(techUnit);
                    unlockedType.insert(techUnit);
                    break;
                }
            }
        }
    }

    void getTechBuildings()
    {
        // For every unit in our tech list, ensure we are building the required buildings
        set<UnitType> toCheck;
        for (auto &type : techList) {
            toCheck.insert(type);
            toCheck.insert(type.whatBuilds().first);
        }

        // Iterate all required branches of buildings that are required for this tech unit
        bool moreToAdd;
        do {
            moreToAdd = false;
            for (auto &check : toCheck) {
                for (auto &pair : check.requiredUnits()) {
                    UnitType type(pair.first);
                    if (com(type) == 0 && toCheck.find(type) == toCheck.end()) {
                        toCheck.insert(type);
                        moreToAdd = true;
                    }
                }
            }
        } while (moreToAdd);

        // For each building we need to check, add to our queue whatever is possible to build based on its required branch
        for (auto &check : toCheck) {

            if (!check.isBuilding())
                continue;

            bool canAdd = true;
            for (auto &pair : check.requiredUnits()) {
                UnitType type(pair.first);
                if (type.isBuilding() && !atPercent(type, 1.00))
                    canAdd = false;
            }

            // Our check doesn't look for required buildings for tech needed for Lurkers
            if (check == Zerg_Lurker)
                buildQueue[Zerg_Lair] = 1;

            // Add extra production
            int s = Players::getSupply(PlayerState::Self);
            if (canAdd && buildCount(check) <= 1) {
                if (check == Protoss_Stargate) {
                    if ((s >= 250 && techList.find(Protoss_Corsair) != techList.end())
                        || (s >= 300 && techList.find(Protoss_Arbiter) != techList.end())
                        || (s >= 100 && techList.find(Protoss_Carrier) != techList.end()))
                        buildQueue[check] = 2;
                    else
                        buildQueue[check] = 1;
                }
                else if (check != Protoss_Gateway)
                    buildQueue[check] = 1;
            }
        }
    }

    void setLearnedBuild(string newBuild, string newOpener, string newTransition) {
        currentBuild = newBuild;
        currentOpener = newOpener;
        currentTransition = newTransition;
    }

    void onFrame()
    {
        Visuals::startPerfTest();
        updateBuild();
        Visuals::endPerfTest("BuildOrder");
    }

    map<UnitType, int>& getBuildQueue() { return buildQueue; }
    map<UnitType, double> getArmyComposition() { return armyComposition; }
    UnitType getTechUnit() { return techUnit; }
    UpgradeType getFirstUpgrade() { return firstUpgrade; }
    TechType getFirstTech() { return firstTech; }
    set <UnitType>& getTechList() { return  techList; }
    set <UnitType>& getUnlockedList() { return  unlockedType; }
    int gasWorkerLimit() { return gasLimit; }
    bool isWorkerCut() { return cutWorkers; }
    bool isUnitUnlocked(UnitType unit) { return unlockedType.find(unit) != unlockedType.end(); }
    bool isTechUnit(UnitType unit) { return techList.find(unit) != techList.end(); }
    bool isOpener() { return inOpeningBook; }
    bool isFastExpand() { return fastExpand; }
    bool shouldScout() { return scout; }
    bool isWallNat() { return wallNat; }
    bool isWallMain() { return wallMain; }
    bool isProxy() { return proxy; }
    bool isHideTech() { return hideTech; }
    bool isPlayPassive() { return playPassive; }
    bool isRush() { return rush; }
    bool isGasTrick() { return gasTrick; }
    bool isSaveLarva() { return saveLarva; }
    bool makeDefensesNow() { return defensesNow; }
    string getCurrentBuild() { return currentBuild; }
    string getCurrentOpener() { return currentOpener; }
    string getCurrentTransition() { return currentTransition; }
}