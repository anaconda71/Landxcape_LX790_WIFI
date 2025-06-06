#pragma once
#include "config.h"
#include "LX790_util.h"
#include "HAL_LX790_V1_1.h"

/**
 * HW / model specific setup
 */
void HAL_setup();

/**
 * do HW communication, read display / button status, update display, send button commands
 * 
 * updates LX790_State:
 *   - segments: 7-segment raw pattern
 *   - point
 *   - clock
 *   - lock
 *   - battery (0 - 3)
 *   - brightness (0 - 15)
 */
void HAL_loop(LX790_State &state);

void HAL_buttonPress(BUTTONS btn);
void HAL_buttonRelease(BUTTONS btn);

