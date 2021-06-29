#include "simulation.hpp"

SDL_Renderer *Simulation::renderer;
SDL_Texture *Simulation::background, *Simulation::player, *Simulation::tile, *Simulation::specialTile, *Simulation::wall, *Simulation::path, *Simulation::visited, *Simulation::specialVisited;
SDL_Rect Simulation::rect;
int Simulation::n, Simulation::k, Simulation::currVertex, Simulation::startVertex, Simulation::endVertex, Simulation::moving, Simulation::dijkstraPending, Simulation::sourceVertex;
unordered_map<int, vector<int>> Simulation::adj;
unordered_map<int, int> Simulation::weights;
unordered_map<int, unordered_map<int, int>> Simulation::distances;
unordered_set<int> Simulation::specialVertices, Simulation::processed, Simulation::pendingSpecial;
priority_queue<pair<int, int>> Simulation::pq;
unordered_set<int>::iterator Simulation::it;

int Simulation::initTextures(SDL_Renderer *_renderer)
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

void Simulation::beginSimulation(int _n, int _k)
{
	Map::generateRandomMaze(n = _n);
	generateSpecialVertices(k = _k);
	assignWeights();
	buildGraph();
	moving = 0;

	rect = {0, 0, WINDOW_WIDTH / n, WINDOW_HEIGHT / n};
	SDL_Event e;

	reRender();
	while (true)
	{
		if (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				return;
			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
				return;
		}
		simulateNextStep();
	}
}

void Simulation::generateSpecialVertices(int k)
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
			weights[n * col + row] = Map::genRandom(MAX_COST, MIN_COST);
			break;
		}
	}
}

void Simulation::assignWeights()
{
	for (int col = 0; col < n; ++col)
		for (int row = 0; row < n; ++row)
			if (Map::map[col][row] == 0)
				weights[n * col + row] = Map::genRandom(MAX_WEIGHT, 1);
}

void Simulation::buildGraph()
{
	for (int col = 1; col < n - 1; ++col)
		for (int row = 1; row < n - 1; ++row)
			if (Map::map[col][row] != 1)
			{
				if (Map::map[col + 1][row] != 1)
				{
					adj[n * col + row].push_back(n * (col + 1) + row);
					adj[n * (col + 1) + row].push_back(n * col + row);
				}
				if (Map::map[col][row + 1] != 1)
				{
					adj[n * col + row].push_back(n * col + row + 1);
					adj[n * col + row + 1].push_back(n * col + row);
				}
			}
	startVertex = n + 1, currVertex = startVertex, endVertex = (n - 2) * (n + 1);
	dijkstraPending = k + !specialVertices.count(currVertex);
	initDijkstra();
}

bool Simulation::simulateNextStep()
{
	if (dijkstraPending)
		return nextDijkstraStep(), false;
	if (moving)
		return updatePos(), false;
	if (currVertex == endVertex)
		return true;
	SDL_Delay(20);
	int D = distances[currVertex][endVertex], best = endVertex, d = D;
	printf("%d %d\n", currVertex, weights[currVertex]);
	for (auto vertex : specialVertices)
	{
		int tempD = distances[currVertex][vertex];
		// @TODO: improve the heuristic
		if (tempD + distances[vertex][endVertex] - weights[vertex] < D && (tempD < d || tempD == d && tempD + distances[vertex][endVertex] - weights[vertex] < d + distances[best][endVertex] - weights[best]))
			best = vertex, d = tempD;
	}
	specialVertices.erase(best);
	moving = 0, currVertex = best;
}

void Simulation::nextDijkstraStep()
{
	SDL_Delay(20);
	if (pq.empty() || pendingSpecial.empty())
	{
		// printf("%d:\n", currVertex);
		// for (auto p : distances[currVertex])
		// 	printf("%d\t%d\n", p.first, p.second);
		// printf("\n");
		if (--dijkstraPending == 0)
			currVertex = startVertex;
		else
		{
			if (dijkstraPending == k)
				currVertex = *(it = specialVertices.begin());
			else
				currVertex = *++it;
			initDijkstra();
		}
		return reRender(), void();
	}
	int v = pq.top().second;
	pq.pop();
	if (processed.count(v))
		return;
	processed.insert(v);
	pendingSpecial.erase(v);
	rect.x = (v / n) * rect.w, rect.y = (v % n) * rect.h;
	SDL_RenderCopy(renderer, specialVertices.count(v) ? specialVisited : visited, NULL, &rect);
	Fonts::displayText(renderer, to_string(weights[v]).c_str(), rect.x + rect.w / 2, rect.y + rect.h / 2);
	SDL_RenderPresent(renderer);
	for (auto u : adj[v])
	{
		int w = specialVertices.count(u) ? 0 : weights[u];
		if (distances[currVertex][v] + w < distances[currVertex][u])
			distances[currVertex][u] = distances[currVertex][v] + w, pq.push({-distances[currVertex][u], u});
	}
}

void Simulation::initDijkstra()
{
	processed.clear(), pendingSpecial = specialVertices;
	pendingSpecial.insert(endVertex);
	while (!pq.empty())
		pq.pop();
	for (auto &p : weights)
		distances[currVertex][p.first] = INT_MAX;
	distances[currVertex][currVertex] = 0;
	pq.push({0, currVertex});
}

// @TODO: literally todo
void Simulation::updatePos()
{
}

void Simulation::reRender()
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
