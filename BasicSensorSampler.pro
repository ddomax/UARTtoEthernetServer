#-------------------------------------------------
#
# Project created by QtCreator 2019-11-04T22:11:51
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = BasicSensorSampler
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        bssamplerwidget.cpp \
        qcustomplot.cpp \
        api/app.cpp \
        api/appinit.cpp \
        api/iconhelper.cpp \
        usercontrol/frminputbox.cpp \
        usercontrol/frmmessagebox.cpp \
    relaychannel.cpp \
    chnlstatus.cpp

HEADERS += \
        bssamplerwidget.h \
        qcustomplot.h \
        api/app.h \
        api/appinit.h \
        api/iconhelper.h \
        api/myhelper.h \
        usercontrol/frminputbox.h \
        usercontrol/frmmessagebox.h \
    relaychannel.h \
    chnlstatus.h

FORMS += \
        bssamplerwidget.ui \
        usercontrol/frminputbox.ui \
        usercontrol/frmmessagebox.ui

win32:RC_FILE   = other/main.rc

RESOURCES += \
        pixmappicture.qrc \
        other/rc.qrc
