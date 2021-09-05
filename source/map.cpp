#include "map.h"
using namespace wsp;

// recursive backtracking algorithm for maze generation
std::vector<std::vector<MazeCell>> RecursiveBacktrackingMaze(int row, int column, std::vector<std::vector<MazeCell>> maze, std::default_random_engine rng) {
	maze[row][column].visited = true;
	char directions[4] = {'n', 's', 'e', 'w'};
	std::shuffle(std::begin(directions), std::end(directions), rng);
	for (int i = 0; i < 4; i++) { // go in each direction
		char dir = directions[i];
		int x = column;
		int y = row;
		if (dir == 'n') y--;
		if (dir == 's') y++;
		if (dir == 'e') x++;
		if (dir == 'w') x--;
		if (y >= 0 && y < (int) maze.size() && x >= 0 && x < (int) maze[0].size() && !maze[y][x].visited) { // if the neighbor is an unvisited cell in bounds
			// update edges and then call again for the new cell
			if (dir == 'n') maze[y + 1][x].north = false;
			if (dir == 's') maze[y][x].north = false;
			if (dir == 'e') maze[y][x].west = false;
			if (dir == 'w') maze[y][x + 1].west = false;
			maze = RecursiveBacktrackingMaze(y, x, maze, rng);
		}
	}
	return maze;
}

int Map::GetSpinningWalls() { return spinningWallCount; }
// delete the map and clear out the wall manager if one is supplied
void Map::Destroy(LayerManager* wallManager) {
	if (wallManager) {
		for (int i = 0; i < (int) wallManager->GetSize(); i++) {
			delete wallManager->GetLayerAt(i);
		}
		wallManager->RemoveAll();
	}
	delete this;
}
// turn the map data into physical walls
void Map::GenerateWalls(LayerManager* wallManager) {
	AddSpinners(wallManager);
	AddWallsFromCells(wallManager);
}
void Map::SpawnTanks(int tankCount, LayerManager* tankManager, int ammo) {
	for (int player = 0; player < tankCount; player++) {
		Tank* tank = new Tank(player, ammo);
		// set initial tank positions (1 in each corner)
		f32 tankXOffset = (cellWidth - tank->GetWidth() + wallThickness) / 2;
		f32 tankYOffset = (cellHeight - tank->GetHeight() + wallThickness) / 2;
		// tankoffset centers in top-left cell, add additional values to put each tank in its respective cell
		if (player == 0) tank->SetPosition(tankXOffset + cellWidth,               tankYOffset + cellHeight);
		if (player == 1) tank->SetPosition(tankXOffset + cellWidth * (width - 2), tankYOffset + cellHeight);
		if (player == 2) tank->SetPosition(tankXOffset + cellWidth,               tankYOffset + cellHeight * (height - 2));
		if (player == 3) tank->SetPosition(tankXOffset + cellWidth * (width - 2), tankYOffset + cellHeight * (height - 2));
		if (player % 2) tank->SetRotation(90); // make every other tank face left instead of right
		tankManager->Append(tank);
	}
}
Map::Map(int screenWidth, int screenHeight, int width, int height, int wallThickness) {
	this->width = width;
	this->height = height;
	this->wallThickness = wallThickness;
	this->cellWidth = (screenWidth - wallThickness) / (f32) width;
	this->cellHeight = (screenHeight - wallThickness) / (f32) height;
	this->spinningWallCount = 0;
	this->rng = std::default_random_engine(time(NULL)); // rng w/ current time (updates each second) as seed
	// initialize and generate a 2d vector w/ a maze
	std::vector<std::vector<MazeCell>> maze(height, std::vector<MazeCell>(width));
	for (int row = 0; row < height; row++) {
		for (int column = 0; column < width; column++) {
			maze[row][column].visited = false;
			maze[row][column].north = true;
			maze[row][column].west = true;
		}
	}
	cells = RecursiveBacktrackingMaze(0, 0, maze, rng);
	// open up the maze a little and create the walls
	OpenUp();
}
void Map::OpenUp() {
	// deletes some walls to make the map more open
	for (int row = 0; row < height; row++) {
		for (int column = 0; column < width; column++) {
			// for each cell, there's a 1/4 chance to take away the north side and a 1/4 chance to take away the west side
			if (!(rng() % 4) && row) cells[row][column].north = false;
			if (!(rng() % 4) && column) cells[row][column].west = false;
		}
	}
}
void Map::AddSpinners(LayerManager* wallManager) {
	// adds spinning walls
	for (int y = 1; y < height; y++) {
		for (int x = 1; x < width; x++) {
			if (!(rng() % 4) && !cells[y][x].north && !cells[y][x].west && !cells[y][x - 1].north && !cells[y - 1][x].west) { // if an open 2x2 of cells, 1/4 chance to add
				spinningWallCount++;
				Quad* wall = CreateWall(wallManager);
				wall->SetHeight(wallThickness);
				wall->SetWidth(cellWidth + wallThickness - 1);
				int wallX = cellWidth * x - wall->GetWidth() / 2 + wallThickness / 2;
				int wallY = cellHeight * y - wall->GetHeight() / 2 + wallThickness / 2;
				wall->SetPosition(wallX, wallY);
				wall->SetRotation(rng() % 180);
			}
		}
	}
}
void Map::AddWallsFromCells(LayerManager* wallManager) {
	// creates the walls
	for (int row = 0; row <= height; row++) {
		for (int column = 0; column <= width; column++) {
			if ((row < height && column < width && cells[row][column].north) || (row == height && column < width)) { // north side of each cell & south border
				Quad* wall = CreateWall(wallManager);
				wall->SetPosition(cellWidth * column, cellHeight * row);
				wall->SetHeight(wallThickness);
				wall->SetWidth(cellWidth + wallThickness - 1);
			}
			if ((row < height && column < width && cells[row][column].west) || (row < height && column == width)) { // west side of each cell & east border
				Quad* wall = CreateWall(wallManager);
				wall->SetPosition(cellWidth * column, cellHeight * row);
				wall->SetWidth(wallThickness);
				wall->SetHeight(cellWidth + wallThickness - 1);
			}
		}
	}
}
// makes a stylized wall quad
Quad* Map::CreateWall(LayerManager* wallManager) {
	Quad* wall = new Quad();
	wall->SetFillColor((GXColor) {63, 63, 63, 255});
	wallManager->Append(wall);
	return wall;
}