#include "McRave.h"

void GridTrackerClass::reset()
{
	// For each tile, draw the current threat onto the tile
	int center = 0;
	for (int x = 0; x <= Broodwar->mapWidth() * 4; x++)
	{
		for (int y = 0; y <= Broodwar->mapHeight() * 4; y++)
		{
			// Debug drawing
			if (antiMobilityGrid[x][y] > 0)
			{
				//Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Broodwar->self()->getColor());
			}

			if (mobilityGrid[x][y] >= 0 && antiMobilityGrid[x][y] == 0)
			{
				/*if (mobilityGrid[x][y] < 4)
				{
				Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Colors::Black);
				}
				else if (mobilityGrid[x][y] >= 4 && mobilityGrid[x][y] < 7)
				{
				Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Colors::Red);
				}
				else if (mobilityGrid[x][y] >= 7 && mobilityGrid[x][y] < 10)
				{
				Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Colors::Blue);
				}
				else if (mobilityGrid[x][y] >= 10)
				{
				Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Colors::Green);
				}*/
			}

			if (eGroundDistanceGrid[x][y] >= 0)
			{
				/*if (eGroundDistanceGrid[x][y] < 4)
				{
				Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Colors::Black);
				}
				else if (eGroundDistanceGrid[x][y] >= 4 && eGroundDistanceGrid[x][y] < 7)
				{
				Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Colors::Red);
				}
				else if (eGroundDistanceGrid[x][y] >= 7 && eGroundDistanceGrid[x][y] < 10)
				{
				Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Colors::Blue);
				}
				else if (eGroundDistanceGrid[x][y] >= 10)
				{
				Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Colors::Green);
				}*/
			}

			/*if (distanceGridHome[x][y] > 0)
			{
			if (distanceGridHome[x][y] < 50)
			{
			Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Colors::Black);
			}
			else if (distanceGridHome[x][y] >= 50 && distanceGridHome[x][y] < 100)
			{
			Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Colors::Red);
			}
			else if (distanceGridHome[x][y] >= 100 && distanceGridHome[x][y] < 150)
			{
			Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Colors::Blue);
			}
			else if (distanceGridHome[x][y] >= 150)
			{
			Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Colors::Green);
			}
			}
			else if (distanceGridHome[x][y] < 0)
			{
			Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Colors::White);
			}*/

			if (eGroundGrid[x][y] > 0)
			{
				//Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Broodwar->enemy()->getColor());
			}
			if (eAirGrid[x][y] > 0)
			{
				//Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Broodwar->enemy()->getColor());
			}
			if (eGroundDistanceGrid[x][y] > 1)
			{
				//Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Broodwar->enemy()->getColor());
			}
			if (eDetectorGrid[x][y] > 0)
			{
				//Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Broodwar->enemy()->getColor());
			}

			if (observerGrid[x][y] > 0)
			{
				//Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Broodwar->self()->getColor());
			}
			if (aClusterGrid[x][y] > 0)
			{
				//Broodwar->drawBoxMap(Position(x * 8, y * 8), Position(x * 8 + 8, y * 8 + 8), Broodwar->self()->getColor());
			}

			if (aClusterGrid[x][y] > center)
			{
				center = aClusterGrid[x][y];
				armyCenter = Position(WalkPosition(x, y));
			}

			// Reset WalkPosition grids
			aClusterGrid[x][y] = 0;
			antiMobilityGrid[x][y] = 0;
			eGroundGrid[x][y] = 0.0;
			eAirGrid[x][y] = 0.0;
			eGroundDistanceGrid[x][y] = 0.0;
			eAirDistanceGrid[x][y] = 0.0;
			observerGrid[x][y] = 0;
			arbiterGrid[x][y] = 0;
			eDetectorGrid[x][y] = 0;

			// Reset TilePosition grids (removes one iteration of the map tiles)
			if (x % 32 == 0 && y && 32 == 0)
			{
				// Reset cluster grids
				eGroundClusterGrid[x / 4][y / 4] = 0;
				eAirClusterGrid[x / 4][y / 4] = 0;

				// Reset other grids
				reserveGrid[x / 4][y / 4] = 0;
				baseGrid[x / 4][y / 4] = 0;
				pylonGrid[x / 4][y / 4] = 0;
				batteryGrid[x / 4][y / 4] = 0;
			}
		}
	}
	return;
}

