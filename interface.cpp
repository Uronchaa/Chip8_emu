#include <SDL/SDL.h>

void setupGraphics(SDL_Surface *ecran, int multi)
{
    SDL_Init(SDL_INIT_VIDEO);
    ecran = SDL_SetVideoMode(64*multi, 32*multi, 32, SDL_HWSURFACE);
    SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 17, 206, 112));
    SDL_WM_SetCaption("Chip-8 Interpreter", NULL);
}
