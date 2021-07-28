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
	struct Heuristic
	{
		static int dijkstraPending;
		// processed: stores the vertices that have been processed by Dijkstra
		// pendingSpecial: keeps a track of the remaining specialVertices which are yet to be processed by Dijkstra
		static unordered_set<int> processed, pendingSpecial;
		// the priority queue to perform Dijkstra
		static priority_queue<pair<int, int>> pq;

		static bool init();
		static void nextDijkstraStep();
		static void initDijkstra();
	};

	struct Brute
	{

		static bool init();
		static bool initDistances();
		static bool FW();
		static bool traverseAllPossibilities();
		static bool simulatePermutation(vector<int> &currSpecial);
	};

	// renderer to display the simulation
	static SDL_Renderer *renderer;
	// textures needed to display different simulation components
	static SDL_Texture *background, *player, *tile, *specialTile, *wall, *path, *visited, *specialVisited;
	// position and size of the texture to display
	static SDL_Rect rect;
	// the adjacency matrix
	static unordered_map<int, vector<int>> adj;
	static int n, k, currVertex, startVertex, endVertex, minDist;
	// the prize or weight of the vertex, depending on whether it is a special vertex or not
	static unordered_map<int, int> weights;
	// store the distances computed
	static unordered_map<int, unordered_map<int, pair<int, int>>> distances;
	// specialVertices: keeps a track of the special vertices in the map
	static unordered_set<int> specialVertices;
	// utility iterator
	static unordered_set<int>::iterator it;
	// SDL_Event
	static SDL_Event e;
	static vector<int> bestPath;

	static void generateSpecialVertices(int k);
	static void assignWeights();
	static bool backtrack();
	static void renderVisit(int v, int delay);
	static void reRender();
	static bool checkEscape();

public:
	static int initTextures(SDL_Renderer *_renderer);
	static void initSimulation(int _n, int _k);
};

#endif
