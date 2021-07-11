#ifndef SIM_H
#define SIM_H

#include <SDL.h>
#include <SDL_image.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <queue>
#include <algorithm>
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

struct Sim
{
private:
	// renderer to display the simulation
	static SDL_Renderer *renderer;

	// textures needed to display different simulation components 
	static SDL_Texture *background, *player, *tile, *specialTile, *wall, *path, *visited, *specialVisited;

	// position and size of the texture to display 
	static SDL_Rect rect;

	// the adjacency matrix syntax: 
	/*
	umap.insert(make_pair("e", 2.718));
	umap["PI"] = 3.14;
	if (umap.find(key) == umap.end())
        cout << key << " not found\n\n";
	*/
	static unordered_map<int, unordered_set<int>> adj;

	static int n, k, currVertex, startVertex, endVertex, inProcess, moving, sourceVertex;

	// the prize or weight of the vertex, depending on whether it is a special vertex or not
	static unordered_map<int, int> weights;

	// store the distances computed via Floyd Warshall (negative weights allowed)
	static unordered_map<int, unordered_map<int, int>> distances;

	// specialVertices: keeps a track of the special vertices in the map
	static unordered_set<int> specialVertices;


	static void generateSpecialVertices(int k);
	static void assignWeights();
	static void buildGraph();
	static bool simulateNextStep();
	static void FW();
	static void nextFWStep();
	static void initDistance();
	static void updatePos();
	static void reRender();

public:
	static int initTextures(SDL_Renderer *_renderer);
	static void beginSimulation(int _n, int _k);
};

#endif
