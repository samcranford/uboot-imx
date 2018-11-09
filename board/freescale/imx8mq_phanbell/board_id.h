#ifndef __BOARD_ID_H__
#define __BOARD_ID_H__

#include <common.h>

// Returns board id based on gpio pins state:
// 0 - P0 boards and EVT boards with 3Gb Micron (MT53B768M32D4NQ-062) DDR
// 1 - EVT boards with 1Gb Hynix DDR
// 2 - EVT boards with 1Gb Micron DDR
// 3 - Invalid
// 4 - DVT boards with 3Gb Micron (MT53B768M32D4NQ-062) DDR
// 5 - DVT boards with 1Gb Hynix DDR
// 6 - DVT boards with 1Gb Micron DDR
// 7 - DVT boards with 3Gb Micron (MT53E768M32D4DT) DDR

int get_board_id(void);
size_t get_ddr_size(void);

#endif
