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
 *   @brief QGC MAVLink video streaming Manager
 *   @author Otavio Busatto Pontes <otavio.pontes@intel.com>
 */

#ifndef MAVLINK_VIDEO_MANAGER_H
#define MAVLINK_VIDEO_MANAGER_H

#include <QObject>
#include <QStringList>

#include "AvahiVideoScanner.h"
#include "MAVLinkProtocol.h"
#include "VideoUtils.h"

class StreamData : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString text MEMBER name NOTIFY nameChanged)

    public:
        StreamData() : QObject(), id(0), name(""), uri("") { }
        StreamData(int vId, QString vName, QString vUri) : QObject () {
            id = vId;
            name = vName;
            uri = vUri;
            format = PixelFormat::MJPG;
        }

        static const int MAX_AVAILABLE_FORMATS = 20;
        int id;
        QString name;
        QString uri;
        int capabilities;
        uint32_t format;
        uint32_t available_formats[MAX_AVAILABLE_FORMATS];
        uint16_t video_resolution_h;
        uint16_t video_resolution_v;
    signals:
        void nameChanged();
};

class AvahiVideoManager : public QObject
{
    Q_OBJECT
public:
    AvahiVideoManager();
    ~AvahiVideoManager();

    Q_PROPERTY(QList<QObject*>  streamList              READ streamList             NOTIFY streamListChanged)
    Q_PROPERTY(int              selectedStream          READ selectedStream         WRITE  setSelectedStream        NOTIFY selectedStreamChanged)
    Q_PROPERTY(QString          currentUri              READ currentUri             NOTIFY currentUriChanged)
    Q_PROPERTY(int              currentCapabilities     READ currentCapabilities    NOTIFY currentSettingsChanged)
    Q_PROPERTY(int              currentFormat           READ currentFormat          NOTIFY currentSettingsChanged)
    Q_PROPERTY(int              currentFormatIndex      READ currentFormatIndex     WRITE  setCurrentFormatIndex    NOTIFY currentFormatChanged)
    Q_PROPERTY(QList<QObject *>      currentFormatList       READ currentFormatList      NOTIFY currentFormatChanged)
    Q_PROPERTY(int              currentFrameSizeIndex      READ currentFrameSizeIndex     WRITE  setCurrentFrameSizeIndex    NOTIFY currentFrameChanged)
    Q_PROPERTY(QList<QObject *>      currentFrameSizeList       READ currentFrameSizeList      NOTIFY currentFrameChanged)

    QList<QObject *> streamList() {
        return avahiScanner.streamList;
    }
    int selectedStream() {
        return _selectedStream;
    }
    QString currentUri();
    int currentCapabilities();
    int currentFormat();
    int currentFormatIndex();
    int currentFrameSizeIndex();
    int currentResolutionH();
    int currentResolutionV();
    QList<QObject*> currentFormatList();
    QList<QObject*> currentFrameSizeList();

    void updateStreamList();
    void setSelectedStream(int index);
    void setCurrentFormatIndex(int formatIndex);
    void setCurrentFrameSizeIndex(int frameSizeIndex);

signals:
    void streamListChanged();
    void selectedStreamChanged();
    void currentFormatChanged();
    void currentSettingsChanged();
    void currentFrameChanged();
    void currentUriChanged();

private:
    void sendStreamGetCmd(int cmd, int streamId);
    void sendStreamSetCmd(int streamId, int format, uint16_t video_resolution_h, uint16_t video_resolution_v);

    int _cameraId;
    int _selectedStream;
    int _formatIndex;
    int _frameSizeIndex;
    LinkInterface *_cameraLink;
    MAVLinkProtocol *_mavlink;
    QList<QObject*> _streamList;
    AvahiVideoScanner avahiScanner;

private slots:
    void _mavlinkMessageReceived(LinkInterface *link, mavlink_message_t message);
    void _videoHeartbeatInfo(LinkInterface *link, int systemId);
    void streamListHasChanged();
public slots:
    void updateCurrentStream();
    void setResolution(int width, int height);
};

#endif // MAVLINK_VIDEO_MANAGER_H
