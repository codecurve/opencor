PROJECT(QScintillaSupportPlugin)

# Add the plugin

ADD_PLUGIN(QScintillaSupport
    SOURCES
        ../../plugininfo.cpp

        src/qscintillawidget.cpp
        src/qscintillasupportplugin.cpp
    HEADERS_MOC
        src/qscintillawidget.h
        src/qscintillasupportplugin.h
    DEFINITIONS
        QSCINTILLA_DLL
    INCLUDE_DIRS
        src
    PLUGIN_DEPENDENCIES
        Core
        QScintilla
    QT_MODULES
        Network
        Widgets
    QT_DEPENDENCIES
        QtCore
        QtGui
        QtNetwork
        QtWidgets
)
