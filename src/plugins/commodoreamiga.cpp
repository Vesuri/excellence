#include <QByteArray>

#include "commodoreamiga.h"

CommodoreAmiga::CommodoreAmiga() : Chunk("CAMG", QByteArray(4, 0))
{
}

CommodoreAmiga::CommodoreAmiga(const Chunk &chunk) : Chunk(chunk)
{
}

CommodoreAmiga::Modes CommodoreAmiga::modes() const
{
    return static_cast<Modes>(ulong(0));
}
