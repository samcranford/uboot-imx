#include <common.h>
#include <command.h>

#include "board_id.h"

#define MAX_BOARD_ID 7
#define UNKNOWN_BOARD_ID 7

const char* board_id_names[] = {
	"P0 and EVT, 3Gb Micron DDR",
	"EVT, 1Gb Hynix DDR",
	"EVT, 1Gb Micron DDR",
	"Invalid",
	"DVT, 3Gb Micron DDR",
	"DVT, 1Gb Hynix DDR",
	"DVT, 1Gb Micron DDR",
	"DVT, 1Gb Micron DDR",
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
