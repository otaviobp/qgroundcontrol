/****************************************************************************
 *
 * Copyright (c) 2016, Intel Corporation
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "VideoUtils.h"

#include <QDebug>

#include <string.h>

/**
 * @file
 *   @brief QGC Video Streaming Utils
 *   @author Ricardo de Almeida Gonzaga <ricardo.gonzaga@intel.com>
 */

extern QString pixelFormatToFourCC(PixelFormat f)
{
    QString fourCC = "";

    fourCC.append((char)(f & 0xFF));
    fourCC.append((char)((f & 0xFF00) >> 8));
    fourCC.append((char)((f & 0xFF0000) >> 16));
    fourCC.append((char)((f & 0xFF000000) >> 24));

    return fourCC;
}