void GridTrackerClass::update()
{
	reset();
	updateMobilityGrids();
	updateAllyGrids();
	updateEnemyGrids();
	updateNeutralGrids();
	updateGroundDistanceGrid();
	return;
}

void GridTrackerClass::updateAllyGrids()
{
	// Clusters and anti mobility from units
	for (auto &u : Units().getMyUnits())
	{
		WalkPosition start = u.second.getWalkPosition();
		int offsetX = u.second.getPosition().x % 32;
		int offsetY = u.second.getPosition().y % 32;

		// Make sure unit is alive and not an Arbiter or Observer
		if (u.second.getDeadFrame() == 0 && u.second.getType() != UnitTypes::Protoss_Arbiter && u.second.getType() != UnitTypes::Protoss_Observer && u.second.getType() != UnitTypes::Terran_Medic)
		{
			for (int x = start.x - 20; x <= start.x + 20 + u.second.getType().tileWidth() * 4; x++)
			{
				for (int y = start.y - 20; y <= start.y + 20 + u.second.getType().tileHeight() * 4; y++)
				{
					if (WalkPosition(x, y).isValid() && (u.second.getPosition()).getDistance(Position((x * 8 + offsetX), (y * 8 + offsetY))) <= 160)
					{
						aClusterGrid[x][y] += 1;
					}
				}
			}
		}
		// Anti mobility doesn't apply to flying units (carriers, scouts, shuttles)
		if (!u.second.getType().isFlyer())
		{
			for (int x = start.x; x <= start.x + u.second.getType().tileWidth() * 4; x++)
			{
				for (int y = start.y; y <= start.y + u.second.getType().tileHeight() * 4; y++)
				{
					if (WalkPosition(x, y).isValid())
					{
						antiMobilityGrid[x][y] = 1;
					}
				}
			}
		}
	}

	// Building Grid update
	for (auto &u : Buildings().getMyBuildings())
	{
		if (u.first->exists())
		{
			// Anti Mobility Grid
			WalkPosition start = u.second.getWalkPosition();
			for (int x = start.x - 2; x < 2 + start.x + u.second.getUnitType().tileWidth() * 4; x++)
			{
				for (int y = start.y - 2; y < 2 + start.y + u.second.getUnitType().tileHeight() * 4; y++)
				{
					if (WalkPosition(x, y).isValid())
					{
						antiMobilityGrid[x][y] = 1;
					}
				}
			}

			// Reserve Grid
			TilePosition tile = u.second.getTilePosition();
			int offset = 0;
			if (u.second.getUnitType() == UnitTypes::Protoss_Gateway || u.second.getUnitType() == UnitTypes::Protoss_Robotics_Facility)
			{
				offset = 1;
			}
			for (int x = tile.x - offset; x < tile.x + u.second.getUnitType().tileWidth() + offset; x++)
			{
				for (int y = tile.y - offset; y < tile.y + u.second.getUnitType().tileHeight() + offset; y++)
				{
					if (TilePosition(x, y).isValid())
					{
						reserveGrid[x][y] = 1;
					}
				}
			}
			// Pylon Grid
			if (u.second.getUnitType() == UnitTypes::Protoss_Pylon)
			{
				for (int x = u.second.getTilePosition().x - 4; x < u.second.getTilePosition().x + u.second.getUnitType().tileWidth() + 4; x++)
				{
					for (int y = u.second.getTilePosition().y - 4; y < u.second.getTilePosition().y + u.second.getUnitType().tileHeight() + 4; y++)
					{
						if (TilePosition(x, y).isValid())
						{
							pylonGrid[x][y] += 1;
						}
					}
				}
			}
			// Shield Battery Grid
			if (u.second.getUnitType() == UnitTypes::Protoss_Shield_Battery)
			{
				for (int x = u.second.getTilePosition().x - 10; x < u.second.getTilePosition().x + u.second.getUnitType().tileWidth() + 10; x++)
				{
					for (int y = u.second.getTilePosition().y - 10; y < u.second.getTilePosition().y + u.second.getUnitType().tileHeight() + 10; y++)
					{
						if (TilePosition(x, y).isValid() && u.second.getPosition().getDistance(Position(TilePosition(x, y))) < 320)
						{
							batteryGrid[x][y] = 1;
						}
					}
				}
			}
			// Bunker Grid
			if (u.second.getUnitType() == UnitTypes::Terran_Bunker)
			{
				for (int x = u.second.getTilePosition().x - 10; x < u.second.getTilePosition().x + u.second.getUnitType().tileWidth() + 10; x++)
				{
					for (int y = u.second.getTilePosition().y - 10; y < u.second.getTilePosition().y + u.second.getUnitType().tileHeight() + 10; y++)
					{
						if (TilePosition(x, y).isValid() && u.second.getPosition().getDistance(Position(TilePosition(x, y))) < 320)
						{
							bunkerGrid[x][y] = 1;
						}
					}
				}
			}
		}
	}

	// Worker Grid update
	for (auto & worker : Workers().getMyWorkers())
	{
		// Anti Mobility Grid
		WalkPosition start = worker.second.getWalkPosition();
		for (int x = start.x; x <= start.x + worker.first->getType().tileWidth() * 4; x++)
		{
			for (int y = start.y; y <= start.y + worker.first->getType().tileHeight() * 4; y++)
			{
				if (WalkPosition(x, y).isValid())
				{
					antiMobilityGrid[x][y] = 1;
				}
			}
		}
	}

	// Base Grid
	for (auto & base : Bases().getMyBases())
	{
		for (int x = base.second.getTilePosition().x - 8; x < base.second.getTilePosition().x + 12; x++)
		{
			for (int y = base.second.getTilePosition().y - 8; y < base.second.getTilePosition().y + 11; y++)
			{
				baseGrid[x][y] = 1;
			}
		}
	}
	return;
}

