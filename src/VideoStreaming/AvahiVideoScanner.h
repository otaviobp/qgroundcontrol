/****************************************************************************
 *
 * Copyright (c) 2017, Intel Corporation
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


/**
 * @file
 *   @brief
 *   @author Otavio Pontes <otavio.pontes@intel.com>
 */

#ifndef AVAHI_VIDEO_SCANNER
#define AVAHI_VIDEO_SCANNER

#include <QObject>
#include <QString>
#include <avahi-client/client.h>

class FrameSize : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString text MEMBER resolution NOTIFY nameChanged)

public:
    QString resolution;
    QString width, height;
    signals:
        void nameChanged();
};

class Format : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString text MEMBER format NOTIFY nameChanged)

public:
    QString format;
    QList<QObject *> frame_sizes;
    signals:
        void nameChanged();
};

class Stream : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString text MEMBER name NOTIFY nameChanged)
public:
    ~Stream() { }
    QString name;
    QString avahi_name;
    QString uri;
    QList<QObject*> formats;
    signals:
        void nameChanged();
};

class AvahiVideoScanner {
public:
    AvahiVideoScanner(void *vMan);
    AvahiClient *client;
    QList<QObject*> streamList;
    void *man;
};


#endif // AVAHI_VIDEO_SCANNER
