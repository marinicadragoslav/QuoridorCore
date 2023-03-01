#ifndef Header_qplugin_drma_plugin_core_interface
#define Header_qplugin_drma_plugin_core_interface

#include "Player.h"
#include "board.h"

namespace qplugin_drma
{
    // Convert absolute core wall position to relative core wall position (value)
    qcore::Position CoreAbsToRelWallPos(qcore::Position absPos, qcore::Orientation orientation); 

    // Convert core wall position to plugin wall position (both type and value)
    Position_t CoreToPluginWallPos(qcore::Position gameWallPos, qcore::Orientation orientation);

    // Convert plugin wall position to core wall position (both type and value)
    qcore::Position PluginToCoreWallPos(Position_t pluginWallPos, Orientation_t orientation);

    // Convert core wall orientation (type) to plugin wall orientation (type)
    Orientation_t CoreToPluginWallOrientation(qcore::Orientation orientation);

    // Convert plugin wall orientation (type) to core wall orientation (type)
    qcore::Orientation PluginToCoreWallOrientation(Orientation_t orientation);

} // end namespace
#endif // Header_qplugin_drma_plugin_core_interface