void GridTrackerClass::updateEnemyGrids()
{
	for (auto &u : Units().getEnUnits())
	{
		WalkPosition start = u.second.getWalkPosition();
		// Cluster grid for storm/stasis (96x96)		
		if (u.second.unit() && u.second.unit()->exists() && !u.second.getType().isBuilding() && !u.second.unit()->isStasised() && !u.second.unit()->isMaelstrommed())
		{
			for (int x = u.second.getTilePosition().x - 1; x <= u.second.getTilePosition().x + 1; x++)
			{
				for (int y = u.second.getTilePosition().y - 1; y <= u.second.getTilePosition().y + 1; y++)
				{
					if (TilePosition(x, y).isValid())
					{
						if (!u.second.getType().isFlyer())
						{
							eGroundClusterGrid[x][y] += 1;
						}
						else
						{
							eAirClusterGrid[x][y] += 1;
						}
						if (u.second.getType() == UnitTypes::Terran_Siege_Tank_Tank_Mode || u.second.getType() == UnitTypes::Terran_Siege_Tank_Siege_Mode)
						{
							stasisClusterGrid[x][y] += 1;
						}
					}
				}
			}
		}

		// If the Unit is alive
		if (u.second.getDeadFrame() == 0)
		{

			// Detector Grid
			if (u.second.getType() == UnitTypes::Protoss_Observer || u.second.getType() == UnitTypes::Protoss_Photon_Cannon || u.second.getType() == UnitTypes::Zerg_Overlord || u.second.getType() == UnitTypes::Zerg_Spore_Colony || u.second.getType() == UnitTypes::Terran_Science_Vessel || u.second.getType() == UnitTypes::Terran_Missile_Turret)
			{
				for (int x = u.second.getWalkPosition().x - 40; x <= 2 + u.second.getWalkPosition().x + 40; x++)
				{
					for (int y = u.second.getWalkPosition().y - 40; y <= 2 + u.second.getWalkPosition().y + 40; y++)
					{
						if (WalkPosition(x, y).isValid() && Position(WalkPosition(x, y)).getDistance(u.second.getPosition()) < u.second.getType().sightRange())
						{
							eDetectorGrid[x][y] = 1;
						}
					}
				}
			}

			// Threat Grids
			for (int x = u.second.getWalkPosition().x - 50; x <= 2 + u.second.getWalkPosition().x + 50; x++)
			{
				for (int y = u.second.getWalkPosition().y - 50; y <= 2 + u.second.getWalkPosition().y + 50; y++)
				{
					if (WalkPosition(x, y).isValid())
					{
						double distance = 1.0 + Position(x * 8, y * 8).getDistance(u.second.getPosition()) - (double(u.second.getType().tileWidth()) * 16.0);

						if (u.second.getAirDamage() > 0 && distance < u.second.getAirRange() + u.second.getSpeed() * 16.0)
						{
							if (distance > 0)
							{
								eAirDistanceGrid[x][y] += u.second.getMaxStrength() / distance;
							}
							else
							{
								eAirDistanceGrid[x][y] += u.second.getMaxStrength();
							}
						}

						if (u.second.getGroundDamage() > 0 && distance < u.second.getGroundRange() + u.second.getSpeed() * 16.0)
						{
							if (distance > 0)
							{
								eGroundDistanceGrid[x][y] += u.second.getMaxStrength() / distance;
							}
							else
							{
								eGroundDistanceGrid[x][y] += u.second.getMaxStrength();
							}
						}

						if (u.second.getGroundDamage() > 0.0 && Position(x * 8, y * 8).getDistance(u.second.getPosition()) - u.second.getType().tileWidth() * 16 < u.second.getGroundRange())
						{
							eGroundGrid[x][y] += u.second.getMaxStrength();
						}
						if (u.second.getAirDamage() > 0.0 && Position(x * 8, y * 8).getDistance(u.second.getPosition()) - u.second.getType().tileWidth() * 16 < u.second.getAirRange())
						{
							eAirGrid[x][y] += u.second.getMaxStrength();
						}
					}
				}
			}

			// Anti Mobility Grid
			if (u.second.getType().isBuilding())
			{
				for (int x = start.x; x < start.x + u.second.getType().tileWidth() * 4; x++)
				{
					for (int y = start.y; y < start.y + u.second.getType().tileHeight() * 4; y++)
					{
						if (WalkPosition(x, y).isValid())
						{
							antiMobilityGrid[x][y] = 1;
						}
					}
				}
			}
			else
			{
				for (int x = start.x; x <= start.x + u.second.getType().tileWidth() * 4; x++)
				{
					for (int y = start.y; y <= start.y + u.second.getType().tileHeight() * 4; y++)
					{
						if (WalkPosition(x, y).isValid())
						{
							antiMobilityGrid[x][y] = 1;
						}
					}
				}
			}
		}
	}
	return;
}

