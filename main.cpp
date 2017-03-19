#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <SDL/SDL.h>
#include "chip8.h"
#include "interface.h"

#define RES 10
#define MILLIS 1000/60

using namespace std;

chip8 MyChip8;

SDL_Surface *ecran = NULL, *fond = NULL, *pixel = NULL;
SDL_Event event, key_event;
SDL_Rect position;
unsigned char gfx[64*32];


int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO); // Initialisation de la SDL
    ecran = SDL_SetVideoMode(64*RES, 32*RES, 32, SDL_HWSURFACE | SDL_DOUBLEBUF); // Ouverture de la fenêtre
    pixel = SDL_CreateRGBSurface(SDL_HWSURFACE, RES, RES, 32, 0, 0, 0, 0);
    SDL_FillRect(pixel, NULL, SDL_MapRGB(ecran->format, 255, 255, 255));
    //INITIALISE INPUT

    MyChip8.initialise();
    MyChip8.loadGame("../Chip8games/PONG");
    int continuer = 1;
    int tempsPrecedent = 0, tempsActuel = 0;


    while (continuer)
    {
        SDL_PollEvent(&event);
        switch(event.type)
        {
        case SDL_QUIT:
            continuer = 0;
            break;
        case SDL_KEYDOWN:
                switch(event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        continuer = 0;
                        break;
                }
                break;
        }
//
        MyChip8.emulateCycle();

        if(MyChip8.drawFlag)
        {
            SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
            for(int y = 0; y < 32; y++)
            {
                for(int x = 0; x < 64; x++)
                {
                    position.x = x*RES;
                    position.y = y*RES;


            if(MyChip8.gfx[x + y*64])
                SDL_BlitSurface(pixel, NULL, ecran, &position); // Collage
                }
            }
            SDL_Flip(ecran);
            MyChip8.drawFlag = false;
        }
        MyChip8.setKeys();

        tempsActuel = SDL_GetTicks();
        if (tempsActuel - tempsPrecedent > MILLIS)
        {
            tempsPrecedent = tempsActuel;
        }
        else /* Si ça fait moins de 30 ms depuis le dernier tour de boucle, on endort le programme le temps qu'il faut */
        {
            SDL_Delay(MILLIS - (tempsActuel - tempsPrecedent));
        }

    }
    SDL_FreeSurface(pixel);
    SDL_Quit(); // Arrêt de la SDL

    return EXIT_SUCCESS; // Fermeture du programme
}

