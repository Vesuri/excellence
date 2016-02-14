#ifndef ILBMPLUGIN_H
#define ILBMPLUGIN_H

#include <QImageIOPlugin>

class ILBMPlugin : public QImageIOPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface" FILE "ilbm.json")

public:
    ILBMPlugin(QObject *parent = 0);

    virtual Capabilities capabilities(QIODevice *device, const QByteArray & format) const;
    virtual QImageIOHandler *create(QIODevice *device, const QByteArray & format = QByteArray()) const;
};

#endif // ILBMPLUGIN_H
