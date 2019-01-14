TEMPLATE      = lib
CONFIG       += plugin static
HEADERS       = ilbmplugin.h \
    chunk.h \
    bitmapheader.h \
    colormap.h \
    commodoreamiga.h \
    body.h
SOURCES       = ilbmplugin.cpp \
    chunk.cpp \
    bitmapheader.cpp \
    colormap.cpp \
    commodoreamiga.cpp \
    body.cpp
TARGET        = $$qtLibraryTarget(ilbm)
