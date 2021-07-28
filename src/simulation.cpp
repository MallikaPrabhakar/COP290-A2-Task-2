#include "simulation.hpp"

SDL_Renderer *Simulation::renderer;
SDL_Texture *Simulation::background, *Simulation::player, *Simulation::tile, *Simulation::specialTile, *Simulation::wall, *Simulation::path, *Simulation::visited, *Simulation::specialVisited;
SDL_Rect Simulation::rect;
int Simulation::n, Simulation::k, Simulation::currVertex, Simulation::startVertex, Simulation::endVertex, Simulation::Heuristic::dijkstraPending, Simulation::minDist, Simulation::test;
unordered_map<int, int> Simulation::weights;
unordered_map<int, unordered_map<int, pair<int, int>>> Simulation::distances;
unordered_set<int> Simulation::specialVertices, Simulation::Heuristic::processed, Simulation::Heuristic::pendingSpecial;
priority_queue<pair<int, int>> Simulation::Heuristic::pq;
unordered_set<int>::iterator Simulation::it;
vector<int> Simulation::bestPath;
SDL_Event Simulation::e;

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

void Simulation::initSimulation(int _n, int _k, int _test)
{
	Map::generateRandomMaze(n = _n);
	generateSpecialVertices(k = _k);
	test = _test;
	assignWeights();
	startVertex = n + 1, currVertex = startVertex, endVertex = (n - 2) * (n + 1);
	rect = {0, 0, WINDOW_WIDTH / n, WINDOW_HEIGHT / n};
	reRender(1);

	if (!Heuristic::init())
		return;
	test = _test;
	Brute::init();
}

bool Simulation::Brute::init()
{
	bestPath.clear();

	if (!test)
	{
		SDL_RenderClear(renderer);
		Fonts::displayText(renderer, string("Performing pre-computation...").c_str(), WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 3, {255, 255, 255});
		SDL_RenderPresent(renderer);
	}

	printf("Running brute\n");
	auto t = clock();
	if (initDistances() && FW() && traverseAllPossibilities())
	{
		printf("Time taken: %.6f\n", (clock() - t) * 1.0 / CLOCKS_PER_SEC);
		return backtrack();
	}
	return false;
}

bool Simulation::Brute::initDistances()
{
	distances.clear();
	for (int col = 1; col < n - 1; col++)
		for (int row = 1; row < n - 1; row++)
			if (Map::map[col][row] != 1)
			{
				if (checkEscape())
					return false;
				if (Map::map[col - 1][row] != 1)
					distances[n * col + row][n * (col - 1) + row] = {max(0, weights[n * (col - 1) + row]), n * col + row};
				if (Map::map[col][row - 1] != 1)
					distances[n * col + row][n * (col) + row - 1] = {max(0, weights[n * (col)-1 + row]), n * col + row};
				if (Map::map[col + 1][row] != 1)
					distances[n * col + row][n * (col + 1) + row] = {max(0, weights[n * (col + 1) + row]), n * col + row};
				if (Map::map[col][row + 1] != 1)
					distances[n * col + row][n * (col) + row + 1] = {max(0, weights[n * (col) + 1 + row]), n * col + row};
			}
	return true;
}

bool Simulation::Brute::FW()
{
	for (auto &p : distances)
		for (auto &q : distances)
			for (auto &r : distances)
			{
				if (checkEscape())
					return false;
				if (q.second.count(p.first) && p.second.count(r.first))
					if (!q.second.count(r.first) || q.second[p.first].first + p.second[r.first].first < q.second[r.first].first)
						q.second[r.first] = {q.second[p.first].first + p.second[r.first].first, p.second[r.first].second};
			}

	for (auto v : specialVertices)
	{
		distances[startVertex][v].first += weights[v];
		for (auto u : specialVertices)
		{
			if (checkEscape())
				return false;
			if (v != u)
				distances[v][u].first += weights[u];
		}
	}

	return true;
}

bool Simulation::Brute::traverseAllPossibilities()
{
	minDist = INT_MAX;
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
			if (!simulatePermutation(currSpecial))
				return false;
		} while (next_permutation(currSpecial.begin(), currSpecial.end()));
	}
	return true;
}

bool Simulation::Brute::simulatePermutation(vector<int> &currSpecial)
{
	reRender();
	int currDist = 0, prevVertex = startVertex;
	renderVisit(startVertex, 100);
	for (auto &v : currSpecial)
	{
		currDist += distances[prevVertex][v].first, prevVertex = v;
		renderVisit(v, 100);
		if (checkEscape())
			return false;
	}
	currDist += distances[prevVertex][endVertex].first;
	renderVisit(endVertex, 200);
	if (currDist < minDist)
		minDist = currDist, bestPath = currSpecial, bestPath.push_back(endVertex);
	return true;
}

bool Simulation::Heuristic::init()
{
	printf("Running heuristic\n");
	auto t = clock();
	Heuristic::dijkstraPending = k + 1;
	bestPath.clear();
	Heuristic::initDijkstra();

	reRender();
	while (true)
	{
		if (checkEscape())
			return false;
		if (dijkstraPending)
		{
			nextDijkstraStep();
			continue;
		}
		if (currVertex == endVertex)
			break;
		int D = distances[currVertex][endVertex].first, best = endVertex, d = D;
		for (auto vertex : pendingSpecial)
		{
			int tempD = distances[currVertex][vertex].first;
			if (tempD + distances[vertex][endVertex].first + weights[vertex] < D && (tempD < d || tempD == d && tempD + distances[vertex][endVertex].first + weights[vertex] < d + distances[best][endVertex].first + weights[best]))
				best = vertex, d = tempD;
		}
		minDist += distances[currVertex][best].first + (pendingSpecial.count(best) ? weights[best] : 0);
		pendingSpecial.erase(best);
		currVertex = best;
		bestPath.push_back(best);
	}
	printf("Time taken: %.6f\n", (clock() - t) * 1.0 / CLOCKS_PER_SEC);
	return backtrack();
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

bool Simulation::backtrack()
{
	test = 0;
	printf("Cost: %d\n", minDist);
	reRender();
	currVertex = startVertex;
	for (auto &vertex : bestPath)
	{
		int v = vertex;
		while (v != currVertex)
		{
			renderVisit(v, 40);
			v = distances[currVertex][v].second;
			if (checkEscape())
				return false;
		}
		renderVisit(v, 100);
		currVertex = vertex;
	}
	while (true)
	{
		if (checkEscape())
			return false;
		if (e.type == SDL_KEYDOWN && (e.key.keysym.sym == SDLK_KP_ENTER || e.key.keysym.sym == SDLK_RETURN))
				return true;
	}
}

void Simulation::renderVisit(int v, int delay)
{
	if (test)
		return;
	SDL_Delay(delay);
	rect.x = (v / n) * rect.w, rect.y = (v % n) * rect.h;
	SDL_RenderCopy(renderer, specialVertices.count(v) ? specialVisited : visited, NULL, &rect);
	Fonts::displayText(renderer, to_string(weights[v]).c_str(), rect.x + rect.w / 2, rect.y + rect.h / 2);
	SDL_RenderPresent(renderer);
}

void Simulation::reRender(bool first)
{
	if (test && !first)
		return;
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

bool Simulation::checkEscape()
{
	if (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
			return true;
		if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
			return true;
	}
	return false;
}
