#ifndef SDLP_H
#define SDLP_H

//sdlp.h
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

//Globals
extern SDL_Renderer* gRenderer;
extern SDL_Window* gWindow;
extern SDL_Texture* pieceTextures[13];
extern SDL_Texture* squareTextures[3];

//Functions
bool initSDL();
bool loadMedia();
void closeSDL();
SDL_Texture* loadTexture( SDL_Surface* );
void createTextures();

#endif
