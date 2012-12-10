//==============================================================================
// Progress bar widget
//==============================================================================

#include "commonwidget.h"
#include "progressbarwidget.h"

//==============================================================================

#include <QPainter>
#include <QPaintEvent>

//==============================================================================

namespace OpenCOR {
namespace Core {

//==============================================================================

ProgressBarWidget::ProgressBarWidget(QWidget *pParent) :
    Widget(QSize(), pParent),
    mWidth(0),
    mValue(0.0)
{
}

//==============================================================================

void ProgressBarWidget::paintEvent(QPaintEvent *pEvent)
{
    // Paint ourselves

    QPainter painter(this);

    int value = mValue*mWidth;

    if (value)
        painter.fillRect(0, 0, value, height(),
                         Core::CommonWidget::highlightColor());

    if (value != mWidth)
        painter.fillRect(value, 0, mWidth-value, height(),
                         Core::CommonWidget::windowColor());

    // Accept the event

    pEvent->accept();
}

//==============================================================================

void ProgressBarWidget::resizeEvent(QResizeEvent *pEvent)
{
    // Keep track of our new width

    mWidth = pEvent->size().width();

    // Default handling of the event

    Widget::resizeEvent(pEvent);
}

//==============================================================================

void ProgressBarWidget::setValue(const double &pValue)
{
    // Update both our value and ourselves, if needed

    double value = qMin(1.0, qMax(pValue, 0.0));

    if (value != mValue) {
        bool needUpdate = int(value*mWidth) != int(mValue*mWidth);

        mValue = value;

        if (needUpdate)
            // Note: normally, we would be using update(), but on Windows many
            //       successive updates will result in a very choppy progress,
            //       so...

            repaint();
    }
}

//==============================================================================

}   // namespace Core
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================