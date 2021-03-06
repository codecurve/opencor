//==============================================================================
// RawView plugin
//==============================================================================

#ifndef RAWVIEWPLUGIN_H
#define RAWVIEWPLUGIN_H

//==============================================================================

#include "guiinterface.h"
#include "i18ninterface.h"
#include "plugininfo.h"

//==============================================================================

namespace OpenCOR {
namespace RawView {

//==============================================================================

PLUGININFO_FUNC RawViewPluginInfo();

//==============================================================================

class RawViewWidget;

//==============================================================================

class RawViewPlugin : public QObject, public GuiInterface, public I18nInterface
{
    Q_OBJECT

    Q_PLUGIN_METADATA(IID "OpenCOR.RawViewPlugin" FILE "rawviewplugin.json")

    Q_INTERFACES(OpenCOR::GuiInterface)
    Q_INTERFACES(OpenCOR::I18nInterface)

public:
    explicit RawViewPlugin();
    ~RawViewPlugin();

    virtual QWidget * viewWidget(const QString &pFileName);
    virtual QWidget * removeViewWidget(const QString &pFileName);
    virtual QString viewName() const;

private:
    QMap<QString, RawViewWidget *> mViewWidgets;
};

//==============================================================================

}   // namespace RawView
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
