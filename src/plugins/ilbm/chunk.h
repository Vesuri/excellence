#ifndef CHUNK_H
#define CHUNK_H

#include <QByteArray>

class Chunk {
public:
    Chunk();
    Chunk(const QByteArray &id, const QByteArray &data);
    explicit Chunk(const QByteArray &chunkData);
    Chunk(const Chunk &chunk) = default;

    bool isNull() const;
    const QByteArray &id() const;
    unsigned size() const;
    QByteArray data(unsigned offset = 0, int length = -1) const;
    char byte(unsigned offset) const;
    unsigned char ubyte(unsigned offset) const;
    short word(unsigned offset) const;
    unsigned short uword(unsigned offset) const;
    unsigned ulong(unsigned offset) const;

    void setId(const QByteArray &id);
    void setSize(unsigned size);
    void setData(const QByteArray &data);
    void setByte(unsigned offset, char byte);
    void setUbyte(unsigned offset, unsigned char ubyte);
    void setWord(unsigned offset, short word);
    void setUword(unsigned offset, unsigned short uword);
    void setUlong(unsigned offset, unsigned ulong);

    QByteArray toByteArray() const;

private:
    QByteArray id_;
    QByteArray data_;
};

#endif // CHUNK_H
