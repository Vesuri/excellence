#ifndef CHUNK_H
#define CHUNK_H

#include <QByteArray>

class Chunk {
public:
    Chunk();
    Chunk(const QByteArray &chunk);
    Chunk(const Chunk &chunk);

    bool isNull() const;
    QByteArray id() const;
    unsigned size() const;
    QByteArray data(int offset = 0, int length = -1) const;
    char byte(int offset) const;
    unsigned char ubyte(int offset) const;
    short word(int offset) const;
    unsigned short uword(int offset) const;
    unsigned ulong(int offset) const;

    void setId(const QByteArray &id);
    void setSize(const unsigned size);
    void setData(const QByteArray &data);
    void setByte(int offset, char byte);
    void setUbyte(int offset, unsigned char ubyte);
    void setWord(int offset, short word);
    void setUword(int offset, unsigned short uword);
    void setUlong(int offset, unsigned ulong);

private:
    QByteArray id_;
    QByteArray data_;
};

#endif // CHUNK_H
