/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


/**
 * @file
 *   @brief QGC Video Receiver
 *   @author Gus Grubba <mavlink@grubba.com>
 */

#include "VideoReceiver.h"

#include "VideoUtils.h"
#include "QGCLoggingCategory.h"

#include <QDebug>
#include <QUrl>

QGC_LOGGING_CATEGORY(VideoReceiverLog, "VideoReceiver")

VideoReceiver::VideoReceiver(QObject* parent)
    : QObject(parent)
#if defined(QGC_GST_STREAMING)
    , _pipeline(NULL)
    , _videoSink(NULL)
    , _socket(NULL)
    , _streamFormat(PixelFormat::MPEG)
    , _serverPresent(false)
#endif
{
#if defined(QGC_GST_STREAMING)
    _timer.setSingleShot(true);
    connect(&_timer, &QTimer::timeout, this, &VideoReceiver::_timeout);
#endif
}

VideoReceiver::~VideoReceiver()
{
#if defined(QGC_GST_STREAMING)
    stop();
    setVideoSink(NULL);
    if(_socket) {
        delete _socket;
    }
#endif
}

#if defined(QGC_GST_STREAMING)
void VideoReceiver::setVideoSink(GstElement* sink)
{
    if (_videoSink) {
        gst_object_unref(_videoSink);
        _videoSink = NULL;
    }
    if (sink) {
        _videoSink = sink;
        gst_object_ref_sink(_videoSink);
    }
}
#endif

#if defined(QGC_GST_STREAMING)
static void rtspsrc_pad_cb(GstElement *rtspsrc, GstPad* pad, GstElement *demux)
{
    gchar *dynamic_pad_name = gst_pad_get_name(pad);

    if (!gst_element_link_pads(rtspsrc, dynamic_pad_name, demux, "sink"))
        qCritical() << "VideoReceiver failed to link rtspsrc pad";

    g_free(dynamic_pad_name);
}
#endif

#if defined(QGC_GST_STREAMING)
void VideoReceiver::_connected()
{
    //-- Server showed up. Now we start the stream.
    _timer.stop();
    _socket->deleteLater();
    _socket = NULL;
    _serverPresent = true;
    restart();
}
#endif

#if defined(QGC_GST_STREAMING)
void VideoReceiver::_socketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    _socket->deleteLater();
    _socket = NULL;
    //-- Try again in 5 seconds
    _timer.start(5000);
}
#endif

#if defined(QGC_GST_STREAMING)
void VideoReceiver::_timeout()
{
    //-- If socket is live, we got no connection nor a socket error
    if(_socket) {
        delete _socket;
        _socket = NULL;
    }
    //-- RTSP will try to connect to the server. If it cannot connect,
    //   it will simply give up and never try again. Instead, we keep
    //   attempting a connection on this timer. Once a connection is
    //   found to be working, only then we actually start the stream.
    QUrl url(_uri);
    _socket = new QTcpSocket;
    connect(_socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &VideoReceiver::_socketError);
    connect(_socket, &QTcpSocket::connected, this, &VideoReceiver::_connected);
    //qDebug() << "Trying to connect to:" << url.host() << url.port();
    _socket->connectToHost(url.host(), url.port());
    _timer.start(5000);
}
#endif

void VideoReceiver::restart()
{
    stop();
    start();
}

