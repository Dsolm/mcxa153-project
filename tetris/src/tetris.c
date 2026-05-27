#include "tetronimo.h"
#include "settings.h"
#include <stdbool.h>

#include <stdlib.h>
#include <stdio.h>


#define WIDTH 10
#define HEIGHT 20
#define SDL3_MODE 1

#if SDL3_MODE
#include <SDL3/SDL.h>
SDL_Renderer* renderer;
#endif

u8 board[HEIGHT][WIDTH] = {0};
u8 falling_type = 0;
u8 falling_rotation = 0;
i8 falling_x = 0;
i8 falling_y = 0;


// 128 x 160
#define RECT_HEIGHT 10
#define RECT_WIDTH 10

static bool
out_of_bounds(i8 x, i8 y) {
  return x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT;
}

static void
tetronimo_spawn(u8 x, u8 y, u8 type, u8 rotation) {
  falling_type = type;
  falling_x = x;
  falling_y = y;
  falling_rotation = rotation;
}
static int
tetris_remove_full_row(int y) {
  for (int x = 0; x < WIDTH; x++) {
    if (board[y][x] == 8) {
      return 0;
    }
  }

  for (int _y = y; _y > 0; _y--) {
    for (int x = 0; x < WIDTH; x++) {
      board[_y][x] = board[_y-1][x];
    }          
  }

  for (int x = 0; x < WIDTH; x++) {
    board[0][x] = 8;
  }
  return 1;
}


void
tetris_draw() {
#if SDL3_MODE
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {

      int r = 255;
      int g = 255;
      int b = 255;
      SDL_SetRenderDrawColor(renderer, r, g, b, 255);
      struct SDL_FRect rect;
      rect.x = (float) (x * RECT_WIDTH);
      rect.y = (float) (y * RECT_HEIGHT);
      rect.w = (float) (RECT_WIDTH);
      rect.h = (float) (RECT_HEIGHT);
      SDL_RenderFillRect(renderer, &rect);

    }    
  }

  
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      if (board[y][x] != 8) {
        /* GFX_FillRect(x*RECT_WIDTH, */
        /*              y*RECT_HEIGHT, */
        /*              RECT_HEIGHT, */
        /*              RECT_WIDTH, */
        /*              (c % 14) + 1);         */

        gfx_color_t c = board[y][x];
        int r = c * 1209312 - 123 * c;
        int g = c * 35489;
        int b = c * 324;
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        struct SDL_FRect rect;
        rect.x = (float) (x * RECT_WIDTH);
        rect.y = (float) (y * RECT_HEIGHT);
        rect.w = (float) (RECT_WIDTH);
        rect.h = (float) (RECT_HEIGHT);
        SDL_RenderFillRect(renderer, &rect);
      }
    }    
  }

  for (int y = 0; y < TSIZE; y++) {
    for (int x = 0; x < TSIZE; x++) {
      int c = pieces[falling_type][falling_rotation][y][x];
      if (c > 0) {
        int posy = falling_y + y;
        int posx = falling_x + x;
        if (!out_of_bounds(posx, posy)) {
          int r = c * 1209312 - 123 * c;
          int g = c * 35489;
          int b = c * 324;
          SDL_SetRenderDrawColor(renderer, r, g, b, 255);
          struct SDL_FRect rect;
          rect.x = (float) (posx * RECT_WIDTH);
          rect.y = (float) (posy * RECT_HEIGHT);
          rect.w = (float) (RECT_WIDTH);
          rect.h = (float) (RECT_HEIGHT);
          SDL_RenderFillRect(renderer, &rect);
        }
      }
    }    
  }
#endif
}

static bool
check_collision() {
  for (int y = 0; y < TSIZE; y++) {
    for (int x = 0; x < TSIZE; x++) {
      if (pieces[falling_type][falling_rotation][y][x] != 0) {
        int real_x = falling_x + x;
        int real_y = falling_y + y;
        if (out_of_bounds(real_x, real_y)) {
          return true;
        } else {
          u8 v = board[real_y][real_x]; 
          if (v != 8) {
            return true;
          }                  
        }
      }
    }    
  }
  return false;
}

static void
falling_rotate() {
  u8 old = falling_rotation;
  falling_rotation = (falling_rotation + 1) % MAX_ROT;
  if (check_collision()) {
    falling_rotation = old;
  }
}

