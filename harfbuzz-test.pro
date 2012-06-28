TEMPLATE = app
CONFIG += console
CONFIG -= qt

INCLUDEPATH = /usr/include/freetype2
LIBS += -lharfbuzz
SOURCES += main.cpp scrptrun.cpp \
    text_itemizer.cpp \
    text_shaping.cpp \
    text_layout.cpp

HEADERS += \
    scrptrun.h \
    text_itemizer.hpp \
    text_shaping.hpp \
    text_layout.hpp