void GridTrackerClass::updateNeutralGrids()
{
	// Resource Grid for Minerals
	for (auto &m : Resources().getMyMinerals())
	{
		for (int x = m.second.getTilePosition().x - 5; x < m.second.getTilePosition().x + m.second.getUnitType().tileWidth() + 5; x++)
		{
			for (int y = m.second.getTilePosition().y - 5; y < m.second.getTilePosition().y + m.second.getUnitType().tileHeight() + 5; y++)
			{
				if (Position(Bases().getMyBases()[m.second.getClosestBase()].getResourcesPosition()).getDistance(Position(TilePosition(x, y))) <= 192 && TilePosition(x, y).isValid() && m.second.getPosition().getDistance(m.second.getClosestBase()->getPosition()) + 64 > Position(x * 32, y * 32).getDistance(m.second.getClosestBase()->getPosition()))
				{
					resourceGrid[x][y] = 1;
				}
			}
		}
	}

	// Resource Grid for Gas
	for (auto &g : Resources().getMyGas())
	{
		for (int x = g.second.getTilePosition().x - 5; x < g.second.getTilePosition().x + g.second.getUnitType().tileWidth() + 5; x++)
		{
			for (int y = g.second.getTilePosition().y - 5; y < g.second.getTilePosition().y + g.second.getUnitType().tileHeight() + 5; y++)
			{
				if (Position(Bases().getMyBases()[g.second.getClosestBase()].getResourcesPosition()).getDistance(Position(TilePosition(x, y))) <= 256 && TilePosition(x, y).isValid() && g.second.getPosition().getDistance(g.second.getClosestBase()->getPosition()) + 64 > Position(x * 32, y * 32).getDistance(g.second.getClosestBase()->getPosition()))
				{
					resourceGrid[x][y] = 1;
				}
			}
		}
	}

	// Anti Mobility Grid -- TODO: Improve by storing the units
	for (auto u : Broodwar->neutral()->getUnits())
	{
		if (u->getType().isFlyer())
		{
			continue;
		}
		int startX = (u->getTilePosition().x * 4);
		int startY = (u->getTilePosition().y * 4);
		for (int x = startX - 2; x < 2 + startX + u->getType().tileWidth() * 4; x++)
		{
			for (int y = startY - 2; y < 2 + startY + u->getType().tileHeight() * 4; y++)
			{
				antiMobilityGrid[x][y] = 1;
			}
		}
	}
	return;
}