static int
tetris_falling_place() {
  for (int y = TSIZE - 1; y >= 0; y--) {
    int real_y = falling_y + y;
    for (int x = 0; x < TSIZE; x++) {
      u8 v = pieces[falling_type][falling_rotation][y][x];
      if (v != 0) {
        int real_y = falling_y + y;
        int real_x = falling_x + x;
        board[real_y][real_x] = v;
      }
    }
  }

  int rows_removed = 0;
  for (int y = 0; y < TSIZE; y++) {
    int real_y = falling_y + y;
    if (real_y < HEIGHT) {
      int row_removed = tetris_remove_full_row(real_y);
      rows_removed += row_removed;
    }
  }
  return rows_removed;
}

/* static void */
/* tetris_fall() { */
/*   falling_y += 1; */
/*   if (check_collision()) { */
/*     falling_y -= 1; */
/*     tetris_falling_place(); */
/*     tetronimo_spawn(0,0,0,0); */
/*   } */
/* } */

static int
tetris_tick() {
  /* for (int y = HEIGHT - 1; y >= 1; y--) { */
  /*   for (int x = 0; x < WIDTH; x++) { */
  /*     if (board[y-1][x] != 8) { */
  /*       board[y][x] = board[y-1][x]; */
  /*     } */
  /*   } */
  /* } */
  /* for (int x = 0; x < WIDTH; x++) { */
  /*   board[HEIGHT - 1][x] = 0; */
  /* } */

  int rows_removed = 0;
  falling_y += 1;
  if (check_collision()) {
    printf("Fell\n");
    falling_y -= 1;
    rows_removed = tetris_falling_place();
    int rotation = rand() % MAX_ROT;
    int type = rand() % 7;
    tetronimo_spawn(0,0,type,rotation);
  }
  int points = 0;
  if (rows_removed == 1) {
    points = 100;
  } else if (rows_removed == 2) {
    points = 300;
  } else if (rows_removed == 3) {
    points = 500;
  } else if (rows_removed == 4) {
    points = 800;
  }
  return points;
}

/// Rotation < 4}
/// type < 8
/* static void */
/* tetronimo_place() { */
/*   for (int _y = 0; _y < TSIZE; _y++) { */
/*     for (int _x = 0; _x < TSIZE; _x++) { */
/*       board[x+_x][y+_y] = pieces[type][rotation][_y][_x];       */
/*     } */
/*   } */
/* } */
void
tetris_input(int dir) {
  falling_x += dir;
  if (check_collision()) {
    falling_x -= dir;    
  }
}

void
tetris_rotate() {
  int last_rotation = falling_rotation;
  falling_rotation = (falling_rotation + 1) % MAX_ROT;
  if (check_collision()) {
    falling_rotation = last_rotation;
  }
}



void
tetris_init() {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      board[y][x] = 8;        
    }
  }
}

#if SDL3_MODE == 1
#include <SDL3/SDL.h>
#endif

#if SDL3_MODE == 1
int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  // Initialize SDL video subsystem
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    return 1;
  }

  // Create window
  SDL_Window *window = SDL_CreateWindow(
                                        "SDL3 Demo",
                                        800,
                                        600,
                                        0
                                        );

  if (!window) {
    SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  tetris_init();
  tetronimo_spawn(0, 0, 0, 0);
  // Create renderer
  renderer = SDL_CreateRenderer(window, NULL);

  if (!renderer) {
    SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  bool running = true;

  Uint64 last_time = SDL_GetTicks();
  Uint64 current_time = last_time;
  SDL_Event event;
  int points = 0;
  int level = 1;
  const int INPUT_DEBOUNCE = 200;
  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        running = false;
        break;
      case SDL_EVENT_KEY_DOWN:
        if (event.key.scancode == SDL_SCANCODE_W) {
          tetris_rotate();
        }
        if (event.key.scancode == SDL_SCANCODE_S) {
          last_time = current_time;
          points += tetris_tick() * level;
        }
        if (event.key.scancode == SDL_SCANCODE_A) {
          tetris_input(-1);
        }
        if (event.key.scancode == SDL_SCANCODE_D) {
          tetris_input(1);
        }
        break;
      }
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    tetris_draw();
    current_time = SDL_GetTicks();
    if (current_time - last_time > 1000) {
      last_time = current_time;
      points += tetris_tick() * level;      
    }
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
#endif