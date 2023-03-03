#ifndef Header_qcore_BASE_DATA
#define Header_qcore_BASE_DATA


#include "Player.h"
#include "QcoreUtil.h" // includes logger

using Position = qcore::Position;
const Position INVALID_POS = {-1, -1};

const char * const DOM = "qplugin::ABV2"; 

namespace qplugin
{
	struct Move
	{
		qcore::WallState wall;
		qcore::Position pos; // caution ! no specified, whose coords are these (might not be needed)
		bool isWallMove = false;

		std::string toString(bool qcoreFormat = false);
		Move flip (bool reverse) const;
		bool operator<(const Move& other) const;
		bool operator==(const Move& other) const;
	};

	struct PlayerInfo
	{
      Position position;
      uint8_t wallsLeft;
	};

	class GlobalData
	{
		public:
			static uint16_t roundNumber;
	};
}

constexpr const int SECONDS_PER_TURN = 5;

#define FULL_STATE_REFRESH   // When disabled, it will only update the internal state instead of fully recreating it (not fully tested)
//#define LOG_MOVES
//#define FAST_TEST_MODE
//#define DUMP_MOVES_LIST

#endif // Header_qcore_DATA_MAPPING
