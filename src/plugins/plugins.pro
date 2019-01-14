TEMPLATE      = lib
CONFIG       += plugin static
HEADERS       = ilbmplugin.h \
    chunk.h \
    bitmapheader.h \
    colormap.h \
    commodoreamiga.h
SOURCES       = ilbmplugin.cpp \
    chunk.cpp \
    bitmapheader.cpp \
    colormap.cpp \
    commodoreamiga.cpp
TARGET        = $$qtLibraryTarget(ilbm)
