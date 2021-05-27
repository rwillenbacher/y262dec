/*
Copyright (c) 2021, Ralf Willenbacher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

void y262dec_motcomp_16x16_00_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_16x16_01_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_16x16_10_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_16x16_11_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262dec_motcomp_16x8_00_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_16x8_01_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_16x8_10_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_16x8_11_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262dec_motcomp_8x16_00_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x16_01_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x16_10_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x16_11_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262dec_motcomp_8x8_00_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x8_01_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x8_10_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x8_11_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262dec_motcomp_8x4_00_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x4_01_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x4_10_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x4_11_put_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262dec_motcomp_16x16_00_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_16x16_01_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_16x16_10_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_16x16_11_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262dec_motcomp_16x8_00_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_16x8_01_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_16x8_10_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_16x8_11_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262dec_motcomp_8x16_00_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x16_01_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x16_10_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x16_11_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262dec_motcomp_8x8_00_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x8_01_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x8_10_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x8_11_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

void y262dec_motcomp_8x4_00_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x4_01_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x4_10_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );
void y262dec_motcomp_8x4_11_avg_sse2( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

