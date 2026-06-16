QT -= gui
QT += core

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = QtConsoleApp

SOURCES += \
    main.cpp \
    node.cpp

HEADERS += \
    node.h

# Default deployment rules
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
