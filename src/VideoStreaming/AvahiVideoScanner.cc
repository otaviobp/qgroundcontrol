/****************************************************************************
 *
 * Copyright (c) 2017, Intel Corporation
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "VideoUtils.h"

#include <QDebug>
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-qt5/qt-watch.h>

#include "AvahiVideoScanner.h"
#include "AvahiVideoManager.h"

/**
 * @file
 *   @brief
 *   @author Otavio Pontes <otavio.pontes@intel.com>
 */

static Format *parse_format(QString format)
{
    QStringList resolutions;
    Format *f = new Format();
    f->format = format.left(4);
    resolutions = format.mid(6, format.size() - 7).split(",");
    for (auto s : resolutions) {
        QStringList r = s.split("x");
        if (r.size() < 2)
            continue;
        FrameSize *fs = new FrameSize();
        fs->resolution = s;
        fs->width = r[0];
        fs->height = r[1];
        f->frame_sizes.append(fs);
    }

    return f;
}

static Stream *create_stream_from_data(const AvahiAddress *address, uint16_t port, const char *name, AvahiStringList *txt)
{
    Stream *s = new Stream();
    const char *split, *text;

    char address_str[AVAHI_ADDRESS_STR_MAX];
    avahi_address_snprint(address_str, sizeof(address_str), address);

    s->uri = "rtsp://";
    s->uri += address_str;
    s->uri += ":";
    s->uri += QString::number(port);
    s->uri += "/";
    s->uri += name;
    qDebug() << "Adding URI: " << s->uri;
    while(txt) {
        text = (const char *)avahi_string_list_get_text(txt);
        //txt = avahi_string_list_reverse(txt);
        split = strchr((const char *)avahi_string_list_get_text(txt), '=');
        if (!(*split))
            continue;
        s->avahi_name = name;
        if (strncmp("name", text, 4) == 0)
            s->name = QString(split + 1);
        else if (strncmp("frame_size", text, sizeof("frame_size") - 1) == 0)
            s->formats.append(parse_format(split + 1));

        txt = avahi_string_list_get_next(txt);
    }
    return s;
}

static void resolve_callback(AvahiServiceResolver *resolver, AvahiIfIndex interface,
                             AvahiProtocol protocol, AvahiResolverEvent event, const char *name,
                             const char *type, const char *domain, const char *host_name,
                             const AvahiAddress *address, uint16_t port, AvahiStringList *txt,
                             AvahiLookupResultFlags flags, void *userdata)
{
    assert(resolver);
    assert(userdata);
    AvahiVideoScanner *scanner = (AvahiVideoScanner *)userdata;

    switch (event) {
    case AVAHI_RESOLVER_FAILURE:
        qDebug() << "Avahi service resolve failed for service '" << name <<
            "' in domain '" << domain << "'. error: " <<
            avahi_strerror(avahi_client_errno(scanner->client));
        break;

    case AVAHI_RESOLVER_FOUND: {
        scanner->streamList.append(create_stream_from_data(address, port, name, txt));
        emit ((AvahiVideoManager *)scanner->man)->streamListChanged();
    }
    }

    avahi_service_resolver_free(resolver);
}
static void browse_callback(AvahiServiceBrowser *sb, AvahiIfIndex interface, AvahiProtocol protocol,
                            AvahiBrowserEvent event, const char *name, const char *type,
                            const char *domain, AvahiLookupResultFlags flags, void *userdata)
{
    AvahiVideoScanner *scanner = (AvahiVideoScanner *)userdata;
    AvahiClient *client = scanner->client;
    assert(sb);

    switch (event) {
    case AVAHI_BROWSER_FAILURE:
        qDebug() << "Avahi Browser Failure";
        break;

    case AVAHI_BROWSER_NEW:
        if (!(avahi_service_resolver_new(client, interface, protocol, name, type, domain,
                                         AVAHI_PROTO_UNSPEC, (AvahiLookupFlags)0, resolve_callback,
                                         scanner)))
            qDebug() << "Failed to start resolver for service. name: '" << name <<
                "' in domain: " << domain << ". error: " <<
                avahi_strerror(avahi_client_errno(client));
        break;

    case AVAHI_BROWSER_REMOVE:
        {
        QMutableListIterator<QObject *> i(scanner->streamList);
        while (i.hasNext()) {
            Stream *s = (Stream *)i.next();
            if (s->avahi_name == name) {
                i.remove();
                emit ((AvahiVideoManager *)scanner->man)->streamListChanged();
            }
        }
        }
        break;

    case AVAHI_BROWSER_ALL_FOR_NOW:
    case AVAHI_BROWSER_CACHE_EXHAUSTED:
        break;
    }
}

static void client_callback(AvahiClient *client, AvahiClientState state, void *userdata)
{
    assert(client);

    if (state == AVAHI_CLIENT_FAILURE) {
        qDebug() << "Creating avahi client failed.";
    }
}

AvahiVideoScanner::AvahiVideoScanner(void *vMan) : man(vMan)
{
    AvahiServiceBrowser *sb = NULL;
    int error;

    client = avahi_client_new(avahi_qt_poll_get(), (AvahiClientFlags)0, client_callback,
                              NULL, &error);
    if (!client) {
        qDebug() << "Failed to create avahi client: %s\n" << avahi_strerror(error);
        return;
    }

    if (!(sb = avahi_service_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "_rtsp._udp",
                                         NULL, (AvahiLookupFlags)0, browse_callback, this))) {
        qDebug() << "Failed to create avahi service browser: " << avahi_strerror(error);
        return;
    }

    return;
}
