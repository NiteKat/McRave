#include "McRave.h"

void TerrainTrackerClass::update()
{
	clock_t myClock;
	double duration = 0.0;
	myClock = clock();

	// Create island regions and natural expansion	
	for (auto &area : theMap.Areas())
	{
		for (auto &base : area.Bases())
		{
			if (area.AccessibleNeighbours().size() == 0)
			{
				islandRegions.emplace(area.Id());
			}
			else
			{
				allBaseLocations.emplace(base.Location());
			}
		}
	}

	// Start location
	playerStartingTilePosition = Broodwar->self()->getStartLocation();
	playerStartingPosition = Position(playerStartingTilePosition);

	// If we see a building, check for closest starting location
	if (enemyBasePositions.size() <= 0)
	{
		for (auto &unit : Units().getEnUnits())
		{
			if (unit.second.getType().isBuilding() && Terrain().getEnemyBasePositions().size() == 0 && unit.second.getPosition().getDistance(Terrain().getPlayerStartingPosition()) > 1600)
			{
				double distance = 0.0;
				TilePosition closest;
				for (auto &base : theMap.StartingLocations())
				{
					if (unit.second.getPosition().getDistance(Position(base)) < distance || distance == 0.0)
					{
						distance = unit.second.getPosition().getDistance(Position(base));
						closest = base;
					}
				}
				if (closest.isValid())
				{
					enemyBasePositions.emplace(Position(closest));
					enemyStartingTilePosition = closest;
					enemyStartingPosition = Position(closest);
					path = theMap.GetPath(playerStartingPosition, enemyStartingPosition);
				}
			}
		}
	}

	for (auto &base : Bases().getMyBases())
	{
		if (base.second.getTilePosition().isValid() && theMap.GetArea(base.second.getTilePosition()))
		{
			allyTerritory.emplace(theMap.GetArea(base.second.getTilePosition())->Id());
		}
	}

	for (auto &base : enemyBasePositions)
	{
		if (base.isValid() && Broodwar->isVisible(TilePosition(base)) && Broodwar->getUnitsInRadius(base, 128, Filter::IsEnemy).size() == 0)
		{
			enemyBasePositions.erase(base);
			break;
		}
	}

	// Establish FFE position
	int x = 0;
	int y = 0;
	const Area* closestA;
	double closestBaseDistance = 0.0, furthestChokeDistance = 0.0, closestChokeDistance = 0.0;
	TilePosition natural;
	if (Broodwar->getFrameCount() > 100)
	{
		for (auto &area : theMap.Areas())
		{
			for (auto &base : area.Bases())
			{
				if (base.Geysers().size() == 0 || area.AccessibleNeighbours().size() == 0)
				{
					continue;
				}

				if (Grids().getDistanceHome(WalkPosition(base.Location())) > 50 && (Grids().getDistanceHome(WalkPosition(base.Location())) < closestBaseDistance || closestBaseDistance == 0))
				{
					closestBaseDistance = Grids().getDistanceHome(WalkPosition(base.Location()));
					closestA = base.GetArea();
					natural = base.Location();
				}
			}
		}
		if (closestA)
		{
			for (auto &choke : closestA->ChokePoints())
			{
				if (choke && Grids().getDistanceHome(choke->Center()) > furthestChokeDistance)
				{
					secondChoke = TilePosition(choke->Center());
					furthestChokeDistance = Grids().getDistanceHome(choke->Center());
				}
				if (choke && (Grids().getDistanceHome(choke->Center()) < closestChokeDistance || closestChokeDistance == 0.0))
				{
					firstChoke = TilePosition(choke->Center());
					closestChokeDistance = Grids().getDistanceHome(choke->Center());
				}
			}
			FFEPosition = TilePosition(secondChoke.x*0.35 + natural.x*0.65, secondChoke.y*0.35 + natural.y*0.65);
		}
	}

	Broodwar->drawCircleMap(Position(secondChoke), 32, Colors::Red);
	Broodwar->drawCircleMap(Position(firstChoke), 32, Colors::Blue);



	duration = 1000.0 * (clock() - myClock) / (double)CLOCKS_PER_SEC;
	//Broodwar->drawTextScreen(200, 80, "Terrain Manager: %d ms", duration);
}

bool TerrainTrackerClass::isInAllyTerritory(Unit unit)
{
	if (unit && unit->exists() && unit->getTilePosition().isValid() && theMap.GetArea(unit->getTilePosition()))
	{
		if (allyTerritory.find(theMap.GetArea(unit->getTilePosition())->Id()) != allyTerritory.end())
		{
			return true;
		}
	}
	return false;
}

Position TerrainTrackerClass::getClosestEnemyBase(Position here)
{
	double closestD = 0.0;
	Position closestP;
	for (auto &base : Terrain().getEnemyBasePositions())
	{
		if (here.getDistance(base) < closestD || closestD == 0.0)
		{
			closestP = base;
			closestD = here.getDistance(base);
		}
	}
	return closestP;
}

void TerrainTrackerClass::removeTerritory(Unit base)
{
	if (base)
	{
		if (enemyBasePositions.find(base->getPosition()) != enemyBasePositions.end())
		{
			enemyBasePositions.erase(base->getPosition());

			if (theMap.GetArea(base->getTilePosition()))
			{
				if (allyTerritory.find(theMap.GetArea(base->getTilePosition())->Id()) != allyTerritory.end())
				{
					allyTerritory.erase(theMap.GetArea(base->getTilePosition())->Id());
				}
			}
		}
	}
}