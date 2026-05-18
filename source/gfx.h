#ifndef GFX_H
#define GFX_H

#include <stdint.h>
#include <stdbool.h>

#define GFX_W       128
#define GFX_HEIGHT  160
#define GFX_FB_SIZE (GFX_W * GFX_HEIGHT / 2)

/* 颜色索引 0..15 */
typedef uint8_t gfx_color_t;

/* 透明色：DrawSprite 用 */
#define GFX_NO_TRANSPARENT 0xFF

extern uint16_t gfx_palette[16];          /* RGB565，用户可改 */
extern uint8_t  gfx_fb[GFX_FB_SIZE];

/* 调色板预设：经典 GameBoy 4 色 + 12 个常用色 */
void GFX_Init(void);

/* 基本操作 */
void GFX_Clear(gfx_color_t c);
void GFX_SetPixel(int x, int y, gfx_color_t c);
gfx_color_t GFX_GetPixel(int x, int y);

/* 形状 */
void GFX_FillRect(int x, int y, int w, int h, gfx_color_t c);
void GFX_DrawRect(int x, int y, int w, int h, gfx_color_t c);
void GFX_DrawHLine(int x, int y, int w, gfx_color_t c);
void GFX_DrawVLine(int x, int y, int h, gfx_color_t c);
void GFX_DrawLine(int x0, int y0, int x1, int y1, gfx_color_t c);
void GFX_DrawCircle(int cx, int cy, int r, gfx_color_t c);
void GFX_FillCircle(int cx, int cy, int r, gfx_color_t c);

/* 文字 (Adafruit 5x7, 占 6x8 含间距) */
void GFX_DrawChar(int x, int y, char ch, gfx_color_t fg, gfx_color_t bg, uint8_t size);
void GFX_DrawString(int x, int y, const char *s, gfx_color_t fg, gfx_color_t bg, uint8_t size);

/* Sprite：data 是 4bpp 打包，高 nibble = 左像素 */
void GFX_DrawSprite(int x, int y, int w, int h, const uint8_t *data,
                    gfx_color_t transparent);

/* 把整个 framebuffer 推到屏幕 */
void GFX_Flush(void);

#endif