void GridTrackerClass::updateMobilityGrids()
{
	if (!mobilityAnalysis && Terrain().isAnalyzed())
	{
		mobilityAnalysis = true;
		/*for (int x = 0; x <= Broodwar->mapWidth() * 4; x++)
		{
		for (int y = 0; y <= Broodwar->mapHeight() * 4; y++)
		{
		mobilityGrid[x][y] = 0;
		}
		}*/
		for (int x = 0; x <= Broodwar->mapWidth() * 4; x++)
		{
			for (int y = 0; y <= Broodwar->mapHeight() * 4; y++)
			{
				if (WalkPosition(x, y).isValid() && theMap.GetMiniTile(WalkPosition(x, y)).Walkable())
				{
					for (int i = -12; i <= 12; i++)
					{
						for (int j = -12; j <= 12; j++)
						{
							// The more tiles around x,y that are walkable, the more mobility x,y has				
							if (WalkPosition(x + i, y + j).isValid() && theMap.GetMiniTile(WalkPosition(x + i, y + j)).Walkable())
							{
								mobilityGrid[x][y] += 1;
							}
						}
					}
					mobilityGrid[x][y] = int(double(mobilityGrid[x][y]) / 56);

					if (getNearestChokepoint(Position(x * 8, y * 8)) && getNearestChokepoint(Position(x * 8, y * 8))->getCenter().getDistance(Position(x * 8, y * 8)) < 320)
					{
						bool notCorner = true;
						int startRatio = int(pow(getNearestChokepoint(Position(x * 8, y * 8))->getCenter().getDistance(Position(x * 8, y * 8)) / 64, 2.0));
						for (int i = 0 - startRatio; i <= startRatio; i++)
						{
							for (int j = 0 - startRatio; j <= 0 - startRatio; j++)
							{
								if (WalkPosition(x + i, y + j).isValid() && !theMap.GetMiniTile(WalkPosition(x + i, y + j)).Walkable())
								{
									notCorner = false;
								}
							}
						}

						if (notCorner)
						{
							mobilityGrid[x][y] = 10;
						}
					}

					// Max a mini grid to 10
					mobilityGrid[x][y] = min(mobilityGrid[x][y], 10);
				}

				// Setup what is possible to check ground distances on
				if (mobilityGrid[x][y] <= 0)
				{
					distanceGridHome[x][y] = -1;
				}
				else if (mobilityGrid[x][y] > 0)
				{
					distanceGridHome[x][y] = 0;
				}
			}
		}
	}
	return;
}

