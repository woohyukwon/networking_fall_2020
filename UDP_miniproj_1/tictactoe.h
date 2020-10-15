#include <stdint.h>

enum MessageType {
		ClientGetGame = 111,
		ServerGameReply,
		ServerInvalidRequestReply,
		ClientResult,
		ServerClientResultCorrect,
		ServerClientResultIncorrect
};

enum ResultType {
	X_WIN = 11,
	O_WIN,
	CATS_GAME,
	INVALID_BOARD
};

struct TTTMessage {
	uint16_t type;
	uint16_t len;
} __attribute__((packed));

struct GetGameMessage {
	struct TTTMessage hdr;
	uint16_t client_id;
} __attribute__((packed));

struct GameSummaryMessage {
	struct TTTMessage hdr;
	uint16_t client_id;
	uint16_t game_id;
	uint16_t x_positions;
	uint16_t o_positions;
} __attribute__((packed));

struct GameResultMessage {
	struct TTTMessage hdr;
	uint16_t game_id;
	uint16_t result;
} __attribute__((packed));
