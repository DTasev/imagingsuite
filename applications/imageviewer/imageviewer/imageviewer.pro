#-------------------------------------------------
#
# Project created by QtCreator 2016-03-18T15:42:46
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ImageViewer
TEMPLATE = app

CONFIG += c++11

CONFIG(release, debug|release): DESTDIR = $$PWD/../../../../Applications
else:CONFIG(debug, debug|release): DESTDIR = $$PWD/../../../../Applications/debug

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target

    unix:macx {
        QMAKE_CXXFLAGS += -fPIC -O2
        INCLUDEPATH += /opt/local/include
        INCLUDEPATH += /opt/local/include/libxml2
        QMAKE_LIBDIR += /opt/local/lib
        INCLUDEPATH += $$PWD/../../../external/mac/include
        DEPENDPATH += $$PWD/../../../external/mac/include
        LIBS += -L$$PWD/../../../external/mac/lib/ -lNeXus.1.0.0 -lNeXusCPP.1.0.0
    }
    else {
        QMAKE_CXXFLAGS += -fPIC -fopenmp -O2
        QMAKE_LFLAGS += -lgomp
        LIBS += -lgomp
        INCLUDEPATH += /usr/include/libxml2
    }

    LIBS += -ltiff -lxml2 -lcfitsio

}

win32 {
    contains(QMAKE_HOST.arch, x86_64):{
        QMAKE_LFLAGS += /MACHINE:X64
    }
    INCLUDEPATH += $$PWD/../../../external/src/linalg $$PWD/../../../external/include $$PWD/../../../external/include/cfitsio $$PWD/../../../external/include/libxml2
    QMAKE_LIBDIR += $$_PRO_FILE_PWD_/../../../external/lib64

    LIBS += -llibxml2_dll -llibtiff -lcfitsio
    QMAKE_CXXFLAGS += /openmp /O2
}

ICON = viewer_icon.icns
RC_ICONS = viewer_icon.ico

SOURCES += main.cpp\
        viewermainwindow.cpp \
    saveasdialog.cpp

HEADERS  += viewermainwindow.h \
    saveasdialog.h

FORMS    += viewermainwindow.ui \
    saveasdialog.ui

CONFIG(release, debug|release):      LIBS += -L$$PWD/../../../../lib
else:CONFIG(debug, debug|release):   LIBS += -L$$PWD/../../../../lib/debug

LIBS += -lkipl -lQtAddons -lReaderConfig

INCLUDEPATH += $$PWD/../../../core/modules/ReaderConfig
DEPENDPATH += $$PWD/../../../core/modules/ReaderConfig

INCLUDEPATH += $$PWD/../../../core/kipl/kipl/include
DEPENDPATH += $$PWD/../../../core/kipl/kipl/src

INCLUDEPATH += $$PWD/../../../GUI/qt/QtAddons
DEPENDPATH += $$PWD/../../../GUI/qt/QtAddons



