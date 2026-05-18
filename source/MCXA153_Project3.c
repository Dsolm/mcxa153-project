#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "display.h"
#include "gfx.h"

static void delay_anim(volatile unsigned int t)
{
    while (t--) {
        __asm volatile ("nop");
    }
}

static void draw_dot(int x, int y, int active)
{
    if (active)
        GFX_FillCircle(x, y, 4, 7);
    else
        GFX_FillCircle(x, y, 3, 2);
}

static void draw_logo_letters(int count)
{
    const char *name = "GAMEMEN";
    int x = 22;
    int y = 52;

    for (int i = 0; i < count; i++) {
        char s[2] = { name[i], '\0' };
        GFX_DrawString(x + i * 12, y, s, 7, 0, 2);
    }
}

int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    Display_Init();
    GFX_Init();

    while (1) {
        for (int i = 1; i <= 7; i++) {
            GFX_Clear(0);
            draw_logo_letters(i);
            GFX_Flush();
            delay_anim(180000);
        }

        for (int frame = 0; frame < 40; frame++) {
            GFX_Clear(0);

            draw_logo_letters(7);
            GFX_DrawString(40, 78, "loading", 3, 0, 1);

            int active = frame % 4;
            draw_dot(44, 105, active == 0);
            draw_dot(58, 105, active == 1);
            draw_dot(72, 105, active == 2);
            draw_dot(86, 105, active == 3);

            GFX_Flush();
            delay_anim(150000);
        }
    }
}
