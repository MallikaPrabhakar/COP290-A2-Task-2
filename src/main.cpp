#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "font.hpp"
#include "map.hpp"
#include "simulation.hpp"
#include "sim.hpp"

#define ICON "../assets/images/icon.tif"

void handleExit(SDL_Renderer *renderer, SDL_Window *window)
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	TTF_Quit();
	Mix_Quit();
	SDL_Quit();
	exit(0);
}

int main(int argc, char *argv[])
{
	// init window
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	SDL_Window *window = SDL_CreateWindow("Hunger Games", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);
	if (window == NULL)
	{
		printf("Could not create window, error encountered: %s\n", SDL_GetError());
		return 1;
	}
	SDL_Surface *iconSurface = IMG_Load(ICON);
	SDL_SetWindowIcon(window, iconSurface);

	// init renderer
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL)
	{
		SDL_DestroyWindow(window);
		SDL_Quit();
		printf("Could not create renderer, error encountered: %s\n", SDL_GetError());
		return 1;
	}
	SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

	// init fonts
	int fonts = Fonts::initFonts();
	if (fonts != 0)
	{
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		printf("Could not load font %d, error encountered: %s\n", fonts, SDL_GetError());
		return 1;
	}

	if (Simulation::initTextures(renderer) != 0 || Sim::initTextures(renderer) != 0)
	{
		printf("Could not initialise textures\n");
		handleExit(renderer, window);
	}

	Map::gen.seed(time(NULL));

	// Simulation::beginSimulation(3, 0);
	Sim::beginSimulation(15, 4);
}
