#pragma once
#include <set>
#include <BWAPI.h>
#include <bwem.h>

namespace BWEB {

    class Station
    {
        const BWEM::Base* base;
        std::set<BWAPI::TilePosition> defenses;
        BWAPI::Position resourceCentroid;

    public:
        Station(BWAPI::Position newResourceCenter, std::set<BWAPI::TilePosition>& newDefenses, const BWEM::Base* newBase)
        {
            resourceCentroid = newResourceCenter;
            defenses = newDefenses;
            base = newBase;
        }

        /// <summary> Returns the central position of the resources associated with this base including geysers. </summary>
        BWAPI::Position getResourceCentroid() { return resourceCentroid; }

        /// <summary> Returns the set of defense locations associated with this base. </summary>
        std::set<BWAPI::TilePosition>& getDefenseLocations() { return defenses; }

        /// <summary> Returns the BWEM base associated with this BWEB base. </summary>
        const BWEM::Base * getBWEMBase() { return base; }

        /// <summary> Returns the number of ground defenses associated with this Station. </summary>
        int getGroundDefenseCount();

        /// <summary> Returns the number of air defenses associated with this Station. </summary>
        int getAirDefenseCount();
    };

    namespace Stations {

        /// <summary> Initializes the building of every BWEB::Station on the map, call it only once per game. </summary>
        void findStations();

        /// <summary> Draws all BWEB Stations. </summary>
        void draw();

        /// <summary> Returns a vector containing every BWEB::Station </summary>
        std::vector<Station>& getStations();

        /// <summary> Returns the closest BWEB::Station to the given TilePosition. </summary>
        Station * getClosestStation(BWAPI::TilePosition);
    }
}