#-------------------------------------------------
#
# Project created by QtCreator 2015-08-27T13:46:49
#
#-------------------------------------------------

QT       += core gui\
            sql\
            printsupport\
            serialport\
            network

QT += multimedia
QT += multimediawidgets

QT += axcontainer

RC_FILE  += YAOGAN_LOGO.rc

#INCLUDEPATH += D:/Qt/Qt5.5.0/5.5/mingw492_32/include/qwt

LIBS += -lws2_32

LIBS    +=  -L $$PWD -l HCNetSDK

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = YaoGan
TEMPLATE = app


SOURCES += main.cpp \
    Settings.cpp \
    tools/MyMenu.cpp \
    tools/MyMessageDialog.cpp \
    tools/ProgressBar.cpp \
    tools/ProgressDialog.cpp \
    tools/Tips3DDialog.cpp \
    PlateRecogRegion.cpp \
    About.cpp \
    AccessDatabase.cpp \
    MyDebug.cpp \
    SocketClient.cpp \
    Global.cpp \
    Weather.cpp \
    RunningStatus.cpp \
    Camera.cpp \
    ControlCenter.cpp \
    MainWindow.cpp \
    ControlPanel.cpp \
    tools/qcustomplot.cpp \
    results/Print.cpp \
    results/Results.cpp \
    results/ViewCarPic.cpp \
    Spectrum.cpp \
    Login.cpp \
    RealTimeTest.cpp \
    CalibrationData.cpp \
    TestSpectrum.cpp \
    operation_flow/ConnectionStatus.cpp \
    operation_flow/AdjustCamera.cpp \
    operation_flow/OperationFlow.cpp \
    operation_flow/CaliDialog.cpp \
    SystemStatus.cpp \
    UpdateSoftwareARM.cpp \
    tools/ChooseFileDialog.cpp \
    user_management/UserManagement.cpp \
    user_management/AddUserDialog.cpp \
    user_management/ChangePassword.cpp \
    site_management/SiteManagement.cpp \
    site_management/AddSite.cpp \
    Revision.cpp \
    MultiCaliDialog.cpp \
    tools/Ployfit.cpp \
    BroadcastCaliStatus.cpp \
    TdlasSettings.cpp \
    led_control.cpp

HEADERS  += \
    Settings.h \
    tools/MyMenu.h \
    tools/MyMessageDialog.h \
    tools/ProgressBar.h \
    tools/ProgressDialog.h \
    tools/Tips3DDialog.h \
    Constant.h \
    hcnetsdk.h \
    PlateRecogRegion.h \
    About.h \
    AccessDatabase.h \
    SocketConstant.h \
    SocketClient.h \
    Global.h \
    Weather.h \
    RunningStatus.h \
    Camera.h \
    ControlCenter.h \
    MainWindow.h \
    ControlPanel.h \
    NetDataStructure.h \
    tools/qcustomplot.h \
    results/Print.h \
    results/Results.h \
    results/ViewCarPic.h \
    Spectrum.h \
    plaympeg4.h \
    Login.h \
    RealTimeTest.h \
    CalibrationData.h \
    TestSpectrum.h \
    operation_flow/ConnectionStatus.h \
    operation_flow/AdjustCamera.h \
    operation_flow/OperationFlow.h \
    operation_flow/CaliDialog.h \
    SystemStatus.h \
    UpdateSoftwareARM.h \
    tools/ChooseFileDialog.h \
    user_management/UserManagement.h \
    user_management/AddUserDialog.h \
    user_management/ChangePassword.h \
    site_management/SiteManagement.h \
    site_management/AddSite.h \
    Revision.h \
    MultiCaliDialog.h \
    tools/Ployfit.h \
    Version.h \
    BroadcastCaliStatus.h \
    TdlasSettings.h \
    led_control.h

RESOURCES += \
    img.qrc

DISTFILES += \
    qtxlsx.pri

FORMS += \
    form.ui \
    Settings.ui \
    tools/MyMenu.ui \
    tools/MyMessageDialog.ui \
    tools/ProgressDialog.ui \
    tools/Tips3DDialog.ui \
    PlateRecogRegion.ui \
    About.ui \
    RunningStatus.ui \
    MainWindow.ui \
    ControlPanel.ui \
    results/PrintForm.ui \
    results/Results.ui \
    results/ViewCarPic.ui \
    Spectrum.ui \
    Login.ui \
    RealTimeTest.ui \
    CalibrationData.ui \
    TestSpectrum.ui \
    HomePageTmp.ui \
    ui/InputSite.ui \
    operation_flow/ConnectionStatus.ui \
    operation_flow/AdjustCamera.ui \
    operation_flow/OperationFlow.ui \
    operation_flow/CaliDialog.ui \
    SystemStatus.ui \
    UpdateSoftwareARM.ui \
    tools/ChooseFileDialog.ui \
    user_management/UserManagement.ui \
    user_management/AddUserDialog.ui \
    user_management/ChangePassword.ui \
    site_management/SiteManagement.ui \
    site_management/AddSite.ui \
    Revision.ui \
    MultiCaliDialog.ui \
    TdlasSettings.ui

include(qtxlsx.pri)

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../Qt5.5.0/5.5/mingw492_32/lib/ -lqwt
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../Qt5.5.0/5.5/mingw492_32/lib/ -lqwtd
#else:unix: LIBS += -L$$PWD/../../../Qt5.5.0/5.5/mingw492_32/lib/ -lqwt

#INCLUDEPATH += $$PWD/../../../Qt5.5.0/5.5/mingw492_32/include
#DEPENDPATH += $$PWD/../../../Qt5.5.0/5.5/mingw492_32/include

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-qtxlsx-Desktop_Qt_5_5_0_MinGW_32bit-Debug/lib/ -lQt5Xlsx
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-qtxlsx-Desktop_Qt_5_5_0_MinGW_32bit-Debug/lib/ -lQt5Xlsxd
#else:unix: LIBS += -L$$PWD/../build-qtxlsx-Desktop_Qt_5_5_0_MinGW_32bit-Debug/lib/ -lQt5Xlsx

#INCLUDEPATH += $$PWD/../build-qtxlsx-Desktop_Qt_5_5_0_MinGW_32bit-Debug/include
#DEPENDPATH += $$PWD/../build-qtxlsx-Desktop_Qt_5_5_0_MinGW_32bit-Debug/include
#QT       += xlsx
