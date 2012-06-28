TEMPLATE = app
CONFIG += console
CONFIG -= qt

INCLUDEPATH = /usr/include/freetype2
LIBS += -lharfbuzz
SOURCES += main.cpp scrptrun.cpp \
    text_itemizer.cpp

HEADERS += \
    scrptrun.h \
    text_itemizer.hpp
