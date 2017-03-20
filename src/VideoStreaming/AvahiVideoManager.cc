/****************************************************************************
 *
 * Copyright (c) 2017, Intel Corporation
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCApplication.h"
#include "AvahiVideoManager.h"
#include "QGCToolbox.h"
#include "LinkInterface.h"
#include "VideoUtils.h"

/**
 * @file
 *   @brief QGC MAVLink video streaming Manager
 *   @author Otavio Pontes <otavio.pontes@intel.com>
 */


QGC_LOGGING_CATEGORY(AvahiVideoManagerLog, "AvahiVideoManagerLog")

//-----------------------------------------------------------------------------
AvahiVideoManager::AvahiVideoManager() : QObject()
    , _cameraId(0)
    , _selectedStream(-1)
    , _formatIndex(-1)
    , _frameSizeIndex(-1)
    , _mavlink(NULL)
    , avahiScanner(this)
{
    connect(this, &AvahiVideoManager::streamListChanged, this, &AvahiVideoManager::streamListHasChanged);
}

//-----------------------------------------------------------------------------
AvahiVideoManager::~AvahiVideoManager()
{
}

void
AvahiVideoManager::streamListHasChanged()
{
    if (avahiScanner.streamList.size() > 0) {
        setSelectedStream(-1);
    }
}

//-----------------------------------------------------------------------------
void
AvahiVideoManager::updateCurrentStream()
{
}

//-----------------------------------------------------------------------------
void
AvahiVideoManager::updateStreamList()
{
//    setSelectedStream(-1);
}

//-----------------------------------------------------------------------------
void
AvahiVideoManager::sendStreamGetCmd(int cmd, int streamId)
{
    Q_UNUSED(cmd);
    Q_UNUSED(streamId);
}

//-----------------------------------------------------------------------------
void
AvahiVideoManager::sendStreamSetCmd(int streamId, int format, uint16_t video_resolution_h, uint16_t video_resolution_v)
{
    Q_UNUSED(streamId);
    Q_UNUSED(format);
    Q_UNUSED(video_resolution_h);
    Q_UNUSED(video_resolution_v);
}

//-----------------------------------------------------------------------------
void
AvahiVideoManager::_videoHeartbeatInfo(LinkInterface *link, int systemId)
{
    Q_UNUSED(link);
    Q_UNUSED(systemId);
}

//-----------------------------------------------------------------------------
void
AvahiVideoManager::setSelectedStream(int index)
{
    _selectedStream = index;
    _formatIndex = -1;
    _frameSizeIndex = -1;

    emit currentFormatChanged();
    emit currentFrameChanged();
    emit currentUriChanged();
    emit currentSettingsChanged();
}

//-----------------------------------------------------------------------------
void
AvahiVideoManager::_mavlinkMessageReceived(LinkInterface* link, mavlink_message_t message)
{
}

//-----------------------------------------------------------------------------
QString
AvahiVideoManager::currentUri()
{
    QString uri,h,w;

    if (_selectedStream < 0)
        return "";
    uri = ((Stream *)avahiScanner.streamList[_selectedStream])->uri;

    if (_formatIndex < 0 || _frameSizeIndex < 0)
        return uri;

    w = ((FrameSize *)((Format *)((Stream *)avahiScanner.streamList[_selectedStream])->formats[_formatIndex])->frame_sizes[_frameSizeIndex])->width;
    h = ((FrameSize *)((Format *)((Stream *)avahiScanner.streamList[_selectedStream])->formats[_formatIndex])->frame_sizes[_frameSizeIndex])->height;
    return uri + "?width=" + w + "&height=" + h;
}

//-----------------------------------------------------------------------------
int
AvahiVideoManager::currentCapabilities()
{
    return 0;
}

//-----------------------------------------------------------------------------
int
AvahiVideoManager::currentFormat()
{
    if (_selectedStream >= 0) {
        Stream *s = ((Stream *)avahiScanner.streamList[_selectedStream]);
        int index = _formatIndex < 0 ? 0 : _formatIndex;
        return fourCCToPixelFormat(((Format *)s->formats[index])->format);
    }

    return PixelFormat::MJPG;
}

//-----------------------------------------------------------------------------
int
AvahiVideoManager::currentFormatIndex()
{
    return _formatIndex;
}

//-----------------------------------------------------------------------------
int
AvahiVideoManager::currentFrameSizeIndex()
{
    return _frameSizeIndex;
}

//-----------------------------------------------------------------------------
QList<QObject *>
AvahiVideoManager::currentFrameSizeList()
{
    if (_selectedStream < 0 || _formatIndex < 0) {
        QList<QObject*> list;
        return list;
    }

    return ((Format *)((Stream *)avahiScanner.streamList[_selectedStream])->formats[_formatIndex])->frame_sizes;
}

//-----------------------------------------------------------------------------
QList<QObject *>
AvahiVideoManager::currentFormatList()
{
    if (_selectedStream < 0) {
        QList<QObject*> list;
        return list;
    }

    return ((Stream *)avahiScanner.streamList[_selectedStream])->formats;
}

//-----------------------------------------------------------------------------
void
AvahiVideoManager::setCurrentFrameSizeIndex(int frameSizeIndex)
{
    _frameSizeIndex = frameSizeIndex;
    emit currentUriChanged();
    emit currentSettingsChanged();
}

//-----------------------------------------------------------------------------
void
AvahiVideoManager::setCurrentFormatIndex(int formatIndex)
{
    _formatIndex = formatIndex;
    _frameSizeIndex = -1;
    emit currentFrameChanged();
    emit currentUriChanged();
}

//-----------------------------------------------------------------------------
void
AvahiVideoManager::setResolution(int width, int height)
{
}
