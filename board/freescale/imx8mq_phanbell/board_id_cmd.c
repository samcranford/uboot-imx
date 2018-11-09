#include <common.h>
#include <command.h>

#include "board_id.h"

#define MAX_BOARD_ID 7
#define UNKNOWN_BOARD_ID 7

const char* board_id_names[] = {
	"P0 and EVT, 3Gb Micron DDR, SKU2",
	"EVT, 1Gb Hynix DDR, SKU3",
	"EVT, 1Gb Micron DDR, SKU1",
	"Invalid",
	"DVT, 3Gb Micron DDR, SKU8",
	"DVT, 1Gb Hynix DDR, SKU5",
	"DVT, 1Gb Micron DDR, SKU6",
	"DVT, 3Gb Micron DDR, SKU7",
	"Unknown"
};

static int do_board_id(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int board_id = get_board_id();
	char* board_id_name;

	if (board_id > MAX_BOARD_ID) {
		board_id = UNKNOWN_BOARD_ID;
	}

	board_id_name = board_id_names[board_id];

	putc('0' + board_id);
	puts(": ");
	puts(board_id_name);
	putc('\n');

	return 0;
}

U_BOOT_CMD(
	boardid, 1, 0, do_board_id,
	"display board ID",
	"\n"
	"    - displays the board ID value from the GPIO pins."
);
