#-------------------------------------------------
#
# Project created by QtCreator 2013-09-02T06:58:44
#
#-------------------------------------------------

QT       += core widgets

TARGET = StdPreprocModulesGUI
TEMPLATE = lib
CONFIG += c++11

CONFIG(release, debug|release): DESTDIR = $$PWD/../../../../../lib
else:CONFIG(debug, debug|release): DESTDIR = $$PWD/../../../../../lib/debug

DEFINES += STDPREPROCMODULESGUI_LIBRARY
DEFINES += USE_UNPUBLISHED

SOURCES += stdpreprocmodulesgui.cpp \
    FullLogNormDlg.cpp \
    WaveletRingCleanDlg.cpp \
    projectionfilterdlg.cpp \
    morphspotcleandlg.cpp \
    polynomialcorrectiondlg.cpp \
    datascalerdlg.cpp \
    adaptivefilterdlg.cpp \
    SpotClean2Dlg.cpp \
    medianmixringcleandlg.cpp \
    generalfilterdlg.cpp

HEADERS += stdpreprocmodulesgui.h\
        StdPreprocModulesGUI_global.h \
    FullLogNormDlg.h \
    WaveletRingCleanDlg.h \
    projectionfilterdlg.h \
    morphspotcleandlg.h \
    polynomialcorrectiondlg.h \
    datascalerdlg.h \
    adaptivefilterdlg.h \
    SpotClean2Dlg.h \
    medianmixringcleandlg.h \
    generalfilterdlg.h


symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xEA388B00
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = StdPreprocModulesGUI.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target

    unix:macx {
  #      QMAKE_MAC_SDK = macosx10.12
        QMAKE_CXXFLAGS += -fPIC -O2
        INCLUDEPATH += /opt/local/include
        INCLUDEPATH += /opt/local/include/libxml2
        QMAKE_LIBDIR += /opt/local/lib
    }
    else {
        QMAKE_CXXFLAGS += -fPIC -fopenmp -O2
        QMAKE_LFLAGS += -lgomp
        INCLUDEPATH += /usr/include/libxml2
    }

    LIBS += -ltiff -lxml2

}

win32 {
    contains(QMAKE_HOST.arch, x86_64):{
        QMAKE_LFLAGS += /MACHINE:X64
    }
    INCLUDEPATH += $$PWD/../../../../../external/src/linalg
    INCLUDEPATH += $$PWD/../../../../../external/include
    INCLUDEPATH += $$PWD/../../../../../external/include/cfitsio
    INCLUDEPATH += $$PWD/../../../../../external/include/libxml2
    QMAKE_LIBDIR += $$_PRO_FILE_PWD_/../../../../../external/lib64

    LIBS += -llibxml2_dll -llibtiff -lcfitsio
    QMAKE_CXXFLAGS += /openmp /O2
}

CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../lib/
else:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../lib/debug/

LIBS += -lkipl -lQtModuleConfigure -lQtAddons -lImagingAlgorithms -lReconFramework -lModuleConfig -lStdPreprocModules

INCLUDEPATH += $$PWD/../../../../gui/qt/QtModuleConfigure
DEPENDPATH += $$PWD/../../../../gui/qt/QtModuleConfigure

INCLUDEPATH += $$PWD/../../../../gui/qt/QtAddons
DEPENDPATH += $$PWD/../../../../gui/qt/QtAddons

INCLUDEPATH += $$PWD/../../../../core/algorithms/ImagingAlgorithms/include
DEPENDPATH += $$PWD/../../../../core/algorithms/ImagingAlgorithms/include

INCLUDEPATH += $$PWD/../../Framework/ReconFramework/include
DEPENDPATH += $$PWD/../../Framework/ReconFramework/include

INCLUDEPATH += $$PWD/../StdPreprocModules/include
DEPENDPATH += $$PWD/../StdPreprocModules/include

INCLUDEPATH += $$PWD/../../../../core/kipl/kipl/include
DEPENDPATH += $$PWD/../../../../core/kipl/kipl/include

INCLUDEPATH += $$PWD/../../../../core/modules/ModuleConfig/include
DEPENDPATH += $$PWD/../../../../core/modules/ModuleConfig/include

FORMS += \
    polynomialcorrectiondlg.ui \
    datascalerdlg.ui \
    adaptivefilterdlg.ui \
    morphspotcleandlg.ui \
    SpotClean2Dlg.ui \
    medianmixringclean.ui \
    projectionfilterdlg.ui \
    WaveletRingCleanDlg.ui \
    generalfilterdlg.ui \
    FullLogNormDlg.ui
