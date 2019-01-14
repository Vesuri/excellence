#include <QtEndian>

#include "chunk.h"

Chunk::Chunk() :
    id_("    "),
    data_()
{
}

Chunk::Chunk(const QByteArray &data) : Chunk()
{
    if (data.size() >= 4) {
        id_ = data.left(4);
    }

    if (data.size() >= 8) {
        qint32 size = qFromBigEndian<qint32>(data.data() + 4);
        data_ = data.mid(8, size);
    }
}

Chunk::Chunk(const Chunk &chunk) :
    id_(chunk.id_),
    data_(chunk.data_)
{
}

bool Chunk::isNull() const
{
    return size() == 0;
}

QByteArray Chunk::id() const
{
    return id_;
}

unsigned Chunk::size() const
{
    return static_cast<unsigned>(data_.size());
}

QByteArray Chunk::data(int offset, int length) const
{
    return data_.mid(offset, length);
}

char Chunk::byte(int offset) const
{
    return data_.at(offset);
}

unsigned char Chunk::ubyte(int offset) const
{
    return static_cast<unsigned char>(data_.at(offset));
}

short Chunk::word(int offset) const
{
    return qFromBigEndian<qint16>(data_.data() + offset);
}

unsigned short Chunk::uword(int offset) const
{
    return qFromBigEndian<quint16>(data_.data() + offset);
}

unsigned Chunk::ulong(int offset) const
{
    return qFromBigEndian<quint32>(data_.data() + offset);
}

void Chunk::setId(const QByteArray &id)
{
    id_ = id;
}

void Chunk::setSize(const unsigned size)
{
    data_.resize(size);
}

void Chunk::setData(const QByteArray &data)
{
    data_ = data;
}

void Chunk::setByte(int offset, char byte)
{
    data_[offset] = byte;
}

void Chunk::setUbyte(int offset, unsigned char ubyte)
{
    data_[offset] = static_cast<char>(ubyte);
}

void Chunk::setWord(int offset, short word)
{
    data_[offset] = static_cast<char>(word >> 8);
    data_[offset + 1] = static_cast<char>(word);
}

void Chunk::setUword(int offset, unsigned short uword)
{
    data_[offset] = static_cast<char>(uword >> 8);
    data_[offset + 1] = static_cast<char>(uword);
}

void Chunk::setUlong(int offset, unsigned ulong)
{
    data_[offset] = static_cast<char>(ulong >> 24);
    data_[offset + 1] = static_cast<char>(ulong >> 16);
    data_[offset + 2] = static_cast<char>(ulong >> 8);
    data_[offset + 3] = static_cast<char>(ulong);
}
