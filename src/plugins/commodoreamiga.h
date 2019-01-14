#ifndef COMMODOREAMIGA_H
#define COMMODOREAMIGA_H

#include "chunk.h"

class QImage;

class CommodoreAmiga : public Chunk {
public:
    enum Modes {
        Lace = 0x0004,
        SuperHires = 0x0020,
        ExtraHalfbrite = 0x0080,
        HAM = 0x0800,
        Hires = 0x8000
    };

    CommodoreAmiga(const QImage &image);
    CommodoreAmiga(const Chunk &chunk);

    Modes modes() const;
};

#endif // COMMODOREAMIGA_H
