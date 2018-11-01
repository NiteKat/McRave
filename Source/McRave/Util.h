#pragma once
#include <BWAPI.h>
#include "Singleton.h"

using namespace BWAPI;
using namespace std;

namespace McRave
{
	class UtilManager
	{
	public:

		UnitInfo * getClosestUnit(Position, Player, UnitType t = UnitTypes::None);
		UnitInfo * getClosestUnit(UnitInfo&, Player, UnitType t = UnitTypes::None);
		UnitInfo * getClosestThreat(UnitInfo&);
		UnitInfo * getClosestBuilder(Position);

		int chokeWidth(const BWEM::ChokePoint *);
		const BWEM::ChokePoint * getClosestChokepoint(Position);
		double getHighestThreat(WalkPosition, UnitInfo&);

		bool unitInRange(UnitInfo& unit);		
		bool reactivePullWorker(UnitInfo& unit);
		bool proactivePullWorker(UnitInfo& unit);
		bool pullRepairWorker(UnitInfo& unit);
		bool accurateThreatOnPath(UnitInfo&, Path&);
		bool rectangleIntersect(Position, Position, Position);

		// Walkability checks
		template<class T>
		bool isWalkable(T here)
		{
			auto start = WalkPosition(here);
			for (int x = start.x; x < start.x + 4; x++) {
				for (int y = start.y; y < start.y + 4; y++) {
					if (Grids().getMobility(WalkPosition(x, y)) == -1)
						return false;
				}
			}
			return true;
		}
		bool isWalkable(WalkPosition start, WalkPosition finish, UnitType);
		
		Position getConcavePosition(UnitInfo&, BWEM::Area const * area = nullptr, Position here = Positions::Invalid);
	};
}

typedef Singleton<McRave::UtilManager> UtilSingleton;
