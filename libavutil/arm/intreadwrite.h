/*
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

#ifndef AVUTIL_ARM_INTREADWRITE_H
#define AVUTIL_ARM_INTREADWRITE_H

#include <stdint.h>
#include "config.h"

#if HAVE_FAST_UNALIGNED && HAVE_INLINE_ASM

#define AV_RN16 AV_RN16
static av_always_inline unsigned AV_RN16(const void *p)
{
    unsigned v;
    __asm__ ("ldrh %0, %1" : "=r"(v) : "m"(*(const uint16_t *)p));
    return v;
}

#define AV_WN16 AV_WN16
static av_always_inline void AV_WN16(void *p, uint16_t v)
{
    __asm__ ("strh %1, %0" : "=m"(*(uint16_t *)p) : "r"(v));
}

#define AV_RN32 AV_RN32
static av_always_inline uint32_t AV_RN32(const void *p)
{
    uint32_t v;
    __asm__ ("ldr  %0, %1" : "=r"(v) : "m"(*(const uint32_t *)p));
    return v;
}

#define AV_WN32 AV_WN32
static av_always_inline void AV_WN32(void *p, uint32_t v)
{
    __asm__ ("str  %1, %0" : "=m"(*(uint32_t *)p) : "r"(v));
}

#define AV_RN64 AV_RN64
static av_always_inline uint64_t AV_RN64(const void *p)
{
    uint64_t v;
    __asm__ ("ldr   %Q0, %1  \n\t"
             "ldr   %R0, %2  \n\t"
             : "=&r"(v)
             : "m"(*(const uint32_t*)p), "m"(*((const uint32_t*)p+1)));
    return v;
}

#define AV_WN64 AV_WN64
static av_always_inline void AV_WN64(void *p, uint64_t v)
{
    __asm__ ("str  %Q2, %0  \n\t"
             "str  %R2, %1  \n\t"
             : "=m"(*(uint32_t*)p), "=m"(*((uint32_t*)p+1))
             : "r"(v));
}

#endif /* HAVE_INLINE_ASM */

#endif /* AVUTIL_ARM_INTREADWRITE_H */
