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
	// renderer to display the simulation
	static SDL_Renderer *renderer;
	// textures needed to display different simulation components
	static SDL_Texture *background, *player, *tile, *specialTile, *wall, *path, *visited, *specialVisited;
	// position and size of the texture to display
	static SDL_Rect rect;
	// the adjacency matrix
	static unordered_map<int, vector<int>> adj;
	static int n, k, currVertex, startVertex, endVertex, moving, dijkstraPending, sourceVertex;
	// the prize or weight of the vertex, depending on whether it is a special vertex or not
	static unordered_map<int, int> weights;
	// store the distances computed via Dijkstra (assumes weight of special vertices to be 0)
	static unordered_map<int, unordered_map<int, int>> distances;
	// specialVertices: keeps a track of the special vertices in the map
	// processed: stores the vertices that have been processed by Dijkstra
	// pendingSpecial: keeps a track of the remaining specialVertices which are yet to be processed by Dijkstra
	static unordered_set<int> specialVertices, processed, pendingSpecial;
	// the priority queue to perform Dijkstra
	static priority_queue<pair<int, int>> pq;
	// iterator to store the next special vertex to process
	static unordered_set<int>::iterator it;

	static void generateSpecialVertices(int k);
	static void assignWeights();
	static bool simulateNextStep();
	static void nextDijkstraStep();
	static void initDijkstra();
	static void updatePos();
	static void reRender();

public:
	static int initTextures(SDL_Renderer *_renderer);
	static void beginSimulation(int _n, int _k);
};

#endif
