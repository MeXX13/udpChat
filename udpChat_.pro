QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    addresswindow.cpp \
    chatwindow.cpp \
    htmlviewer.cpp \
    main.cpp \
    mainwindow.cpp \
    messageparser.cpp \
    udpclient.cpp

HEADERS += \
    addresswindow.h \
    chatwindow.h \
    commonTypes.h \
    htmlviewer.h \
    mainwindow.h \
    messageparser.h \
    random.hpp \
    udpclient.h

FORMS += \
    addresswindow.ui \
    chatwindow.ui \
    mainwindow.ui

INCLUDEPATH += /usr/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
