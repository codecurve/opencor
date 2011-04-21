#include "cellml.h"
#include "centralwidget.h"
#include "fileorganiserwidget.h"
#include "tabwidget.h"

#include "ui_centralwidget.h"

#include <QDir>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QSettings>
#include <QTextStream>
#include <QUrl>

#include <qsciscintilla.h>
#include <qscilexerxml.h>

CentralWidget::CentralWidget(QWidget *pParent) :
    QWidget(pParent),
    CommonWidget(pParent),
    mUi(new Ui::CentralWidget)
{
    // Set up the UI

    mUi->setupUi(this);

    // Allow for things to be dropped on us

    setAcceptDrops(true);

    // Create and add our tab widget

    mTabWidget = new TabWidget(this);

    mTabWidget->setUsesScrollButtons(true);
    // Note #1: the above property is style dependent and it happens that it's
    //          not enabled on Mac OS X, so... set it in all cases, even though
    //          it's already set on Windows and Linux, but one can never know
    //          what the future holds, so...
    // Note #2: if the above property is not enabled and many files are opened,
    //          then the central widget will widen reducing the width of any
    //          dock window which is clearly not what we want, so...

    mUi->verticalLayout->addWidget(mTabWidget);

    // A connection to handle the closing of a tab

    connect(mTabWidget, SIGNAL(tabCloseRequested(int)),
            this, SLOT(closeTab(int)));
}

CentralWidget::~CentralWidget()
{
    // Close all the tabs (and therefore files)

    while (mTabWidget->count()) {
        closeTab(0);
    }

    // Delete the UI

    delete mUi;
}

static const QString SettingsOpenedFiles = "OpenedFiles";
static const QString SettingsActiveFile  = "ActiveFile";

void CentralWidget::loadSettings(QSettings &pSettings)
{
    pSettings.beginGroup(objectName());
        // Retrieve the files that were previously opened

        QStringList openedFiles;

        openedFiles = pSettings.value(SettingsOpenedFiles).toStringList();

        for (int i = 0; i < openedFiles.count(); ++i)
            openFile(openedFiles.at(i));

        // Retrieve the active file

        activateFile(openedFiles.at(pSettings.value(SettingsActiveFile).toInt()));
    pSettings.endGroup();
}

void CentralWidget::saveSettings(QSettings &pSettings)
{
    pSettings.beginGroup(objectName());
        // Keep track of the files that are opened

        QStringList openedFiles;

        for (int i = 0; i < mTabWidget->count(); ++i)
            openedFiles << mTabWidget->tabToolTip(i);

        pSettings.setValue(SettingsOpenedFiles, openedFiles);

        // Keep track of the active file

        pSettings.setValue(SettingsActiveFile, mTabWidget->currentIndex());
    pSettings.endGroup();
}

bool CentralWidget::openFile(const QString &pFileName)
{
    if (!QFileInfo(pFileName).exists())
        // The file doesn't exist, so...

        return false;

    // Create an editor for the file

    QsciScintilla *scintilla = new QsciScintilla(this);
    QsciLexerXML *lexer      = new QsciLexerXML(this);

    QFont defaultFont(DefaultFontFamily, DefaultFontSize);

    // The font that will be used for XML-like and non XML-like files, resp.

    lexer->setDefaultFont(defaultFont);
    lexer->setFont(defaultFont);

    // A few settings for the editor itself

    scintilla->setFolding(QsciScintilla::BoxedTreeFoldStyle);
    scintilla->setLexer(lexer);

    // Set the contents of the file to the editor

    QFile file(pFileName);

    file.open(QIODevice::ReadOnly);

    scintilla->setText(QTextStream(&file).readAll());
    scintilla->setReadOnly(!(QFile::permissions(pFileName) & QFile::WriteUser));

    file.close();

    // Create a new tab and have the editor as its contents

    QFileInfo fileInfo = pFileName;

    mTabWidget->setCurrentIndex(mTabWidget->addTab(scintilla,
                                                   (!fileInfo.completeSuffix().compare(CellmlFileExtension, Qt::CaseInsensitive))?
                                                       fileInfo.baseName():
                                                       fileInfo.fileName()));

    // Set the full name of the file as the tool tip for the new tab

    mTabWidget->setTabToolTip(mTabWidget->currentIndex(),
                              QDir::toNativeSeparators(pFileName));

    // Give the focus to the newly created editor

    mTabWidget->currentWidget()->setFocus();

    // Everything went fine, so...

    return true;
}

