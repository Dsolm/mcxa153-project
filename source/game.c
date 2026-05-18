#include "game.h"
#include "gfx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 简易 LCG，避免拉 stdlib rand 的种子问题 */
static uint32_t g_rng = 0xC0FFEE12u;
static int rand_range(int lo, int hi) {
    g_rng = g_rng * 1664525u + 1013904223u;
    return lo + (int)((g_rng >> 8) % (uint32_t)(hi - lo + 1));
}

/* 把 RNG 种子混入一点变化，main 里调用一次即可 */
void Game_SeedFromTick(uint32_t t) { g_rng ^= t * 2654435761u; }

static void make_question(game_question_t *q)
{
    int op = rand_range(0, 2);
    q->op = (game_op_t)op;
    switch (op) {
    case OP_ADD:
        q->a = rand_range(1, 50);
        q->b = rand_range(1, 50);
        q->answer = q->a + q->b;
        break;
    case OP_SUB: {
        int x = rand_range(1, 50);
        int y = rand_range(1, 50);
        if (x < y) { int t = x; x = y; y = t; }   /* 保证非负 */
        q->a = x; q->b = y; q->answer = x - y;
        break;
    }
    case OP_MUL:
        q->a = rand_range(2, 9);
        q->b = rand_range(2, 9);
        q->answer = q->a * q->b;
        break;
    }
}

static void start_round(game_t *g)
{
    g->state = GAME_PLAYING;
    g->question_idx = 0;
    g->correct_count = 0;
    g->feedback = FB_NONE;
    g->feedback_left_ms = 0;
    g->input_len = 0;
    g->input[0] = 0;
    g->time_left_ms = GAME_TIME_PER_Q_SEC * 1000;
    make_question(&g->cur);
}

static void next_question(game_t *g)
{
    g->question_idx++;
    if (g->question_idx >= GAME_ROUND_QUESTIONS) {
        g->state = GAME_RESULT;
        return;
    }
    g->input_len = 0;
    g->input[0] = 0;
    g->time_left_ms = GAME_TIME_PER_Q_SEC * 1000;
    make_question(&g->cur);
}

static int parse_input(const game_t *g)
{
    int v = 0;
    for (int i = 0; i < g->input_len; i++) v = v * 10 + (g->input[i] - '0');
    return v;
}

static void submit(game_t *g)
{
    if (g->input_len == 0) return;     /* 空输入不算提交 */
    int v = parse_input(g);
    if (v == g->cur.answer) {
        g->correct_count++;
        g->feedback = FB_CORRECT;
    } else {
        g->feedback = FB_WRONG;
    }
    g->feedback_left_ms = 800;
}

void Game_Init(game_t *g)
{
    memset(g, 0, sizeof *g);
    g->state = GAME_MENU;
}

void Game_OnKey(game_t *g, char key)
{
    /* D 任何状态都重新开始 */
    if (key == 'D') { start_round(g); return; }

    if (g->state == GAME_MENU) {
        if (key == 'A') start_round(g);
        return;
    }

    if (g->state == GAME_RESULT) {
        if (key == 'A') start_round(g);
        return;
    }

    /* PLAYING */
    if (g->feedback != FB_NONE) {
        /* 反馈期间忽略输入，避免误触跳题 */
        return;
    }

    if (key >= '0' && key <= '9') {
        if (g->input_len < GAME_ANSWER_MAX_DIGITS) {
            g->input[g->input_len++] = key;
            g->input[g->input_len] = 0;
        }
    } else if (key == 'A') {
        submit(g);
    } else if (key == 'B') {
        g->input_len = 0;
        g->input[0] = 0;
    } else if (key == 'C') {
        /* 跳过算错 */
        g->feedback = FB_WRONG;
        g->feedback_left_ms = 600;
    }
    /* * / # 未使用 */
}

void Game_Tick(game_t *g, int dt_ms)
{
    if (g->state != GAME_PLAYING) return;

    if (g->feedback != FB_NONE) {
        g->feedback_left_ms -= dt_ms;
        if (g->feedback_left_ms <= 0) {
            g->feedback = FB_NONE;
            next_question(g);
        }
        return;
    }

    g->time_left_ms -= dt_ms;
    if (g->time_left_ms <= 0) {
        g->time_left_ms = 0;
        g->feedback = FB_TIMEOUT;
        g->feedback_left_ms = 800;
    }
}

/* ===== 渲染 ===== */

/* 调色板索引（用 gfx.c 默认的）：
 * 0 黑 / 3 白 / 4 红 / 5 绿 / 6 蓝 / 7 黄 / 8 青 / 2 浅灰
 */
#define COL_BG     0
#define COL_FG     3
#define COL_ACCENT 7
#define COL_OK     5
#define COL_BAD    4
#define COL_DIM    2

