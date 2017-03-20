/****************************************************************************
 *
 * Copyright (c) 2017, Intel Corporation
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "VideoUtils.h"

#include <string.h>

/**
 * @file
 *   @brief QGC Video Streaming Utils
 *   @author Ricardo de Almeida Gonzaga <ricardo.gonzaga@intel.com>
 */

static QHash<PixelFormat, FormatPipelineElements> _pixelFormatElementsHash {
      // RGB formats
      { PixelFormat::RGB1,  { } },
      { PixelFormat::R444,  { } },
      { PixelFormat::RGB0,  { } },
      { PixelFormat::RGBP,  { } },
      { PixelFormat::RGBQ,  { } },
      { PixelFormat::RGBR,  { } },
      { PixelFormat::BGR3,  { } },
      { PixelFormat::RGB3,  { } },
      { PixelFormat::BGR4,  { } },
      { PixelFormat::RGB4,  { } },
      // Grey formats
      { PixelFormat::GREY,  { } },
      { PixelFormat::Y10,   { } },
      { PixelFormat::Y16,   { } },
      // Palette formats
      { PixelFormat::PAL8,  { } },
      // Luminance+Chrominance formats
      { PixelFormat::YVU9,  { } },
      { PixelFormat::YV12,  { } },
      { PixelFormat::YUYV,  { "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)YUYV", "rtpjpegdepay", NULL, "jpegdec" } },
      { PixelFormat::YYUV,  { } },
      { PixelFormat::YVYU,  { } },
      { PixelFormat::UYVY,  { } },
      { PixelFormat::VYUY,  { } },
      { PixelFormat::F422P, { } },
      { PixelFormat::F411P, { } },
      { PixelFormat::Y41P,  { } },
      { PixelFormat::Y444,  { } },
      { PixelFormat::YUVO,  { } },
      { PixelFormat::YUVP,  { } },
      { PixelFormat::YUV4,  { } },
      { PixelFormat::YUV9,  { } },
      { PixelFormat::YU12,  { } },
      { PixelFormat::HI24,  { } },
      { PixelFormat::HM12,  { } },
      // Two planes -- one Y, one Cr + Cb interleaved
      { PixelFormat::NV12,  { "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264", "rtph264depay", "h264parse", "avdec_h264" } },
      { PixelFormat::NV21,  { "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264", "rtph264depay", "h264parse", "avdec_h264" } },
      { PixelFormat::NV16,  { "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264", "rtph264depay", "h264parse", "avdec_h264" } },
      { PixelFormat::NV61,  { "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264", "rtph264depay", "h264parse", "avdec_h264" } },
      // Bayer formats
      { PixelFormat::BA81,  { } },
      { PixelFormat::GBRG,  { } },
      { PixelFormat::GRBG,  { } },
      { PixelFormat::RGGB,  { } },
      { PixelFormat::BG10,  { } },
      { PixelFormat::GB10,  { } },
      { PixelFormat::BA10,  { } },
      { PixelFormat::RG10,  { } },
      { PixelFormat::BD10,  { } },
      { PixelFormat::BYR2,  { } },
      // Compressed formats
      { PixelFormat::MJPG,  { "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)MJPG", "rtpjpegdepay", NULL, "jpegdec" } },
      { PixelFormat::JPEG,  { NULL, "rtpjpegdepay", NULL, "jpegdec" } },
      { PixelFormat::dvsd,  { NULL, "rtpdvdepay", NULL, "dvdec" } },
      { PixelFormat::MPEG,  { "application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264", "rtph264depay", "h264parse", "avdec_h264" } },
      // Vendor-specific formats
      { PixelFormat::CPIA,  { } },
      { PixelFormat::WNVA,  { } },
      { PixelFormat::S910,  { } },
      { PixelFormat::S920,  { } },
      { PixelFormat::PWC1,  { } },
      { PixelFormat::PWC2,  { } },
      { PixelFormat::E625,  { } },
      { PixelFormat::S501,  { } },
      { PixelFormat::S505,  { } },
      { PixelFormat::S508,  { } },
      { PixelFormat::S561,  { } },
      { PixelFormat::P207,  { } },
      { PixelFormat::M310,  { } },
      { PixelFormat::SONX,  { } },
      { PixelFormat::F905C, { } },
      { PixelFormat::PJPG,  { } },
      { PixelFormat::F0511, { } },
      { PixelFormat::F0518, { } },
      { PixelFormat::S680,  { } },
};

extern QString pixelFormatToFourCC(PixelFormat f)
{
    QString fourCC = "";

    fourCC.append((char)(f & 0xFF));
    fourCC.append((char)((f & 0xFF00) >> 8));
    fourCC.append((char)((f & 0xFF0000) >> 16));
    fourCC.append((char)((f & 0xFF000000) >> 24));

    return fourCC;
}

extern struct FormatPipelineElements getFormatPipelineElements(PixelFormat f)
{
    return _pixelFormatElementsHash[f];
}

#define fourcc(a,b,c,d) ((uint32_t)(a) << 0) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24)

extern int fourCCToPixelFormat(QString fourCC)
{
    if (fourCC.size() < 4)
        return 0;

    return fourcc(fourCC.at(0).toLatin1(), fourCC.at(1).toLatin1(), fourCC.at(2).toLatin1(), fourCC.at(3).toLatin1());
}

#undef fourcc
