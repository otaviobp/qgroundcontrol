/****************************************************************************
 *
 * Copyright (c) 2017, Intel Corporation
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCApplication.h"
#include "MavlinkVideoManager.h"
#include "QGCToolbox.h"
#include "LinkInterface.h"
#include "VideoUtils.h"

/**
 * @file
 *   @brief QGC MAVLink video streaming Manager
 *   @author Otavio Pontes <otavio.pontes@intel.com>
 */


QGC_LOGGING_CATEGORY(MavlinkVideoManagerLog, "MavlinkVideoManagerLog")

//-----------------------------------------------------------------------------
MavlinkVideoManager::MavlinkVideoManager() : QObject()
    , _cameraId(0)
    , _selectedStream(-1)
    , _mavlink(NULL)
{
}

//-----------------------------------------------------------------------------
MavlinkVideoManager::~MavlinkVideoManager()
{
}

//-----------------------------------------------------------------------------
void
MavlinkVideoManager::updateCurrentStream()
{
    if (_selectedStream >= 0)
        sendStreamGetCmd(VIDEO_STREAM_GET_CMD_STREAM_SETTINGS,
            ((StreamData *)_streamList[_selectedStream])->id);
}

//-----------------------------------------------------------------------------
void
MavlinkVideoManager::updateStreamList()
{
    setSelectedStream(-1);

    if (_mavlink == NULL) {
        _mavlink = qgcApp()->toolbox()->mavlinkProtocol();
        connect(_mavlink, &MAVLinkProtocol::videoHeartbeatInfo, this, &MavlinkVideoManager::_videoHeartbeatInfo);
        connect(_mavlink, &MAVLinkProtocol::messageReceived, this, &MavlinkVideoManager::_mavlinkMessageReceived);
    } else
        sendStreamGetCmd(VIDEO_STREAM_GET_CMD_STREAMS, 0);
}

//-----------------------------------------------------------------------------
void
MavlinkVideoManager::sendStreamGetCmd(int cmd, int streamId)
{
    mavlink_message_t message;
    mavlink_msg_video_stream_get_pack_chan(_mavlink->getSystemId(),
            _mavlink->getComponentId(), _cameraLink->mavlinkChannel(), &message,
            _cameraId, MAV_COMP_ID_VIDEO_STREAM, streamId, cmd);

    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int len = mavlink_msg_to_send_buffer(buffer, &message);

    _cameraLink->writeBytesSafe((const char*)buffer, len);
}

//-----------------------------------------------------------------------------
void
MavlinkVideoManager::sendStreamSetCmd(int streamId, int format, uint16_t video_resolution_h, uint16_t video_resolution_v)
{
    mavlink_message_t message;
    mavlink_msg_set_video_stream_settings_pack_chan(_mavlink->getSystemId(),
            _mavlink->getComponentId(), _cameraLink->mavlinkChannel(), &message,
            _cameraId, MAV_COMP_ID_VIDEO_STREAM, streamId, format,
            video_resolution_h, video_resolution_v, "");

    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int len = mavlink_msg_to_send_buffer(buffer, &message);

    _cameraLink->writeBytesSafe((const char*)buffer, len);
}

//-----------------------------------------------------------------------------
void
MavlinkVideoManager::_videoHeartbeatInfo(LinkInterface *link, int systemId)
{
    if (systemId == _cameraId)
        return;

    qCDebug(MavlinkVideoManagerLog) << "MavlinkVideoManager: First camera heartbeat info received";
    _cameraId = systemId;
    _cameraLink = link;
    sendStreamGetCmd(VIDEO_STREAM_GET_CMD_STREAMS, 0);
}

//-----------------------------------------------------------------------------
void
MavlinkVideoManager::setSelectedStream(int index)
{
    _selectedStream = index;
    if (index >= 0)
        sendStreamGetCmd(VIDEO_STREAM_GET_CMD_STREAM_SETTINGS,
            ((StreamData *)_streamList[index])->id);
    else {
        qDeleteAll(_streamList);
        _streamList.clear();
        emit streamListChanged();
    }

    emit currentSettingsChanged();
    emit selectedStreamChanged();
}

