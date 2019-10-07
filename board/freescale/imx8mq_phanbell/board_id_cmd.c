#include <common.h>
#include <command.h>

#include "board_id.h"

#define MAX_BOARD_ID 7
#define UNKNOWN_BOARD_ID 7

const char* board_id_names[] = {
	"P0 and EVT, 3GB Micron DDR, SKU2",
	"EVT, 1GB Hynix DDR, SKU3",
	"EVT, 1GB Micron DDR, SKU1",
	"DVT, 2GB Kingston SKU4",
	"DVT, 3GB Micron DDR, SKU8",
	"DVT, 1GB Hynix DDR, SKU5",
	"DVT, 1GB Micron DDR, SKU6",
	"DVT, 3GB Micron DDR, SKU7",
	"Unknown"
};

const char* baseboard_id_names[] = {
	"Invalid",
	"Enterprise",
	"Yorktown",
	"Unknown"
};

static int do_board_id(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int board_id = get_board_id();
	const char* board_id_name;

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

static int do_baseboard_id(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int baseboard_id = get_baseboard_id();
	const char* baseboard_id_name;

	if (baseboard_id >= ARRAY_SIZE(baseboard_id_names)) {
		baseboard_id_name = baseboard_id_names[ARRAY_SIZE(baseboard_id_names) - 1];
	}

	baseboard_id_name = baseboard_id_names[baseboard_id];

	putc('0' + baseboard_id);
	puts(": ");
	puts(baseboard_id_name);
	putc('\n');

	return 0;
}

U_BOOT_CMD(
	boardid, 1, 0, do_board_id,
	"display board ID",
	"\n"
	"    - displays the board ID value from the GPIO pins."
);

U_BOOT_CMD(
	baseboardid, 1, 0, do_baseboard_id,
	"display baseboard ID",
	"\n"
	"   - displays the board ID value for the baseboard."
);
