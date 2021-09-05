#ifndef TANK_MAP_H
#define TANK_MAP_H

#include <stdlib.h>
#include <gccore.h>
#include <wiisprite.h>
#include <vector>
#include <time.h>
#include <algorithm>
#include <random>

#include "tank.h"

using namespace wsp;

// contains info about a cell in a maze, used for map generation; only north and west edges are described since the adjacent cells will have info about the other edges
struct MazeCell {
	bool visited;
	bool north;
	bool west;
};

// recursive backtracking algorithm for maze generation
std::vector<std::vector<MazeCell>> RecursiveBacktrackingMaze(int row, int column, std::vector<std::vector<MazeCell>> maze, std::default_random_engine rng);

class Map {
	public:
	 	int GetSpinningWalls();
		// delete the map and clear out the wall manager if one is supplied
		void Destroy(LayerManager* wallManager = NULL);
		// turn the map data into physical walls
		void GenerateWalls(LayerManager* wallManager);
		void SpawnTanks(int tankCount, LayerManager* tankManager, int ammo);
	 	Map(int screenWidth, int screenHeight, int width, int height, int wallThickness);
	private:
	 	std::vector<std::vector<MazeCell>> cells;
		int width;
		int height;
		f32 cellWidth;
		f32 cellHeight;
		int wallThickness;
		int spinningWallCount;
		std::default_random_engine rng;
		void OpenUp();
		void AddSpinners(LayerManager* wallManager);
		void AddWallsFromCells(LayerManager* wallManager);
		Quad* CreateWall(LayerManager* wallManager);
};

#endif