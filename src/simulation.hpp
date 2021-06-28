#ifndef SIM_H
#define SIM_H

#include <SDL.h>
#include <SDL_image.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <boost/container_hash/hash.hpp>
#include "map.hpp"
#include "font.hpp"

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 900
#define MAX_COST 30
#define MIN_COST 10
#define MAX_WEIGHT 5

using namespace std;

template <typename A, typename B>
struct hash<typename std::pair<A, B>>
{
	size_t operator()(const typename std::pair<A, B> &p) const { return boost::hash_value(p); }
};

struct Simulation
{
private:
	static SDL_Renderer *renderer;
	static SDL_Texture *background, *player, *tile, *specialTile, *wall, *path, *visited, *specialVisited;
	static SDL_Rect rect;
	static unordered_map<int, vector<pair<int, int>>> adj;
	static int n, k, currVertex, startVertex, endVertex, moving, dijkstraPending, sourceVertex;
	static unordered_map<int, int> weights;
	static unordered_map<int, unordered_map<int, int>> distances;
	static unordered_set<int> specialVertices, processed, pendingSpecial;
	static priority_queue<pair<int, int>> pq;
	static unordered_set<int>::iterator it;

	static void generateSpecialVertices(int k);
	static void assignWeights();
	static void buildGraph();
	static bool simulateNextStep();
	static void nextDijkstra();
	static void initDijkstra();
	static void updatePos();
	static void reRender();

public:
	static int initTextures(SDL_Renderer *_renderer);
	static void beginSimulation(int _n, int _k);
};

#endif
