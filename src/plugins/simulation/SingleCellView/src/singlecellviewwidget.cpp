//==============================================================================
// Single cell view widget
//==============================================================================

#include "cellmlfilemanager.h"
#include "cellmlfileruntime.h"
#include "coreutils.h"
#include "progressbarwidget.h"
#include "propertyeditorwidget.h"
#include "singlecellviewcontentswidget.h"
#include "singlecellviewgraphpanelplotwidget.h"
#include "singlecellviewgraphpanelswidget.h"
#include "singlecellviewgraphpanelwidget.h"
#include "singlecellviewinformationparameterswidget.h"
#include "singlecellviewinformationsimulationwidget.h"
#include "singlecellviewinformationsolverswidget.h"
#include "singlecellviewinformationwidget.h"
#include "singlecellviewplugin.h"
#include "singlecellviewsimulation.h"
#include "singlecellviewwidget.h"
#include "toolbarwidget.h"
#include "usermessagewidget.h"

//==============================================================================

#include "ui_singlecellviewwidget.h"

//==============================================================================

#include <QBrush>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFrame>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>
#include <QSettings>
#include <QSplitter>
#include <QTextEdit>
#include <QTimer>
#include <QVariant>
#include <QPointer>
#include <assert.h>

//==============================================================================

#include "float.h"

//==============================================================================

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_wheel.h"

//==============================================================================

namespace OpenCOR {
namespace SingleCellView {

//==============================================================================

static const QString OutputTab  = "&nbsp;&nbsp;&nbsp;&nbsp;";
static const QString OutputGood = " style=\"color: green;\"";
static const QString OutputInfo = " style=\"color: navy;\"";
static const QString OutputBad  = " style=\"color: maroon;\"";
static const QString OutputBrLn = "<br/>\n";

//==============================================================================

SingleCellViewWidgetCurveData::SingleCellViewWidgetCurveData
(
 const QString &pFileName,
 SingleCellViewSimulation *pSimulation,
 QSharedPointer<CellMLSupport::CellMLFileRuntimeParameter> pParameter
) :
    mFileName(pFileName),
    mSimulation(pSimulation),
    mParameterY(pSimulation->data()->isDAETypeSolver() ?
                pParameter->DAEData() : pParameter->ODEData()),
    mAttached(true), mPlottedCurve(0), mPlottedPoint(0)
{
  int repeats = mSimulation->data()->solverProperties()["nrepeats"].toInt();
  for (int i = 0; i < repeats; i++) {
      mCurves << QSharedPointer<SingleCellViewGraphPanelPlotCurve>(new SingleCellViewGraphPanelPlotCurve());
      mCurves.last()->setData(new SingleCellViewQwtCurveDataAdaptor
                              (pSimulation, i, this));
  }
}

//==============================================================================

QString SingleCellViewWidgetCurveData::fileName() const
{
    // Return our file name

    return mFileName;
}

//==============================================================================

QSharedPointer<CellMLSupport::CellMLFileRuntimeCompiledModelParameter>
SingleCellViewWidgetCurveData::parameter() const
{
    // Return our parameter

    return mParameterY;
}

//==============================================================================

const QList<QSharedPointer<SingleCellViewGraphPanelPlotCurve> > SingleCellViewWidgetCurveData::curves() const
{
    // Return our curve

    return mCurves;
}

//==============================================================================

