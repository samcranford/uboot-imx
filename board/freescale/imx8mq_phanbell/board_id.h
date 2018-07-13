#ifndef __BOARD_ID_H__
#define __BOARD_ID_H__

#include <common.h>

// Returns board id based on gpio pins state:
// 0 - P0 boards and EVT boards with 3Gb Micron DDR
// 1 - EVT boards with 1Gb Hynix DDR
// 2 - EVT boards with 1Gb Micron DDR

int get_board_id(void);
size_t get_ddr_size(void);

#endif
