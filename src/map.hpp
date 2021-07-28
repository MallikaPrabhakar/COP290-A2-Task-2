#ifndef MAP_H
#define MAP_H

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <vector>
#include <array>
#include <stack>
#include <random>
#include <assert.h>

#define REMOVE_PROB 0.20

using namespace std;

struct Map
{
	static vector<vector<int>> map;
	static int n;

	static boost::random::mt19937 gen;

	static void generateRandomMaze(int _n);
	static vector<vector<array<bool, 4>>> generateRandomWalls();
	static void removeWalls(vector<vector<array<bool, 4>>> &walls);
	static int genRandom(int M, int m = 0);
};


#endif
