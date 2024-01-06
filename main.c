#include <float.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <SDL2/SDL.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef float    f32;
typedef double   f64;

//////////////////////////////////

#define WINDOW_WIDTH       320u
#define WINDOW_HEIGHT      200u
#define WINDOW_BUFFER_SIZE 64000u
#define WINDOW_FLAGS       0u
#define RENDERER_FLAGS     SDL_RENDERER_ACCELERATED

//////////////////////////////////

#define RGBA_BLACK 0x000000FF
#define RGBA_WHITE 0xFFFFFFFF
#define RGBA_GRAY  0x808080FF

//////////////////////////////////

typedef struct {
    SDL_Window *p_sdlWindow;
    SDL_Renderer *p_renderer;

    SDL_Texture *p_windowTexture;
    u32 windowPixelBuffer[WINDOW_BUFFER_SIZE];
} Window;

typedef struct GameInstance_ GameInstance;

u8 init_window(Window *p_window);

void render(Window *p_window, GameInstance *p_gameInstance);
void render_walls(Window *p_window, GameInstance *p_gameInstance);
void render_top_down_view(Window *p_window, GameInstance *p_gameInstance);

//////////////////////////////////

typedef struct{
    u64 lastTick;
    u64 deltaTime;
} Clock;

Clock g_clock = { 0u, 0u };

void update_clock();

//////////////////////////////////

typedef struct { f32 x; f32 y; } Vec2f32;
void rotate(Vec2f32 *vec, f32 angle);

typedef struct { u32 x; u32 y; } Vec2u32;

//////////////////////////////////

#define WORLD_WIDTH 15u
#define WORLD_HEIGHT 15u
#define WORLD_SIZE 225u

const u8 g_WORLD[WORLD_SIZE] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,1,0,1,0,1,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,1,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,
    1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,
    1,0,1,0,0,0,0,0,0,0,0,1,1,0,1,
    1,0,1,1,1,1,0,0,0,0,0,0,0,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
};

//////////////////////////////////

typedef struct {
    Vec2f32 position;
    f32 movementSpeed;
    Vec2f32 direction;
    f32 rotationSpeed;
    Vec2f32 cameraPlane;
} Player;

//////////////////////////////////

struct GameInstance_ {
    Player player;
};

//////////////////////////////////

int main(int argc, char *argv[]);

//////////////////////////////////

void setup(Window *p_window, GameInstance *p_gameInstance);

//////////////////////////////////

void game_logic(Window *p_window, GameInstance *p_gameInstance);

//////////////////////////////////

void player_logic(Window *p_window, GameInstance *p_gameInstance);

//////////////////////////////////

void main_loop(Window *p_window, GameInstance *p_gameInstance);

//////////////////////////////////

void handleSDLEvents(Window *p_window);

//////////////////////////////////

u8 init_window(Window *p_window) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 0u;
    }

    p_window->p_sdlWindow = SDL_CreateWindow(
        "RaycasterEngine v.0.1",
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        WINDOW_FLAGS
    );

    if (!p_window->p_sdlWindow) {
        return 0u;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

    p_window->p_renderer = SDL_CreateRenderer(p_window->p_sdlWindow, -1, RENDERER_FLAGS);

    if (!p_window->p_sdlWindow) {
        return 0u;
    }
    
    p_window->p_windowTexture = SDL_CreateTexture(
        p_window->p_renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        WINDOW_WIDTH,
        WINDOW_HEIGHT
    );

    for (u32 _ = 0u; _ < WINDOW_BUFFER_SIZE; _++) {
        p_window->windowPixelBuffer[_] = RGBA_WHITE;
    }

    return 1u;
}

//////////////////////////////////

void update_clock() {
    u64 timePassed = SDL_GetTicks64();
    g_clock.deltaTime = timePassed - g_clock.lastTick;
    g_clock.lastTick = timePassed;
}

//////////////////////////////////

void rotate(Vec2f32 *p_vec, f32 angle) {
   Vec2f32 oldVec = { p_vec->x, p_vec->y };
   p_vec->x = oldVec.x * cos(angle) - oldVec.y * sin(angle);
   p_vec->y = oldVec.x * sin(angle) + oldVec.y * cos(angle);
}

//////////////////////////////////

void handleSDLEvents(Window *p_window) {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                exit(0);
                break;
            default:
                break;
        }
    }
}

//////////////////////////////////