//-----------------------------------------------------------------------------
void
MavlinkVideoManager::_mavlinkMessageReceived(LinkInterface* link, mavlink_message_t message)
{
    Q_UNUSED(link);
    if (_cameraId == 0 || message.sysid != _cameraId)
        return;

    if (message.msgid == MAVLINK_MSG_ID_HEARTBEAT)
        return;

    qCDebug(MavlinkVideoManagerLog) << "MavlinkVideoManager: Message received: " << message.msgid;

    if (message.msgid == MAVLINK_MSG_ID_VIDEO_STREAM_URI) {
        mavlink_video_stream_uri_t stream;
        mavlink_msg_video_stream_uri_decode(&message, &stream);

        //Avoid adding duplicates
        for (QObject *data : _streamList)
            if (((StreamData *)data)->id == stream.id)
                return;

        _streamList.append(new StreamData(stream.id, stream.name, stream.uri));
        emit streamListChanged();
        if (_selectedStream < 0)
            setSelectedStream(0);

    } else if (message.msgid == MAVLINK_MSG_ID_VIDEO_STREAM_SETTINGS) {
        StreamData *cur = NULL;

        mavlink_video_stream_settings_t stream;
        mavlink_msg_video_stream_settings_decode(&message, &stream);

        for (QObject *data : _streamList)
            if (((StreamData *)data)->id == stream.id)
                cur = (StreamData *)data;

        if (!cur) {
            cur = new StreamData(stream.id, "", "");
            _streamList.append(new StreamData(stream.id, stream.name, stream.uri));
            emit streamListChanged();
        }

        cur->name = stream.name;
        cur->uri = stream.uri;
        cur->capabilities = stream.capabilities;
        cur->format = stream.format;
        cur->video_resolution_h = stream.video_resolution_h;
        cur->video_resolution_v = stream.video_resolution_v;

        for (int i = 0; i < StreamData::MAX_AVAILABLE_FORMATS; i++)
            cur->available_formats[i] = stream.available_formats[i];

        if (((StreamData *)_streamList[_selectedStream])->id == cur->id) {
            emit cur->nameChanged();
            emit currentSettingsChanged();
        }
    }
}

//-----------------------------------------------------------------------------
QString
MavlinkVideoManager::currentUri()
{
    if (_selectedStream < 0)
        return "";

    return ((StreamData *)_streamList[_selectedStream])->uri;
}

//-----------------------------------------------------------------------------
int
MavlinkVideoManager::currentCapabilities()
{
    if (_selectedStream < 0)
        return 0;

    return ((StreamData *)_streamList[_selectedStream])->capabilities;
}

//-----------------------------------------------------------------------------
int
MavlinkVideoManager::currentFormat()
{
    if (_selectedStream < 0)
        return 0;

    return ((StreamData *)_streamList[_selectedStream])->format;
}

//-----------------------------------------------------------------------------
int
MavlinkVideoManager::currentFormatIndex()
{
    StreamData *data;

    if (_selectedStream < 0)
        return -1;

    data = (StreamData *)_streamList[_selectedStream];
    for (int i = 0; i < StreamData::MAX_AVAILABLE_FORMATS && data->available_formats[i]; i++)
        if (data->format == data->available_formats[i])
            return i;
    return -1;
}

//-----------------------------------------------------------------------------
int
MavlinkVideoManager::currentResolutionH()
{
    if (_selectedStream < 0)
        return 0;

    return ((StreamData *)_streamList[_selectedStream])->video_resolution_h;
}

//-----------------------------------------------------------------------------
int
MavlinkVideoManager::currentResolutionV()
{
    if (_selectedStream < 0)
        return 0;

    return ((StreamData *)_streamList[_selectedStream])->video_resolution_v;
}

//-----------------------------------------------------------------------------
QStringList
MavlinkVideoManager::currentFormatList()
{
    QStringList list;
    StreamData *data;

    if (_selectedStream < 0)
        return list;

    data = (StreamData *)_streamList[_selectedStream];
    for (int i = 0; i < StreamData::MAX_AVAILABLE_FORMATS && data->available_formats[i]; i++)
        list << pixelFormatToFourCC((PixelFormat) data->available_formats[i]);

    return list;
}

//-----------------------------------------------------------------------------
void
MavlinkVideoManager::setCurrentFormatIndex(int formatIndex)
{
    StreamData *data;

    if (_selectedStream < 0 ||
        formatIndex >= StreamData::MAX_AVAILABLE_FORMATS || formatIndex < 0)
        return;

    data = (StreamData *)_streamList[_selectedStream];
    data->format = data->available_formats[formatIndex];

    sendStreamSetCmd(data->id, data->format, 0, 0);
    emit currentSettingsChanged();
}
//-----------------------------------------------------------------------------
void
MavlinkVideoManager::setResolution(int width, int height)
{
    StreamData *data;

    if (_selectedStream < 0)
        return;

    data = (StreamData *)_streamList[_selectedStream];

    sendStreamSetCmd(data->id, 0, width, height);
    emit currentSettingsChanged();
}