void GridTrackerClass::updateObserverMovement(Unit observer)
{
	WalkPosition destination = WalkPosition(SpecialUnits().getMyObservers()[observer].getDestination());

	for (int x = destination.x - 40; x <= destination.x + 40; x++)
	{
		for (int y = destination.y - 40; y <= destination.y + 40; y++)
		{
			// Create a circle of detection rather than a square
			if (WalkPosition(x, y).isValid() && SpecialUnits().getMyObservers()[observer].getDestination().getDistance(Position(WalkPosition(x, y))) < 320)
			{
				observerGrid[x][y] = 1;
			}
		}
	}
	return;
}

void GridTrackerClass::updateArbiterMovement(Unit arbiter)
{
	WalkPosition destination = WalkPosition(SpecialUnits().getMyArbiters()[arbiter].getDestination());

	for (int x = destination.x - 20; x <= destination.x + 20; x++)
	{
		for (int y = destination.y - 20; y <= destination.y + 20; y++)
		{
			// Create a circle of detection rather than a square
			if (WalkPosition(x, y).isValid() && SpecialUnits().getMyArbiters()[arbiter].getDestination().getDistance(Position(WalkPosition(x, y))) < 160)
			{
				arbiterGrid[x][y] = 1;
			}
		}
	}
	return;
}

void GridTrackerClass::updateAllyMovement(Unit unit, WalkPosition here)
{
	for (int x = here.x - unit->getType().width() / 16; x <= here.x + unit->getType().width() / 16; x++)
	{
		for (int y = here.y - unit->getType().height() / 16; y <= here.y + unit->getType().height() / 16; y++)
		{
			if (WalkPosition(x, y).isValid())
			{
				antiMobilityGrid[x][y] = 1;
			}
		}
	}
	return;
}

void GridTrackerClass::updateReservedLocation(UnitType building, TilePosition here)
{
	// When placing a building, reserve the tiles so no further locations are placed there
	for (int x = here.x; x < here.x + building.tileWidth(); x++)
	{
		for (int y = here.y; y < here.y + building.tileHeight(); y++)
		{
			reserveGrid[x][y] = 1;
		}
	}
	return;
}

void GridTrackerClass::updateGroundDistanceGrid()
{
	// TODO: Goal with this grid is to create a ground distance grid from home for unit micro
	// Need to check for islands
	if (mobilityAnalysis && !distanceAnalysis && Terrain().isAnalyzed() && Broodwar->getFrameCount() > 500)
	{
		WalkPosition start = WalkPosition(Terrain().getPlayerStartingPosition());
		distanceGridHome[start.x][start.y] = 1;
		distanceAnalysis = true;
		bool done = false;
		int cnt = 0;
		int segment = 0;
		clock_t myClock;
		double duration;
		myClock = clock();

		while (!done)
		{
			duration = (clock() - myClock) / (double)CLOCKS_PER_SEC;
			if (duration > 2)
			{
				break;
			}
			done = true;
			cnt++;
			segment += 8;
			for (int x = 0; x <= Broodwar->mapWidth() * 4; x++)
			{
				for (int y = 0; y <= Broodwar->mapHeight() * 4; y++)
				{
					// If any of the grid is 0, we're not done yet
					if (distanceGridHome[x][y] == 0 && theMap.GetMiniTile(WalkPosition(x, y)).AreaId() > 0)
					{
						done = false;
					}
					if (distanceGridHome[x][y] == cnt)
					{
						for (int i = x - 1; i <= x + 1; i++)
						{
							for (int j = y - 1; j <= y + 1; j++)
							{
								if (distanceGridHome[i][j] == 0 && Position(WalkPosition(i, j)).getDistance(Position(start)) <= segment)
								{
									distanceGridHome[i][j] = cnt + 1;
								}
							}
						}
					}
				}
			}
		}
		Broodwar << "Distance Grid Analysis time: " << duration << endl;
		if (duration > 2)
		{
			Broodwar << "Hit maximum, check for islands." << endl;
		}
	}
}