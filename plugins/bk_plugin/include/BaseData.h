#ifndef Header_qcore_BASE_DATA
#define Header_qcore_BASE_DATA


#include "Player.h"
#include "QcoreUtil.h" // includes logger

using Position = qcore::Position;
const Position INVALID_POS = {-1, -1};

const char * const DOM = "qplugin::BK_Plugin"; 

namespace qplugin
{
	struct Move
	{
		qcore::WallState wall;
		qcore::Position pos; // caution ! no specified, whose coords are these (might not be needed)
		bool isWallMove = false;
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

#endif // Header_qcore_DATA_MAPPING