QString CentralWidget::closeFile(const int &pIndex)
{
    // Close the file at the given tab index or the current tab index, if no tab
    // index is provided, and then return the name of the file that was closed,
    // if any

    int realIndex = (pIndex != -1)?pIndex:mTabWidget->currentIndex();

    QsciScintilla *scintilla = qobject_cast<QsciScintilla *>(mTabWidget->widget(realIndex));

    if (scintilla) {
        // There is a file currently opened, so first retrieve its filename

        QString fileName = mTabWidget->tabToolTip(realIndex);

        // Next, we must close the tab

        mTabWidget->removeTab(realIndex);

        // Then, we must release the allocated memory for the widget that the
        // tab used to contain

        delete scintilla;

        // Finally, we return the filename of the file we have just closed

        return fileName;
    } else {
        // There is no file currently opened, so...

        return QString();
    }
}

bool CentralWidget::activateFile(const QString &pFileName)
{
    // Go through the different tabs and check whether one of them corresponds
    // to the requested file

    QString realFileName = QDir::toNativeSeparators(pFileName);

    for (int i = 0; i < mTabWidget->count(); ++i)
        if (!mTabWidget->tabToolTip(i).compare(realFileName)) {
            // We have found the file, so set the current index to that of its
            // tab

            mTabWidget->setCurrentIndex(i);

            // Then, give the focus to the editor

            mTabWidget->currentWidget()->setFocus();

            // We were able to activate the file, so...

            return true;
        }

    // We couldn't find the file, so...

    return false;
}

void CentralWidget::closeTab(const int &pIndex)
{
    // We want to close a tab, so close the file that is associated with it, and
    // let people know about it

    QString fileName = closeFile(pIndex);

    if (!fileName.isEmpty())
        // The closing of the file was successful, so...

        emit fileClosed(fileName);
}

void CentralWidget::dragEnterEvent(QDragEnterEvent *pEvent)
{
    // Accept the proposed action for the event, but only if we are dropping
    // URIs or items from our file organiser

    if (   (pEvent->mimeData()->hasFormat(FileSystemMimeType))
        && (!pEvent->mimeData()->urls().isEmpty()))
        // Note: we test the list of URLs in case we are trying to drop one or
        //       several folders (and no file) from the file organiser, in which
        //       case the list of URLs will be empty...

        pEvent->acceptProposedAction();
    else
        pEvent->ignore();
}

void CentralWidget::dragMoveEvent(QDragMoveEvent *pEvent)
{
    // Accept the proposed action for the event
    // Note: this is required when dragging external objects...

    pEvent->acceptProposedAction();
}

void CentralWidget::dropEvent(QDropEvent *pEvent)
{
    // Retrieve the name of the various files that have been dropped

    QList<QUrl> urlList = pEvent->mimeData()->urls();
    QStringList fileNames;

    for (int i = 0; i < urlList.count(); ++i)
    {
        QString fileName = urlList.at(i).toLocalFile();
        QFileInfo fileInfo = fileName;

        if (fileInfo.isFile()) {
            if (fileInfo.isSymLink()) {
                // We are dropping a symbolic link, so retrieve its target and
                // check that it exists, and if it does then add it

                fileName = fileInfo.symLinkTarget();

                if (QFileInfo(fileName).exists())
                    fileNames.append(fileName);
            } else {
                // We are dropping a file, so we can just add it

                fileNames.append(fileName);
            }
        }
    }

    // fileNames may contain duplicates (in case we dropped some symbolic
    // links), so remove them

    fileNames.removeDuplicates();

    // Let people know that some files have been dropped on us

    emit filesDropped(fileNames);

    // Accept the proposed action for the event

    pEvent->acceptProposedAction();
}