void render(Window *p_window, GameInstance *p_gameInstance) {
    SDL_RenderClear(p_window->p_renderer);

    render_walls(p_window, p_gameInstance);

    SDL_UpdateTexture(
        p_window->p_windowTexture,
        NULL,
        p_window->windowPixelBuffer,
        WINDOW_WIDTH * sizeof(u32)
    );

    SDL_RenderCopy(p_window->p_renderer, p_window->p_windowTexture, NULL, NULL);
    
    render_top_down_view(p_window, p_gameInstance);

    SDL_RenderPresent(p_window->p_renderer);

    for (u32 _ = 0u; _ < WINDOW_BUFFER_SIZE; _++) {
        p_window->windowPixelBuffer[_] = RGBA_WHITE;
    }
}

//////////////////////////////////

void render_walls(Window *p_window, GameInstance *p_gameInstance) {
    const Vec2f32 playerPosition = p_gameInstance->player.position;
    const Vec2f32 playerDirection = p_gameInstance->player.direction; 
    const Vec2f32 cameraPlane = p_gameInstance->player.cameraPlane;

    for (u16 windowRow = 0u; windowRow < WINDOW_WIDTH; windowRow++) {
        // this is in range -1 to 1, where 0 is right in front of the player
        f32 rayPositionOnCameraPlane = 2.f * ((float)(windowRow)/(float)(WINDOW_WIDTH)) - 1.f;  
        Vec2f32 currentRayDirection = {
            playerDirection.x + cameraPlane.x * rayPositionOnCameraPlane,
            playerDirection.y + cameraPlane.y * rayPositionOnCameraPlane
        };

        Vec2f32 deltaDistance;
        (currentRayDirection.x == 0.f) ? (deltaDistance.x = FLT_MAX) : (deltaDistance.x = fabs(1 / currentRayDirection.x));
        (currentRayDirection.y == 0.f) ? (deltaDistance.y = FLT_MAX) : (deltaDistance.y = fabs(1 / currentRayDirection.y));

        Vec2u32 worldCoordinates = { (u32)(playerPosition.x), (u32)(playerPosition.y) };

        Vec2f32 travelledDistance;
        i8 stepX; i8 stepY;

        if (currentRayDirection.x > 0.f) {
            travelledDistance.x = ((float)(worldCoordinates.x + 1u) - playerPosition.x) * deltaDistance.x;
            stepX = 1;
        } else {
            travelledDistance.x = (playerPosition.x - (float)(worldCoordinates.x)) * deltaDistance.x;
            stepX = -1;
        }

        if (currentRayDirection.y > 0.f) {
            travelledDistance.y = ((float)(worldCoordinates.y + 1u) - playerPosition.y) * deltaDistance.y;
            stepY = 1;
        } else {
            travelledDistance.y = (playerPosition.y - (float)(worldCoordinates.y)) * deltaDistance.y;
            stepY = -1;
        }

        u8 face = 0u; // 0: vertical wall face
                      // 1: horizontal wall face

        u8 hit = 0u;
        while (hit == 0u) {
            if (travelledDistance.x < travelledDistance.y) {
                travelledDistance.x += deltaDistance.x;
                worldCoordinates.x += stepX;
                face = 0u;
            } else {
                travelledDistance.y += deltaDistance.y;
                worldCoordinates.y += stepY;
                face = 1u;
            }

            if (g_WORLD[worldCoordinates.x + worldCoordinates.y * WORLD_WIDTH]) {
                hit = 1u;
            }
        }

        f32 perpendicularDistanceToWall;

        (face == 0u) ? (perpendicularDistanceToWall = travelledDistance.x - deltaDistance.x)
                     : (perpendicularDistanceToWall = travelledDistance.y - deltaDistance.y);

        u32 wallLineHeight = (float)WINDOW_HEIGHT / perpendicularDistanceToWall;

        i32 lineTop = WINDOW_HEIGHT / 2u - wallLineHeight / 2u;
        if (lineTop < 0) { lineTop = 0; }
        i32 lineBottom = WINDOW_HEIGHT / 2u + wallLineHeight / 2u;
        if (lineBottom >= WINDOW_HEIGHT) { lineBottom = WINDOW_HEIGHT - 1; }

        u32 color;
        if (face == 0) {
            color = RGBA_BLACK;
        } else {
            color = RGBA_GRAY;
        }

        for (u32 y = lineTop; y <= lineBottom; y++) {
            p_window->windowPixelBuffer[windowRow + y * WINDOW_WIDTH] = color;
        }
    }   
}

//////////////////////////////////

