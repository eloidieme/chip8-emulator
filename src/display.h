#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <SDL3/SDL.h>

void setPixel(SDL_Renderer* renderer, SDL_Texture* texture, uint8_t x, uint8_t y, uint8_t color);

#endif