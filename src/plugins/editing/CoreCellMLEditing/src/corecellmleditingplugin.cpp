#include "corecellmleditingplugin.h"

namespace OpenCOR {
namespace CoreCellMLEditing {

PLUGININFO_FUNC CoreCellMLEditingPluginInfo()
{
    Descriptions descriptions;

    descriptions.insert("en", "The core CellML editing plugin for OpenCOR");
    descriptions.insert("fr", "L'extension d'�dition CellML de base pour OpenCOR");

    return PluginInfo(PluginInfo::V001,
                      PluginInfo::Gui,
                      PluginInfo::Editing,
                      false,
                      QStringList() << "CoreEditing" << "CellML",
                      descriptions);
}

Q_EXPORT_PLUGIN2(CoreCellMLEditing, CoreCellMLEditingPlugin)

void CoreCellMLEditingPlugin::initialize()
{
    // Create our different File|New actions

    mFileNewCellml1_0Action = newAction(mMainWindow);
    mFileNewCellml1_1Action = newAction(mMainWindow);

    // Set our settings

    mGuiSettings->addMenuAction(GuiMenuActionSettings::FileNew, mFileNewCellml1_0Action);
    mGuiSettings->addMenuAction(GuiMenuActionSettings::FileNew, mFileNewCellml1_1Action);
}

void CoreCellMLEditingPlugin::retranslateUi()
{
    // Retranslate our different File|New actions

    retranslateAction(mFileNewCellml1_0Action, tr("CellML 1.0 File"),
                      tr("Create a new CellML 1.0 file"));
    retranslateAction(mFileNewCellml1_1Action, tr("CellML 1.1 File"),
                      tr("Create a new CellML 1.1 file"));
}

} }
