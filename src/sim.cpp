#include "sim.hpp"

SDL_Renderer *Sim::renderer;
SDL_Texture *Sim::background, *Sim::player, *Sim::tile, *Sim::specialTile, *Sim::wall, *Sim::path, *Sim::visited, *Sim::specialVisited;
SDL_Rect Sim::rect;
int Sim::n, Sim::k, Sim::currVertex, Sim::startVertex, Sim::endVertex, Sim::inProcess, Sim::moving, Sim::sourceVertex, Sim::minDist = INT_MAX;
unordered_map<int, int> Sim::weights;
unordered_map<int, unordered_map<int, pair<int, int>>> Sim::distances;
unordered_set<int> Sim::specialVertices;
vector<int> Sim::bestPermutation;

//initialise textures
int Sim::initTextures(SDL_Renderer *_renderer)
{
	renderer = _renderer;
	SDL_Surface *surface;

	surface = IMG_Load("../assets/images/background.tif");
	background = SDL_CreateTextureFromSurface(renderer, surface);

	surface = IMG_Load("../assets/images/player.tif");
	player = SDL_CreateTextureFromSurface(renderer, surface);

	surface = IMG_Load("../assets/images/tile.tif");
	tile = SDL_CreateTextureFromSurface(renderer, surface);

	surface = IMG_Load("../assets/images/special_tile.tif");
	specialTile = SDL_CreateTextureFromSurface(renderer, surface);

	surface = IMG_Load("../assets/images/wall.tif");
	wall = SDL_CreateTextureFromSurface(renderer, surface);

	surface = IMG_Load("../assets/images/path.tif");
	path = SDL_CreateTextureFromSurface(renderer, surface);

	surface = IMG_Load("../assets/images/visited.tif");
	visited = SDL_CreateTextureFromSurface(renderer, surface);

	surface = IMG_Load("../assets/images/visited_special.tif");
	specialVisited = SDL_CreateTextureFromSurface(renderer, surface);

	return 0;
}

//The function responsible for simulation
void Sim::beginSimulation(int _n, int _k)
{
	Map::generateRandomMaze(n = _n); //sets up walls for the maze
	generateSpecialVertices(k = _k); //negative valued cells are generated with random values (stores location in specialVertices as n*col+row)
	assignWeights();				 //assign a positive weight to non negative weighted cells
	startVertex = n + 1, currVertex = startVertex, endVertex = (n - 2) * (n + 1), moving = 0;
	initDistance(); //sets up the distance graph ([pos i: [pos j, weight]])

	rect = {0, 0, WINDOW_WIDTH / n, WINDOW_HEIGHT / n};
	SDL_Event e;

	SDL_RenderClear(renderer);
	Fonts::displayText(renderer, string("Performing pre-computation...").c_str(), WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 3, {255, 255, 255});
	SDL_RenderPresent(renderer);

	FW();
	traverseAllPossibilities();

	while (true)
	{
		if (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				return;
			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
				return;
		}
	}
}

//generates the vertices which increase the point value
void Sim::generateSpecialVertices(int k)
{
	assert(k <= 2 * (n / 2) * (n / 2) - 1);
	while (k--)
	{
		while (true)
		{
			int col = Map::genRandom(n - 1), row = Map::genRandom(n - 1);
			if (Map::map[col][row])
				continue;
			Map::map[col][row] = 2;
			specialVertices.insert(n * col + row);
			weights[n * col + row] = Map::genRandom(-MIN_COST, -MAX_COST);
			break;
		}
	}
}

//assigns weights to cells
void Sim::assignWeights()
{
	for (int col = 0; col < n; ++col)
		for (int row = 0; row < n; ++row)
			if (Map::map[col][row] == 0)
				weights[n * col + row] = Map::genRandom(MAX_WEIGHT, 1);
}

