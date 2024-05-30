#include "./display.h"

void setPixel(SDL_Renderer* renderer, SDL_Texture* texture, uint8_t x, uint8_t y, uint8_t color) {
    void* pixels;
    int pitch;

    if (!SDL_LockTexture(texture, NULL, &pixels, &pitch)) {
        printf("SDL_LockTexture Error: %s\n", SDL_GetError());
        return;
    }

    uint16_t* pixelPtr = (uint16_t*)pixels;
    uint16_t pixelPosition = (y * (pitch / 4)) + x;
    pixelPtr[pixelPosition] = color;
    SDL_UnlockTexture(texture);
}
