//==============================================================================
// Preferences window
//==============================================================================

#ifndef PREFERENCESWINDOW_H
#define PREFERENCESWINDOW_H

//==============================================================================

#include "commonwidget.h"

//==============================================================================

#include <QDialog>

//==============================================================================

namespace Ui {
    class PreferencesWindow;
}

//==============================================================================

namespace OpenCOR {

//==============================================================================

class MainWindow;

//==============================================================================

class PreferencesWindow : public QDialog, public Core::CommonWidget
{
    Q_OBJECT

public:
    explicit PreferencesWindow(MainWindow *pMainWindow = 0);
    ~PreferencesWindow();

private:
    Ui::PreferencesWindow *mGui;
};

//==============================================================================

}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================