    QList<QSharedPointer<SingleCellViewGraphPanelPlotCurve> > SingleCellViewWidgetCurveData::curves()
{
    // Return our curve

    return mCurves;
}

void SingleCellViewWidgetCurveData::updateCurves(SingleCellViewGraphPanelPlotWidget* pPlot, bool pActive)
{
    int initialSize = mCurves.size();
    int repeats = mSimulation->data()->solverProperties()["nrepeats"].toInt();
    int sizeDiff = mCurves.size() - repeats;
    if (sizeDiff > 0) {
        QList<QSharedPointer<SingleCellViewGraphPanelPlotCurve> >::iterator
            startDeletingAt(mCurves.end());
        QList<QSharedPointer<SingleCellViewGraphPanelPlotCurve> >::iterator
            startDetachingAt(mCurves.end());
        startDeletingAt -= sizeDiff;
        
        for (startDetachingAt -= sizeDiff; startDetachingAt != mCurves.end();
             startDetachingAt++)
            pPlot->detach(startDetachingAt->data());
        mCurves.erase(startDeletingAt, mCurves.end());
    } else if (sizeDiff < 0) {
        for (int repeatNo = initialSize; repeatNo < repeats;
             repeatNo++) {
            mCurves << QSharedPointer<SingleCellViewGraphPanelPlotCurve>(new SingleCellViewGraphPanelPlotCurve());
            mCurves.last()->setData(new SingleCellViewQwtCurveDataAdaptor
                                    (mSimulation, repeatNo, this));
        }
    }
    assert(mCurves.size() == repeats);

    if (isAttached() && pActive)
        for (int i = 0; i < mCurves.size(); i++)
            pPlot->attach(mCurves[i].data());
    else
        for (int i = 0; i < mCurves.size(); i++)
            pPlot->detach(mCurves[i].data());
}

//==============================================================================

bool SingleCellViewWidgetCurveData::isAttached() const
{
    // Return our attached status

    return mAttached;
}

//==============================================================================

void SingleCellViewWidgetCurveData::setAttached(const bool &pAttached)
{
    // Set our attached status

    mAttached = pAttached;
}

SingleCellViewQwtCurveDataAdaptor::SingleCellViewQwtCurveDataAdaptor
(
 SingleCellViewSimulation* pSimulation,
 int pWhichRepeat,
 SingleCellViewWidgetCurveData* pCurveData
)
  : mSimulationResults(pSimulation->results()),
    mWhichRepeat(pWhichRepeat)
{
    QSharedPointer<CellMLSupport::CellMLFileRuntimeCompiledModelParameter> pY = 
        pCurveData->modelParameterY();
    switch (pY->type()) {
    case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::Voi:
        mSampleY = &SingleCellViewQwtCurveDataAdaptor::sampleBvar;
        break;
    case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::Constant:
    case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::ComputedConstant:
        mConstantYIndex = pY->index();
        mSampleY = &SingleCellViewQwtCurveDataAdaptor::sampleConstantY;
        break;
    case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::State:
        mSampleYIndex = pY->index();
        mSampleY = &SingleCellViewQwtCurveDataAdaptor::sampleStateY;
        break;
    case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::Rate:
        mSampleYIndex = pY->index();
        mSampleY = &SingleCellViewQwtCurveDataAdaptor::sampleRateY;
        break;
    case CellMLSupport::CellMLFileRuntimeCompiledModelParameter::Algebraic:
        mSampleYIndex = pY->index();
        mSampleY = &SingleCellViewQwtCurveDataAdaptor::sampleAlgebraicY;
        break;
    default:
        mConstantYIndex = -1;
        mSampleY = &SingleCellViewQwtCurveDataAdaptor::sampleConstantY;
    }

    updateSize();
}

QRectF SingleCellViewQwtCurveDataAdaptor::boundingRect() const
{
    return d_boundingRect;
}

size_t SingleCellViewQwtCurveDataAdaptor::size() const
{
    return mSize;
}

void SingleCellViewQwtCurveDataAdaptor::updateSize()
{
    mSize = (mSimulationResults->points().size()<=mWhichRepeat) ?
        0 : mSimulationResults->points()[mWhichRepeat].size();
    d_boundingRect = qwtBoundingRect(*this);
}

QPointF SingleCellViewQwtCurveDataAdaptor::sample(size_t i) const
{
    if (i >= mSize)
        return QPointF(0.0, 0.0);

    return QPointF(sampleBvar(i), (this->*mSampleY)(i));
}

double SingleCellViewQwtCurveDataAdaptor::sampleBvar(size_t i) const
{
    return mSimulationResults->points()[mWhichRepeat][i];
}

double SingleCellViewQwtCurveDataAdaptor::sampleStateY(size_t i) const
{
    return mSimulationResults->states()[mWhichRepeat][i][mSampleYIndex];
}

double SingleCellViewQwtCurveDataAdaptor::sampleRateY(size_t i) const
{
    return mSimulationResults->rates()[mWhichRepeat][i][mSampleYIndex];
}

double SingleCellViewQwtCurveDataAdaptor::sampleAlgebraicY(size_t i) const
{
    return mSimulationResults->algebraic()[mWhichRepeat][i][mSampleYIndex];
}

double SingleCellViewQwtCurveDataAdaptor::sampleConstantY(size_t) const
{
    if (mConstantYIndex < 0)
        return 0.0;
    return mSimulationResults->constants()[mWhichRepeat][mConstantYIndex];
}


//==============================================================================

SingleCellViewWidget::SingleCellViewWidget(SingleCellViewPlugin *pPluginParent,
                                           QWidget *pParent) :
    ViewWidget(pParent),
    mGui(new Ui::SingleCellViewWidget),
    mPluginParent(pPluginParent),
    mSimulation(0),
    mSimulations(QMap<QString, SingleCellViewSimulation *>()),
    mStoppedSimulations(QList<QPointer<SingleCellViewSimulation> >()),
    mProgresses(QMap<QString, int>()),
    mResets(QMap<QString, bool>()),
    mDelays(QMap<QString, int>()),
    mAxesSettings(QMap<QString, AxisSettings>()),
    mSplitterWidgetSizes(QList<int>()),
    mRunActionEnabled(true),
    mCurvesData(QMap<QString, SingleCellViewWidgetCurveData *>()),
    mOldSimulationResultsSizes(QMap<SingleCellViewSimulation *, QPair<qulonglong, qulonglong> >()),
    mCheckResultsSimulations(QList<QPointer<SingleCellViewSimulation> >())
{
    // Set up the GUI

    mGui->setupUi(this);

    // Create a wheel (and a label to show its value) to specify the delay (in
    // milliseconds) between the output of two data points

    mDelayWidget = new QwtWheel(this);
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    QWidget *delaySpaceWidget = new QWidget(this);
#endif
    mDelayValueWidget = new QLabel(this);

    mDelayWidget->setBorderWidth(0);
    mDelayWidget->setFixedSize(0.07*qApp->desktop()->screenGeometry().width(),
                               0.5*mDelayWidget->height());
    mDelayWidget->setFocusPolicy(Qt::NoFocus);
    mDelayWidget->setRange(0.0, 50.0);
    mDelayWidget->setWheelBorderWidth(0);

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    delaySpaceWidget->setFixedWidth(4);
#endif

    connect(mDelayWidget, SIGNAL(valueChanged(double)),
            this, SLOT(updateDelayValue(const double &)));

    setDelayValue(0);

    // Create a tool bar widget with different buttons

    mToolBarWidget = new Core::ToolBarWidget(this);

    mToolBarWidget->addAction(mGui->actionRunPauseResume);
    mToolBarWidget->addAction(mGui->actionStop);
    mToolBarWidget->addSeparator();
    mToolBarWidget->addAction(mGui->actionReset);
    mToolBarWidget->addSeparator();
    mToolBarWidget->addWidget(mDelayWidget);
#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    mToolBarWidget->addWidget(delaySpaceWidget);
#endif
    mToolBarWidget->addWidget(mDelayValueWidget);
/*---GRY--- THE BELOW SHOULD BE RE-ENABLED AT SOME POINT...
    mToolBarWidget->addSeparator();
    mToolBarWidget->addAction(mGui->actionDebugMode);
    mToolBarWidget->addSeparator();
    mToolBarWidget->addAction(mGui->actionAdd);
    mToolBarWidget->addAction(mGui->actionRemove);
*/
    mToolBarWidget->addSeparator();
    mToolBarWidget->addAction(mGui->actionCsvExport);

    mTopSeparator = Core::newLineWidget(this);

    mGui->layout->addWidget(mToolBarWidget);
    mGui->layout->addWidget(mTopSeparator);

    // Create our splitter widget and keep track of its movement
    // Note: we need to keep track of its movement so that saveSettings() can
    //       work fine even when mContentsWidget is not visible (which happens
    //       when a CellML file cannot be run for some reason or another)...

    mSplitterWidget = new QSplitter(Qt::Vertical, this);

    connect(mSplitterWidget, SIGNAL(splitterMoved(int,int)),
            this, SLOT(splitterWidgetMoved()));

    // Create our contents widget

    mContentsWidget = new SingleCellViewContentsWidget(this);

    mContentsWidget->setObjectName("Contents");

    // Keep track of changes to some of our simulation and solvers properties

    connect(mContentsWidget->informationWidget()->simulationWidget(), SIGNAL(propertyChanged(Core::Property *)),
            this, SLOT(simulationPropertyChanged(Core::Property *)));
    connect(mContentsWidget->informationWidget()->solversWidget(), SIGNAL(propertyChanged(Core::Property *)),
            this, SLOT(solversPropertyChanged(Core::Property *)));

    // Keep track of whether we can remove graph panels

    connect(mContentsWidget->graphPanelsWidget(), SIGNAL(removeGraphPanelsEnabled(const bool &)),
            mGui->actionRemove, SLOT(setEnabled(bool)));

    // Keep track of which parameters to show/hide

    connect(mContentsWidget->informationWidget()->parametersWidget(), SIGNAL(showParameter(const QString &, QSharedPointer<CellMLSupport::CellMLFileRuntimeParameter>, const bool &)),
            this, SLOT(showParameter(const QString &, QSharedPointer<CellMLSupport::CellMLFileRuntimeParameter>, const bool &)));

    // Create and add our invalid simulation message widget
    mErrorType = SingleCellViewWidget::General; // To avoid undefined memory usage.
    mInvalidModelMessageWidget = new Core::UserMessageWidget(":/oxygen/actions/help-about.png",
                                                             pParent);

    mGui->layout->addWidget(mInvalidModelMessageWidget);

    // Create our simulation output widget with a layout on which we put a
    // separating line and our simulation output list view
    // Note: the separating line is because we remove, for aesthetical reasons,
    //       the border of our simulation output list view...

    QWidget *simulationOutputWidget = new QWidget(this);
    QVBoxLayout *simulationOutputLayout= new QVBoxLayout(simulationOutputWidget);

    simulationOutputLayout->setMargin(0);
    simulationOutputLayout->setSpacing(0);

    simulationOutputWidget->setLayout(simulationOutputLayout);

    mOutputWidget = new QTextEdit(this);

    mOutputWidget->setFrameStyle(QFrame::NoFrame);

    simulationOutputLayout->addWidget(Core::newLineWidget(this));
    simulationOutputLayout->addWidget(mOutputWidget);

    // Populate our splitter and use as much space as possible for it by asking
    // for its height to be that of the desktop's, and then add our splitter to
    // our single cell view widget

    mSplitterWidget->addWidget(mContentsWidget);
    mSplitterWidget->addWidget(simulationOutputWidget);

    mSplitterWidget->setSizes(QList<int>() << qApp->desktop()->screenGeometry().height() << 1);

    mGui->layout->addWidget(mSplitterWidget);

    // Create our (thin) simulation progress widget

    mProgressBarWidget = new Core::ProgressBarWidget(this);

    mProgressBarWidget->setFixedHeight(3);
    mProgressBarWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    mBottomSeparator = Core::newLineWidget(this);

    mGui->layout->addWidget(mBottomSeparator);
    mGui->layout->addWidget(mProgressBarWidget);

    // Make our contents widget our focus proxy

    setFocusProxy(mContentsWidget);
}

//==============================================================================

SingleCellViewWidget::~SingleCellViewWidget()
{
    // Delete our simulation objects

    foreach (SingleCellViewSimulation *simulation, mSimulations)
        delete simulation;

    // Delete our curves' data

    foreach (SingleCellViewWidgetCurveData *curveData, mCurvesData) {
        delete curveData;
    }

    // Delete the GUI

    delete mGui;
}

//==============================================================================

void SingleCellViewWidget::retranslateUi()
{
    // Retranslate our GUI

    mGui->retranslateUi(this);

    // Retranslate our delay and delay value widgets

    mDelayWidget->setToolTip(tr("Delay"));
    mDelayValueWidget->setToolTip(mDelayWidget->toolTip());

    mDelayWidget->setStatusTip(tr("Delay in milliseconds between two data points"));
    mDelayValueWidget->setStatusTip(mDelayWidget->statusTip());

    // Retranslate our run/pause action

    updateRunPauseAction(mRunActionEnabled);

    // Retranslate our invalid model message

    updateInvalidModelMessageWidget();

    // Retranslate our contents widget

    mContentsWidget->retranslateUi();
}

//==============================================================================

static const QString SettingsSizesCount = "SizesCount";
static const QString SettingsSize       = "Size%1";

//==============================================================================

void SingleCellViewWidget::loadSettings(QSettings *pSettings)
{
    // Retrieve and set the sizes of our splitter

    int sizesCount = pSettings->value(SettingsSizesCount, 0).toInt();

    if (sizesCount) {
        mSplitterWidgetSizes = QList<int>();

        for (int i = 0; i < sizesCount; ++i)
            mSplitterWidgetSizes << pSettings->value(SettingsSize.arg(i)).toInt();

        mSplitterWidget->setSizes(mSplitterWidgetSizes);
    }

    // Retrieve the settings of our contents widget

    pSettings->beginGroup(mContentsWidget->objectName());
        mContentsWidget->loadSettings(pSettings);
    pSettings->endGroup();

    // Retrieve our active graph panel
//---GRY--- WE SHOULD STOP USING mActiveGraphPanel ONCE WE HAVE RE-ENABLED THE
//          ADDITION/REMOVAL OF GRAPH PANELS...

    mActiveGraphPanel = mContentsWidget->graphPanelsWidget()->activeGraphPanel();

    mActiveGraphPanel->plot()->setFixedAxisX(true);
}

//==============================================================================

void SingleCellViewWidget::saveSettings(QSettings *pSettings) const
{
    // Keep track of our splitter sizes

    pSettings->setValue(SettingsSizesCount, mSplitterWidgetSizes.count());

    for (int i = 0, iMax = mSplitterWidgetSizes.count(); i < iMax; ++i)
        pSettings->setValue(SettingsSize.arg(i), mSplitterWidgetSizes[i]);

    // Keep track of the settings of our contents widget

    pSettings->beginGroup(mContentsWidget->objectName());
        mContentsWidget->saveSettings(pSettings);
    pSettings->endGroup();
}

//==============================================================================

void SingleCellViewWidget::output(const QString &pMessage)
{
    // Move to the end of the output (just in case...)

    mOutputWidget->moveCursor(QTextCursor::End);

    // Make sure that the output ends as expected and if not then add BrLn to it

    static const QString EndOfOutput = "<br /></p></body></html>";

    if (mOutputWidget->toHtml().right(EndOfOutput.size()).compare(EndOfOutput))
        mOutputWidget->insertHtml(OutputBrLn);

    // Output the message and make sure that it's visible

    mOutputWidget->insertHtml(pMessage);
    mOutputWidget->ensureCursorVisible();
}

//==============================================================================

void SingleCellViewWidget::updateSimulationMode()
{
    bool simulationModeEnabled = mSimulation->isRunning() || mSimulation->isPaused();

    // Update our run/pause action

    updateRunPauseAction(!mSimulation->isRunning() || mSimulation->isPaused());

    // Enable/disable our stop action

    mGui->actionStop->setEnabled(simulationModeEnabled);

    // Enable or disable our simulation and solvers widgets

    mContentsWidget->informationWidget()->simulationWidget()->setEnabled(!simulationModeEnabled);
    mContentsWidget->informationWidget()->solversWidget()->setEnabled(!simulationModeEnabled);

    // Enable/disable our export to CSV

    mGui->actionCsvExport->setEnabled(    mSimulation->results()->size()
                                      && !simulationModeEnabled);

    // Give the focus to our focus proxy, in case we leave our simulation mode
    // (so that the user can modify simulation data, etc.)

    if (!simulationModeEnabled)
        focusProxy()->setFocus();
}

//==============================================================================

void SingleCellViewWidget::updateRunPauseAction(const bool &pRunActionEnabled)
{
    mRunActionEnabled = pRunActionEnabled;

    mGui->actionRunPauseResume->setIcon(pRunActionEnabled?
                                            QIcon(":/oxygen/actions/media-playback-start.png"):
                                            QIcon(":/oxygen/actions/media-playback-pause.png"));

    bool simulationPaused = mSimulation && mSimulation->isPaused();

    mGui->actionRunPauseResume->setIconText(pRunActionEnabled?
                                                simulationPaused?
                                                    tr("Resume"):
                                                    tr("Run"):
                                                tr("Pause"));
    mGui->actionRunPauseResume->setStatusTip(pRunActionEnabled?
                                                 simulationPaused?
                                                     tr("Resume the simulation"):
                                                     tr("Run the simulation"):
                                                 tr("Pause the simulation"));
    mGui->actionRunPauseResume->setText(pRunActionEnabled?
                                            simulationPaused?
                                                tr("&Resume"):
                                                tr("&Run"):
                                            tr("&Pause"));
    mGui->actionRunPauseResume->setToolTip(pRunActionEnabled?
                                               simulationPaused?
                                                   tr("Resume"):
                                                   tr("Run"):
                                               tr("Pause"));
}

//==============================================================================

void SingleCellViewWidget::updateInvalidModelMessageWidget()
{
    // Update our invalid model message

    mInvalidModelMessageWidget->setMessage("<div align=center>"
                                           "    <p>"
                                          +((mErrorType == InvalidCellMLFile)?
                                                "        "+tr("Sorry, but the <strong>%1</strong> view requires a valid CellML file to work...").arg(mPluginParent->viewName()):
                                                "        "+tr("Sorry, but the <strong>%1</strong> view requires a valid simulation environment to work...").arg(mPluginParent->viewName())
                                           )
                                          +"    </p>"
                                           "    <p>"
                                           "        <small><em>("+tr("See below for more information.")+")</em></small>"
                                           "    </p>"
                                           "</div>");
}

//==============================================================================

void SingleCellViewWidget::initialize(const QString &pFileName)
{
    // Keep track of our simulation data for our previous model and finalise a
    // few things, if needed

    SingleCellViewSimulation *previousSimulation = mSimulation;
    SingleCellViewInformationWidget *informationWidget = mContentsWidget->informationWidget();
    SingleCellViewInformationSimulationWidget *simulationWidget = informationWidget->simulationWidget();
    SingleCellViewInformationSolversWidget *solversWidget = informationWidget->solversWidget();
    SingleCellViewInformationParametersWidget *parametersWidget = informationWidget->parametersWidget();

    if (previousSimulation) {
        // There is a previous simulation, so backup a few things

        QString previousFileName = previousSimulation->fileName();

        simulationWidget->backup(previousFileName);
        solversWidget->backup(previousFileName);

        // Keep track of the status of the reset action and of the value of the
        // delay widget

        mResets.insert(previousFileName, mGui->actionReset->isEnabled());
        mDelays.insert(previousFileName, mDelayWidget->value());

        // Keept rack of our graph panel's plot's axes settings

        AxisSettings axisSettings;

        axisSettings.minX = mActiveGraphPanel->plot()->minX();
        axisSettings.maxX = mActiveGraphPanel->plot()->maxX();
        axisSettings.minY = mActiveGraphPanel->plot()->minY();
        axisSettings.maxY = mActiveGraphPanel->plot()->maxY();

        axisSettings.localMinX = mActiveGraphPanel->plot()->localMinX();
        axisSettings.localMaxX = mActiveGraphPanel->plot()->localMaxX();
        axisSettings.localMinY = mActiveGraphPanel->plot()->localMinY();
        axisSettings.localMaxY = mActiveGraphPanel->plot()->localMaxY();

        mAxesSettings.insert(previousFileName, axisSettings);

        // Keep track of the attachment status of our curves

        foreach (SingleCellViewWidgetCurveData *curveData, mCurvesData)
            if (!curveData->fileName().compare(previousFileName))
                curveData->setAttached(curveData->curves().first() != NULL &&
                                       curveData->curves().first()->plot() != NULL);
    }

    // Retrieve our simulation object for the current model, if any

    bool newSimulation = false;

    CellMLSupport::CellMLFile *cellmlFile = CellMLSupport::CellMLFileManager::instance()->cellmlFile(pFileName);
    CellMLSupport::CellMLFileRuntime *cellmlFileRuntime = cellmlFile->runtime();

    mSimulation = mSimulations.value(pFileName);

    if (!mSimulation) {
        // No simulation object currently exists for the model, so create one

        mSimulation = new SingleCellViewSimulation(pFileName, cellmlFileRuntime);

        newSimulation = true;

        // Set our simulation object's delay

        mSimulation->setDelay(mDelayWidget->value());

        // Create a few connections

        connect(mSimulation, SIGNAL(running(QPointer<OpenCOR::SingleCellView::SingleCellViewSimulation>, bool)),
                this, SLOT(simulationRunning(QPointer<OpenCOR::SingleCellView::SingleCellViewSimulation>,
                                             bool)));
        connect(mSimulation, SIGNAL(paused(QPointer<OpenCOR::SingleCellView::SingleCellViewSimulation>)),
                this, SLOT(simulationPaused(QPointer<OpenCOR::SingleCellView::SingleCellViewSimulation>)));
        connect(mSimulation, SIGNAL(stopped(QPointer<OpenCOR::SingleCellView::SingleCellViewSimulation>,int)),
                this, SLOT(simulationStopped(QPointer<OpenCOR::SingleCellView::SingleCellViewSimulation>,
                                             int)));

        connect(mSimulation, SIGNAL(error(QPointer<OpenCOR::SingleCellView::SingleCellViewSimulation>,
                                          const QString &)),
                this, SLOT(simulationError(QPointer<OpenCOR::SingleCellView::SingleCellViewSimulation>,
                                           const QString &)));

        connect(mSimulation->data(), SIGNAL(modified(QPointer<OpenCOR::SingleCellView::SingleCellViewSimulationData>,
                                                     const bool &)),
                this, SLOT(simulationDataModified(QPointer<OpenCOR::SingleCellView::SingleCellViewSimulationData>,
                                                  const bool &)));

        // Keep track of our simulation object

        mSimulations.insert(pFileName, mSimulation);
    }

    // Retrieve the status of the reset action and the value of the delay widget

    mGui->actionReset->setEnabled(mResets.value(pFileName, false));

    setDelayValue(mDelays.value(pFileName, 0));

    // Reset our file tab icon and stop tracking our simulation progress, in
    // case our simulation is running

    if (mSimulation->isRunning()) {
        mProgresses.remove(mSimulation->fileName());

        emit updateFileTabIcon(mSimulation->fileName(), QIcon());
    }

    // Determine whether the CellML file has a valid runtime

    bool validCellMLFileRuntime = cellmlFileRuntime && cellmlFileRuntime->isValid();

    // Retrieve our variable of integration, if any

    QSharedPointer<CellMLSupport::CellMLFileRuntimeParameter> variableOfIntegration;
    if (validCellMLFileRuntime)
        variableOfIntegration = cellmlFileRuntime->variableOfIntegration();

    // Output some information about our CellML file

    QString information = QString();

    if (!mOutputWidget->document()->isEmpty())
        information += "<hr/>\n";

    information += "<strong>"+pFileName+"</strong>"+OutputBrLn;
    information += OutputTab+"<strong>"+tr("Runtime:")+"</strong> ";

    if (validCellMLFileRuntime && variableOfIntegration) {
        // A valid runtime could be retrieved for the CellML file and we have a
        // variable of integration

        QString additionalInformation = QString();

        information += "<span"+OutputGood+">"+tr("valid")+"</span>."+OutputBrLn;
        // TODO: Replace this with something useful.
        // information += QString(OutputTab+"<strong>"+tr("Model type:")+"</strong> <span"+OutputInfo+">%1%2</span>."+OutputBrLn).arg((cellmlFileRuntime->modelType() == CellMLSupport::CellMLFileRuntime::Ode)?tr("ODE"):tr("DAE"),
        //        additionalInformation);
    } else {
        // Either we couldn't retrieve a runtime for the CellML file or we
        // could, but it's not valid or it's valid but then we don't have a
        // variable of integration
        // Note: in the case of a valid runtime and no variable of integration,
        //       we really shouldn't consider the runtime to be valid, hence we
        //       handle the no variable of integration case here...

        mErrorType = InvalidCellMLFile;

        updateInvalidModelMessageWidget();

        information += "<span"+OutputBad+">"+(cellmlFileRuntime?tr("invalid"):tr("none"))+"</span>."+OutputBrLn;

        if (validCellMLFileRuntime)
            information += OutputTab+"<span"+OutputBad+"><strong>"+tr("Error:")+"</strong> "+tr("the model must have at least one ODE or DAE")+".</span>"+OutputBrLn;
        else
            foreach (const CellMLSupport::CellMLFileIssue &issue,
                     cellmlFileRuntime?cellmlFileRuntime->issues():cellmlFile->issues())
                information += QString(OutputTab+"<span"+OutputBad+"><strong>%1</strong> %2</span>."+OutputBrLn).arg((issue.type() == CellMLSupport::CellMLFileIssue::Error)?tr("Error:"):tr("Warning:"),
                                                                                                                     issue.message());
    }

    output(information);

    // Enable/disable our run/pause action depending on whether we have a
    // variable of integration

    mGui->actionRunPauseResume->setEnabled(variableOfIntegration);

    // Update our simulation mode

    updateSimulationMode();

    // We need to initially detach curves with non-matching file names now because
    // updateResults can lead to graph data being accessed.
    foreach (SingleCellViewWidgetCurveData *curveData, mCurvesData)
        curveData->updateCurves(mActiveGraphPanel->plot(),
                                !curveData->fileName().compare(pFileName));

    // Update our previous (if any) and current simulation results

    if (   previousSimulation
        && (previousSimulation->isRunning() || previousSimulation->isPaused()))
        updateResults(previousSimulation,
                      previousSimulation->results()->repeatSize(),
                      previousSimulation->results()->size());

    updateResults(mSimulation, mSimulation->results()->repeatSize(),
                  mSimulation->results()->size(), true);

    // Check that we have a valid runtime

    bool hasError = true;

    if (validCellMLFileRuntime && variableOfIntegration) {
        // We have both a valid runtime and a variable of integration
        // Note: if we didn't have a valid runtime, then there would be no need
        //       to output an error message since one would have already been
        //       generated while trying to get the runtime (see above)...

        // Retrieve the unit of the variable of integration

        // Show our contents widget in case it got previously hidden
        // Note: indeed, if it was to remain hidden then some initialisations
        //       wouldn't work (e.g. the solvers widget has property editor
        //       which all properties need to be removed and if the contents
        //       widget is not visible, then upon repopulating the property
        //       editor, scrollbars will be shown even though they are not
        //       needed), so...

        mContentsWidget->setVisible(true);

        // Initialise our GUI's simulation, solvers and parameters widgets
        // Note: this will also initialise some of our simulation data (i.e. our
        //       simulation's starting point and simulation's NLA solver's
        //       properties) which is needed since we want to be able to reset
        //       our simulation below...

        simulationWidget->initialize(pFileName, cellmlFileRuntime, mSimulation->data());
        solversWidget->initialize(pFileName, cellmlFileRuntime, mSimulation->data());
        parametersWidget->initialize(pFileName, cellmlFileRuntime, mSimulation->data());

        hasError = false;
    }

    // Check if an error occurred and, if so, show/hide some widgets

    bool previousHasError = mInvalidModelMessageWidget->isVisible();

    mToolBarWidget->setVisible(!hasError);
    mTopSeparator->setVisible(!hasError);

    mContentsWidget->setVisible(!hasError);
    mInvalidModelMessageWidget->setVisible(hasError);

    mBottomSeparator->setVisible(!hasError);
    mProgressBarWidget->setVisible(!hasError);

    // Make sure that the last output message is visible
    // Note: indeed, to (re)show some widgets (see above) will change the height
    //       of our output widget, messing up the vertical scroll bar a bit (if
    //       visible), resulting in the output being shifted a bit, so...

    if (previousHasError != hasError) {
        qApp->processEvents();

        mOutputWidget->ensureCursorVisible();
    }

    // Attach/detach the curves, based on whether they are associated with then
    // given file name
    

    foreach (SingleCellViewWidgetCurveData *curveData, mCurvesData)
        curveData->updateCurves(mActiveGraphPanel->plot(),
                                !curveData->fileName().compare(pFileName));

    // Retrieve our graph panel's plot's axes settings and replot our graph
    // panel's plot, if available

    if (mAxesSettings.contains(pFileName)) {
        // We have some axes settings for the given file name, so retrieve its
        // graph panel's plot's axes settings

        AxisSettings axisSettings = mAxesSettings.value(pFileName);

        mActiveGraphPanel->plot()->setMinMaxX(axisSettings.minX,
                                              axisSettings.maxX);
        mActiveGraphPanel->plot()->setMinMaxY(axisSettings.minY,
                                              axisSettings.maxY);

        mActiveGraphPanel->plot()->setLocalMinMaxX(axisSettings.localMinX,
                                                   axisSettings.localMaxX);
        mActiveGraphPanel->plot()->setLocalMinMaxY(axisSettings.localMinY,
                                                   axisSettings.localMaxY);
    } else {
        // We don't have any X axis settings for the given file name, so first
        // initialise our simulation's properties

        simulationPropertyChanged(simulationWidget->startingPointProperty(), false);
        simulationPropertyChanged(simulationWidget->endingPointProperty(), false);
        simulationPropertyChanged(simulationWidget->pointIntervalProperty(), false);

        // Now, initialise our graph panel's plot's X axis settings

        mActiveGraphPanel->plot()->setLocalMinMaxX(mActiveGraphPanel->plot()->minX(),
                                                   mActiveGraphPanel->plot()->maxX());
    }

    // If no error occurred and if we are dealing with a new simulation, then
    // reset both its data and its results (well, initialise in the case of its
    // data)

    if (!hasError && newSimulation) {
        mSimulation->data()->reset();
        mSimulation->results()->reset();
    }

    // Check our graph panel's plot's local axes and then replot our graph
    // panel's plot
    // Note: we always want to replot everything, hence our passing false to
    //       checkLocalAxes()...

    mActiveGraphPanel->plot()->checkLocalAxes(false);
    mActiveGraphPanel->plot()->replotNow();

    // Allow/prevent interaction with our graph panel's plot

    mActiveGraphPanel->plot()->setInteractive(!mSimulation->isRunning());
}

//==============================================================================

bool SingleCellViewWidget::isManaged(const QString &pFileName) const
{
    // Return whether the given file name is managed

    return mSimulations.value(pFileName);
}

//==============================================================================

void SingleCellViewWidget::finalize(const QString &pFileName)
{
    // Remove our simulation object, should there be one for the given file name

    SingleCellViewSimulation *simulation = mSimulations.value(pFileName);

    if (simulation) {
        // There is a simulation object for the given file name, so delete it
        // and remove it from our list

        delete simulation;

        mSimulations.remove(pFileName);

        // Reset our memory of the current simulation object, but only if it's
        // the same as our simulation object

        if (simulation == mSimulation)
            mSimulation = 0;
    }

    // Remove our curves' data associated with the given file name, if any

    QList<QString> fileNames = QList<QString>();
    QList<QSharedPointer<CellMLSupport::CellMLFileRuntimeCompiledModelParameter> > parameters =
        QList<QSharedPointer<CellMLSupport::CellMLFileRuntimeCompiledModelParameter> >();

    foreach (SingleCellViewWidgetCurveData *curveData, mCurvesData)
        if (!curveData->fileName().compare(pFileName)) {
            // Keep track of the file name and parameter of the curve data
            fileNames << curveData->fileName();
            parameters << curveData->parameter();

            delete curveData;
        }

    for (int i = 0, iMax = fileNames.count(); i < iMax; ++i)
        mCurvesData.remove(parameterKey(fileNames[i], parameters[i]));

    // Remove various information associated with the given file name

    mProgresses.remove(pFileName);

    mResets.remove(pFileName);
    mDelays.remove(pFileName);

    mAxesSettings.remove(pFileName);

    // Finalize a few things in our GUI's simulation, solvers and parameters
    // widgets

    SingleCellViewInformationWidget *informationWidget = mContentsWidget->informationWidget();

    informationWidget->simulationWidget()->finalize(pFileName);
    informationWidget->solversWidget()->finalize(pFileName);
    informationWidget->parametersWidget()->finalize(pFileName);
}

//==============================================================================

int SingleCellViewWidget::tabBarIconSize() const
{
    // Return the size of a file tab icon

    return style()->pixelMetric(QStyle::PM_TabBarIconSize, 0, this);
}

//==============================================================================

QIcon SingleCellViewWidget::fileTabIcon(const QString &pFileName) const
{
    // Return a file tab icon which shows the given file's simulation progress

    SingleCellViewSimulation *simulation = mSimulations.value(pFileName);
    int progress = simulation?mProgresses.value(simulation->fileName(), -1):-1;

    if (simulation && (progress != -1)) {
        // Create an image that shows the progress of our simulation

        QImage tabBarIcon = QImage(tabBarIconSize(), mProgressBarWidget->height()+2,
                                   QImage::Format_ARGB32_Premultiplied);
        QPainter tabBarIconPainter(&tabBarIcon);

        tabBarIconPainter.setBrush(QBrush(CommonWidget::windowColor()));
        tabBarIconPainter.setPen(QPen(CommonWidget::borderColor()));

        tabBarIconPainter.drawRect(0, 0, tabBarIcon.width()-1, tabBarIcon.height()-1);
        tabBarIconPainter.fillRect(1, 1, progress, tabBarIcon.height()-2,
                                   CommonWidget::highlightColor());

        return QIcon(QPixmap::fromImage(tabBarIcon));
    } else {
        // No simulation object currently exists for the model, so...

        return QIcon();
    }
}

//==============================================================================

void SingleCellViewWidget::on_actionRunPauseResume_triggered()
{
    // Run or resume our simulation, or pause it

    if (mRunActionEnabled) {
        if (mSimulation->isPaused()) {
            // Our simulation is paused, so resume it

            mSimulation->resume();
        } else {
            // Protect ourselves against two successive (very) quick attempts at
            // trying to run a simulation

            static bool handlingAction = false;

            if (handlingAction || mSimulation->isRunning())
                return;

            handlingAction = true;

            // Our simulation is not paused, so finish any editing of our
            // simulation information

            mContentsWidget->informationWidget()->finishEditing();;

            // Now, we would normally retrieve our simulation properties, but
            // there is no need for it since they have already been retrieved
            // (see simulationPropertyChanged())...

            // Retrieve our solvers' properties
            // Note: we don't need to retrieve the NLA solver's properties since
            //       we already have them (see solversPropertyChanged())...

            SingleCellViewSimulationData *simulationData = mSimulation->data();
            SingleCellViewInformationSolversWidget *solversWidget = mContentsWidget->informationWidget()->solversWidget();

            simulationData->setSolverName(solversWidget->solverData()->solversListProperty()->value()->text());

            foreach (Core::Property *property, solversWidget->solverData()->solversProperties().value(simulationData->solverName()))
                simulationData->addSolverProperty
                (
                 property->id(),
                 (property->value()->type() == Core::PropertyItem::Integer)?
                 Core::PropertyEditorWidget::integerPropertyItem(property->value()):
                 Core::PropertyEditorWidget::doublePropertyItem(property->value()));

            // Check how much memory is needed to run our simulation

            bool runSimulation = true;

            double freeMemory = Core::freeMemory();
            double requiredMemory = mSimulation->requiredMemory();

            if (requiredMemory > freeMemory) {
                // More memory is required to run our simulation than is
                // currently available, so let our user know about it

                QMessageBox::warning(qApp->activeWindow(), tr("Run the simulation"),
                                     tr("Sorry, but the simulation requires %1 of memory and you have only %2 left.").arg(Core::sizeAsString(requiredMemory), Core::sizeAsString(freeMemory)));

                runSimulation = false;
            }

            // Run our simulation, if possible/wanted

            if (runSimulation) {
                // Reset our local X axis

                mActiveGraphPanel->plot()->setLocalMinMaxX(mActiveGraphPanel->plot()->minX(),
                                                           mActiveGraphPanel->plot()->maxX());

                // Reset our simulation settings

                mOldSimulationResultsSizes.insert(mSimulation, QPair<qulonglong, qulonglong>(0, 0));

                mSimulation->results()->reset();

                foreach (SingleCellViewWidgetCurveData *curveData, mCurvesData)
                    curveData->updateCurves(mActiveGraphPanel->plot(),
                                            !curveData->fileName().compare(mSimulation->fileName()));

                updateResults(mSimulation, 0, 0);

                // Effectively run our simulation, if possible

                mSimulation->run();
                // else
                //     QMessageBox::warning(qApp->activeWindow(), tr("Run the simulation"),
                //                          tr("Sorry, but we could not allocate all the memory required for the simulation."));
            }

            // We are done handline the action, so...

            handlingAction = false;
        }
    } else {
        // Pause our simulation

        mSimulation->pause();
    }
}

//==============================================================================

void SingleCellViewWidget::on_actionStop_triggered()
{
    // Stop our simulation

    mSimulation->stop();
}

//==============================================================================

void SingleCellViewWidget::on_actionReset_triggered()
{
    // Reset our simulation parameters
    mSimulation->data()->reset();
}

//==============================================================================

void SingleCellViewWidget::on_actionDebugMode_triggered()
{
//---GRY--- TO BE DONE...
}

//==============================================================================

void SingleCellViewWidget::on_actionAdd_triggered()
{
    // Ask our graph panels widget to add a new graph panel

    mContentsWidget->graphPanelsWidget()->addGraphPanel();
}

//==============================================================================

void SingleCellViewWidget::on_actionRemove_triggered()
{
    // Ask our graph panels widget to remove the current graph panel

    mContentsWidget->graphPanelsWidget()->removeGraphPanel();
}

//==============================================================================

void SingleCellViewWidget::on_actionCsvExport_triggered()
{
    // Export our simulation data results to a CSV file

    QString fileName = Core::getSaveFileName(tr("Export to a CSV file"),
                                             QString(),
                                             tr("CSV File")+" (*.csv)");

    if (!fileName.isEmpty())
        mSimulation->results()->exportToCsv(fileName);
}

//==============================================================================

void SingleCellViewWidget::updateDelayValue(const double &pDelayValue)
{
    // Update our delay value widget

    mDelayValueWidget->setText(QLocale().toString(pDelayValue)+" ms");

    // Also update our simulation object

    if (mSimulation)
        mSimulation->setDelay(pDelayValue);
}

//==============================================================================

void SingleCellViewWidget::simulationRunning
(
 QPointer<SingleCellViewSimulation> pSimulation,
 bool pIsResuming
)
{
    // Our simulation is running, so do a few things, but only we are dealing
    // with the active simulation

    if (pSimulation == mSimulation) {
        // Reset our local axes' values, if resuming (since the user might have
        // been zooming in/out, etc.)

        if (pIsResuming)
            mActiveGraphPanel->plot()->resetLocalAxes();

        // Update our simulation mode and check for results

        updateSimulationMode();

        checkResults(mSimulation);

        // Prevent interaction with our graph panel's plot

        mActiveGraphPanel->plot()->setInteractive(!mSimulation->isRunning());
    }
}

//==============================================================================

void SingleCellViewWidget::simulationPaused(QPointer<SingleCellViewSimulation> pSimulation)
{
    // Our simulation is paused, so do a few things, but only we are dealing
    // with the active simulation

    if (pSimulation == mSimulation) {
        // Update our simulation mode and parameters, and check for results

        updateSimulationMode();

        mContentsWidget->informationWidget()->parametersWidget()->updateParameters();

        checkResults(mSimulation);

        // Allow interaction with our graph panel's plot

        mActiveGraphPanel->plot()->setInteractive(mSimulation->isPaused());
    }
}

//==============================================================================

void SingleCellViewWidget::simulationStopped
(
 QPointer<SingleCellViewSimulation> pSimulation,
 int pElapsedTime
)
{
    // We want a short delay before resetting the progress bar and the file tab
    // icon, so that the user can really see when our simulation has completed

    enum {
        ResetDelay = 169
    };

    // Our simulation worker has stopped, so do a few things, but only if we are
    // dealing with the active simulation
    if (pSimulation.isNull())
      return;

    // This code runs in the main thread, which is the only place that simulations
    // are ever deleted, so pSimulation.data() is good for the rest of this call.
    SingleCellViewSimulation* simulation = pSimulation.data();

    if (simulation == mSimulation) {
        // Output the elapsed time, if valid, and reset our progress bar (with a
        // short delay)

        if (pElapsedTime != -1) {
            // We have a valid elapsed time, so show our simulation time

            SingleCellViewSimulationData *simulationData = mSimulation->data();
            QString solversInformation = QString();

            solversInformation += simulationData->solverName();

            output(QString(OutputTab+"<strong>"+tr("Simulation time:")+"</strong> <span"+OutputInfo+">"+tr("%1 s using %2").arg(QString::number(0.001*pElapsedTime, 'g', 3), solversInformation)+"</span>."+OutputBrLn));
        }

        QTimer::singleShot(ResetDelay, this, SLOT(resetProgressBar()));

        // Update our parameters and simulation mode

        mContentsWidget->informationWidget()->parametersWidget()->updateParameters();

        updateSimulationMode();

        // Allow interaction with our graph panel's plot

        mActiveGraphPanel->plot()->setInteractive(!mSimulation->isRunning());
    }

    // Remove our tracking of our simulation progress and let people know that
    // we should reset our file tab icon, but only if we are the active
    // simulation

    // Stop keeping track of our simulation progress
    mProgresses.remove(simulation->fileName());

    // Reset our file tab icon (with a bit of a delay)
    // Note: we can't directly pass simulation to resetFileTabIcon(), so
    //       instead we use mStoppedSimulations which is a list of
    //       simulations in case several simulations were to stop at around
    //       the same time...
    
    if (simulation != mSimulation) {
        mStoppedSimulations << pSimulation;
        
        QTimer::singleShot(ResetDelay, this, SLOT(resetFileTabIcon()));
    }
}

//==============================================================================

void SingleCellViewWidget::resetProgressBar()
{
    // Reset our progress bar

    mProgressBarWidget->setValue(0.0);
}

//==============================================================================

void SingleCellViewWidget::resetFileTabIcon()
{
    // Let people know that we want to reset our file tab icon

    QPointer<SingleCellViewSimulation> simulation = mStoppedSimulations.first();
    mStoppedSimulations.removeFirst();

    if (simulation.isNull())
        return;
    // If it isn't null, it is safe for the remainder of this call since this runs
    // on the main thread, which is the only place simulation is deleted.

    emit updateFileTabIcon(simulation->fileName(), QIcon());
}

//==============================================================================

void SingleCellViewWidget::simulationError
(
 QPointer<SingleCellViewSimulation> pSimulation,
 const QString &pMessage,
 const ErrorType &pErrorType
)
{
    // Output the simulation error, but only if we are dealing with the active
    // simulation

    if (pSimulation.isNull() || pSimulation == mSimulation) {
        // Note: we test for simulation to be valid since simulationError() can
        //       also be called directly (as opposed to being called as a
        //       response to a signal) as is done in initialize() above...

        mErrorType = pErrorType;

        updateInvalidModelMessageWidget();

        output(OutputTab+"<span"+OutputBad+"><strong>"+tr("Error:")+"</strong> "+pMessage+".</span>"+OutputBrLn);
    }
}

//==============================================================================

void SingleCellViewWidget::simulationDataModified
(
 QPointer<SingleCellViewSimulationData> pSimulationData,
 const bool &pIsModified
)
{
    // Update our refresh action, but only if we are dealing with the active
    // simulation

    if (pSimulationData == mSimulation->data())
        mGui->actionReset->setEnabled(pIsModified);
}

//==============================================================================

SingleCellViewSimulation * SingleCellViewWidget::simulation() const
{
    // Return our (current) simulation

    return mSimulation;
}

//==============================================================================

void SingleCellViewWidget::setDelayValue(const int &pDelayValue)
{
    // Set the value of our delay widget

    mDelayWidget->setValue(pDelayValue);

    updateDelayValue(pDelayValue);
}

//==============================================================================

SingleCellViewContentsWidget * SingleCellViewWidget::contentsWidget() const
{
    // Return our contents widget

    return mContentsWidget;
}

//==============================================================================

void SingleCellViewWidget::splitterWidgetMoved()
{
    // Our splitter has been moved, so keep track of its new sizes

    mSplitterWidgetSizes = mSplitterWidget->sizes();
}

//==============================================================================

void SingleCellViewWidget::simulationPropertyChanged(Core::Property *pProperty, bool pNeedReset)
{
    // Update one of our simulation's properties and, if needed, update the
    // minimum or maximum value for our X axis
    // Note: with regards to the starting point property, we need to update it
    //       because it's can potentially have an effect on the value of our
    //       'computed constants' and 'variables'...

    bool needUpdating = true;

    if (pProperty == mContentsWidget->informationWidget()->simulationWidget()->startingPointProperty()) {
        mSimulation->data()->setStartingPoint(Core::PropertyEditorWidget::doublePropertyItem(pProperty->value()),
                                              pNeedReset);
    } else if (pProperty == mContentsWidget->informationWidget()->simulationWidget()->endingPointProperty()) {
        mSimulation->data()->setEndingPoint(Core::PropertyEditorWidget::doublePropertyItem(pProperty->value()));
    } else if (pProperty == mContentsWidget->informationWidget()->simulationWidget()->pointIntervalProperty()) {
        mSimulation->data()->setPointInterval(Core::PropertyEditorWidget::doublePropertyItem(pProperty->value()));

        needUpdating = false;
    }

    // Update the minimum/maximum values of our X axis and replot ourselves, if
    // needed

    if (needUpdating) {
        if (mSimulation->data()->startingPoint() < mSimulation->data()->endingPoint()) {
            mActiveGraphPanel->plot()->setMinMaxX(mSimulation->data()->startingPoint(),
                                                  mSimulation->data()->endingPoint());
            mActiveGraphPanel->plot()->setLocalMinMaxX(mActiveGraphPanel->plot()->minX(),
                                                       mActiveGraphPanel->plot()->maxX());
        } else {
            mActiveGraphPanel->plot()->setMinMaxX(mSimulation->data()->endingPoint(),
                                                  mSimulation->data()->startingPoint());
            mActiveGraphPanel->plot()->setLocalMinMaxX(mActiveGraphPanel->plot()->minX(),
                                                       mActiveGraphPanel->plot()->maxX());
        }

        mActiveGraphPanel->plot()->replotNow();
    }
}

//==============================================================================

// Called when a solver property changes. We update the corresponding property
// on the solver.
void SingleCellViewWidget::solversPropertyChanged(Core::Property *pProperty)
{
    SingleCellViewInformationSolversWidget *solversWidget = mContentsWidget->informationWidget()->solversWidget();

    if (pProperty == solversWidget->solverData()->solversListProperty())
        mSimulation->data()->setSolverName(pProperty->value()->text());
    else if (pProperty->id() == "debug")
        mSimulation->data()->setDebug(Core::PropertyEditorWidget::booleanPropertyItem
                                      (pProperty->value()));
    else
      mSimulation->data()->addSolverProperty
        (pProperty->id(),
         (pProperty->value()->type() == Core::PropertyItem::Integer)?
         Core::PropertyEditorWidget::integerPropertyItem(pProperty->value()):
         Core::PropertyEditorWidget::doublePropertyItem(pProperty->value()));
}

//==============================================================================

QString SingleCellViewWidget::parameterKey(const QString pFileName,
                                                QSharedPointer<CellMLSupport::CellMLFileRuntimeCompiledModelParameter>
                                                pParameter)
{
    // Return the for the given parameter
    return pFileName+"|"+QString::number(pParameter->type())+"|"+QString::number(pParameter->index());
}

//==============================================================================

void SingleCellViewWidget::showParameter
(
 const QString &pFileName,
 QSharedPointer<CellMLSupport::CellMLFileRuntimeParameter> pParameter,
 const bool &pShow
)
{
    // Determine the key for the given parameter

    QString key = parameterKey(pFileName,
                                    mSimulation->data()->isDAETypeSolver() ? 
                                      pParameter->DAEData() : pParameter->ODEData());

    // Retrieve the curve data associated with the key, if any

    SingleCellViewWidgetCurveData *curveData = mCurvesData.value(key);

    // Check whether to show/hide a curve

    if (curveData) {
        // We already have a curve, so just make it visible/invisible and update
        // our curve's data, in case we are to make it visible
        curveData->setAttached(pShow);
        curveData->updateCurves(mActiveGraphPanel->plot(), pShow);
    } else if (pShow) {
        // We don't have a curve, but we want one so create one, as well as some
        // data for it

        SingleCellViewWidgetCurveData *curveData = new SingleCellViewWidgetCurveData(pFileName, mSimulation, pParameter);
        curveData->setAttached(pShow);
        curveData->updateCurves(mActiveGraphPanel->plot(), true);

        // Keep track of our curve data
        mCurvesData.insert(key, curveData);
    }

    // Check our graph panel's plot's local axes before replotting everything
    // Note: we always want to replot, hence our passing false as an argument to
    //       resetLocalAxes()...

    mActiveGraphPanel->plot()->resetLocalAxes(false);
    mActiveGraphPanel->plot()->replotNow();
}

//==============================================================================

void SingleCellViewWidget::updateResults(SingleCellViewSimulation *pSimulation,
                                         const qulonglong &pRepeatSize,
                                         const qulonglong &pSize,
                                         const bool &pReplot)
{
    // Update our curves, if any and only if actually necessary

    SingleCellViewSimulation *simulation = pSimulation?pSimulation:mSimulation;

    // Our simulation worker has made some progress, so update our progress bar,
    // but only if we are dealing with the active simulation

    if (simulation == mSimulation) {
        // We are dealing with the active simulation, so update our curves and
        // progress bar, and enable/disable the reset action

        // Enable/disable the reset action
        // Note: normally, our simulation worker would, for each point interval,
        //       call SingleCellViewSimulationData::checkForModifications(),
        //       but this would result in a signal being emitted (and then
        //       handled by SingleCellViewWidget::simulationDataModified()),
        //       resulting in some time overhead, so we check things here
        //       instead...

        mGui->actionReset->setEnabled(mSimulation->data()->isModified());

        // Update our progress bar
        mProgressBarWidget->setValue(mSimulation->progress());

        if (pReplot || (pSize <= 1)) {
            // We want to initialise the plot and/or there is no data to plot,
            // so replot our active graph panel
            mActiveGraphPanel->plot()->replotNow();
            return;
        }

        // We aren't replotting everything, so compute what we need to plot.
        foreach (SingleCellViewWidgetCurveData *curveData, mCurvesData) {
            qulonglong whichPoint = curveData->plottedPoint(), whichRepeat;

            if (curveData->plottedCurve() >= pRepeatSize || curveData->plottedPoint() >= pSize) {
                curveData->plottedCurve(0);
                curveData->plottedPoint(0);
            }

            // Skip curves with zero repeats and curves with no plot.
            if (curveData->curves().size() == 0)
                continue;
            if (!curveData->curves().first()->plot())
                continue;

            for (whichRepeat = curveData->plottedCurve();
                 whichRepeat < pRepeatSize; whichRepeat++, whichPoint=0) {
                qulonglong nPoints = (whichRepeat == pRepeatSize - 1) ?
                    pSize : pSimulation->results()->points()[whichRepeat].size();
                if (nPoints == 0) {
                    if (whichRepeat != 0)
                        whichRepeat--;
                    break;
                }
                if (nPoints == 0 || whichPoint >= nPoints - 1)
                    continue;
                mActiveGraphPanel->plot()->drawCurveSegment(curveData->curves()[whichRepeat],
                                                            whichPoint, nPoints - 1);
            }
            curveData->plottedCurve(whichRepeat);
            curveData->plottedPoint(whichPoint);
        }
    } else {
        // We are dealing with another simulation, so simply create an icon that
        // shows the other simulation's progress and let people know about it
        // Note: we need to retrieve the name of the file associated with the
        //       other simulation since we have only one simulation object at
        //       any given time, and anyone handling the updateFileTabIcon()
        //       signal (e.g. CentralWidget) won't be able to tell for which
        //       simulation the update is...

        int oldProgress = mProgresses.value(simulation->fileName(), -1);
        int newProgress = (tabBarIconSize()-2)*simulation->progress();
        // Note: tabBarIconSize()-2 because we want a one-pixel wide
        //       border...

        if (newProgress != oldProgress) {
            // The progress has changed (or we want to force the updating of a
            // specific simulation), so keep track of its new value and update
            // our file tab icon

            mProgresses.insert(simulation->fileName(), newProgress);

            // Let people know about the file tab icon to be used for the
            // model

            emit updateFileTabIcon(simulation->fileName(),
                                   fileTabIcon(simulation->fileName()));
        }
    }
}

//==============================================================================

void SingleCellViewWidget::checkResults(SingleCellViewSimulation *pSimulation)
{
    // Update our simulation results size

    qulonglong simulationRepeatResultsSize = pSimulation->results()->repeatSize();
    qulonglong simulationResultsSize = pSimulation->results()->size();

    // Update our results, but only if needed

    QPair<qulonglong, qulonglong> resultSizes
        (mOldSimulationResultsSizes.value(pSimulation));
    if (simulationRepeatResultsSize != resultSizes.first ||
        simulationResultsSize != resultSizes.second) {
        mOldSimulationResultsSizes.insert
            (
             pSimulation,
             QPair<qulonglong, qulonglong>(simulationRepeatResultsSize, simulationResultsSize));

        updateResults(pSimulation, simulationRepeatResultsSize, simulationResultsSize);
    }

    // Ask to recheck our simulation's results, but only if our simulation is
    // still running

    if (   pSimulation->isRunning()
        || (simulationResultsSize != pSimulation->results()->size())) {
        // Note: we cannot ask QTimer::singleShot() to call checkResults() since
        //       it expects a pointer to a simulation as a parameter, so instead
        //       we call a method with no arguments which will make use of our
        //       list to know which simulation should be passed as an argument
        //       to checkResults()...

        mCheckResultsSimulations << pSimulation;

        QTimer::singleShot(0, this, SLOT(callCheckResults()));
    }
}

//==============================================================================

void SingleCellViewWidget::callCheckResults()
{
    // Retrieve the simulation for which we want to call checkResults() and then
    // call checkResults()

    if (mCheckResultsSimulations.empty())
        return;

    QPointer<SingleCellViewSimulation> simulation = mCheckResultsSimulations.first();

    mCheckResultsSimulations.removeFirst();

    if (!simulation.isNull())
        checkResults(simulation.data());
}

//==============================================================================

}   // namespace SingleCellView
}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
