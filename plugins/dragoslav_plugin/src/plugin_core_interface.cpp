#include "plugin_core_interface.h"

namespace qplugin
{
    qcore::Position CoreAbsToRelWallPos(qcore::Position absPos, qcore::Orientation orientation)   
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


    Position_t CoreToPluginWallPos(qcore::Position coreWallPos, qcore::Orientation orientation)
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


    qcore::Position PluginToCoreWallPos(Position_t pluginWallPos, Orientation_t orientation)
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


    Orientation_t CoreToPluginWallOrientation(qcore::Orientation orientation)
    {
        return (orientation == qcore::Orientation::Horizontal ? H : V);
    }


    qcore::Orientation PluginToCoreWallOrientation(Orientation_t orientation)
    {
        return (orientation == H ? qcore::Orientation::Horizontal : qcore::Orientation::Vertical);
    }
}