void render_top_down_view(Window *p_window, GameInstance *p_gameInstance) {
    f32 wallBlockSize = ((float)WINDOW_HEIGHT / (float)WORLD_HEIGHT) * 0.5f;
    u32 roundedBlockSize = round(wallBlockSize);

   Vec2f32 playerPosition; 
   playerPosition.x = p_gameInstance->player.position.x * wallBlockSize;
   playerPosition.y = p_gameInstance->player.position.y * wallBlockSize;

   Vec2f32 playerDirection = p_gameInstance->player.direction;

   for (u32 y = 0u; y < WORLD_HEIGHT; y++) {
       for (u32 x = 0u; x < WORLD_WIDTH; x++) {
           if (g_WORLD[x + y * WORLD_WIDTH] == 0u) {
               continue;
           }

           SDL_Rect wallBlock = {
               x * roundedBlockSize,
               y * roundedBlockSize,
               roundedBlockSize,
               roundedBlockSize
           };

           SDL_SetRenderDrawColor(p_window->p_renderer, 0u, 0u, 0u, 255u);
           SDL_RenderFillRect(p_window->p_renderer, &wallBlock);
       }
   }

   SDL_SetRenderDrawColor(p_window->p_renderer, 255u, 0u, 0u, 255u);
   SDL_RenderDrawLine(
       p_window->p_renderer,
       round(playerPosition.x),
       round(playerPosition.y),
       round(playerPosition.x + playerDirection.x * roundedBlockSize),
       round(playerPosition.y + playerDirection.y * roundedBlockSize)
       );

 SDL_SetRenderDrawColor(p_window->p_renderer, 0u, 0u, 255u, 255u);
   SDL_RenderDrawLine(
       p_window->p_renderer,
       round(playerPosition.x),
       round(playerPosition.y),
       round(playerPosition.x + p_gameInstance->player.cameraPlane.x * roundedBlockSize),
       round(playerPosition.y + p_gameInstance->player.cameraPlane.y * roundedBlockSize)
       );

   SDL_Rect playerRect = {
       round(playerPosition.x - roundedBlockSize * 0.25f),
       round(playerPosition.y - roundedBlockSize * 0.25f),
       round(roundedBlockSize * 0.5f),
       round(roundedBlockSize * 0.5f)
   };

   SDL_SetRenderDrawColor(p_window->p_renderer, 0u, 255u, 0u, 255u);
   SDL_RenderFillRect(p_window->p_renderer, &playerRect);
}

//////////////////////////////////

void setup(Window *p_window, GameInstance *p_gameInstance) {
   init_window(p_window);

   p_gameInstance->player.position.x = (float)WORLD_WIDTH * 0.5f;
   p_gameInstance->player.position.y = (float)WORLD_HEIGHT * 0.5f;
   p_gameInstance->player.movementSpeed = 0.005f;
   p_gameInstance->player.direction.x = 0.f;
   p_gameInstance->player.direction.y = -1.f;
   p_gameInstance->player.rotationSpeed = 0.005f;
    p_gameInstance->player.cameraPlane.x = 0.7f;
    p_gameInstance->player.cameraPlane.y = 0.f;
}

//////////////////////////////////

void game_logic(Window *p_window, GameInstance *p_gameInstance) {
   player_logic(p_window, p_gameInstance);
}

//////////////////////////////////

void player_logic(Window *p_window, GameInstance *p_gameInstance) {
   const Uint8 *keyStates = SDL_GetKeyboardState(NULL);
   f32 movementSpeed = p_gameInstance->player.movementSpeed;
   f32 rotationSpeed = p_gameInstance->player.rotationSpeed;
   if (keyStates[SDL_SCANCODE_W]) {
        p_gameInstance->player.position.x +=
            p_gameInstance->player.direction.x * movementSpeed * g_clock.deltaTime;
        p_gameInstance->player.position.y +=
            p_gameInstance->player.direction.y * movementSpeed * g_clock.deltaTime;
   }
   if (keyStates[SDL_SCANCODE_S]) {
         p_gameInstance->player.position.x -=
            p_gameInstance->player.direction.x * movementSpeed * g_clock.deltaTime;
        p_gameInstance->player.position.y -=
            p_gameInstance->player.direction.y * movementSpeed * g_clock.deltaTime;
   }
   f32 rotationAngle = rotationSpeed * g_clock.deltaTime;
   if (keyStates[SDL_SCANCODE_D]) {
        rotate(&(p_gameInstance->player.direction), rotationAngle);
        rotate(&(p_gameInstance->player.cameraPlane), rotationAngle);
   }
   if (keyStates[SDL_SCANCODE_A]) {
       rotate(&(p_gameInstance->player.direction), -rotationAngle);
       rotate(&(p_gameInstance->player.cameraPlane), -rotationAngle);
   }
}

//////////////////////////////////
void main_loop(Window *p_window, GameInstance *p_gameInstance) {
    while (1) {
        handleSDLEvents(p_window);
    
        game_logic(p_window, p_gameInstance);

        render(p_window, p_gameInstance);

        update_clock();
    } 
}

//////////////////////////////////

int main(int argc, char *argv[]) {
    Window window;
    GameInstance gameInstance;
    
    setup(&window, &gameInstance);
    main_loop(&window, &gameInstance);

    return 0;
}
