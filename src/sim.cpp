#include "sim.hpp"

SDL_Renderer *Sim::renderer;
SDL_Texture *Sim::background, *Sim::player, *Sim::tile, *Sim::specialTile, *Sim::wall, *Sim::path, *Sim::visited, *Sim::specialVisited;
SDL_Rect Sim::rect;
int Sim::n, Sim::k, Sim::currVertex, Sim::startVertex, Sim::endVertex, Sim::inProcess, Sim::moving, Sim::sourceVertex;
unordered_map<int, unordered_set<int>> Sim::adj;
unordered_map<int, int> Sim::weights;
unordered_map<int, unordered_map<int, int>> Sim::distances;
unordered_set<int> Sim::specialVertices;

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
	initDistance();					 //sets up the distance graph ([pos i: [pos j, weight]])
	startVertex = n + 1, currVertex = startVertex, endVertex = (n - 2) * (n + 1);
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
			weights[n * col + row] = Map::genRandom(MAX_COST, MIN_COST);
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

/*
//simulates the next step
bool Sim::simulateNextStep()
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

//performs the next step
void Sim::nextDijkstraStep()
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
*/
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
				{
					distances[n * col + row][n * (col - 1) + row] = weights[n * (col - 1) + row];
				}
				if (Map::map[col][row - 1] != 1)
				{
					distances[n * col + row][n * (col) + row - 1] = weights[n * (col)-1 + row];
				}
				if (Map::map[col + 1][row] != 1)
				{
					distances[n * col + row][n * (col + 1) + row] = weights[n * (col + 1) + row];
				}
				if (Map::map[col][row + 1] != 1)
				{
					distances[n * col + row][n * (col) + row + 1] = weights[n * (col) + 1 + row];
				}
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
				{
					if (q.second.count(r.first))
					{
						q.second[r.first] = min(q.second[r.first], q.second[p.first] + p.second[r.first]);
					}
					else
					{
						q.second[r.first] = q.second[p.first] + p.second[r.first];
					}
				}
			}
		}
	}
}

//
void Sim::nextFWStep()
{
}

bool Sim::simulateNextStep()
{
	if (inProcess)
		return nextFWStep(), false;
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

// @TODO: literally todo
void Sim::updatePos()
{
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