static void draw_centered(int y, const char *s, uint8_t fg, uint8_t bg, uint8_t size)
{
    int w = 0; for (const char *p = s; *p; p++) w++;
    int px = (GFX_W - w * 6 * size) / 2;
    if (px < 0) px = 0;
    GFX_DrawString(px, y, s, fg, bg, size);
}

static void render_menu(game_t *g)
{
    (void)g;
    GFX_Clear(COL_BG);
    draw_centered(20, "MATH", COL_ACCENT, COL_BG, 3);
    draw_centered(55, "PRACTICE", COL_ACCENT, COL_BG, 2);

    draw_centered(95,  "Press A to start", COL_FG, COL_BG, 1);
    draw_centered(110, "10 questions",     COL_DIM, COL_BG, 1);
    draw_centered(122, "10 sec each",      COL_DIM, COL_BG, 1);

    GFX_DrawString(2, 148, "A:OK B:CLR C:SKP D:RST", COL_DIM, COL_BG, 1);
}

static const char *op_str(game_op_t op)
{
    switch (op) { case OP_ADD: return "+"; case OP_SUB: return "-"; case OP_MUL: return "x"; }
    return "?";
}

static void render_playing(game_t *g)
{
    char buf[24];

    GFX_Clear(COL_BG);

    /* 顶部状态条 */
    snprintf(buf, sizeof buf, "Q %d/%d", g->question_idx + 1, GAME_ROUND_QUESTIONS);
    GFX_DrawString(4, 4, buf, COL_FG, COL_BG, 1);

    snprintf(buf, sizeof buf, "OK %d", g->correct_count);
    GFX_DrawString(80, 4, buf, COL_OK, COL_BG, 1);

    /* 计时条 */
    int bar_w = (g->time_left_ms * (GFX_W - 8)) / (GAME_TIME_PER_Q_SEC * 1000);
    if (bar_w < 0) bar_w = 0;
    uint8_t bar_col = (g->time_left_ms < 3000) ? COL_BAD : COL_OK;
    GFX_DrawRect(4, 16, GFX_W - 8, 6, COL_DIM);
    GFX_FillRect(5, 17, bar_w - 2 > 0 ? bar_w - 2 : 0, 4, bar_col);

    /* 题目大字 */
    snprintf(buf, sizeof buf, "%d %s %d", g->cur.a, op_str(g->cur.op), g->cur.b);
    draw_centered(45, buf, COL_FG, COL_BG, 2);
    draw_centered(70, "=", COL_FG, COL_BG, 2);

    /* 输入框 */
    GFX_DrawRect(20, 92, GFX_W - 40, 22, COL_DIM);
    const char *show = g->input_len > 0 ? g->input : "_";
    /* 输入用 size=2 居中 */
    int chars = g->input_len > 0 ? g->input_len : 1;
    int px = (GFX_W - chars * 6 * 2) / 2;
    GFX_DrawString(px, 96, show, COL_ACCENT, COL_BG, 2);

    /* 反馈覆盖 */
    if (g->feedback == FB_CORRECT) {
        draw_centered(125, "CORRECT!", COL_OK, COL_BG, 2);
    } else if (g->feedback == FB_WRONG) {
        char fb[24];
        snprintf(fb, sizeof fb, "WRONG = %d", g->cur.answer);
        draw_centered(125, fb, COL_BAD, COL_BG, 1);
    } else if (g->feedback == FB_TIMEOUT) {
        char fb[24];
        snprintf(fb, sizeof fb, "TIME! = %d", g->cur.answer);
        draw_centered(125, fb, COL_BAD, COL_BG, 1);
    } else {
        GFX_DrawString(2, 148, "A:OK B:CLR C:SKP D:RST", COL_DIM, COL_BG, 1);
    }
}

static void render_result(game_t *g)
{
    char buf[24];
    GFX_Clear(COL_BG);
    draw_centered(20, "RESULT", COL_ACCENT, COL_BG, 2);

    snprintf(buf, sizeof buf, "%d / %d", g->correct_count, GAME_ROUND_QUESTIONS);
    draw_centered(55, buf, COL_FG, COL_BG, 3);

    const char *grade;
    uint8_t gcol;
    if (g->correct_count == GAME_ROUND_QUESTIONS) { grade = "PERFECT"; gcol = COL_ACCENT; }
    else if (g->correct_count >= 8) { grade = "GREAT";   gcol = COL_OK; }
    else if (g->correct_count >= 5) { grade = "OK";      gcol = COL_FG; }
    else                            { grade = "PRACTICE";gcol = COL_BAD; }
    draw_centered(95, grade, gcol, COL_BG, 2);

    draw_centered(125, "A: again", COL_FG, COL_BG, 1);
    draw_centered(137, "D: menu reset", COL_DIM, COL_BG, 1);
}

void Game_Render(game_t *g)
{
    switch (g->state) {
    case GAME_MENU:    render_menu(g);    break;
    case GAME_PLAYING: render_playing(g); break;
    case GAME_RESULT:  render_result(g);  break;
    }
    GFX_Flush();
}