//initialise the distance graph
void Sim::initDistance()
{
	for (int col = 1; col < n - 1; col++)
	{
		for (int row = 1; row < n - 1; row++)
		{
			if (Map::map[col][row] != 1)
			{
				if (Map::map[col - 1][row] != 1)
					distances[n * col + row][n * (col - 1) + row] = {max(0, weights[n * (col - 1) + row]), n * col + row};
				if (Map::map[col][row - 1] != 1)
					distances[n * col + row][n * (col) + row - 1] = {max(0, weights[n * (col)-1 + row]), n * col + row};
				if (Map::map[col + 1][row] != 1)
					distances[n * col + row][n * (col + 1) + row] = {max(0, weights[n * (col + 1) + row]), n * col + row};
				if (Map::map[col][row + 1] != 1)
					distances[n * col + row][n * (col) + row + 1] = {max(0, weights[n * (col) + 1 + row]), n * col + row};
			}
		}
	}
}

//start FW
void Sim::FW()
{
	for (auto &p : distances)
	{
		for (auto &q : distances)
		{
			for (auto &r : distances)
			{
				if (q.second.count(p.first) && p.second.count(r.first))
					if (!q.second.count(r.first) || q.second[p.first].first + p.second[r.first].first < q.second[r.first].first)
						q.second[r.first] = {q.second[p.first].first + p.second[r.first].first, p.second[r.first].second};
			}
		}
	}
	for (auto v : specialVertices)
	{
		distances[startVertex][v].first += weights[v];
		for (auto u : specialVertices)
			if (v != u)
				distances[v][u].first += weights[u];
	}
}

void Sim::traverseAllPossibilities()
{
	for (int mask = 0; mask < (1 << k); ++mask)
	{
		vector<int> currSpecial;
		auto it = specialVertices.begin();
		for (int i = 0; i < k; ++i)
		{
			if (mask & (1 << i))
				currSpecial.push_back(*it);
			++it;
		}
		sort(currSpecial.begin(), currSpecial.end());
		do
		{
			simulatePermutation(currSpecial);
		} while (next_permutation(currSpecial.begin(), currSpecial.end()));
	}
}

void Sim::simulatePermutation(vector<int> &currSpecial)
{
	reRender();
	int currDist = 0, prevVertex = startVertex;
	renderVisit(startVertex, 100);
	for (auto &v : currSpecial)
	{
		currDist += distances[prevVertex][v].first, prevVertex = v;
		renderVisit(v, 100);
	}
	currDist += distances[prevVertex][endVertex].first;
	renderVisit(endVertex, 100);
	SDL_Delay(100);
	if (currDist < minDist)
		minDist = currDist, bestPermutation = currSpecial;
}

// @TODO: literally todo
void Sim::updatePos()
{
}

void Sim::renderVisit(int v, int delay)
{
	SDL_Delay(delay);
	rect.x = (v / n) * rect.w, rect.y = (v % n) * rect.h;
	SDL_RenderCopy(renderer, specialVertices.count(v) ? specialVisited : visited, NULL, &rect);
	Fonts::displayText(renderer, to_string(weights[v]).c_str(), rect.x + rect.w / 2, rect.y + rect.h / 2);
	SDL_RenderPresent(renderer);
}

void Sim::reRender()
{
	SDL_RenderClear(renderer);
	for (int col = 0; col < n; ++col)
		for (int row = 0; row < n; ++row)
		{
			rect.x = rect.w * col;
			rect.y = rect.h * row;
			switch (Map::map[col][row])
			{
			case 2:
				SDL_RenderCopy(renderer, specialTile, NULL, &rect);
				break;
			case 1:
				SDL_RenderCopy(renderer, wall, NULL, &rect);
				break;
			default:
				SDL_RenderCopy(renderer, tile, NULL, &rect);
				break;
			}
			if (Map::map[col][row] != 1)
				Fonts::displayText(renderer, to_string(weights[n * col + row]).c_str(), rect.x + rect.w / 2, rect.y + rect.h / 2);
		}
	SDL_RenderPresent(renderer);
}
