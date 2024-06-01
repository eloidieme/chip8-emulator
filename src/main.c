/* C Standard Library */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* Third-party Libraries */
#include <SDL3/SDL.h>

/* Header files */
#include "./macros.h"
#include "./stack.h"
#include "./chip8.h"
#include "./font.h"

Uint32 timerCallback(Uint32 interval, void* param) {
	Chip8* chip = (Chip8*)param;

	if (chip->delayT > 0) {
		chip->delayT--;
	}

	if (chip->soundT > 0) {
		chip->soundT--;
	}

	return interval;
}

int main(int argc, char* argv[argc+1]) {
	/* SDL Initialization */
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("SDL_Init Error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("CHIP-8 Emulator", SCREEN_W*20, SCREEN_H*20, 0);
	if (window == NULL) {
		printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) {
		SDL_DestroyWindow(window);
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA4444, SDL_TEXTUREACCESS_STREAMING, SCREEN_W, SCREEN_H);
	if (texture == NULL) {
		printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}
	
	/* Logging Initialization */
	FILE* output = fopen(LOG_FNAME, "w");
	fputs("PC,Instruction,Index,Stack,"
		"V0,V1,V2,V3,V4,V5,V6,V7,V8,V9,"
		"VA,VB,VC,VD,VE,VF,Delay,Sound\n", output);

	/* Timing Initialization */
	double clockRate = CLOCK_RATE_HZ;
	struct timespec start, end;
	uint64_t delta_us;
	int64_t remainingTime;
	uint64_t timeInterval = (1 / clockRate) * 1E6;

	/* Chip Initialization */
	Chip8 chip = initChip(0x200);
	loadFont(&chip, font);
	loadRom(&chip, argv[1]);
	uint16_t inst = 0x0;

	/* SDL Timer Callback */
	SDL_AddTimer(1000 / 60, timerCallback, &chip);

	/* Main Loop */
	SDL_bool done = SDL_FALSE;
	while (!done) {
		/* Event Loop */
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				done = SDL_TRUE;
			}
			if (event.type == SDL_EVENT_KEY_DOWN) {
				switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_1:
						chip.keys[0x0] = 1;
						break;
					case SDL_SCANCODE_2:
						chip.keys[0x1] = 1;
						break;
					case SDL_SCANCODE_3:
						chip.keys[0x2] = 1;
						break;
					case SDL_SCANCODE_4:
						chip.keys[0x3] = 1;
						break;
					case SDL_SCANCODE_A:
						chip.keys[0x4] = 1;
						break;
					case SDL_SCANCODE_Z:
						chip.keys[0x5] = 1;
						break;
					case SDL_SCANCODE_E:
						chip.keys[0x6] = 1;
						break;
					case SDL_SCANCODE_R:
						chip.keys[0x7] = 1;
						break;
					case SDL_SCANCODE_Q:
						chip.keys[0x8] = 1;
						break;
					case SDL_SCANCODE_S:
						chip.keys[0x9] = 1;
						break;
					case SDL_SCANCODE_D:
						chip.keys[0xA] = 1;
						break;
					case SDL_SCANCODE_F:
						chip.keys[0xB] = 1;
						break;
					case SDL_SCANCODE_W:
						chip.keys[0xC] = 1;
						break;
					case SDL_SCANCODE_X:
						chip.keys[0xD] = 1;
						break;
					case SDL_SCANCODE_C:
						chip.keys[0xE] = 1;
						break;
					case SDL_SCANCODE_V:
						chip.keys[0xF] = 1;
						break;
					default:
						break;
				}
			}
			if (event.type == SDL_EVENT_KEY_UP) {
				switch (event.key.keysym.scancode) {
					case SDL_SCANCODE_1:
						chip.keys[0x0] = 0;
						break;
					case SDL_SCANCODE_2:
						chip.keys[0x1] = 0;
						break;
					case SDL_SCANCODE_3:
						chip.keys[0x2] = 0;
						break;
					case SDL_SCANCODE_4:
						chip.keys[0x3] = 0;
						break;
					case SDL_SCANCODE_A:
						chip.keys[0x4] = 0;
						break;
					case SDL_SCANCODE_Z:
						chip.keys[0x5] = 0;
						break;
					case SDL_SCANCODE_E:
						chip.keys[0x6] = 0;
						break;
					case SDL_SCANCODE_R:
						chip.keys[0x7] = 0;
						break;
					case SDL_SCANCODE_Q:
						chip.keys[0x8] = 0;
						break;
					case SDL_SCANCODE_S:
						chip.keys[0x9] = 0;
						break;
					case SDL_SCANCODE_D:
						chip.keys[0xA] = 0;
						break;
					case SDL_SCANCODE_F:
						chip.keys[0xB] = 0;
						break;
					case SDL_SCANCODE_W:
						chip.keys[0xC] = 0;
						break;
					case SDL_SCANCODE_X:
						chip.keys[0xD] = 0;
						break;
					case SDL_SCANCODE_C:
						chip.keys[0xE] = 0;
						break;
					case SDL_SCANCODE_V:
						chip.keys[0xF] = 0;
						break;
					default:
						break;
				}
			}
		}

		fprintf(output, "%#.4x,", chip.pc);

		/* Fetch-Execute-Decode */
		inst = fetch(&chip);
		clock_gettime(CLOCK_MONOTONIC_RAW, &start);
		decodeExecute(inst, &chip, renderer, texture);
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);

		fprintf(output, "%#.4x,%#.4x,%#.4x,",
			inst,
			chip.index,
			chip.stack.content[chip.stack.top]
		);

		for (size_t k = 0; k < N_REGISTERS; ++k) {
			fprintf(output, "%#.4x,", chip.regs[k]);
		}

		fprintf(output, "%#.4x,%#.4x\n",
			chip.delayT,
			chip.soundT
		);

		/* Timing Handling */
		delta_us = (end.tv_sec - start.tv_sec) * 1E6 + (end.tv_nsec- start.tv_nsec) / 1E3;
		remainingTime = (timeInterval - delta_us);
		if (remainingTime > 0) {
			SDL_DelayNS(remainingTime * 1000);
		}

		/* Rendering */
		SDL_RenderPresent(renderer);
		SDL_RenderTexture(renderer, texture, NULL, NULL);
	}

	fclose(output);

	/* SDL Clean-up */
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return EXIT_SUCCESS;
}
