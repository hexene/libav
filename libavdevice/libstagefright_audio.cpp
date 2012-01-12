/*
 * Interface to the Android audio capture device via libmedia
 * similar $ANDROID/media/libstagefright/AudioSource.cpp
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

#include <binder/ProcessState.h>
#include <media/AudioRecord.h>
#include <new>

#include "libstagefright_audio.h"

#define AUDIO_BLOCK_SIZE 4096
using namespace android;
#define RCTX(context) ((class AudioRecord*)(ctx->priv_ptr))

int sf_audio_init(RecorderCtx *ctx)
{
     int err;

     uint32_t flags = android::AudioRecord::RECORD_AGC_ENABLE |
          AudioRecord::RECORD_NS_ENABLE  |
          AudioRecord::RECORD_IIR_ENABLE;
     int channels = ctx->channels > 1 ?
          AudioSystem::CHANNEL_IN_STEREO:
          AudioSystem::CHANNEL_IN_MONO;

     LOGV("source: %d, sample_rate: %d, channels: %d",
          ctx->input_source, ctx->sample_rate, ctx->channels);
     if (!(ctx->channels == 1 || ctx->channels == 2)) {
          err = EINVAL;
          goto fail;
     }

     if (RCTX(ctx)) {
          LOGE("AudioRecord is busy");
          err = EBUSY;
          goto fail;
     }
     ctx->priv_ptr = new AudioRecord(ctx->input_source, ctx->sample_rate,
                                     AudioSystem::PCM_16_BIT, channels,
                                     2 * AUDIO_BLOCK_SIZE / sizeof(int16_t),
                                     /* Enable ping-pong buffers */ flags);

     ctx->mInitCheck = RCTX(ctx)->initCheck();
     if (ctx->mInitCheck != OK) {
          LOGE("AudioRecord init failed %d", ctx->mInitCheck);
          err = EIO;
          goto fail;
     }

     ctx->frame_size = AUDIO_BLOCK_SIZE;
#if HAVE_BIGENDIAN
     ctx->codec_id = CODEC_ID_PCM_S16BE;
#else
     ctx->codec_id = CODEC_ID_PCM_S16LE;
#endif
     return 0;
fail:
     return err;
}

int sf_audio_read(RecorderCtx *ctx, AVPacket *pkt)
{
     int64_t delta, num, blost;
     class AudioRecord* recorder;
     av_log(ctx->fctx, AV_LOG_ERROR, "%s Enter sz:%d\n", __FUNCTION__, pkt->size);
     if (!ctx->mStarted) {
          status_t err = ctx->mInitCheck;
          if (ctx->mInitCheck == OK)
               err = RCTX(ctx)->start();
          if (err != OK) {
               LOGE("Failed to start AudioRecord : %d", err);
               return AVERROR(EIO);
          }
          ctx->mStarted = 1;
          ctx->start_ts = av_gettime();
          ctx->pos = 0;
     }
     recorder = RCTX(ctx);
     num = recorder->read(pkt->data, pkt->size);
     if (num <= 0){
          av_free_packet(pkt);
          pkt->size = 0;
          LOGE("Read from AudioRecord returns: %lld", num);
          if (num < 0)
               return AVERROR(EIO);
          else
               return AVERROR_EOF;
     }
     pkt->size = num;
     /* Advance position in to number bytes lost */
     blost = recorder->getInputFramesLost() << 1;
     ctx->pos += blost;
     delta = (ctx->pos * 1000000LL) / (ctx->sample_rate * ctx->channels);
     pkt->pts = ctx->start_ts + delta;
     /* Advance pos by the number of bytes returned */
     ctx->pos += num;
     if (blost)
          LOGV("Lost %lld bytes while recording", blost);

     return 0;
}

void sf_audio_destroy(RecorderCtx *ctx)
{
     class AudioRecord *rec = RCTX(ctx);
     if (!rec)
          return;
     if (ctx->mStarted) {
          rec->stop();
          ctx->mStarted = 0;
     }
     ctx->priv_ptr = NULL;
     delete rec;
}
