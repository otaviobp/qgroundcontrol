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

#include "MAVLinkProtocol.h"

class StreamData : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString text MEMBER name NOTIFY nameChanged)

    public:
        StreamData() : QObject(), id(0), name(""), uri("") { }
        StreamData(int vId, QString vName, QString vUri) : QObject () {
            id = vId;
            name = vName;
            uri = vUri;
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

class MavlinkVideoManager : public QObject
{
    Q_OBJECT
public:
    MavlinkVideoManager();
    ~MavlinkVideoManager();

    Q_PROPERTY(QList<QObject*>  streamList              READ streamList             NOTIFY streamListChanged)
    Q_PROPERTY(int              selectedStream          READ selectedStream         WRITE  setSelectedStream        NOTIFY selectedStreamChanged)
    Q_PROPERTY(QString          currentUri              READ currentUri             NOTIFY currentSettingsChanged)
    Q_PROPERTY(int              currentCapabilities     READ currentCapabilities    NOTIFY currentSettingsChanged)
    Q_PROPERTY(int              currentFormat           READ currentFormat          NOTIFY currentSettingsChanged)
    Q_PROPERTY(int              currentFormatIndex      READ currentFormatIndex     WRITE  setCurrentFormatIndex    NOTIFY currentSettingsChanged)
    Q_PROPERTY(int              currentResolutionH      READ currentResolutionH     NOTIFY currentSettingsChanged)
    Q_PROPERTY(int              currentResolutionV      READ currentResolutionV     NOTIFY currentSettingsChanged)
    Q_PROPERTY(QStringList      currentFormatList       READ currentFormatList      NOTIFY currentSettingsChanged)

    QList<QObject *> streamList() {
        return _streamList;
    }
    int selectedStream() {
        return _selectedStream;
    }
    QString currentUri();
    int currentCapabilities();
    int currentFormat();
    int currentFormatIndex();
    int currentResolutionH();
    int currentResolutionV();
    QStringList currentFormatList();

    void updateStreamList();
    void setSelectedStream(int index);
    void setCurrentFormatIndex(int formatIndex);

signals:
    void streamListChanged();
    void selectedStreamChanged();
    void currentSettingsChanged();

private:
    void sendStreamGetCmd(int cmd, int streamId);
    void sendStreamSetCmd(int streamId, int format, uint16_t video_resolution_h, uint16_t video_resolution_v);

    int _cameraId;
    int _selectedStream;
    LinkInterface *_cameraLink;
    MAVLinkProtocol *_mavlink;
    QList<QObject*> _streamList;

private slots:
    void _mavlinkMessageReceived(LinkInterface *link, mavlink_message_t message);
    void _videoHeartbeatInfo(LinkInterface *link, int systemId);
public slots:
    void updateCurrentStream();
    void setResolution(int width, int height);
};

#endif // MAVLINK_VIDEO_MANAGER_H
