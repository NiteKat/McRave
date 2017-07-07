#pragma once
#include <BWAPI.h>
#include "Singleton.h"

using namespace BWAPI;
using namespace std;

class PylonTrackerClass
{
	map<TilePosition, int> smallLocations;
	map<TilePosition, int> mediumLocations;
	map<TilePosition, int> largeLocations;
public:
	void storePylon(Unit);
	void storePower(Unit);
	bool hasPower(TilePosition, UnitType);
};

typedef Singleton<PylonTrackerClass> PylonTracker;