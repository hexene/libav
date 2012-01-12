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

#ifndef AVDEVICE_LIBSTAGEFRIGHT_AUDIO_H
#define AVDEVICE_LIBSTAGEFRIGHT_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "libavutil/log.h"
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libstagefright_audio.h"

/* AudioRecorder context */
typedef struct RecorderCtx {
    AVFormatContext *fctx;
    uint32_t input_source;
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t frame_size;	/* in bytes */
    uint64_t pos;		/* virtual position in bytes */
    uint64_t start_ts;		/* initial timestamp */
    enum CodecID codec_id;
    void *priv_ptr;		/* class AudioRecord */
    int mInitCheck;
    unsigned int mStarted:1;
} RecorderCtx;

    int sf_audio_init(RecorderCtx *ctx);
    int sf_audio_read(RecorderCtx *ctx, AVPacket *pkt);
    void sf_audio_destroy(RecorderCtx *ctx);

#ifdef __cplusplus
}
#endif

#endif /* AVDEVICE_LIBSTAGEFRIGHT_AUDIO_H */