void VideoReceiver::start()
{
#if defined(QGC_GST_STREAMING)
    if (_videoSink == NULL) {
        qCritical() << "VideoReceiver::start() failed because video sink is not set";
        return;
    }

    if (_uri.isEmpty()) {
        qCritical() << "VideoReceiver::start() failed because URI is not specified";
        return;
    }

    bool isUdp = _uri.contains("udp://");

    //-- For RTSP, check to see if server is there first
    if(!_serverPresent && !isUdp) {
        _timer.start(100);
        return;
    }

    bool gstreamer_success = false;

    GstElement*     dataSource  = NULL;
    GstCaps*        caps        = NULL;
    GstElement*     demux       = NULL;
    GstElement*     parser      = NULL;
    GstElement*     decoder     = NULL;

    do {
        if ((_pipeline = gst_pipeline_new("receiver")) == NULL) {
            qCritical() << "VideoReceiver::start() failed. Error with gst_pipeline_new()";
            break;
        }

        if(isUdp) {
            dataSource = gst_element_factory_make("udpsrc", "udp-source");
        } else {
            dataSource = gst_element_factory_make("rtspsrc", "rtsp-source");
        }

        if (!dataSource) {
            qCritical() << "VideoReceiver::start() failed. Error with data source for gst_element_factory_make()";
            break;
        }

        if(isUdp) {
            g_object_set(G_OBJECT(dataSource), "uri", qPrintable(_uri), "caps", caps, NULL);
        } else {
            g_object_set(G_OBJECT(dataSource), "location", qPrintable(_uri), "latency", 0, "udp-reconnect", 1, "timeout", 5000000, NULL);
        }

        FormatPipelineElements elements = getFormatPipelineElements(_streamFormat);

        qDebug() << "Creating pipeline for " << _uri;
        if (!elements.demux || !elements.decoder) {
            elements = getFormatPipelineElements(PixelFormat::JPEG);
            if (!elements.demux || !elements.decoder) {
                qCritical() << "VideoReceiver::start() failed. No GStreamer pipeline found for '" << pixelFormatToFourCC(_streamFormat) << "'";
                break;
            }

            qCDebug(VideoReceiverLog) << "VideoReceiver::start(): No GStreamer pipeline for '" << pixelFormatToFourCC(_streamFormat) << "'. Using the default pipeline.";
        } else
            qCDebug(VideoReceiverLog) << "VideoReceiver::start(): Using GStreamer pipeline for '" << pixelFormatToFourCC(_streamFormat) << "'.";

        if (isUdp && elements.caps) {
            if ((caps = gst_caps_from_string(elements.caps)) == NULL) {
                qCritical() << "VideoReceiver::start() failed. Error with gst_caps_from_string('" << elements.caps << "')";
                break;
            }

            g_object_set(G_OBJECT(dataSource), "caps", caps, NULL);
        }

        if ((demux = gst_element_factory_make(elements.demux, "demux")) == NULL) {
            qCritical() << "VideoReceiver::start() failed. Error with gst_element_factory_make('" << elements.demux << "')";
            break;
        }

        if(!isUdp) {
            g_signal_connect(dataSource, "pad-added", G_CALLBACK(rtspsrc_pad_cb), demux);
        }

        if (elements.parser) {
            if ((parser = gst_element_factory_make(elements.parser, "parser")) == NULL) {
                qCritical() << "VideoReceiver::start() failed. Error with gst_element_factory_make" << elements.parser << "')";
                break;
            }
        }

        if ((decoder = gst_element_factory_make(elements.decoder, "decoder")) == NULL) {
            qCritical() << "VideoReceiver::start() failed. Error with gst_element_factory_make('" << elements.decoder << "')";
            break;
        }

        if (parser) {
            gst_bin_add_many(GST_BIN(_pipeline), dataSource, demux, parser, decoder, _videoSink, NULL);
            if (gst_element_link_many(demux, parser, decoder, _videoSink, NULL) != (gboolean)TRUE) {
                qCritical() << "VideoReceiver::start() failed. Error with gst_element_link_many()";
                break;
            }
        } else {
            gst_bin_add_many(GST_BIN(_pipeline), dataSource, demux, decoder, _videoSink, NULL);
            if (gst_element_link_many(demux, decoder, _videoSink, NULL) != (gboolean)TRUE) {
                qCritical() << "VideoReceiver::start() failed. Error with gst_element_link_many()";
                break;
            }
        }

        dataSource = demux = parser = decoder = NULL;

        GstBus* bus = NULL;

        if ((bus = gst_pipeline_get_bus(GST_PIPELINE(_pipeline))) != NULL) {
            gst_bus_add_watch(bus, _onBusMessage, this);
            gst_object_unref(bus);
            bus = NULL;
        }

        gstreamer_success = gst_element_set_state(_pipeline, GST_STATE_PLAYING) != GST_STATE_CHANGE_FAILURE;

    } while(0);

    if (caps != NULL) {
        gst_caps_unref(caps);
        caps = NULL;
    }

    if (!gstreamer_success) {
        qCritical() << "VideoReceiver::start() failed";

        if (decoder != NULL) {
            gst_object_unref(decoder);
            decoder = NULL;
        }

        if (parser != NULL) {
            gst_object_unref(parser);
            parser = NULL;
        }

        if (demux != NULL) {
            gst_object_unref(demux);
            demux = NULL;
        }

        if (dataSource != NULL) {
            gst_object_unref(dataSource);
            dataSource = NULL;
        }

        if (_pipeline != NULL) {
            gst_object_unref(_pipeline);
            _pipeline = NULL;
        }
    }

#endif
}

void VideoReceiver::stop()
{
#if defined(QGC_GST_STREAMING)
    if (_pipeline != NULL) {
        gst_element_set_state(_pipeline, GST_STATE_NULL);
        gst_object_unref(_pipeline);
        _pipeline = NULL;
        _serverPresent = false;
    }
#endif
}

void VideoReceiver::setUri(const QString & uri)
{
    _uri = uri;
}

void VideoReceiver::setStreamFormat(PixelFormat format)
{
    _streamFormat = format;
}

#if defined(QGC_GST_STREAMING)
void VideoReceiver::_onBusMessage(GstMessage* msg)
{
    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
        stop();
        break;
    case GST_MESSAGE_ERROR:
        do {
            gchar* debug;
            GError* error;
            gst_message_parse_error(msg, &error, &debug);
            g_free(debug);
            qCritical() << error->message;
            g_error_free(error);
        } while(0);
        stop();
        break;
    default:
        break;
    }
}
#endif

#if defined(QGC_GST_STREAMING)
gboolean VideoReceiver::_onBusMessage(GstBus* bus, GstMessage* msg, gpointer data)
{
    Q_UNUSED(bus)
    Q_ASSERT(msg != NULL && data != NULL);
    VideoReceiver* pThis = (VideoReceiver*)data;
    pThis->_onBusMessage(msg);
    return TRUE;
}
#endif
