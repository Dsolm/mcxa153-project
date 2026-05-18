#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <stdbool.h>

#define GAME_ROUND_QUESTIONS  10
#define GAME_TIME_PER_Q_SEC   10
#define GAME_ANSWER_MAX_DIGITS 4   /* 最大输入位数: 99*9=891, 50+50=100, 留 4 位 */

typedef enum {
    GAME_MENU,
    GAME_PLAYING,
    GAME_RESULT,
} game_state_t;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
} game_op_t;

typedef struct {
    int a, b;
    game_op_t op;
    int answer;
} game_question_t;

typedef struct {
    game_state_t state;
    int question_idx;            /* 0..GAME_ROUND_QUESTIONS-1 */
    int correct_count;
    game_question_t cur;

    char input[GAME_ANSWER_MAX_DIGITS + 1];
    int  input_len;
    bool input_negative;          /* 减法结果可能用户想输负数？我们保证非负，所以这个先留着 false */

    int  time_left_ms;            /* 当前题剩余毫秒 */

    /* 反馈显示用 */
    enum { FB_NONE, FB_CORRECT, FB_WRONG, FB_TIMEOUT } feedback;
    int  feedback_left_ms;        /* 反馈显示 800ms */
} game_t;

void Game_Init(game_t *g);
void Game_OnKey(game_t *g, char key);
void Game_Tick(game_t *g, int dt_ms);    /* 5ms 调一次 */
void Game_Render(game_t *g);             /* 主循环调，画到 framebuffer 后 Flush */

#endif
