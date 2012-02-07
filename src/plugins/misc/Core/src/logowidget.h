//==============================================================================
// Logo widget
//==============================================================================

#ifndef LOGOWIDGET_H
#define LOGOWIDGET_H

//==============================================================================

#include <QWidget>

//==============================================================================

namespace Ui {
    class LogoWidget;
}

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

class LogoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LogoWidget(QWidget *pParent = 0);
    ~LogoWidget();

protected:
    virtual void paintEvent(QPaintEvent *pEvent);

private:
    Ui::LogoWidget *mUi;

    QPixmap mLogo;

    int mLogoWidth;
    int mLogoHeight;
};

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================

#endif

//==============================================================================
// End of file
//==============================================================================