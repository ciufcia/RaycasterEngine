#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_BUFFER_SIZE 307200
const int WINDOW_FLAGS = 0;
const int RENDERER_FLAGS = SDL_RENDERER_ACCELERATED;

typedef struct {
    SDL_Window *pWindow;
    SDL_Renderer *pRenderer;
} App;

typedef struct {
    bool quit;
} Events;

typedef struct {
    unsigned long lastTick;
    unsigned long deltaTime;
} Clock;

Clock clock = {0, 0};

void tickClock() {
    unsigned long timeBetweenTicks = SDL_GetTicks64();
    clock.deltaTime = timeBetweenTicks - clock.lastTick;
    clock.lastTick = timeBetweenTicks;
}

#define CREATE_CLEARED_EVENTS(name) Events name = {false}; 

typedef struct { int x; int y; } Vec2i;
typedef struct { unsigned int x; unsigned int y; } Vec2u;
typedef struct { float x; float y; } Vec2f;

typedef struct { Vec2f playerPos; } GameLogicData;

const unsigned short WORLD_WIDTH = 7;
const unsigned short WORLD_HEIGHT = 7;
const unsigned char WORLD[] = {
    1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 1,
    1, 0, 1, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1
};

const unsigned short WORLD_VIEW_BLOCK_SIZE = WINDOW_HEIGHT / WORLD_HEIGHT * 0.3;
const unsigned short WORLD_VIEW_PLAYER_SIZE = WORLD_VIEW_BLOCK_SIZE * 0.3;
const unsigned short WORLD_VIEW_HALF_PLAYER_SIZE = WORLD_VIEW_PLAYER_SIZE / 2.f;

void initApp(App *pApp) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		exit(1);
    }

    pApp->pWindow = SDL_CreateWindow("RaycasterEngine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_FLAGS);

    if (!pApp->pWindow) {
        printf("Failed to open %d x %d window: %s\n", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_GetError());
		exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    pApp->pRenderer = SDL_CreateRenderer(pApp->pWindow, -1, RENDERER_FLAGS);

    if (!pApp->pRenderer) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
		exit(1);
	}
}

void handleSDLEvents(Events *events) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                events->quit = true;
                return;
            default:
                break;
        }
    }
}

void renderWorldView(App *pApp, GameLogicData *pGameLogicData) {
    SDL_Rect rect;
    
    for (short x = 0; x < WORLD_WIDTH; x++) {
        for (short y = 0; y < WORLD_HEIGHT; y++) {
            if (WORLD[x + y * WORLD_WIDTH] == 0) {
                continue;
            }

            rect.x = x * WORLD_VIEW_BLOCK_SIZE;
            rect.y = y * WORLD_VIEW_BLOCK_SIZE;
            rect.w = WORLD_VIEW_BLOCK_SIZE;
            rect.h = WORLD_VIEW_BLOCK_SIZE;

            SDL_SetRenderDrawColor(pApp->pRenderer, 255, 255, 255, 255);
            SDL_RenderFillRect(pApp->pRenderer, &rect);
        }
    }

    SDL_Rect playerRect = {
        pGameLogicData->playerPos.x - WORLD_VIEW_HALF_PLAYER_SIZE,
        pGameLogicData->playerPos.y - WORLD_VIEW_HALF_PLAYER_SIZE,
        WORLD_VIEW_PLAYER_SIZE,
        WORLD_VIEW_PLAYER_SIZE
    };

    SDL_SetRenderDrawColor(pApp->pRenderer, 0, 255, 255, 255);
    SDL_RenderFillRect(pApp->pRenderer, &playerRect);
}

void updatePlayer(GameLogicData *pGameLogicData) {
    const Uint8 *keyStates = SDL_GetKeyboardState(NULL);
    if (keyStates[SDL_SCANCODE_W]) {
        pGameLogicData->playerPos.y -= 0.05f * clock.deltaTime;
    }
    if (keyStates[SDL_SCANCODE_S]) {
        pGameLogicData->playerPos.y += 0.05f * clock.deltaTime;
    }
    if (keyStates[SDL_SCANCODE_A] == 1) {
        pGameLogicData->playerPos.x -= 0.05f * clock.deltaTime;
    }
    if (keyStates[SDL_SCANCODE_D]) {
        pGameLogicData->playerPos.x += 0.05f * clock.deltaTime;
    }
}

void runGameLogic(App *pApp, GameLogicData *pGameLogicData) {
    updatePlayer(pGameLogicData);
}

void runMainLoop(App *pApp) {
    bool bRunning = true;

    SDL_Texture *pWindowTexture = SDL_CreateTexture(
        pApp->pRenderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    GameLogicData gameLogicData = {
        {
            (float)(WORLD_WIDTH * WORLD_VIEW_BLOCK_SIZE) / 2.f,
            (float)(WORLD_HEIGHT * WORLD_VIEW_BLOCK_SIZE) / 2.f
        }
    };

    unsigned int pixels[WINDOW_BUFFER_SIZE] = {0};

    while (bRunning) {
        CREATE_CLEARED_EVENTS(events);

        handleSDLEvents(&events);

        if (events.quit) {
            bRunning = false;
            break;
        }

        runGameLogic(pApp, &gameLogicData);

        SDL_RenderClear(pApp->pRenderer);

        SDL_UpdateTexture(pWindowTexture, NULL, pixels, WINDOW_WIDTH * sizeof(unsigned int));
        SDL_RenderCopy(pApp->pRenderer, pWindowTexture, NULL, NULL);
        renderWorldView(pApp, &gameLogicData);
        SDL_RenderPresent(pApp->pRenderer);

        tickClock();
    }
}

int main(int argc, char *argv[]) {
    App app;

    initApp(&app);

    runMainLoop(&app);

    return 0;
}