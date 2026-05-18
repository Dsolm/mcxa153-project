#ifndef PIN_MUX_H_
#define PIN_MUX_H_

#include "fsl_common.h"

#ifdef __cplusplus
extern "C" {
#endif

void BOARD_InitBootPins(void);
void BOARD_InitPins(void);

#ifdef __cplusplus
}
#endif

#endif /* PIN_MUX_H_ */
