#include "simulation.hpp"

SDL_Renderer *Simulation::renderer;
SDL_Texture *Simulation::background, *Simulation::player, *Simulation::tile, *Simulation::specialTile, *Simulation::wall, *Simulation::path, *Simulation::visited, *Simulation::specialVisited;
SDL_Rect Simulation::rect;
int Simulation::n, Simulation::k, Simulation::currVertex, Simulation::startVertex, Simulation::endVertex, Simulation::moving, Simulation::Heuristic::dijkstraPending, Simulation::Brute::minDist;
unordered_map<int, int> Simulation::weights;
unordered_map<int, unordered_map<int, pair<int, int>>> Simulation::distances;
unordered_set<int> Simulation::specialVertices, Simulation::Heuristic::processed, Simulation::Heuristic::pendingSpecial;
priority_queue<pair<int, int>> Simulation::Heuristic::pq;
unordered_set<int>::iterator Simulation::it;
vector<int> Simulation::Brute::bestPermutation;

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

void Simulation::initSimulation(int _n, int _k)
{
	Map::generateRandomMaze(n = _n);
	generateSpecialVertices(k = _k);
	assignWeights();
	startVertex = n + 1, currVertex = startVertex, endVertex = (n - 2) * (n + 1);
	moving = 0;
	rect = {0, 0, WINDOW_WIDTH / n, WINDOW_HEIGHT / n};

	Heuristic::init();
	Brute::init();
}

void Simulation::Brute::init()
{
	Brute::initDistances();

	SDL_RenderClear(renderer);
	Fonts::displayText(renderer, string("Performing pre-computation...").c_str(), WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 3, {255, 255, 255});
	SDL_RenderPresent(renderer);

	FW();
	traverseAllPossibilities();
}

void Simulation::Brute::initDistances()
{
	distances.clear();
	for (int col = 1; col < n - 1; col++)
		for (int row = 1; row < n - 1; row++)
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

void Simulation::Brute::FW()
{
	for (auto &p : distances)
		for (auto &q : distances)
			for (auto &r : distances)
				if (q.second.count(p.first) && p.second.count(r.first))
					if (!q.second.count(r.first) || q.second[p.first].first + p.second[r.first].first < q.second[r.first].first)
						q.second[r.first] = {q.second[p.first].first + p.second[r.first].first, p.second[r.first].second};

	for (auto v : specialVertices)
	{
		distances[startVertex][v].first += weights[v];
		for (auto u : specialVertices)
			if (v != u)
				distances[v][u].first += weights[u];
	}
}

void Simulation::Brute::traverseAllPossibilities()
{
	for (int mask = 0; mask < (1 << k); ++mask)
	{
		vector<int> currSpecial;
		it = specialVertices.begin();
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

void Simulation::Brute::simulatePermutation(vector<int> &currSpecial)
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

void Simulation::Heuristic::init()
{
	Heuristic::dijkstraPending = k + 1;
	Heuristic::initDijkstra();

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
		Heuristic::simulateNextStep();
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
			weights[n * col + row] = Map::genRandom(-MIN_COST, -MAX_COST);
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

bool Simulation::Heuristic::simulateNextStep()
{
	if (dijkstraPending)
		return nextDijkstraStep(), false;
	if (moving)
		return updatePos(), false;
	if (currVertex == endVertex)
		return true;
	SDL_Delay(20);
	int D = distances[currVertex][endVertex].first, best = endVertex, d = D;
	printf("%d %d\n", currVertex, weights[currVertex]);
	for (auto vertex : pendingSpecial)
	{
		int tempD = distances[currVertex][vertex].first;
		// @TODO: improve the heuristic
		if (tempD + distances[vertex][endVertex].first + weights[vertex] < D && (tempD < d || tempD == d && tempD + distances[vertex][endVertex].first + weights[vertex] < d + distances[best][endVertex].first + weights[best]))
			best = vertex, d = tempD;
	}
	pendingSpecial.erase(best);
	moving = 0, currVertex = best;
}

void Simulation::Heuristic::nextDijkstraStep()
{
	if (pq.empty() || pendingSpecial.empty())
	{
		if (--dijkstraPending == 0)
			currVertex = startVertex, pendingSpecial = specialVertices;
		else
		{
			if (dijkstraPending == k)
				currVertex = *(it = specialVertices.begin());
			else
				currVertex = *++it;
			if (currVertex == startVertex)
				return;
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
	renderVisit(v, 20);

	auto updateDist = [&](int u)
	{
		int w = specialVertices.count(u) ? 0 : weights[u];
		if (distances[currVertex][v].first + w < distances[currVertex][u].first)
		{
			distances[currVertex][u] = {distances[currVertex][v].first + w, v};
			pq.push({-distances[currVertex][u].first, u});
		}
	};

	int col = v / n, row = v % n;
	if (Map::map[col + 1][row] != 1)
		updateDist(n * (col + 1) + row);
	if (Map::map[col][row + 1] != 1)
		updateDist(n * col + row + 1);
	if (Map::map[col - 1][row] != 1)
		updateDist(n * (col - 1) + row);
	if (Map::map[col][row - 1] != 1)
		updateDist(n * col + row - 1);
}

void Simulation::Heuristic::initDijkstra()
{
	processed.clear(), pendingSpecial = specialVertices;
	pendingSpecial.insert(endVertex);
	while (!pq.empty())
		pq.pop();
	for (auto &p : weights)
		distances[currVertex][p.first] = {INT_MAX, -1};
	distances[currVertex][currVertex] = {0, -1};
	pq.push({0, currVertex});
}

// @TODO: literally todo
void Simulation::updatePos()
{
}

void Simulation::renderVisit(int v, int delay)
{
	SDL_Delay(delay);
	rect.x = (v / n) * rect.w, rect.y = (v % n) * rect.h;
	SDL_RenderCopy(renderer, specialVertices.count(v) ? specialVisited : visited, NULL, &rect);
	Fonts::displayText(renderer, to_string(weights[v]).c_str(), rect.x + rect.w / 2, rect.y + rect.h / 2);
	SDL_RenderPresent(renderer);
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
