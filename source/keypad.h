/*
 * keypad.h - 4x4 软膜键盘扫描驱动 (MCXA153)
 *
 * 行(输出): t1=P1_10, t2=P1_12, t3=P1_13, t4=P2_0
 * 列(输入): ti1=P3_31, ti2=P3_30, ti3=P3_29, ti4=P1_11 (Pullup enabled)
 */

#ifndef KEYPAD_H_
#define KEYPAD_H_

#include <stdint.h>

#define KEYPAD_NO_KEY   ((char)0)

void KEYPAD_Init(void);
void KEYPAD_Scan(void);
char KEYPAD_GetKey(void);

#endif /* KEYPAD_H_ */
