#include "font.hpp"

TTF_Font *Fonts::fonts[FONTSCOUNT];

int Fonts::initFonts()
{
	TTF_Init();
	if ((fonts[0] = TTF_OpenFont(FONT_MEDIUM, 15)) == NULL)
		return 1;
	if ((fonts[1] = TTF_OpenFont(FONT_MEDIUM, 18)) == NULL)
		return 2;
	if ((fonts[2] = TTF_OpenFont(FONT_MEDIUM, 21)) == NULL)
		return 3;
	if ((fonts[3] = TTF_OpenFont(FONT_MEDIUM, 24)) == NULL)
		return 4;
	if ((fonts[4] = TTF_OpenFont(FONT_MEDIUM, 27)) == NULL)
		return 5;
	if ((fonts[5] = TTF_OpenFont(FONT_MEDIUM, 30)) == NULL)
		return 6;
	return 0;
}

void Fonts::displayText(SDL_Renderer *renderer, const char *text, int x, int y, int fontNum, SDL_Color color)
{
	SDL_Surface *surface = TTF_RenderText_Solid(fonts[fontNum], text, color);
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	SDL_Rect pos;
	pos.w = surface->w;
	pos.h = surface->h;
	pos.x = x - pos.w / 2;
	pos.y = y - pos.h / 2;
	SDL_RenderCopy(renderer, texture, NULL, &pos);
}
