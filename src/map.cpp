#include "map.hpp"

vector<vector<int>> Map::map;
int Map::n;
boost::random::mt19937 Map::gen;

void Map::generateRandomMaze(int _n)
{
	n = _n;
	assert(n > 1 && (n & 1));
	map = vector<vector<int>>(n, vector<int>(n, 1));
	vector<vector<array<bool, 4>>> walls = generateRandomWalls();
	removeWalls(walls);

	for (int i = 0; i < n / 2; ++i)
	{
		vector<int> col1(n, 0), col2(n, 0);
		assert(walls[i][0][0]);
		col1[0] = col2[0] = 1;
		for (int j = 0; j < n / 2; ++j)
		{
			if (walls[i][j][3])
				col1[2 * j] = col1[2 * j + 1] = col1[2 * j + 2] = 1;
			if (walls[i][j][2])
				col1[2 * j + 2] = col2[2 * j + 2] = 1;
			if (i > 0 && walls[i - 1][j][2])
				col1[2 * j + 2] = 1;
		}
		map[2 * i] = col1;
		map[2 * i + 1] = col2;
	}
}

vector<vector<array<bool, 4>>> Map::generateRandomWalls()
{
	vector<vector<bool>> visited(n / 2, vector<bool>(n / 2, 0));
	vector<vector<array<bool, 4>>> walls(visited.size(), vector<array<bool, 4>>(visited[0].size(), {1, 1, 1, 1}));
	stack<pair<int, int>> partlyProcessed;
	pair<int, int> xy = {genRandom(n / 2 - 1), genRandom(n / 2 - 1)};

	while (true)
	{
		visited[xy.first][xy.second] = 1;
		vector<int> directions;
		if (xy.second != 0 && !visited[xy.first][xy.second - 1])
			directions.push_back(0);
		if (xy.first != n / 2 - 1 && !visited[xy.first + 1][xy.second])
			directions.push_back(1);
		if (xy.second != n / 2 - 1 && !visited[xy.first][xy.second + 1])
			directions.push_back(2);
		if (xy.first != 0 && !visited[xy.first - 1][xy.second])
			directions.push_back(3);

		if (directions.empty())
		{
			if (partlyProcessed.empty())
				break;
			xy = partlyProcessed.top();
			partlyProcessed.pop();
			continue;
		}

		if (directions.size() > 1)
			partlyProcessed.push(xy);
		int direction = directions[genRandom(directions.size() - 1)];
		walls[xy.first][xy.second][direction] = 0;
		if (direction == 0)
			--xy.second;
		else if (direction == 1)
			++xy.first;
		else if (direction == 2)
			++xy.second;
		else
			--xy.first;
		walls[xy.first][xy.second][(direction + 2) % 4] = 0;
	}

	return walls;
}

void Map::removeWalls(vector<vector<array<bool, 4>>> &walls)
{
	int removed = 0;
	while (removed < (n / 2) * (n / 2) * REMOVE_PROB)
	{
		int row = genRandom(n / 2 - 1), col = genRandom(n / 2 - 1);
		vector<int> directions;
		if (row != 0 && walls[col][row][0])
			directions.push_back(0);
		if (col != n / 2 - 1 && walls[col][row][1])
			directions.push_back(1);
		if (row != n / 2 - 1 && walls[col][row][2])
			directions.push_back(2);
		if (col != 0 && walls[col][row][3])
			directions.push_back(3);
		for (int direction : directions)
		{
			if (genRandom(100) >= 100 * REMOVE_PROB)
				continue;
			++removed;
			walls[col][row][direction] = 0;
			if (direction == 0)
				--row;
			else if (direction == 1)
				++col;
			else if (direction == 2)
				++row;
			else
				--col;
			walls[col][row][(direction + 2) % 4] = 0;
		}
	}
}

int Map::genRandom(int M, int m)
{
	boost::random::uniform_int_distribution<> dist(m, M);
	return dist(Map::gen);
}
