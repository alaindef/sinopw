TEMPLATE = lib
#CONFIG -= app_bundle
QT -= gui core
CONFIG -= qt
CONFIG += dll warn_on plugin no_plugin_name_prefix release thread

TARGET = PIDPlugin
QMAKE_EXTENSION_SHLIB = xpl

#INCLUDEPATH += /usr/include/python2.7
INCLUDEPATH += ./SDK/CHeaders/XPLM
INCLUDEPATH += ./SDK/CHeaders/Widgets
INCLUDEPATH += ./SDK/CHeaders/Wrappers

DEFINES += "XPLM200=1"
DEFINES += "XPLM210=0"
DEFINES += "XPLM300=0"
DEFINES += "XPLM301=0"
DEFINES += "APL=0"
DEFINES += "IBM=0"
DEFINES += "LIN=1"

QMAKE_CXXFLAGS += -fvisibility=hidden -std=c++0x -pthread
LIBS += -pthread

#XPSDK_HOME="./SDK"
#XP_HOME="C:\\X-Plane 11\\"
#LIBS += -lpthread
#LIBS += -lXPLM_64 -lXPWidgets_64

SOURCES += PIDPlugin.cpp

#target.path = ./build/($$TARGET)/64
#INSTALLS += target
