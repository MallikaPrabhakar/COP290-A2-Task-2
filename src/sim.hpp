#ifndef SIM2_H
#define SIM2_H

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
#include "simulation.hpp"

using namespace std;

struct Sim
{
private:
	// renderer to display the simulation
	static SDL_Renderer *renderer;

	// textures needed to display different simulation components 
	static SDL_Texture *background, *player, *tile, *specialTile, *wall, *path, *visited, *specialVisited;

	// position and size of the texture to display 
	static SDL_Rect rect;

	static int n, k, currVertex, startVertex, endVertex, inProcess, moving, sourceVertex, minDist;

	// the prize or weight of the vertex, depending on whether it is a special vertex or not
	static unordered_map<int, int> weights;

	// store the distances computed via Floyd Warshall (negative weights allowed)
	static unordered_map<int, unordered_map<int, pair<int, int>>> distances;

	// specialVertices: keeps a track of the special vertices in the map
	static unordered_set<int> specialVertices;

	// keep track of the permutation which gives the best answer
	static vector<int> bestPermutation;

	static void generateSpecialVertices(int k);
	static void assignWeights();
	static void buildGraph();
	static void FW();
	static void traverseAllPossibilities();
	static void simulatePermutation(vector<int> &currSpecial);
	static void initDistance();
	static void updatePos();
	static void renderVisit(int v, int delay);
	static void reRender();

public:
	static int initTextures(SDL_Renderer *_renderer);
	static void beginSimulation(int _n, int _k);
};

#endif
