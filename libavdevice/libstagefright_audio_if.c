/*
 * Interface to the Android audio capture device via libmedia
 *
 * Copyright (C) 2012 Dmitry Monakhov
 *
 * This file is part of Libav.
 *
 * Libav is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Libav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Libav; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libstagefright_audio.h"

typedef struct
{
    AVClass *class;
    RecorderCtx rctx;
}ASndData;

static int audio_read_header(AVFormatContext *s1, AVFormatParameters *ap)
{
    RecorderCtx *ctx = s1->priv_data;
    AVStream *st;
    int ret;

    st = av_new_stream(s1, 0);
    if (!st)
        return AVERROR(ENOMEM);

    ctx->fctx = s1;
    ret = sf_audio_init(ctx);
    if (ret != 0) {
        av_log(s1, AV_LOG_ERROR, "sf_audio_init failed: %d\n", ret);
        return AVERROR(ret);
    }

    /* take real parameters */
    st->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    st->codec->codec_id = ctx->codec_id;
    st->codec->sample_rate = ctx->sample_rate;
    st->codec->channels = ctx->channels;
    av_set_pts_info(st, 64, 1, 1000000);  /* 64 bits pts in us */
    return 0;
}

static int audio_read_packet(AVFormatContext *s1, AVPacket *pkt)
{
    RecorderCtx *ctx = s1->priv_data;
    int ret;

    if ((ret = av_new_packet(pkt, ctx->frame_size)) < 0)
        return ret;

    return sf_audio_read(ctx, pkt);
}

static int audio_read_close(AVFormatContext *s1)
{
    RecorderCtx *ctx = s1->priv_data;
    sf_audio_destroy(ctx);
    return 0;
}

static const AVOption options[] = {
    { "input_source", " Input source index",
      offsetof(RecorderCtx, input_source), FF_OPT_TYPE_INT, {.dbl = 1}, 0, INT_MAX, AV_OPT_FLAG_DECODING_PARAM },
    { "sample_rate", "",
      offsetof(RecorderCtx, sample_rate), FF_OPT_TYPE_INT, {.dbl = 48000}, 1, INT_MAX, AV_OPT_FLAG_DECODING_PARAM },
    { "channels", "", offsetof(RecorderCtx, channels), FF_OPT_TYPE_INT, {.dbl = 2},     1, 2, AV_OPT_FLAG_DECODING_PARAM },
    { NULL },
};

static const AVClass asnd_demuxer_class = {
    .class_name     = "asnd demuxer",
    .item_name      = av_default_item_name,
    .option         = options,
    .version        = LIBAVUTIL_VERSION_INT,
};

AVInputFormat ff_libstagefright_audio_demuxer = {
    "asnd",
    NULL_IF_CONFIG_SMALL("Android audio capture interface"),
    sizeof(ASndData),
    NULL,
    audio_read_header,
    audio_read_packet,
    audio_read_close,
    .flags = AVFMT_NOFILE,
    .priv_class = &asnd_demuxer_class,
};

