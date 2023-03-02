#include "drma_player.h"

using namespace qcore::literals;
using namespace std::chrono_literals;

namespace qplugin
{
   const char * const DOM = "drma";

   drmaPlayer::drmaPlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game) :
      qcore::Player(id, name, game)
   {
   }

   /** This function defines the behaviour of the player. It is called by the game when it is my turn, and it 
    *  needs to end with a call to one of the action functions: move(...), placeWall(...).
    */      
   void drmaPlayer::doNextMove()
   {
      std::chrono::time_point<std::chrono::steady_clock> tStart = std::chrono::steady_clock::now();

      if ((turn++) == 0)
      {
         // on first entry
         board = NewDefaultBoard();

         #if (RUN_TESTS)
            RunAllTests(board);
         #endif
      }


      // get game info ----------------------------------------------------------------------------------------------------------------------
      uint8_t              myID           = getId(); // 0 (if I am the first player to move) or 1
      uint8_t              oppID          = (myID ? 0 : 1);
      qcore::Position      myPos          = getPosition();
      qcore::BoardStatePtr boardState     = getBoardState();
      qcore::PlayerState   oppState       = boardState->getPlayers(oppID).at(oppID);
      qcore::Position      oppPos         = (oppState.position).rotate(2); // always rotate, as player positions are relative
      qcore::PlayerAction  lastAct        = boardState->getLastAction();
      qcore::ActionType    lastActType    = lastAct.actionType;
      qcore::Orientation   lastActWallOr  = lastAct.wallState.orientation;
      qcore::Position      lastActWallPos = (myID == 0 ? lastAct.wallState.position : 
                                                CoreAbsToRelWallPos(lastAct.wallState.position, lastActWallOr)); // only convert when I am 2nd to move
      uint8_t              myWallsLeft    = getWallsLeft();
      uint8_t              oppWallsLeft   = oppState.wallsLeft;


      // update board structure -------------------------------------------------------------------------------------------------------------
      UpdatePos(board, ME, {myPos.x, myPos.y});
      UpdatePos(board, OPPONENT, {oppPos.x, oppPos.y});

      if (lastActType == qcore::ActionType::Wall)
      {  
         Position_t wallPos = CoreToPluginWallPos(lastActWallPos, lastActWallOr);
         Orientation_t wallOr = CoreToPluginWallOrientation(lastActWallOr);

         PlaceWall(board, OPPONENT, GetWallByPosAndOrientation(board, wallPos, wallOr));
      }

      UpdatePossibleMoves(board, ME);
      UpdatePossibleMoves(board, OPPONENT);

      bool foundMinPathOpp;
      uint8_t minPathOpp = FindMinPathLen(board, OPPONENT, &foundMinPathOpp);

      bool foundMinPathMe;
      uint8_t minPathMe = FindMinPathLen(board, ME, &foundMinPathMe);


      // log info ---------------------------------------------------------------------------------------------------------------------------
      LOG_INFO(DOM) << "---------------------------------------------------------------";
      LOG_INFO(DOM) << "  Turn count = " << turn;
      LOG_INFO(DOM) << "  Me  (" << (int)myID << ") pos = [" << (int)myPos.x << ", " << (int)myPos.y << "], walls = " << (int)myWallsLeft;
      LOG_INFO(DOM) << "  Opp (" << (int)oppID << ") pos = [" << (int)oppPos.x << ", " << (int)oppPos.y << "], walls = " << (int)oppWallsLeft;
      LOG_INFO(DOM) << "  Last act = " << (lastActType == qcore::ActionType::Invalid ? "Invalid" : 
                                                (lastActType == qcore::ActionType::Move ? "Move" : "Wall"));

      if (lastActType == qcore::ActionType::Wall)
      {         
         if (lastActWallOr == qcore::Orientation::Horizontal)
         {
            LOG_INFO(DOM) << "  Wall dir = H, wall pos (core)   = [" << (int)lastActWallPos.x << ", " << (int)lastActWallPos.y << "]";
            LOG_INFO(DOM) << "                wall pos (plugin) = [" << (lastActWallPos.x - 1) << ", " << (int)lastActWallPos.y << "]";
         }
         else
         {
            LOG_INFO(DOM) << "  Wall dir = V, wall pos (core)   = [" << (int)lastActWallPos.x << ", " << (int)lastActWallPos.y << "]";
            LOG_INFO(DOM) << "                wall pos (plugin) = [" << (int)(lastActWallPos.x) << ", " << (lastActWallPos.y - 1) << "]";
         }
      }     

      debug_PrintMyPossibleMoves(board);
      debug_PrintOppPossibleMoves(board);

      LOG_INFO(DOM) << "  My minpath =  " << (int)minPathMe << ", (found = " << (foundMinPathMe ? "true" : "false") << ")";
      LOG_INFO(DOM) << "  Opp minpath = " << (int)minPathOpp << ", (found = " << (foundMinPathOpp ? "true" : "false") << ")";
      LOG_INFO(DOM) << "";

      debug_PrintBoard(board);


      // Reduce the branching factor in the first part of the game --------------------------------------------------------------------------

      // disable all walls if no walls were placed yet and the players are close to their home base
      if ((myWallsLeft + oppWallsLeft == 20) && myPos.x > 5 && oppPos.x < 3 && !areAllWallsDisabled)
      {
         DisableWallsSubset(board, ALL_WALLS);
         areAllWallsDisabled = true;
         LOG_INFO(DOM) << "  Disabled all walls";
      }
      
      // enable all walls if a wall was placed or the players have advanced enough
      if ((myWallsLeft + oppWallsLeft < 20 || myPos.x <= 5 || oppPos.x >= 3) && areAllWallsDisabled)
      {
         EnableWallsSubset(board, ALL_WALLS);
         areAllWallsDisabled = false;
         LOG_INFO(DOM) << "  Enabled all walls";
      }

      // disable first and last col vertical walls until at least 2 walls were placed
       if ((myWallsLeft + oppWallsLeft > 18) && !areFirstAndLastColVertWallsDisabled && !areAllWallsDisabled)
      {
         DisableWallsSubset(board, VERT_WALLS_FIRST_LAST_COL);
         areFirstAndLastColVertWallsDisabled = true;
         LOG_INFO(DOM) << "  Disabled First Last Col Vertical walls";
      }

      // enable first and last col vertical walls after the first 2 walls were placed
      if ((myWallsLeft + oppWallsLeft <= 18) && areFirstAndLastColVertWallsDisabled)
      {
         EnableWallsSubset(board, VERT_WALLS_FIRST_LAST_COL);
         areFirstAndLastColVertWallsDisabled = false;
         LOG_INFO(DOM) << "  Enabled First Last Col Vertical walls";
      }
      
      // disable corner walls until at least 4 walls have been placed
      if ((myWallsLeft + oppWallsLeft > 16) && !areCornerWallsDisabled && !areAllWallsDisabled)
      {
         DisableWallsSubset(board, CORNER_WALLS);
         areCornerWallsDisabled = true;
         LOG_INFO(DOM) << "  Disabled Corner walls";
      }
      
      // enable corner walls after the first 4 walls were placed
      if ((myWallsLeft + oppWallsLeft <= 16) && areCornerWallsDisabled)
      {
         EnableWallsSubset(board, CORNER_WALLS);
         areCornerWallsDisabled = false;
         LOG_INFO(DOM) << "  Enabled Corner walls";
      }

      // Figure out best play ---------------------------------------------------------------------------------------------------------------

      Play_t bestPlay;
      
      // Shiller opening if possible
      if (myID == 0 && turn == 3 && myWallsLeft == 10 && oppWallsLeft == 10)
      {
         bestPlay = {PLACE_WALL, NULL_MOVE, GetWallByPosAndOrientation(board, {7, 4}, V)};
         debug_PrintPlay(bestPlay);
      }
      else // Minimax
      {
         bool hasTimedOut = false;

         // First minimax pass with (depth - 1): to make sure there is a best play if minimax with full depth times out
         Minimax(board, ME, MINIMAX_DEPTH - 1, NEG_INFINITY, POS_INFINITY, tStart, false, NULL);

         Play_t bestPlayFirstPass = GetBestPlayForLevel(MINIMAX_DEPTH - 1); 
         LOG_INFO(DOM) << "  After Minimax first pass:";
         debug_PrintPlay(bestPlayFirstPass);

         // Second minimax pass with full depth - can timeout:
         hasTimedOut = false;
         Minimax(board, ME, MINIMAX_DEPTH, NEG_INFINITY, POS_INFINITY, tStart, true, &hasTimedOut);
         
         // Get final best play
         if (hasTimedOut)
         {
            LOG_INFO(DOM) << "  Minimax second pass timed out! Proceed with best play from first pass... ";
            bestPlay = bestPlayFirstPass;
         }
         else
         {
            bestPlay = GetBestPlayForLevel(MINIMAX_DEPTH);
            LOG_INFO(DOM) << "  After Minimax second pass:";
         }

         debug_PrintPlay(bestPlay);
      }

      LOG_INFO(DOM) << "---------------------------------------------------------------";

      // Perform best play ------------------------------------------------------------------------------------------------------------------
      if (bestPlay.action == PLACE_WALL)
      {
         // place wall in the plugin logic
         PlaceWall(board, ME, GetWallByPosAndOrientation(board, bestPlay.wall->pos, bestPlay.wall->orientation));

         // place wall in the game core
         placeWall(qcore::WallState{PluginToCoreWallPos(bestPlay.wall->pos, bestPlay.wall->orientation), 
                     PluginToCoreWallOrientation(bestPlay.wall->orientation)});

         return;
      }
      else if (bestPlay.action == MAKE_MOVE)
      {
         // make move in the plugin logic
         MakeMove(board, ME, bestPlay.moveID);

         // make move in the game core
         move((myPos.x + board->moves[ME][bestPlay.moveID].xDiff), 
               (myPos.y + board->moves[ME][bestPlay.moveID].yDiff));

         return;
      }
      else // something went wrong, perform dummy move --------------------------------------------------------------------------------------
      {
         LOG_INFO(DOM) << "Player " << (int)getId() << " has f*cked up.. Fallback on dummy player... ";

         myPos = getPosition() * 2;
         qcore::BoardMap map;
         std::list<qcore::Position> pos;

         getBoardState()->createBoardMap(map, getId());

         for (uint8_t i = 0; i < qcore::BOARD_MAP_SIZE; i += 2)
         {
            qcore::Position p(0, i);
            map(p) = qcore::BoardMap::Invalid;
            pos.push_back(p);
         }

         auto checkPos = [&](const qcore::Position& p, qcore::Direction dir) -> bool
         {
            if (myPos == p)
            {
               move(dir);
               return true;
            }

            if(map(p) < qcore::BoardMap::HorizontalWall)
            {
               map(p) = qcore::BoardMap::Invalid;
               pos.emplace_back(p);
            }

            return false;
         };

         while (not pos.empty())
         {
            auto p = pos.front();
            pos.pop_front();

            if ((map(p + 1_x) == 0 and checkPos(p + 2_x, qcore::Direction::Up)) or
               (map(p - 1_x) == 0 and checkPos(p - 2_x, qcore::Direction::Down)) or
               (map(p + 1_y) == 0 and checkPos(p + 2_y, qcore::Direction::Left)) or
               (map(p - 1_y) == 0 and checkPos(p - 2_y, qcore::Direction::Right)))
            {
               return;
            }
         }

         LOG_WARN(DOM) << "Something went wrong. Making a random move.";
         move(qcore::Direction::Down) or move(qcore::Direction::Left) or move(qcore::Direction::Right) or move(qcore::Direction::Up);
      }
   }

   qcore::Position drmaPlayer::CoreAbsToRelWallPos(qcore::Position absPos, qcore::Orientation orientation)   
   {
      qcore::Position relPos;

      // flip around the board center, on both axis
      relPos.x = qcore::BOARD_SIZE - absPos.x;
      relPos.y = qcore::BOARD_SIZE - absPos.y;

      // the wall's position now refers to the wrong end of the wall, so adjust for that
      relPos.x -= (orientation == qcore::Orientation::Vertical ? 2 : 0);
      relPos.y -= (orientation == qcore::Orientation::Horizontal ? 2 : 0);

      return relPos;
   }


   drmaPlayer::Position_t drmaPlayer::CoreToPluginWallPos(qcore::Position coreWallPos, qcore::Orientation orientation)
   {
      Position_t pluginWallPos;

      if (orientation == qcore::Orientation::Horizontal)
      {
         pluginWallPos.x = coreWallPos.x - 1;
         pluginWallPos.y = coreWallPos.y;
      }
      else
      {           
         pluginWallPos.x = coreWallPos.x;
         pluginWallPos.y = coreWallPos.y - 1;
      }

      return pluginWallPos;
   }


   qcore::Position drmaPlayer::PluginToCoreWallPos(Position_t pluginWallPos, Orientation_t orientation)
   {
      qcore::Position coreWallPos;

      if (orientation == H)
      {
         coreWallPos.x = pluginWallPos.x + 1;
         coreWallPos.y = pluginWallPos.y;
      }
      else
      {           
         coreWallPos.x = pluginWallPos.x;
         coreWallPos.y = pluginWallPos.y + 1;
      }

      return coreWallPos;
   }


   drmaPlayer::Orientation_t drmaPlayer::CoreToPluginWallOrientation(qcore::Orientation orientation)
   {
      return (orientation == qcore::Orientation::Horizontal ? H : V);
   }


   qcore::Orientation drmaPlayer::PluginToCoreWallOrientation(Orientation_t orientation)
   {
      return (orientation == H ? qcore::Orientation::Horizontal : qcore::Orientation::Vertical);
   }

   drmaPlayer::Board_t* drmaPlayer::NewDefaultBoard(void)
   {
      Board_t* board = (Board_t*)calloc(1, sizeof(Board_t));

      // init tiles
      for (int8_t x = 0; x < BOARD_SZ; x++)
      {
            for (int8_t y = 0; y < BOARD_SZ; y++)
            {
               // set current tile's position
               board->tiles[x][y].pos = {x, y};

               // link tile to its neighbours. Link to NULL if the tile is on the border.
               board->tiles[x][y].north = ((x > 0)              ?   &(board->tiles[x - 1][y]) : NULL);
               board->tiles[x][y].south = ((x < (BOARD_SZ - 1)) ?   &(board->tiles[x + 1][y]) : NULL);
               board->tiles[x][y].west  = ((y > 0)              ?   &(board->tiles[x][y - 1]) : NULL);
               board->tiles[x][y].east  = ((y < (BOARD_SZ - 1)) ?   &(board->tiles[x][y + 1]) : NULL);

               // mark first row tiles as goal tiles for me, and last row tiles as goal tiles for opponent
               board->tiles[x][y].isGoalFor = ((x == 0) ? ME : ((x == (BOARD_SZ - 1)) ? OPPONENT : NOBODY));           
            }
      }

      // init walls
      for (int8_t o = H; o <= V; o++) // orientation V or H
      {
            for (int8_t x = 0; x < (BOARD_SZ - 1); x++)
            {
               for (int8_t y = 0; y < (BOARD_SZ - 1); y++)
               {
                  board->walls[o][x][y].pos = {x, y};
                  board->walls[o][x][y].orientation = (Orientation_t)o;
                  board->walls[o][x][y].permission = WALL_PERMITTED;
                  board->walls[o][x][y].isEnabled = true;

                  // set tiles that this wall separates when placed
                  Tile_t* referenceTile = &(board->tiles[x][y]);
                  board->walls[o][x][y].northwest = referenceTile;
                  board->walls[o][x][y].northeast = referenceTile->east;
                  board->walls[o][x][y].southwest = referenceTile->south;
                  board->walls[o][x][y].southeast = referenceTile->south->east;

                  // link to walls that this wall forbids when placed
                  if (o == H)
                  {
                        // each horizontal wall forbids 2 other horizontal walls and a vertical wall
                        board->walls[o][x][y].forbidsPrev  = ((y == 0) ?  NULL : &(board->walls[H][x][y - 1]));
                        board->walls[o][x][y].forbidsNext  = ((y == (BOARD_SZ - 2)) ? NULL : &(board->walls[H][x][y + 1]));
                        board->walls[o][x][y].forbidsCompl = &(board->walls[V][x][y]);
                  }
                  else
                  {
                        // each vertical wall forbids 2 other vertical walls and a horizontal wall
                        board->walls[o][x][y].forbidsPrev  = ((x == 0) ?     NULL : &(board->walls[V][x - 1][y]));
                        board->walls[o][x][y].forbidsNext  = ((x == (BOARD_SZ - 2)) ? NULL : &(board->walls[V][x + 1][y]));
                        board->walls[o][x][y].forbidsCompl = &(board->walls[H][x][y]);
                  }                
               }
            }
      }

      // init number of walls left
      board->wallsLeft[ME] = 10;
      board->wallsLeft[OPPONENT] = 10;

      // set initial player positions
      board->playerPos[ME] = {(BOARD_SZ - 1), BOARD_SZ / 2};
      board->playerPos[OPPONENT] = {0, BOARD_SZ / 2};

      // set other player for each player :)
      board->otherPlayer[ME] = OPPONENT;
      board->otherPlayer[OPPONENT] = ME;

      // init moveIDs and set the difference on x and y axes that each move implies
      for (int moveID = MOVE_FIRST; moveID <= MOVE_LAST; moveID++)
      {
            board->moves[ME][moveID].moveID = (MoveID_t)moveID;
            board->moves[OPPONENT][moveID].moveID = (MoveID_t)moveID;

            switch (moveID)
            {
               case MOVE_NORTH:
                  board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = -1;
                  board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff =  0;
               break;
               case MOVE_SOUTH:
                  board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = +1;
                  board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff =  0;
               break;
               case MOVE_WEST:
                  board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff =  0;
                  board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = -1;
               break;
               case MOVE_EAST:
                  board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff =  0;
                  board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = +1;
               break;
               case JUMP_NORTH:
                  board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = -2;
                  board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff =  0;
               break;
               case JUMP_SOUTH:
                  board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = +2;
                  board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff =  0;
               break;
               case JUMP_WEST:
                  board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff =  0;
                  board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = -2;
               break;
               case JUMP_EAST:
                  board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff =  0;
                  board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = +2;
               break;
               case JUMP_NORTH_WEST:
                  board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = -1;
                  board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = -1;
               break;
               case JUMP_NORTH_EAST:
                  board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = -1;
                  board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = +1;
               break;
               case JUMP_SOUTH_WEST:
                  board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = +1;
                  board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = -1;
               break;
               case JUMP_SOUTH_EAST:
                  board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = +1;
                  board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = +1;
               break;
               default: // this should never happen
                  board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = 0;
                  board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = 0;
            }
      }

      // init plays
      uint8_t numPlays = 0;

      for (int8_t o = V; o >= H; o--) // orientation V or H
      {
            for (int8_t x = 0; x < (BOARD_SZ - 1); x++)
            {
               for (int8_t y = 0; y < (BOARD_SZ - 1); y++)
               {
                  board->plays[numPlays].action = PLACE_WALL;
                  board->plays[numPlays].wall = &(board->walls[o][x][y]);
                  board->plays[numPlays].move = NULL;
                  board->plays[numPlays].player = NOBODY;
                  numPlays++;
               }
            }
      }

      for (int8_t pl = ME; pl <= OPPONENT; pl++) // for each player
      {
            for (int8_t moveID = MOVE_FIRST; moveID <= MOVE_LAST; moveID++)
            {
               board->plays[numPlays].action = MAKE_MOVE;
               board->plays[numPlays].move = &(board->moves[pl][moveID]);
               board->plays[numPlays].wall = NULL;
               board->plays[numPlays].player = (Player_t)pl;
               numPlays++;
            }
      }

      return board;
   }

   drmaPlayer::Wall_t* drmaPlayer::GetWallByPosAndOrientation(Board_t* board, Position_t wallPos, Orientation_t wallOr)
   {
      return &(board->walls[wallOr][wallPos.x][wallPos.y]);
   }

   void drmaPlayer::UpdatePos(Board_t* board, Player_t player, Position_t pos)
   {
      board->playerPos[player] = pos;
   }

   void drmaPlayer::UpdateWallsLeft(Board_t* board, Player_t player, uint8_t wallsLeft)
   {
      board->wallsLeft[player] = wallsLeft;
   }

   void drmaPlayer::PlaceWall(Board_t* board, Player_t player, Wall_t* wall)
   {
      // Placing a wall means:
      // 1. removing links (graph edges) between tiles separated by the wall
      if(wall->orientation == H)
      {
         wall->northwest->south = NULL;
         wall->northeast->south = NULL;
         wall->southwest->north = NULL;
         wall->southeast->north = NULL;
      }
      else
      {
         wall->northwest->east = NULL;
         wall->northeast->west = NULL;
         wall->southwest->east = NULL;
         wall->southeast->west = NULL;
      }

      // 2. Forbidding the given wall, along with the walls it displaces, from future use.
      // Any decrease in the permission level of a wall renders that wall forbidden.
      // Multiple levels of permission are needed because a wall can be forbidden by 1 or more walls.
      DECREASE_WALL_PERMISSION(wall);
      DECREASE_WALL_PERMISSION(wall->forbidsPrev);
      DECREASE_WALL_PERMISSION(wall->forbidsNext);
      DECREASE_WALL_PERMISSION(wall->forbidsCompl);

      // 3. Adjusting number of walls left for given player
      board->wallsLeft[player]--;
   }

   void drmaPlayer::UndoWall(Board_t* board, Player_t player, Wall_t* wall)
   {
      // Undoing a wall means:
      // 1. Restoring links (graph edges) between tiles separated by the wall
      if(wall->orientation == H)
      {
         wall->northwest->south = wall->southwest;
         wall->northeast->south = wall->southeast;
         wall->southwest->north = wall->northwest;
         wall->southeast->north = wall->northeast;
      }
      else
      {
         wall->northwest->east = wall->northeast;
         wall->northeast->west = wall->northwest;
         wall->southwest->east = wall->southeast;
         wall->southeast->west = wall->southwest;
      }

      // 2. Increasing the permissions for the current wall and the walls it is displacing.
      // A wall can be forbidden by 1 or more walls, so an increase in the permission level doesn't necessarily mean it is permitted.
      // A wall will be permitted only when its permission level is max.
      INCREASE_WALL_PERMISSION(wall->forbidsCompl);
      INCREASE_WALL_PERMISSION(wall->forbidsNext);
      INCREASE_WALL_PERMISSION(wall->forbidsPrev);
      INCREASE_WALL_PERMISSION(wall);

      // 3. Adjusting number of walls left for given player
      board->wallsLeft[player]++;
   }


   void drmaPlayer::UpdatePossibleMoves(Board_t* board, Player_t player)
   {
      Tile_t* pT = GetPlayerTile(board, player);
      Tile_t* oT = GetPlayerTile(board, board->otherPlayer[player]);

      board->moves[player][MOVE_NORTH].isPossible = ((pT->north) && (pT->north != oT) ? true : false);
      board->moves[player][MOVE_SOUTH].isPossible = ((pT->south) && (pT->south != oT) ? true : false);
      board->moves[player][MOVE_EAST].isPossible = ((pT->east) && (pT->east != oT) ? true : false);
      board->moves[player][MOVE_WEST].isPossible = ((pT->west) && (pT->west != oT) ? true : false);

      board->moves[player][JUMP_NORTH].isPossible = ((pT->north) && (pT->north == oT) && (oT->north) ? true : false);
      board->moves[player][JUMP_SOUTH].isPossible = ((pT->south) && (pT->south == oT) && (oT->south) ? true : false);
      board->moves[player][JUMP_EAST].isPossible = ((pT->east) && (pT->east == oT) && (oT->east) ? true : false);
      board->moves[player][JUMP_WEST].isPossible = ((pT->west) && (pT->west == oT) && (oT->west) ? true : false);

      board->moves[player][JUMP_NORTH_EAST].isPossible = (((pT->north) && (pT->north == oT) && (!oT->north) && (oT->east)) ? true :          // jump N -> E
                                                         (((pT->east) && (pT->east == oT) && (!oT->east) && (oT->north)) ? true : false)); // jump E -> N
      board->moves[player][JUMP_NORTH_WEST].isPossible = (((pT->north) && (pT->north == oT) && (!oT->north) && (oT->west)) ? true :          // jump N -> W
                                                         (((pT->west) && (pT->west == oT) && (!oT->west) && (oT->north)) ? true : false)); // jump W -> N                          
      board->moves[player][JUMP_SOUTH_EAST].isPossible = (((pT->south) && (pT->south == oT) && (!oT->south) && (oT->east)) ? true :          // jump S -> E
                                                         (((pT->east) && (pT->east == oT) && (!oT->east) && (oT->south)) ? true : false)); // jump E -> S
      board->moves[player][JUMP_SOUTH_WEST].isPossible = (((pT->south) && (pT->south == oT) && (!oT->south) && (oT->west)) ? true :          // jump S -> W
                                                         (((pT->west) && (pT->west == oT) && (!oT->west) && (oT->south)) ? true : false)); // jump W -> S
   }


   // Check that move is possible BEFORE calling this!
   void drmaPlayer::MakeMove(Board_t* board, Player_t player, MoveID_t moveID)
   {
      board->playerPos[player].x += board->moves[player][moveID].xDiff;
      board->playerPos[player].y += board->moves[player][moveID].yDiff;
   }

   // Only call this in pair with MakeMove()
   void drmaPlayer::UndoMove(Board_t* board, Player_t player, MoveID_t moveID)
   {
      board->playerPos[player].x -= board->moves[player][moveID].xDiff;
      board->playerPos[player].y -= board->moves[player][moveID].yDiff;
   }

   drmaPlayer::Tile_t* drmaPlayer::GetPlayerTile(Board_t* board, Player_t player)
   {
      return (&(board->tiles[board->playerPos[player].x][board->playerPos[player].y]));
   }

   bool drmaPlayer::HasPlayerWon(Board_t* board, Player_t player)
   {
      return (GetPlayerTile(board, player)->isGoalFor == player ? true : false);
   }

   void drmaPlayer::EnableWallsSubset(Board_t* board, WallsSubset_t subset)
   {
      for (int8_t o = H; o <= V; o++) // orientation V or H
      {
         for (int8_t x = 0; x < (BOARD_SZ - 1); x++)
         {
               for (int8_t y = 0; y < (BOARD_SZ - 1); y++)
               {
                  switch (subset)
                  {
                     case ALL_WALLS:
                           board->walls[o][x][y].isEnabled = true;
                     break;
                     case CORNER_WALLS:
                           if ((x == 0 || x == BOARD_SZ - 2) && (y == 0 || y == BOARD_SZ - 2))
                           {
                              board->walls[o][x][y].isEnabled = true;
                           }
                     break;
                     case VERT_WALLS_FIRST_LAST_COL:
                           if ((o == V) && (y == 0 || y == (BOARD_SZ - 2)))
                           {
                              board->walls[o][x][y].isEnabled = true;
                           }
                     break;
                     default:
                           board->walls[o][x][y].isEnabled = true;
                  }
               }
         }
      }
   }

   void drmaPlayer::DisableWallsSubset(Board_t* board, WallsSubset_t subset)
   {
      for (int8_t o = H; o <= V; o++) // orientation V or H
      {
         for (int8_t x = 0; x < (BOARD_SZ - 1); x++)
         {
               for (int8_t y = 0; y < (BOARD_SZ - 1); y++)
               {
                  switch (subset)
                  {
                     case ALL_WALLS:
                           board->walls[o][x][y].isEnabled = false;
                     break;
                     case CORNER_WALLS:
                           if ((x == 0 || x == BOARD_SZ - 2) && (y == 0 || y == BOARD_SZ - 2))
                           {
                              board->walls[o][x][y].isEnabled = false;
                           }
                     break;
                     case VERT_WALLS_FIRST_LAST_COL:
                           if ((o == V) && (y == 0 || y == (BOARD_SZ - 2)))
                           {
                              board->walls[o][x][y].isEnabled = false;
                           }
                     break;
                     default:
                           board->walls[o][x][y].isEnabled = false;
                  }
               }
         }
      }
   }

   // Static evaluation of the board position, from my perspective (maximizing player). A high score is good for me.
   int drmaPlayer::StaticEval(Board_t* board)
   {
      bool foundMinPathMe;
      bool foundMinPathOpp;

      uint8_t myMinPath = FindMinPathLen(board, ME, &foundMinPathMe);
      uint8_t oppMinPath = FindMinPathLen(board, OPPONENT, &foundMinPathOpp);

      if ((!foundMinPathMe) || (!foundMinPathOpp))
      {
         // there is no valid path for one of the players in the current position
         return ERROR_NO_PATH;
      }
      else
      {
         int winTheGameScore = (myMinPath == 0 ? 1 : (oppMinPath == 0? -1 : 0));
         int pathScore = oppMinPath - myMinPath;
         int wallScore = board->wallsLeft[ME] - board->wallsLeft[OPPONENT];
         int closerToEnemyBaseScore = ((BOARD_SZ - 1) - board->playerPos[OPPONENT].x) - board->playerPos[ME].x;

         return (winTheGameScore * 100000 + pathScore * 1000 + wallScore * 10 + closerToEnemyBaseScore);
      }
   }

   int drmaPlayer::Minimax(Board_t* board, Player_t player, uint8_t level, int alpha, int beta,
                  std::chrono::time_point<std::chrono::steady_clock> tStart, bool canTimeOut, bool *hasTimedOut)
   {
      UpdatePossibleMoves(board, ME);
      UpdatePossibleMoves(board, OPPONENT);

      if (HasPlayerWon(board, ME) || HasPlayerWon(board, OPPONENT) || (level == 0))
      {
         return StaticEval(board);
      }
      else
      {
         int score = (player == ME ? NEG_INFINITY : POS_INFINITY); // initialize to worst possible score

         if (canTimeOut && (level == 1))
         {
               std::chrono::time_point<std::chrono::steady_clock> tNow = std::chrono::steady_clock::now();
               if (std::chrono::duration_cast<std::chrono::milliseconds>(tNow - tStart).count() > MINIMAX_TIMEOUT_MS)
               {
                  *hasTimedOut = true;
                  return score;
               }
         }

         for (uint32_t i = 0; i < COUNT(board->plays); i++)
         {
               NominalPlay_t play = board->plays[i];
               bool prune = false;
               Action_t currentAction = NULL_ACTION;
               MoveID_t currentMoveID = NULL_MOVE;
               Wall_t* currentWall = NULL;

               if ((play.action == PLACE_WALL) && (board->wallsLeft[player]) && 
                     (play.wall->permission == WALL_PERMITTED) && (play.wall->isEnabled))
               {
                  PlaceWall(board, player, play.wall);
                  currentAction = PLACE_WALL;
                  currentWall = play.wall;
               }
               else if (play.action == MAKE_MOVE && play.player == player && play.move->isPossible)
               {
                  MakeMove(board, player, play.move->moveID);
                  currentAction = MAKE_MOVE;
                  currentMoveID = play.move->moveID;
               }

               if (currentAction != NULL_ACTION)
               {
                  int tempScore = Minimax(board, board->otherPlayer[player], level - 1, alpha, beta, tStart, canTimeOut, hasTimedOut);

                  if (IS_VALID(tempScore) && (!canTimeOut || (canTimeOut && !(*hasTimedOut))))
                  {
                     if(player == ME)
                     {
                           if (tempScore > score)
                           {
                              score = tempScore;
                              bestPlays[level] = { currentAction, currentMoveID, currentWall};
                           }

                           if (tempScore > alpha)
                           {
                              alpha = tempScore;
                           }
                     }
                     else
                     {
                           if (tempScore < score)
                           {
                              score = tempScore;
                              bestPlays[level] = { currentAction, currentMoveID, currentWall };
                           }

                           if (tempScore < beta)
                           {
                              beta = tempScore;
                           }
                     }

                     if (beta <= alpha)
                     {
                           prune = true;
                     }
                  }

                  if (currentAction == PLACE_WALL)
                  {
                     UndoWall(board, player, play.wall);
                  }
                  else
                  {
                     UndoMove(board, player, play.move->moveID);
                  }

                  UpdatePossibleMoves(board, ME);
                  UpdatePossibleMoves(board, OPPONENT);

                  if (canTimeOut && (*hasTimedOut))
                  {
                     return score;
                  }

                  if (prune)
                  {
                     break;
                  }
               }
         }

         return score;
      }
   }

   drmaPlayer::Play_t drmaPlayer::GetBestPlayForLevel(uint8_t level)
   {
      return bestPlays[level];
   }

   uint8_t drmaPlayer::FindMinPathLen(Board_t* board, Player_t player, bool* found)
   {
      if (GetPlayerTile(board, player)->isGoalFor == player)
      {
         // player is in the enemy base
         *found = true;
         return 0;
      }

      QueueInit();
      FoundSubpathsInit();

      uint8_t minPathLen = INFINITE_LEN;

      // start with the player's tile
      Subpath_t source = 
      { 
         GetPlayerTile(board, player), 
         NULL, 
         0 
      };

      QueuePush(source);

      // Breadth-First-Search. 
      // Stops when min path was found for a goal tile or when queue empty (meaning no goal tile is reachable).
      while((goalTilesReached == 0) && !IsQueueEmpty())
      {
         Subpath_t* item = QueuePop();

         int8_t x = item->tile->pos.x;
         int8_t y = item->tile->pos.y;

         // if min path was not yet found for the current tile
         if (foundSubpaths[x][y].tile == NULL)
         {
               // save path info
               memcpy(&(foundSubpaths[x][y]), item, sizeof(Subpath_t));

               // if the tile reached is a goal-tile for the current player
               if (item->tile->isGoalFor == player)
               {
                  // flag it as reached
                  goalTilesReached |= (1U << y);

                  // update min path length
                  if (minPathLen > item->pathLen)
                  {
                     minPathLen = item->pathLen;
                     destination = item; // debug
                  }
               }

               // go through neighbour tiles and add them to the queue if min path to them was not found yet
               if (item->tile->north && !IsMinPathFoundForTile(item->tile->north))
               {
                  Subpath_t newItem = 
                  {
                     item->tile->north, 
                     item->tile, 
                     ((uint8_t)(item->pathLen + 1))
                  };
                  QueuePush(newItem);
               }

               if (item->tile->west && !IsMinPathFoundForTile(item->tile->west))
               {
                  Subpath_t newItem = 
                  {
                     item->tile->west, 
                     item->tile, 
                     ((uint8_t)(item->pathLen + 1))
                  };
                  QueuePush(newItem);
               }

               if (item->tile->east && !IsMinPathFoundForTile(item->tile->east))
               {
                  Subpath_t newItem = 
                  {
                     item->tile->east, 
                     item->tile, 
                     ((uint8_t)(item->pathLen + 1))
                  };
                  QueuePush(newItem);
               }

               if (item->tile->south && !IsMinPathFoundForTile(item->tile->south))
               {
                  Subpath_t newItem = 
                  {
                     item->tile->south, 
                     item->tile, 
                     ((uint8_t)(item->pathLen + 1))
                  };
                  QueuePush(newItem);
               }
         }
      }
      
      *found = (!!goalTilesReached);

      return minPathLen;
   }

   void drmaPlayer::QueueInit(void)
   {
      queueNext = 0;
      queueFirst = 0;
   }

   bool drmaPlayer::IsQueueEmpty(void)
   {
      return (queueNext == queueFirst);
   }

   void drmaPlayer::QueuePush(Subpath_t item)
   {
      queue[queueNext++] = item;
   }

   drmaPlayer::Subpath_t* drmaPlayer::QueuePop(void)
   {
      return &(queue[queueFirst++]);
   }

   void drmaPlayer::FoundSubpathsInit(void)
   {
      memset(foundSubpaths, 0, sizeof(foundSubpaths));
      goalTilesReached = 0;
   }

   bool drmaPlayer::IsMinPathFoundForTile(Tile_t* tile)
   {
      return (foundSubpaths[tile->pos.x][tile->pos.y].pathLen != 0);
   }
   

   #if (PRINT_DEBUG_INFO)
   void drmaPlayer::ClearBuff(void)
   {
      memset(buff, 0, sizeof(buff));
   }

   const char* drmaPlayer::debug_ConvertMoveIDToString(MoveID_t moveID)
   {   
      switch (moveID)
      {
         case MOVE_NORTH:        return "M-N";
         case MOVE_SOUTH:        return "M-S";
         case MOVE_WEST:         return "M-W";
         case MOVE_EAST:         return "M-E";
         case JUMP_NORTH:        return "J-N";
         case JUMP_SOUTH:        return "J-S";
         case JUMP_EAST:         return "J-E";
         case JUMP_WEST:         return "J-W";
         case JUMP_NORTH_EAST:   return "J-N-E";
         case JUMP_NORTH_WEST:   return "J-N-W";
         case JUMP_SOUTH_EAST:   return "J-S-E";
         case JUMP_SOUTH_WEST:   return "J-S-W";
         default:                return "F*ck";
      }
   }

   char* drmaPlayer::debug_PrintMyPossibleMoves(Board_t* board)
   {    
      ClearBuff();
      sprintf(buff + strlen(buff), "  My possible moves: ");
      
      for (int i = MOVE_FIRST; i <= MOVE_LAST; i++)
      {
         if (board->moves[ME][i].isPossible)
         {
               sprintf(buff + strlen(buff), "[%s],", debug_ConvertMoveIDToString((MoveID_t)i));
         }
      }
      LOG_INFO(DOM) << buff;
      return buff;
   }

   char* drmaPlayer::debug_PrintOppPossibleMoves(Board_t* board)
   {    
      ClearBuff();
      sprintf(buff + strlen(buff), "  Opp possible moves: ");
      
      for (int i = MOVE_FIRST; i <= MOVE_LAST; i++)
      {
         if (board->moves[OPPONENT][i].isPossible)
         {
               sprintf(buff + strlen(buff), "[%s],", debug_ConvertMoveIDToString((MoveID_t)i));
         }
      }
      LOG_INFO(DOM) << buff;
      return buff;
   }    

   void drmaPlayer::debug_PrintPlay(Play_t play)
   {
      ClearBuff();
      sprintf(buff + strlen(buff), "  Best Play: ");
      if (play.action == MAKE_MOVE)
      {
         sprintf(buff + strlen(buff), "  Make move [%s]", debug_ConvertMoveIDToString(play.moveID));
      }
      else if (play.action == PLACE_WALL)
      {
         sprintf(buff + strlen(buff), "  Place wall: ");
         if (play.wall->orientation == H)
         {
               sprintf(buff + strlen(buff), "H[%d, %d],", play.wall->pos.x, play.wall->pos.y);
         }
         else
         {
               sprintf(buff + strlen(buff), "V[%d, %d],", play.wall->pos.x, play.wall->pos.y);
         }
      }
      else
      {
         sprintf(buff + strlen(buff), "  NO BEST PLAY FOUND");
      }
      LOG_INFO(DOM) << buff;
   }

   void drmaPlayer::debug_PrintBoard(Board_t* board)
   {
      char tiles[BOARD_SZ * BOARD_SZ] = { 0 };
      char vertw[BOARD_SZ * (BOARD_SZ - 1)] = { 0 };
      char horizw[BOARD_SZ * (BOARD_SZ - 1)] = { 0 };

      uint8_t ti = 0; 
      uint8_t hi = 0;
      uint8_t vi = 0;

      for (uint8_t i = 0; i < BOARD_SZ; i++)
      {
         for (uint8_t j = 0; j < BOARD_SZ; j++)
         {
               // populate vertical wall array
               if (j < BOARD_SZ - 1) 
               {
                  vertw[vi++] = ((board->tiles[i][j].east == NULL) ? '|' : ' ');
               }

               // populate horiz wall array
               if (i < BOARD_SZ - 1) 
               {
                  horizw[hi++] = ((board->tiles[i][j].south == NULL) ? '-' : ' ');
               }

               // populate tiles array
               if (i == board->playerPos[ME].x && j == board->playerPos[ME].y) 
               { 
                  tiles[ti++] = 'M'; // 'M' for my postion
                  continue;
               }
               if (i == board->playerPos[OPPONENT].x && j == board->playerPos[OPPONENT].y) 
               { 
                  tiles[ti++] = 'O'; // 'O' for opponent's position
                  continue;  
               }
               tiles[ti++] = ' ';
         }
      }
      
      ti = 0;
      hi = 0;
      vi = 0;

      LOG_INFO(DOM) << "        0     1     2     3     4     5     6     7     8   ";
      LOG_INFO(DOM) << "     ╔═════════════════════════════════════════════════════╗";

      for (int i = 0; i < 8; i++) // repeat 8 times
      {
         // row i (tiles, vert walls)
         ClearBuff();    
         sprintf(buff + strlen(buff), "   %d ║", i); // row index
         for (int j = 0; j < 8; j++)
         {
               sprintf(buff + strlen(buff), "  %c", tiles[ti++]);
               sprintf(buff + strlen(buff), "  %c", vertw[vi++]);
         }
         sprintf(buff + strlen(buff), "  %c  ║", tiles[ti++]);
         LOG_INFO(DOM) << buff;

         // row i horiz walls
         ClearBuff();    
         sprintf(buff + strlen(buff), "     ║ ");
         for (int j = 0; j < 8; j++)
         {
               sprintf(buff + strlen(buff), "%c%c%c - ", horizw[hi], horizw[hi], horizw[hi]);
               hi++;
         }
         sprintf(buff + strlen(buff), "%c%c%c ║", horizw[hi], horizw[hi], horizw[hi]);
         hi++;    
         LOG_INFO(DOM) << buff;
      }

      // last row (only tiles and vert walls)
      ClearBuff();    
      sprintf(buff + strlen(buff), "   %d ║", 8); // row index
      for (int j = 0; j < 8; j++)
      {
         sprintf(buff + strlen(buff), "  %c", tiles[ti++]);
         sprintf(buff + strlen(buff), "  %c", vertw[vi++]);
      }
      sprintf(buff + strlen(buff), "  %c  ║", tiles[ti++]);
      LOG_INFO(DOM) << buff;

      LOG_INFO(DOM) << "     ╚═════════════════════════════════════════════════════╝ ";
   }  
   #endif

   #if (PRINT_DEBUG_INFO && RUN_TESTS)
      void drmaPlayer::debug_PrintTestMessage(const char* msg)
      {
         LOG_WARN(DOM) << msg;
      }

      void drmaPlayer::debug_PrintTestPassed(void)
      {
         LOG_WARN(DOM) << "TEST PASSED!";
      }

      void drmaPlayer::debug_PrintTestFailed(void)
      {
         LOG_ERROR(DOM) << "TEST FAILED!";
      }

      void drmaPlayer::debug_PrintTestErrorMsg(const char* errMsg)
      {
         LOG_ERROR(DOM) << errMsg;
      }

      void drmaPlayer::debug_PrintTestMinPaths(int minPathMe, int minPathOpp)
      {
         LOG_INFO(DOM) << "  Me: " << minPathMe << ", Opponent: " << minPathOpp;
      }


   void drmaPlayer::PlaceWalls(Board_t* board, TestWall_t* walls, int8_t wallsCount)
   {
      if (walls)
      {
         for (int i = 0 ; i < wallsCount; i++)
         {
               PlaceWall(board, ME, &(board->walls[walls[i].ori][walls[i].x][walls[i].y]));
         }
      }
   }

   void drmaPlayer::UndoWalls(Board_t* board, TestWall_t* walls, int8_t wallsCount)
   {
      if (walls)
      {
         for (int i = 0 ; i < wallsCount; i++)
         {
               UndoWall(board, ME, &(board->walls[walls[i].ori][walls[i].x][walls[i].y]));
         }
      }
   }

   void drmaPlayer::CheckBoardStructure(Board_t* board, TestTileLink_t* tileLinksToTest, int8_t tileLinksCount,
                     TestWallPermission_t* permissionsToCheck, int8_t permissionsToCheckCount)
   { 
      const char* errMsg;    

      // check tile links
      bool err = false;
      for (int i = 0; i < BOARD_SZ; i++)  // go through all tiles from the board and check their links against the given links
      {
         for (int j = 0; j < BOARD_SZ; j++)
         {
               TestTileLink_t n = { board->tiles[i][j].pos.x, board->tiles[i][j].pos.y, N };
               bool found = false;
               for (int c = 0; c < tileLinksCount; c++)
               {
                  if (n.x == tileLinksToTest[c].x && n.y == tileLinksToTest[c].y && n.dir == tileLinksToTest[c].dir) 
                  {
                     found = true;
                  }
               }
               if (found && board->tiles[i][j].north != NULL) // a link from the board is not NULL but it should be
               {
                  err = true;
                  errMsg = "Found a link that should be NULL but it's not!";
                  debug_PrintTestErrorMsg(errMsg);
               }
               if (!found && board->tiles[i][j].north == NULL) // a link from the board is NULL but shouldn't be
               {
                  err = true;
                  errMsg = "Found a link that is NULL but shouldn't be!";
                  debug_PrintTestErrorMsg(errMsg);
               }

               TestTileLink_t s = { board->tiles[i][j].pos.x, board->tiles[i][j].pos.y, S };
               found = false;
               for (int c = 0; c < tileLinksCount; c++)
               {
                  if (s.x == tileLinksToTest[c].x && s.y == tileLinksToTest[c].y && s.dir == tileLinksToTest[c].dir) 
                  {
                     found = true;
                  }
               }
               if (found && board->tiles[i][j].south != NULL) // a link from the board is not NULL but it should be
               {
                  err = true;
                  errMsg = "Found a link that should be NULL but it's not!";
                  debug_PrintTestErrorMsg(errMsg);
               }
               if (!found && board->tiles[i][j].south == NULL) // a link from the board is NULL but shouldn't be
               {
                  err = true;
                  errMsg = "Found a link that is NULL but shouldn't be!";
                  debug_PrintTestErrorMsg(errMsg);
               }

               TestTileLink_t e = { board->tiles[i][j].pos.x, board->tiles[i][j].pos.y, E };
               found = false;
               for (int c = 0; c < tileLinksCount; c++)
               {
                  if (e.x == tileLinksToTest[c].x && e.y == tileLinksToTest[c].y && e.dir == tileLinksToTest[c].dir) 
                  {
                     found = true;
                  }
               }
               if (found && board->tiles[i][j].east != NULL) // a link from the board is not NULL but it should be
               {
                  err = true;
                  errMsg = "Found a link that should be NULL but it's not!";
                  debug_PrintTestErrorMsg(errMsg);
               }
               if (!found && board->tiles[i][j].east == NULL) // a link from the board is NULL but shouldn't be
               {
                  err = true;
                  errMsg = "Found a link that is NULL but shouldn't be!";
                  debug_PrintTestErrorMsg(errMsg);
               }

               TestTileLink_t w = { board->tiles[i][j].pos.x, board->tiles[i][j].pos.y, W };
               found = false;
               for (int c = 0; c < tileLinksCount; c++)
               {
                  if (w.x == tileLinksToTest[c].x && w.y == tileLinksToTest[c].y && w.dir == tileLinksToTest[c].dir) 
                  {
                     found = true;
                  }
               }
               if (found && board->tiles[i][j].west != NULL) // a link from the board is not NULL but it should be
               {
                  err = true;
                  errMsg = "Found a link that should be NULL but it's not!";
                  debug_PrintTestErrorMsg(errMsg);
               }
               if (!found && board->tiles[i][j].west == NULL) // a link from the board is NULL but shouldn't be
               {
                  err = true;
                  errMsg = "Found a link that is NULL but shouldn't be!";
                  debug_PrintTestErrorMsg(errMsg);
               }
         }
      }

         // permissions check
         for (int i = 0; i < BOARD_SZ - 1; i++)  // go through all H and V walls on the board and check their permission level against the given level
         {                                       // if there are no given permissions, check that all the levels are WALL_PERMITTED
            for (int j = 0; j < BOARD_SZ - 1; j++)
            {   
                  TestWallPermission_t toCheck;
                  bool found;

                  // H walls -------------------------------------------------------------------------------------------------------------------------------------
                  toCheck = { H, board->walls[H][i][j].pos.x, board->walls[H][i][j].pos.y, board->walls[H][i][j].permission};
                  found = false;

                  if (permissionsToCheck)
                  {
                     for (int c = 0; c < permissionsToCheckCount; c++)
                     {
                        if (toCheck.ori == permissionsToCheck[c].ori && toCheck.x == permissionsToCheck[c].x && toCheck.y == permissionsToCheck[c].y)  
                        {
                              found = true;
                              if (toCheck.permission != permissionsToCheck[c].permission) // the permission level of the wall on the board is different from the given one
                              {
                                 err = true;
                                 errMsg = "Found a permission level that is different on the board than given in the test!";
                                 debug_PrintTestErrorMsg(errMsg);
                              }
                        }
                     }
                  }

                  if (!found && toCheck.permission != WALL_PERMITTED) // there are walls on the board with a permission level different from WALL_PERMITTED, and there shouldn't be
                  {
                     err = true;
                     errMsg = "Found a FORBIDDEN permission level on the board that is not given in the test!";
                     debug_PrintTestErrorMsg(errMsg);
                  }

                  // V walls -------------------------------------------------------------------------------------------------------------------------------------
                  toCheck = { V, board->walls[V][i][j].pos.x, board->walls[V][i][j].pos.y, board->walls[V][i][j].permission};
                  found = false;

                  if (permissionsToCheck)
                  {
                     for (int c = 0; c < permissionsToCheckCount; c++)
                     {
                        if (toCheck.ori == permissionsToCheck[c].ori && toCheck.x == permissionsToCheck[c].x && toCheck.y == permissionsToCheck[c].y)  
                        {
                              found = true;
                              if (toCheck.permission != permissionsToCheck[c].permission) // the permission level of the wall on the board is different from the given one
                              {
                                 err = true;
                                 errMsg = "Found a permission level that is different on the board than given in the test!";
                                 debug_PrintTestErrorMsg(errMsg);
                              }
                        }
                     }
                  }

                  if (!found && toCheck.permission != WALL_PERMITTED) // there are walls on the board with a permission level different from WALL_PERMITTED, and there shouldn't be
                  {
                     err = true;
                     errMsg = "Found a FORBIDDEN permission level on the board that is not given in the test!";
                     debug_PrintTestErrorMsg(errMsg);
                  }
            }
         }

         if (err)
         {
            debug_PrintTestFailed();
         }
         else
         {
            debug_PrintTestPassed();
         }
   }

   void drmaPlayer::test_1_CheckInitialBoardStructure(Board_t* board)
   {
      debug_PrintTestMessage("Test 1.1:");

      // define some walls to place
      TestWall_t* wallsToPlace = NULL; // no walls placed for this test

      // define tile links that should be NULL after walls are placed
      TestTileLink_t tileLinksToTest[] =
      {
         DEFAULT_NULL_TILE_LINKS // check default tile links for this test (only links of border tiles should be NULL if no walls were placed)
      };

      // define some walls that should have the permission level NOT equal to WALL_PERMITTED
      TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
      
      PlaceWalls(board, wallsToPlace, 0);
      
      CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);        
   }


   void drmaPlayer::test_2_PlaceThenUndoOneHorizWallThatIsNotOnTheBorder(Board_t* board)
   {
      {
         debug_PrintTestMessage("Test 2.1:");

         // define some walls to place
         TestWall_t wallsToPlace[] =
         {
               { H, 1, 2 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 1, 2, S },
               { 2, 2, N },
               { 1, 3, S },
               { 2, 3, N }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { H, 1, 2, WALL_FORBIDDEN_BY_1 },
               { H, 1, 1, WALL_FORBIDDEN_BY_1 },
               { H, 1, 3, WALL_FORBIDDEN_BY_1 },
               { V, 1, 2, WALL_FORBIDDEN_BY_1 },
         };
         
         PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }
      {
         debug_PrintTestMessage("Test 2.2:");

         // define some walls to undo
         TestWall_t wallsToUndo[] =
         {
               { H, 1, 2 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS // check default tile links for this test
         };

         // define some walls that should have the permission level NOT equal to WALL_PERMITTED
         TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
         
         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
      }
   }

   void drmaPlayer::test_3_PlaceThenUndoOneVertWallThatIsNotOnTheBorder(Board_t* board)
   {
      {
         debug_PrintTestMessage("Test 3.1:");

         // define some walls to place
         TestWall_t wallsToPlace[] =
         {
               { V, 6, 2 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 6, 2, E },
               { 6, 3, W },
               { 7, 2, E },
               { 7, 3, W }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { V, 5, 2, WALL_FORBIDDEN_BY_1 },
               { V, 7, 2, WALL_FORBIDDEN_BY_1 },
               { H, 6, 2, WALL_FORBIDDEN_BY_1 },
               { V, 6, 2, WALL_FORBIDDEN_BY_1 },
         };
         
         PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }
      {
         debug_PrintTestMessage("Test 3.2:");

         // define some walls to undo
         TestWall_t wallsToUndo[] =
         {
               { V, 6, 2 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS // check default tile links for this test
         };

         // define some walls that should have the permission level NOT equal to WALL_PERMITTED
         TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
         
         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
      }
   }


   void drmaPlayer::test_4_PlaceThenUndoOneHorizWallThatIsOnTheBorder(Board_t* board)
   {
      {
         debug_PrintTestMessage("Test 4.1:");

         // define some walls to place
         TestWall_t wallsToPlace[] =
         {
               { H, 7, 0 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 7, 0, S },
               { 8, 0, N },
               { 7, 1, S },
               { 8, 1, N }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {            
               { H, 7, 1, WALL_FORBIDDEN_BY_1 },
               { V, 7, 0, WALL_FORBIDDEN_BY_1 },
               { H, 7, 0, WALL_FORBIDDEN_BY_1 },
         };
         
         PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }
      {
         debug_PrintTestMessage("Test 4.2:");

         // define some walls to undo
         TestWall_t wallsToUndo[] =
         {
               { H, 7, 0 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS // check default tile links for this test
         };

         // define some walls that should have the permission level NOT equal to WALL_PERMITTED
         TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
         
         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
      }
      {
         debug_PrintTestMessage("Test 4.3:");

         // define some walls to place
         TestWall_t wallsToPlace[] =
         {
               { H, 7, 7 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 7, 7, S },
               { 8, 7, N },
               { 7, 8, S },
               { 8, 8, N }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {            
               { H, 7, 6, WALL_FORBIDDEN_BY_1 },
               { V, 7, 7, WALL_FORBIDDEN_BY_1 },
               { H, 7, 7, WALL_FORBIDDEN_BY_1 },
         };
         
         PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }
      {
         debug_PrintTestMessage("Test 4.4:");

         // define some walls to undo
         TestWall_t wallsToUndo[] =
         {
               { H, 7, 7 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS // check default tile links for this test
         };

         // define some walls that should have the permission level NOT equal to WALL_PERMITTED
         TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
         
         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
      }
   }

   void drmaPlayer::test_5_PlaceThenUndoOneVertWallThatIsOnTheBorder(Board_t* board)
   {
      {
         debug_PrintTestMessage("Test 5.1:");

         // define some walls to place
         TestWall_t wallsToPlace[] =
         {
               { V, 0, 7 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 0, 7, E },
               { 0, 8, W },
               { 1, 7, E },
               { 1, 8, W }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { V, 1, 7, WALL_FORBIDDEN_BY_1 },
               { H, 0, 7, WALL_FORBIDDEN_BY_1 },
               { V, 0, 7, WALL_FORBIDDEN_BY_1 },
         };
         
         PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }
      {
         debug_PrintTestMessage("Test 5.2:");

         // define some walls to undo
         TestWall_t wallsToUndo[] =
         {
               { V, 0, 7 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS // check default tile links for this test
         };

         // define some walls that should have the permission level NOT equal to WALL_PERMITTED
         TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
         
         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
      }
      {
         debug_PrintTestMessage("Test 5.3:");

         // define some walls to place
         TestWall_t wallsToPlace[] =
         {
               { V, 0, 0 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 0, 0, E },
               { 0, 1, W },
               { 1, 0, E },
               { 1, 1, W }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { V, 1, 0, WALL_FORBIDDEN_BY_1 },
               { H, 0, 0, WALL_FORBIDDEN_BY_1 },
               { V, 0, 0, WALL_FORBIDDEN_BY_1 },
         };
         
         PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }
      {
         debug_PrintTestMessage("Test 5.4:");

         // define some walls to undo
         TestWall_t wallsToUndo[] =
         {
               { V, 0, 0 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS // check default tile links for this test
         };

         // define some walls that should have the permission level NOT equal to WALL_PERMITTED
         TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
         
         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
      }
   }


   void drmaPlayer::test_6_PlaceTwoConsecutiveHorizWallsAndUndoThem(Board_t* board)
   {
      {
         debug_PrintTestMessage("Test 6.1:");

         // define some walls to place
         TestWall_t wallsToPlace[] =
         {
               { H, 1, 2 },
               { H, 1, 4 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 1, 2, S },
               { 2, 2, N },
               { 1, 3, S },
               { 2, 3, N },
               { 1, 4, S },
               { 2, 4, N },
               { 1, 5, S },
               { 2, 5, N }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { H, 1, 2, WALL_FORBIDDEN_BY_1 },
               { H, 1, 1, WALL_FORBIDDEN_BY_1 },
               { H, 1, 3, WALL_FORBIDDEN_BY_2 }, // This is forbidden by both H 1 2 and H 1 4
               { V, 1, 2, WALL_FORBIDDEN_BY_1 },
               { H, 1, 4, WALL_FORBIDDEN_BY_1 },
               { H, 1, 5, WALL_FORBIDDEN_BY_1 },
               { V, 1, 4, WALL_FORBIDDEN_BY_1 },
         };

         PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));
         
         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }
      {
         debug_PrintTestMessage("Test 6.2:");

         // define some walls to undo
         TestWall_t wallsToUndo[] =
         {
               { H, 1, 4 } // Undo last wall from previous test (Undoing is done in the reverse order compared to placing)
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 1, 2, S },
               { 2, 2, N },
               { 1, 3, S },
               { 2, 3, N }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { H, 1, 2, WALL_FORBIDDEN_BY_1 },
               { H, 1, 1, WALL_FORBIDDEN_BY_1 },
               { H, 1, 3, WALL_FORBIDDEN_BY_1 },
               { V, 1, 2, WALL_FORBIDDEN_BY_1 }
         };
         
         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck)); 
      }

      {
         debug_PrintTestMessage("Test 6.3:");
         
         // define some walls to undo
         TestWall_t wallsToUndo[] =
         {
               { H, 1, 2 } // Undo the first wall from previous test (Undoing is done in the reverse order compared to placing)
         };
         
         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS // check default tile links for this test
         };

         // define some walls that should have the permission level NOT equal to WALL_PERMITTED
         TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
         
         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
      }
   }


   void drmaPlayer::test_7_Place2HorizWallsAndOneVertWallBetweenThemAndThenUndoAll(Board_t* board)
   {
      {
         debug_PrintTestMessage("Test 7.1:");

         // define some walls to place
         TestWall_t wallsToPlace[] =
         {
               { H, 1, 2 },
               { H, 1, 4 },
               { V, 1, 3 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 1, 2, S },
               { 2, 2, N },
               { 1, 3, S },
               { 2, 3, N },
               { 1, 4, S },
               { 2, 4, N },
               { 1, 5, S },
               { 2, 5, N },
               { 1, 3, E },
               { 1, 4, W },
               { 2, 3, E },
               { 2, 4, W }

         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { H, 1, 2, WALL_FORBIDDEN_BY_1 },
               { H, 1, 1, WALL_FORBIDDEN_BY_1 },
               { H, 1, 3, WALL_FORBIDDEN_BY_3 }, // This is forbidden by H 1 2 and H 1 4 and V 1 3
               { V, 1, 2, WALL_FORBIDDEN_BY_1 },
               { H, 1, 4, WALL_FORBIDDEN_BY_1 },
               { H, 1, 5, WALL_FORBIDDEN_BY_1 },
               { V, 1, 4, WALL_FORBIDDEN_BY_1 },
               { V, 1, 3, WALL_FORBIDDEN_BY_1 },
               { V, 0, 3, WALL_FORBIDDEN_BY_1 },
               { V, 2, 3, WALL_FORBIDDEN_BY_1 }
         };

         PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));
         
         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }

      {
         debug_PrintTestMessage("Test 7.2:");

         // undo last wall
         TestWall_t wallsToUndo[] =
         {
               { V, 1, 3 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 1, 2, S },
               { 2, 2, N },
               { 1, 3, S },
               { 2, 3, N },
               { 1, 4, S },
               { 2, 4, N },
               { 1, 5, S },
               { 2, 5, N }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { H, 1, 2, WALL_FORBIDDEN_BY_1 },
               { H, 1, 1, WALL_FORBIDDEN_BY_1 },
               { H, 1, 3, WALL_FORBIDDEN_BY_2 }, // This is forbidden by both H 1 2 and H 1 4
               { V, 1, 2, WALL_FORBIDDEN_BY_1 },
               { H, 1, 4, WALL_FORBIDDEN_BY_1 },
               { H, 1, 5, WALL_FORBIDDEN_BY_1 },
               { V, 1, 4, WALL_FORBIDDEN_BY_1 },
         };

         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));
         
         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }

      {
         debug_PrintTestMessage("Test 7.3:");
         
         // undo remaining walls
         TestWall_t wallsToUndo[] =
         {
               { H, 1, 4 },
               { H, 1, 2 }
         };
         
         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS // check default tile links for this test
         };

         // define some walls that should have the permission level NOT equal to WALL_PERMITTED
         TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
         
         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
      }
   }


   void drmaPlayer::test_8_Place2VertWallsAndOneHorizWallAndThenUndoAll(Board_t* board)
   {
      {
         debug_PrintTestMessage("Test 8.1:");

         // define some walls to place
         TestWall_t wallsToPlace[] =
         {
               { V, 7, 7 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 7, 7, E },
               { 7, 8, W },
               { 8, 7, E },
               { 8, 8, W }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { V, 7, 7, WALL_FORBIDDEN_BY_1 },
               { V, 6, 7, WALL_FORBIDDEN_BY_1 },
               { H, 7, 7, WALL_FORBIDDEN_BY_1 }
         };

         PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));
         
         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }

      {
         debug_PrintTestMessage("Test 8.2:");

         // define some walls to place
         TestWall_t wallsToPlace[] =
         {
               { V, 6, 6 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 7, 7, E },
               { 7, 8, W },
               { 8, 7, E },
               { 8, 8, W },
               { 6, 6, E },
               { 6, 7, W },
               { 7, 6, E },
               { 7, 7, W }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { V, 7, 7, WALL_FORBIDDEN_BY_1 },
               { V, 6, 7, WALL_FORBIDDEN_BY_1 },
               { H, 7, 7, WALL_FORBIDDEN_BY_1 },
               { V, 6, 6, WALL_FORBIDDEN_BY_1 },
               { V, 7, 6, WALL_FORBIDDEN_BY_1 },
               { H, 6, 6, WALL_FORBIDDEN_BY_1 },
               { V, 5, 6, WALL_FORBIDDEN_BY_1 },
         };

         PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));
         
         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }

      {
         debug_PrintTestMessage("Test 8.3:");

         // define some walls to place
         TestWall_t wallsToPlace[] =
         {
               { H, 7, 6 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 7, 7, E },
               { 7, 8, W },
               { 8, 7, E },
               { 8, 8, W },
               { 6, 6, E },
               { 6, 7, W },
               { 7, 6, E },
               { 7, 7, W },
               { 7, 6, S },
               { 8, 6, N },
               { 7, 7, S },
               { 8, 7, N }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { V, 7, 7, WALL_FORBIDDEN_BY_1 },
               { V, 6, 7, WALL_FORBIDDEN_BY_1 },
               { H, 7, 7, WALL_FORBIDDEN_BY_2 },
               { V, 6, 6, WALL_FORBIDDEN_BY_1 },
               { V, 7, 6, WALL_FORBIDDEN_BY_2 },
               { H, 6, 6, WALL_FORBIDDEN_BY_1 },
               { V, 5, 6, WALL_FORBIDDEN_BY_1 },
               { H, 7, 6, WALL_FORBIDDEN_BY_1 },
               { H, 7, 5, WALL_FORBIDDEN_BY_1 },
         };

         PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));
         
         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }

      {
         debug_PrintTestMessage("Test 8.4:");

         // Undo last wall
         TestWall_t wallsToUndo[] =
         {
               { H, 7, 6 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 7, 7, E },
               { 7, 8, W },
               { 8, 7, E },
               { 8, 8, W },
               { 6, 6, E },
               { 6, 7, W },
               { 7, 6, E },
               { 7, 7, W }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { V, 7, 7, WALL_FORBIDDEN_BY_1 },
               { V, 6, 7, WALL_FORBIDDEN_BY_1 },
               { H, 7, 7, WALL_FORBIDDEN_BY_1 },
               { V, 6, 6, WALL_FORBIDDEN_BY_1 },
               { V, 7, 6, WALL_FORBIDDEN_BY_1 },
               { H, 6, 6, WALL_FORBIDDEN_BY_1 },
               { V, 5, 6, WALL_FORBIDDEN_BY_1 },
         };

         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));
         
         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }

      {
         debug_PrintTestMessage("Test 8.5:");

         // Undo wall that was placed second
         TestWall_t wallsToUndo[] =
         {
               { V, 6, 6 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 7, 7, E },
               { 7, 8, W },
               { 8, 7, E },
               { 8, 8, W }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { V, 7, 7, WALL_FORBIDDEN_BY_1 },
               { V, 6, 7, WALL_FORBIDDEN_BY_1 },
               { H, 7, 7, WALL_FORBIDDEN_BY_1 }
         };

         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));
         
         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }

      {
         debug_PrintTestMessage("Test 8.6:");

         // undo first wall
         TestWall_t wallsToUndo[] =
         {
               { V, 7, 7 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS // check default tile links for this test
         };

         // define some walls that should have the permission level NOT equal to WALL_PERMITTED
         TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
         
         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
      }
   }

   void drmaPlayer::test_9_PlaceAndUndoGroupsOf3Walls(Board_t* board)
   {
      {
         debug_PrintTestMessage("Test 9.1:");

         // define some walls to place
         TestWall_t wallsToPlace[] =
         {
               { H, 7, 0 },
               { V, 6, 0 },
               { H, 5, 1 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 7, 0, S },
               { 8, 0, N },
               { 7, 1, S },
               { 8, 1, N },
               { 6, 0, E },
               { 6, 1, W },
               { 7, 0, E },
               { 7, 1, W },
               { 5, 1, S },
               { 6, 1, N },
               { 5, 2, S },
               { 6, 2, N }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { H, 7, 0, WALL_FORBIDDEN_BY_1 },
               { H, 7, 1, WALL_FORBIDDEN_BY_1 },
               { V, 7, 0, WALL_FORBIDDEN_BY_2 },
               { V, 5, 0, WALL_FORBIDDEN_BY_1 },
               { V, 6, 0, WALL_FORBIDDEN_BY_1 },
               { H, 5, 0, WALL_FORBIDDEN_BY_1 },
               { H, 5, 1, WALL_FORBIDDEN_BY_1 },
               { H, 5, 2, WALL_FORBIDDEN_BY_1 },
               { V, 5, 1, WALL_FORBIDDEN_BY_1 },
               { H, 6, 0, WALL_FORBIDDEN_BY_1 }
         };

         PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));
         
         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }

      {
         debug_PrintTestMessage("Test 9.2:");

         // define some walls to place
         TestWall_t wallsToPlace[] =
         {
               { H, 4, 3 },
               { H, 5, 3 },
               { V, 6, 2 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 7, 0, S },
               { 8, 0, N },
               { 7, 1, S },
               { 8, 1, N },
               { 6, 0, E },
               { 6, 1, W },
               { 7, 0, E },
               { 7, 1, W },
               { 5, 1, S },
               { 6, 1, N },
               { 5, 2, S },
               { 6, 2, N },
               { 4, 3, S },
               { 5, 3, N },
               { 4, 4, S },
               { 5, 4, N },
               { 5, 3, S },
               { 6, 3, N },
               { 5, 4, S },
               { 6, 4, N },
               { 6, 2, E },
               { 6, 3, W },
               { 7, 2, E },
               { 7, 3, W }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { H, 7, 0, WALL_FORBIDDEN_BY_1 },
               { H, 7, 1, WALL_FORBIDDEN_BY_1 },
               { V, 7, 0, WALL_FORBIDDEN_BY_2 },
               { V, 5, 0, WALL_FORBIDDEN_BY_1 },
               { V, 6, 0, WALL_FORBIDDEN_BY_1 },
               { H, 5, 0, WALL_FORBIDDEN_BY_1 },
               { H, 5, 1, WALL_FORBIDDEN_BY_1 },
               { H, 5, 2, WALL_FORBIDDEN_BY_2 },
               { V, 5, 1, WALL_FORBIDDEN_BY_1 },
               { H, 6, 0, WALL_FORBIDDEN_BY_1 },
               { H, 4, 3, WALL_FORBIDDEN_BY_1 },
               { V, 4, 3, WALL_FORBIDDEN_BY_1 },
               { H, 4, 2, WALL_FORBIDDEN_BY_1 },
               { H, 4, 4, WALL_FORBIDDEN_BY_1 },
               { H, 5, 3, WALL_FORBIDDEN_BY_1 },
               { H, 5, 4, WALL_FORBIDDEN_BY_1 },
               { V, 5, 3, WALL_FORBIDDEN_BY_1 },
               { V, 7, 2, WALL_FORBIDDEN_BY_1 },
               { V, 5, 2, WALL_FORBIDDEN_BY_1 },
               { H, 6, 2, WALL_FORBIDDEN_BY_1 },
               { V, 6, 2, WALL_FORBIDDEN_BY_1 }
         };

         PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));
         
         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
      }

      {
         debug_PrintTestMessage("Test 9.3:");

         // undo walls from 9.2 in reverse order
         TestWall_t wallsToUndo[] =
         {
               { V, 6, 2 },
               { H, 5, 3 },
               { H, 4, 3 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS,
               { 7, 0, S },
               { 8, 0, N },
               { 7, 1, S },
               { 8, 1, N },
               { 6, 0, E },
               { 6, 1, W },
               { 7, 0, E },
               { 7, 1, W },
               { 5, 1, S },
               { 6, 1, N },
               { 5, 2, S },
               { 6, 2, N }
         };

         // define some walls that should be flagged as forbidden after the placement of walls above
         TestWallPermission_t permissionsToCheck[] =
         {
               { H, 7, 0, WALL_FORBIDDEN_BY_1 },
               { H, 7, 1, WALL_FORBIDDEN_BY_1 },
               { V, 7, 0, WALL_FORBIDDEN_BY_2 },
               { V, 5, 0, WALL_FORBIDDEN_BY_1 },
               { V, 6, 0, WALL_FORBIDDEN_BY_1 },
               { H, 5, 0, WALL_FORBIDDEN_BY_1 },
               { H, 5, 1, WALL_FORBIDDEN_BY_1 },
               { H, 5, 2, WALL_FORBIDDEN_BY_1 },
               { V, 5, 1, WALL_FORBIDDEN_BY_1 },
               { H, 6, 0, WALL_FORBIDDEN_BY_1 }
         };

         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));
         
         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));

      }

      {
         debug_PrintTestMessage("Test 9.4:");

         // undo walls from 9.1 in reverse order
         TestWall_t wallsToUndo[] =
         {
               { H, 5, 1 },
               { V, 6, 0 },
               { H, 7, 0 }
         };

         // define tile links that should be NULL after walls are placed
         TestTileLink_t tileLinksToTest[] =
         {
               DEFAULT_NULL_TILE_LINKS // check default tile links for this test
         };

         // define some walls that should have the permission level NOT equal to WALL_PERMITTED
         TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
         
         UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

         CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);

      }
   }

   void drmaPlayer::StringToMap(const char* stringInput, char* mapOutput, int8_t* myPosX, int8_t* myPosY, int8_t* oppPosX, int8_t* oppPosY)
   {
      int n = 0, i = 0, j = 0;

      // copy const string to char array for tokenizing
      char mapString[1000];
      strcpy(mapString, stringInput);
      mapString[strlen(stringInput) + 1] = 0;

      char* token = strtok(mapString, ",");
      n++;

      while (token != NULL) 
      {
         mapOutput[i * 17 + j] = token[0];

         if (mapOutput[i * 17 + j] == '0') 
         {
               *myPosX = i;
               *myPosY = j;
         }

         if (mapOutput[i * 17 + j] == '1')
         {
               *oppPosX = i;
               *oppPosY = j;
         }

         j++;

         if (n % 17 == 0)
         {
               i++;
               j = 0;
         }

         token = strtok(NULL, ",");
         n++;
      }
   }


   int drmaPlayer::TestGetMinPathToTargetTile(char* map, int playerX, int playerY, int targetX, int targetY)
   {
      int distances[17][17] = { 0 };
      distances[playerX][playerY] = 1;
      int scan = 0;
      bool noPath;


      while (1)
      {
         int i, j;
         scan++;
         noPath = true;

         for (i = 0; i < 17; i += 2)
         {
               for (j = 0; j < 17; j += 2)
               {
                  if (distances[i][j] == 0)
                  {   
                     if (i - 2 >= 0 && map[(i - 1) * 17 + j] != '=' && distances[i - 2][j] == scan)
                     {
                           distances[i][j] = distances[i - 2][j] + 1;
                           noPath = false;
                           if (targetX == i && targetY == j)
                           {
                              return (distances[i][j] - 1);
                           }
                     }
                     else if (i + 2 <= 16 && map[(i + 1) * 17 + j] != '=' && distances[i + 2][j] == scan)
                     {
                           distances[i][j] = distances[i + 2][j] + 1;
                           noPath = false;
                           if (targetX == i && targetY == j)
                           {
                              return (distances[i][j] - 1);
                           }
                     }
                     else if (j - 2 >= 0 && map[i * 17 + (j - 1)] != '|' && distances[i][j - 2] == scan)
                     {
                           distances[i][j] = distances[i][j - 2] + 1;
                           noPath = false;
                           if (targetX == i && targetY == j)
                           {
                              return (distances[i][j] - 1);
                           }
                     }
                     else if (j + 2 <= 16 && map[i* 17 + (j + 1)] != '|' && distances[i][j + 2] == scan)
                     {
                           distances[i][j] = distances[i][j + 2] + 1;
                           noPath = false;
                           if (targetX == i && targetY == j)
                           {
                              return (distances[i][j] - 1);
                           }
                     }          
                  }
               }
         }

         if (noPath)
         {
               return 0xFFFF;
         }
      }
   }


   int drmaPlayer::TestGetMinPathForMe(char* map, int myX, int myY)
   {
      int min = 0xFFFF;

      for (int j = 0; j < 17; j += 2)
      {
         int tempMin = TestGetMinPathToTargetTile(map, myX, myY, 0, j);
         if (min > tempMin)
         {
               min = tempMin;
         }
      }

      return min;
   }

   int drmaPlayer::TestGetMinPathForOpp(char* map, int oppX, int oppY)
   {
      int min = 0xFFFF;

      for (int j = 0; j < 17; j += 2)
      {
         int tempMin = TestGetMinPathToTargetTile(map, oppX, oppY, 16, j);
         if (min > tempMin)
         {
               min = tempMin;
         }
      }

      return min;
   }

   void drmaPlayer::MapToBoard(char* map, Board_t* board, int8_t myMapPosX, int8_t myMapPosY, int8_t oppMapPosX, int8_t oppMapPosY)
   {
      for (int i = 0; i < 17; i += 2)
      {
         for (int j = 0; j < 17; j += 2)
         {
               Tile_t* tile = &(board->tiles[i / 2][j / 2]);

               if ((j <= 14) && map[i * 17 + (j + 1)] == '|') // there is a wall to the east of the current tile
               {
                  // remove horizontal links
                  tile->east->west = NULL;
                  tile->east = NULL;                
               }

               if ((i <= 14) && map[(i + 1) * 17 + j] == '=') // there is a wall to the south of the current tile
               {
                  // remove vertical links
                  tile->south->north = NULL;
                  tile->south = NULL;                
               }
         }
      }

      board->playerPos[ME] = { (int8_t)(myMapPosX / 2), (int8_t)(myMapPosY / 2)};
      board->playerPos[OPPONENT] = { (int8_t)(oppMapPosX / 2), (int8_t)(oppMapPosY / 2)};
   }

   bool drmaPlayer::IsMinPathAndPossibleMovesTestPassed(const char* stringInput, const char* possibleMovesMeInput, const char* possibleMovesOppInput)
   {
      int8_t myMapPosX;
      int8_t myMapPosY;
      int8_t oppMapPosX;
      int8_t oppMapPosY;
      
      // convert input to a 17 x 17 char map and get player positions
      char map[17][17] = { 0 };
      StringToMap(stringInput, (char*)map, &myMapPosX, &myMapPosY, &oppMapPosX, &oppMapPosY);

      // create a new board
      Board_t* testBoard = NewDefaultBoard();

      // Update the new board with the given test configuration
      MapToBoard((char*)map, testBoard, myMapPosX, myMapPosY, oppMapPosX, oppMapPosY);

      // Get min path the test way
      debug_PrintTestMessage("  Min path Test:");
      int minPathMeTest = TestGetMinPathForMe((char*)map, myMapPosX, myMapPosY);
      int minPathOppTest = TestGetMinPathForOpp((char*)map, oppMapPosX, oppMapPosY);
      debug_PrintTestMinPaths(minPathMeTest, minPathOppTest);

      // Get min path the plugin way
      bool found;
      debug_PrintTestMessage("  Min path Plugin:");
      int minPathMePlugin = FindMinPathLen(testBoard, ME, &found);
      int minPathOppPlugin = FindMinPathLen(testBoard, OPPONENT, &found);
      debug_PrintTestMinPaths(minPathMePlugin, minPathOppPlugin);

      // Update possible moves for both
      UpdatePossibleMoves(testBoard, ME);
      UpdatePossibleMoves(testBoard, OPPONENT);

      bool ret = true;

      // Print possible moves from test
      debug_PrintTestMessage("  Possible Moves Test:");
      debug_PrintTestMessage(possibleMovesMeInput);
      debug_PrintTestMessage(possibleMovesOppInput);

      // Print possible moves from plugin
      debug_PrintTestMessage("  Possible Moves Plugin:");

      if (strcmp(debug_PrintMyPossibleMoves(testBoard), possibleMovesMeInput) != 0 ||
         strcmp(debug_PrintOppPossibleMoves(testBoard), possibleMovesOppInput) != 0 ||
         (minPathMePlugin != minPathMeTest) || 
               (minPathOppPlugin != minPathOppTest))
               {
                  ret = false;
               }

      free(testBoard);

      // report
      if (ret)
      {
         return true;
      }

      return false;
   }

   void drmaPlayer::test_10_MinPathAndPossibleMoves(void)
   {
      debug_PrintTestMessage("Test 10: Min path and possible moves");

      const char* stringInput = 
      ", ,., ,., ,., ,.,1,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,.,0,., ,., ,., ,., ,";

      const char* possibleMovesMeInput = "  My possible moves: [M-N],[M-E],[M-W],";
      const char* possibleMovesOppInput = "  Opp possible moves: [M-S],[M-E],[M-W],";
      

      if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }
   }

   void drmaPlayer::test_11_MinPathAndPossibleMoves(void)
   {
      debug_PrintTestMessage("Test 11: Min path and possible moves");

      const char* stringInput = 
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,.,1,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,.,0,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,=,=,=,.,=,=,=,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

      const char* possibleMovesMeInput = "  My possible moves: [M-N],[M-E],[M-W],";
      const char* possibleMovesOppInput = "  Opp possible moves: [M-N],[M-S],[M-E],[M-W],";
      

      if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }
   }


   void drmaPlayer::test_12_MinPathAndPossibleMoves(void)
   {
      debug_PrintTestMessage("Test 12: Min path and possible moves");

      const char* stringInput = 
      ", ,., ,., ,|, ,., ,|, ,., ,., ,., ,"
      ",.,.,.,.,.,|,.,.,.,|,.,.,.,.,.,.,.,"
      ", ,., ,., ,|, ,., ,|, ,., ,., ,., ,"
      ",.,.,.,.,.,.,=,=,=,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",=,=,=,.,=,=,=,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,=,=,=,.,=,=,=,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,.,1,|, ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,|,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,.,0,|, ,., ,., ,., ,"
      ",.,.,.,.,.,.,=,=,=,.,=,=,=,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

      const char* possibleMovesMeInput = "  My possible moves: [M-W],[J-N-W],";
      const char* possibleMovesOppInput = "  Opp possible moves: [M-W],[J-S-W],";
      

      if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }
   }

   void drmaPlayer::test_13_MinPathAndPossibleMoves(void)
   {
      debug_PrintTestMessage("Test 13: Min path and possible moves");

      const char* stringInput = 
      ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,|,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,|,0,.,1,., ,., ,., ,., ,"
      ",.,.,=,=,=,.,.,.,.,.,=,=,=,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,|,.,.,.,|,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,=,=,=,.,=,=,=,.,.,"
      ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

      const char* possibleMovesMeInput = "  My possible moves: [M-N],[M-S],[J-E],";
      const char* possibleMovesOppInput = "  Opp possible moves: [M-N],[M-S],[M-E],[J-N-W],[J-S-W],";
      

      if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }
   }


   void drmaPlayer::test_14_MinPathAndPossibleMoves(void)
   {
      debug_PrintTestMessage("Test 14: Min path and possible moves");

      const char* stringInput = 
      ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,|,=,=,=,.,=,=,=,.,=,=,=,"
      ", ,., ,., ,|,0,.,1,., ,., ,., ,., ,"
      ",.,.,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,|,.,.,.,|,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,=,=,=,.,=,=,=,.,.,"
      ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

      const char* possibleMovesMeInput = "  My possible moves: [J-E],";
      const char* possibleMovesOppInput = "  Opp possible moves: [M-E],";
      

      if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }
   }


   void drmaPlayer::test_15_MinPathAndPossibleMoves(void)
   {
      debug_PrintTestMessage("Test 15: Min path and possible moves");

      const char* stringInput = 
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",=,=,=,.,=,=,=,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,.,0,.,1,|, ,., ,., ,., ,"
      ",.,.,=,=,=,.,.,.,.,|,=,=,=,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,|, ,., ,., ,., ,"
      ",.,|,.,.,.,|,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,=,=,=,.,=,=,=,.,.,"
      ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

      const char* possibleMovesMeInput = "  My possible moves: [M-S],[M-W],[J-N-E],[J-S-E],";
      const char* possibleMovesOppInput = "  Opp possible moves: [M-N],[M-S],[J-W],";
      

      if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }
   }


   void drmaPlayer::test_16_MinPathAndPossibleMoves(void)
   {
      debug_PrintTestMessage("Test 16: Min path and possible moves");

      const char* stringInput = 
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,|,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
      ", ,|, ,., ,.,0,., ,|, ,., ,., ,., ,"
      ",.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,.,.,"
      ", ,|, ,., ,.,1,., ,|, ,|, ,., ,., ,"
      ",.,|,.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,|, ,., ,., ,"
      ",.,.,.,.,.,.,.,.,=,=,=,.,=,=,=,.,.,"
      ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

      const char* possibleMovesMeInput = "  My possible moves: [M-E],[M-W],[J-S-E],[J-S-W],";
      const char* possibleMovesOppInput = "  Opp possible moves: [M-E],[M-W],[J-N-E],[J-N-W],";
      

      if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }
   }

   void drmaPlayer::test_17_MinPathAndPossibleMoves(void)
   {
      debug_PrintTestMessage("Test 17: Min path and possible moves");

      const char* stringInput = 
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,|,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
      ", ,|, ,., ,., ,., ,|, ,., ,., ,., ,"
      ",.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,.,0,|, ,|, ,., ,., ,"
      ",.,|,.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,"
      ", ,|, ,., ,., ,.,1,., ,|, ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,=,=,=,.,.,"
      ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

      const char* possibleMovesMeInput = "  My possible moves: [M-N],[M-W],[J-S],";
      const char* possibleMovesOppInput = "  Opp possible moves: [M-S],[M-E],[M-W],[J-N],";
      

      if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }
   }

   void drmaPlayer::test_18_MinPathAndPossibleMoves(void)
   {
      debug_PrintTestMessage("Test 18: Min path and possible moves");

      const char* stringInput = 
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,|,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
      ", ,|, ,., ,., ,., ,|, ,., ,., ,., ,"
      ",.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,|, ,|, ,., ,., ,"
      ",.,|,.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,|, ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,=,=,=,.,.,"
      ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,.,0,.,1,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

      const char* possibleMovesMeInput = "  My possible moves: [M-N],[M-S],[M-W],[J-N-E],[J-S-E],";
      const char* possibleMovesOppInput = "  Opp possible moves: [M-N],[M-S],[J-W],";
      

      if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }
   }


   void drmaPlayer::test_19_MinPathAndPossibleMoves(void)
   {
      debug_PrintTestMessage("Test 19: Min path and possible moves");

      const char* stringInput = 
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,|,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
      ", ,|, ,., ,., ,., ,|, ,., ,., ,., ,"
      ",.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,|, ,|, ,., ,., ,"
      ",.,|,.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,|, ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,=,=,=,.,.,"
      ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ",1,.,0,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

      const char* possibleMovesMeInput = "  My possible moves: [M-N],[M-S],[M-E],[J-N-W],[J-S-W],";
      const char* possibleMovesOppInput = "  Opp possible moves: [M-N],[M-S],[J-E],";
      

      if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }
   }


   void drmaPlayer::test_20_MinPathAndPossibleMoves(void)
   {
      debug_PrintTestMessage("Test 20: Min path and possible moves");

      const char* stringInput = 
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,|,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
      ", ,|, ,., ,., ,., ,|, ,., ,., ,., ,"
      ",.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,|, ,|, ,., ,., ,"
      ",.,|,.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,"
      ", ,|, ,., ,.,1,.,0,., ,|, ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,=,=,=,.,.,"
      ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

      const char* possibleMovesMeInput = "  My possible moves: [M-N],[M-S],[M-E],[J-W],";
      const char* possibleMovesOppInput = "  Opp possible moves: [M-S],[M-W],[J-E],";
      

      if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }
   }


   void drmaPlayer::test_21_MinPathAndPossibleMoves(void)
   {
      debug_PrintTestMessage("Test 21: Min path and possible moves");

      const char* stringInput = 
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,|,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
      ", ,|, ,., ,., ,., ,|, ,., ,., ,., ,"
      ",.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,|, ,|, ,., ,., ,"
      ",.,|,.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,|, ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,=,=,=,.,.,"
      ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,.,1,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,.,0,., ,., ,";

      const char* possibleMovesMeInput = "  My possible moves: [M-E],[M-W],[J-N],";
      const char* possibleMovesOppInput = "  Opp possible moves: [M-N],[M-E],[M-W],[J-S-E],[J-S-W],";
      

      if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }
   }


   void drmaPlayer::test_22_MinPathAndPossibleMoves(void)
   {
      debug_PrintTestMessage("Test 22: Min path and possible moves");

      const char* stringInput = 
      ", ,|, ,., ,., ,., ,., ,., ,.,1,., ,"
      ",.,|,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
      ", ,|, ,., ,., ,., ,|, ,., ,.,0,., ,"
      ",.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,|, ,|, ,., ,., ,"
      ",.,|,.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,|, ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,=,=,=,.,.,"
      ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
      ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
      ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
      ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

      const char* possibleMovesMeInput = "  My possible moves: [M-S],[M-E],[M-W],[J-N-E],[J-N-W],";
      const char* possibleMovesOppInput = "  Opp possible moves: [M-E],[M-W],[J-S],";
      

      if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }
   }


   void drmaPlayer::DuplicateBoard(Board_t* origBoard, Board_t* copyBoard)
   {
      // memcopy what doesn't contain pointers (and can change)
      memcpy(copyBoard->playerPos, origBoard->playerPos, sizeof(origBoard->playerPos));
      memcpy(copyBoard->wallsLeft, origBoard->wallsLeft, sizeof(origBoard->wallsLeft));
      memcpy(copyBoard->moves[0], origBoard->moves[0], sizeof(origBoard->moves[0]));
      memcpy(copyBoard->moves[1], origBoard->moves[1], sizeof(origBoard->moves[1]));

      // go through all tiles and copy the NULL pointers only
      for (int i = 0; i < BOARD_SZ; i++)
      {
         for (int j = 0; j < BOARD_SZ; j++)
         {
               Tile_t* tileCopy = &(copyBoard->tiles[i][j]);
               Tile_t* tileOrig = &(origBoard->tiles[i][j]);

               if (tileOrig->north == NULL)
               {
                  tileCopy->north = NULL;
               }

               if (tileOrig->south == NULL)
               {
                  tileCopy->south = NULL;
               }

               if (tileOrig->east == NULL)
               {
                  tileCopy->east = NULL;
               }

               if (tileOrig->west == NULL)
               {
                  tileCopy->west = NULL;
               }
         }
      }

      // copy wall permissions
      for (int o = H; o <= V; o++)
      {
         for (int i = 0; i < BOARD_SZ - 1; i++)
         {
               for (int j = 0; j < BOARD_SZ - 1; j++)
               {
                  Wall_t* wallOrig = &(origBoard->walls[o][i][j]);
                  Wall_t* wallCopy = &(copyBoard->walls[o][i][j]);

                  wallCopy->permission = wallOrig->permission;
               }
         }
      }
   }

   bool drmaPlayer::AreBoardsEqual(Board_t* b1, Board_t* b2)
   {
      bool ret = true;

      if (memcmp(b1->playerPos, b2->playerPos, sizeof(b1->playerPos)) != 0)
      {
         ret = false;
      }

      if (memcmp(b1->wallsLeft, b2->wallsLeft, sizeof(b1->wallsLeft)) != 0)
      {
         ret = false;
      }

      if (memcmp(b1->moves[0], b2->moves[0], sizeof(b1->moves[0])) != 0)
      {
         ret = false;
         debug_PrintTestMessage("moves!");
      }

      if (memcmp(b1->moves[1], b2->moves[1], sizeof(b1->moves[1])) != 0)
      {
         ret = false;
         debug_PrintTestMessage("moves!");
      }

      for (int i = 0; i < BOARD_SZ; i++)
      {
         for (int j = 0; j < BOARD_SZ; j++)
         {
               Tile_t* t1N = b1->tiles[i][j].north;
               Tile_t* t1S = b1->tiles[i][j].south;
               Tile_t* t1E = b1->tiles[i][j].east;
               Tile_t* t1W = b1->tiles[i][j].west;

               Tile_t* t2N = b2->tiles[i][j].north;
               Tile_t* t2S = b2->tiles[i][j].south;
               Tile_t* t2E = b2->tiles[i][j].east;
               Tile_t* t2W = b2->tiles[i][j].west;

               bool oldRet = ret;

               if (t1N == NULL && t2N != NULL) ret = false;
               if (t2N == NULL && t1N != NULL) ret = false;
               if (t1E == NULL && t2E != NULL) ret = false;
               if (t2E == NULL && t1E != NULL) ret = false;
               if (t1W == NULL && t2W != NULL) ret = false;
               if (t2W == NULL && t1W != NULL) ret = false;
               if (t1S == NULL && t2S != NULL) ret = false;
               if (t2S == NULL && t1S != NULL) ret = false;

               if (oldRet && !ret)
               {
                  debug_PrintTestMessage("tiles!");
               }
         }
      }

      for (int o = H; o <= V; o++)
      {
         for (int i = 0; i < BOARD_SZ - 1; i++)
         {
               for (int j = 0; j < BOARD_SZ - 1; j++)
               {
                  Wall_t* w1 = &(b1->walls[o][i][j]);
                  Wall_t* w2 = &(b2->walls[o][i][j]);

                  if(w1->permission != w2->permission)
                  {
                     ret = false;
                  }
               }
         }
      }

      return ret;
   }

   bool drmaPlayer::IsRecursiveRunOkForTestPossibleMoves(Board_t* board, Board_t* referenceBoard, uint8_t level)
   {
      bool ret = true;

      UpdatePossibleMoves(board, ME);
      UpdatePossibleMoves(referenceBoard, ME);

      if (level == 0)
      {
         if (!AreBoardsEqual(board, referenceBoard))
         {
               debug_PrintTestMessage("Deepest level Before Upd: Boards not equal and should be!");
               ret = false;
         }
      }
      else
      {
         if (level == 1)
         {
               if (!AreBoardsEqual(board, referenceBoard))
               {
                  debug_PrintTestMessage("LVL 1: Boards not equal and should be!");
                  ret = false;
               }
         }

         // go through walls
         for (int o = H; o <= V; o++)
         {
               for (int i = 0; i < BOARD_SZ - 1; i++)
               {
                  for (int j = 0; j < BOARD_SZ - 1; j++)
                  {
                     Wall_t* wall = &(board->walls[o][i][j]);

                     if (wall->permission == WALL_PERMITTED)
                     {
                           // make a copy of the reference board
                           Board_t* boardTemp = NewDefaultBoard();
                           DuplicateBoard(referenceBoard, boardTemp);

                           if (!AreBoardsEqual(board, boardTemp))
                           {
                              debug_PrintTestMessage("After second duplication: Boards not equal and should be!");
                              ret = false;
                           }

                           Wall_t* refWall = &(boardTemp->walls[o][i][j]);

                           PlaceWall(board, ME, wall);
                           PlaceWall(boardTemp, ME, refWall);

                           if (!AreBoardsEqual(board, boardTemp))
                           {
                              debug_PrintTestMessage("After wall placement: Boards not equal and should be!");
                              ret = false;
                           }

                           if (!IsRecursiveRunOkForTestPossibleMoves(board, boardTemp, level - 1))
                           {
                              ret = false;
                           }

                           UndoWall(board, ME, wall);

                           // If this next line is missing: on a deeper level the possible moves are updated
                           // on the board to a different state that is not applicable on this level.
                           UpdatePossibleMoves(board, ME);


                           if (!AreBoardsEqual(board, referenceBoard))
                           {
                              debug_PrintTestMessage("After wall undo: Boards not equal and should be!");
                              ret = false;
                           }

                           free(boardTemp);
                     }
                  }
               }
         }

         // go through moves
         for (int moveID = MOVE_FIRST; moveID <= MOVE_LAST; moveID++)
         {
               if (board->moves[ME][moveID].isPossible)
               {
                  // make a copy of the reference board
                  Board_t* boardTemp = NewDefaultBoard();
                  DuplicateBoard(referenceBoard, boardTemp);

                  if (!AreBoardsEqual(board, boardTemp))
                  {
                     debug_PrintTestMessage("After second duplication (moves): Boards not equal and should be!");
                     ret = false;
                  }

                  MakeMove(board, ME, (MoveID_t)moveID);
                  MakeMove(boardTemp, ME, (MoveID_t)moveID);

                  if (!AreBoardsEqual(board, boardTemp))
                  {
                     debug_PrintTestMessage("After move making: Boards not equal and should be!");
                     ret = false;
                  }

                  if(!IsRecursiveRunOkForTestPossibleMoves(board, boardTemp, level - 1))
                  {
                     ret = false;
                  }

                  UndoMove(board, ME, (MoveID_t)moveID);

                  // If this next line is missing: on a deeper level the possible moves are updated
                  // on the board to a different state that is not applicable on this level.
                  UpdatePossibleMoves(board, ME);

                  if (!AreBoardsEqual(board, referenceBoard))
                  {
                     debug_PrintTestMessage("After move undo: Boards not equal and should be!");
                     ret = false;
                  }


                  free(boardTemp);
               }
         }
      }

      return ret;
   }


   void drmaPlayer::test_23_TestPossibleMovesRecursiveCorrectnessDefaultPlayerPos(Board_t* board, uint8_t level)
   {
      debug_PrintTestMessage("Test 23: Possible moves recursive correctness (default player positions)");

      // create a copy of the board
      Board_t* boardCopy = NewDefaultBoard();
      DuplicateBoard(board, boardCopy);

      if (!AreBoardsEqual(board, boardCopy))
      {
         debug_PrintTestMessage("After creation: Boards not equal and should be!");
      }

      if (IsRecursiveRunOkForTestPossibleMoves(board, boardCopy, level))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }

      free(boardCopy);
   }


   void drmaPlayer::test_24_TestPossibleMovesRecursiveCorrectnessDifferentPlayerPos(Board_t* board, uint8_t level)
   {
      debug_PrintTestMessage("Test 24: Possible moves recursive correctness (modified player positions)");

      // create a copy of the board
      Board_t* boardCopy = NewDefaultBoard();
      DuplicateBoard(board, boardCopy);

      UpdatePos(board, ME, {5, 4});
      UpdatePos(board, OPPONENT, {3, 4});

      UpdatePos(boardCopy, ME, {5, 4});
      UpdatePos(boardCopy, OPPONENT, {3, 4});

      if (!AreBoardsEqual(board, boardCopy))
      {
         debug_PrintTestMessage("After creation: Boards not equal and should be!");
      }

      if (IsRecursiveRunOkForTestPossibleMoves(board, boardCopy, level))
      {
         debug_PrintTestPassed();
      }
      else
      {
         debug_PrintTestFailed();
      }

      free(boardCopy);
   }

   void drmaPlayer::RunAllTests(Board_t* board)
   {
      test_1_CheckInitialBoardStructure(board);
      test_2_PlaceThenUndoOneHorizWallThatIsNotOnTheBorder(board);
      test_3_PlaceThenUndoOneVertWallThatIsNotOnTheBorder(board);
      test_4_PlaceThenUndoOneHorizWallThatIsOnTheBorder(board);
      test_5_PlaceThenUndoOneVertWallThatIsOnTheBorder(board);
      test_6_PlaceTwoConsecutiveHorizWallsAndUndoThem(board);
      test_7_Place2HorizWallsAndOneVertWallBetweenThemAndThenUndoAll(board);
      test_8_Place2VertWallsAndOneHorizWallAndThenUndoAll(board);
      test_9_PlaceAndUndoGroupsOf3Walls(board);
      test_10_MinPathAndPossibleMoves();
      test_11_MinPathAndPossibleMoves();
      test_12_MinPathAndPossibleMoves();
      test_13_MinPathAndPossibleMoves();
      test_14_MinPathAndPossibleMoves();
      test_15_MinPathAndPossibleMoves();
      test_16_MinPathAndPossibleMoves();
      test_17_MinPathAndPossibleMoves();
      test_18_MinPathAndPossibleMoves();
      test_19_MinPathAndPossibleMoves();
      test_20_MinPathAndPossibleMoves();
      test_21_MinPathAndPossibleMoves();
      test_22_MinPathAndPossibleMoves();
      test_23_TestPossibleMovesRecursiveCorrectnessDefaultPlayerPos(board, 2);
      test_24_TestPossibleMovesRecursiveCorrectnessDifferentPlayerPos(board, 2);
   }
   #endif

} // end namespace
