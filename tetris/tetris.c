#include <stdint.h>

#define WIDTH 20
#define HEIGHT 10
u8 board[WIDTH][HEIGHT];

// 128 x 160
#define RECT_HEIGHT 10
#define RECT_WIDTH 10
void
draw_board() {
  gfx_color_t c = 1;
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      if (board > 0) {
        GLF_FillRect(x*RECT_WIDTH,
                     y*RECT_HEIGHT,
                     RECT_HEIGHT,
                     RECT_WIDTH,
                     (c % 14) + 1);        
      }
      c++;
    }    
  }
}

void
tetris_init() {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      board[y][x] = 1;
    }
  }
}
