/*
Copyright (c) 2013-2021, Ralf Willenbacher
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

#include <arm_neon.h>

#include "y262dec.h"


#define MC_FUNC_NEON( name, i_width, i_height, hpelidx )	\
void y262dec_motcomp_##name##_put_neon( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride )		\
{														\
	int32_t i_x, i_y;									\
														\
	if( hpelidx == 0 )									\
	{													\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_a;                         \
                v8_a = vld1_u8( pui8_src );             \
                vst1_u8( pui8_dst, v8_a );              \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_a;                       \
                v16_a = vld1q_u8( pui8_src );           \
                vst1q_u8( pui8_dst, v16_a );            \
            }                                           \
			pui8_src += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else if( hpelidx == 1 )								\
	{													\
		uint8_t *pui8_src1, *pui8_src2;					\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + 1;						\
														\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_a, v8_b;                   \
                v8_a = vld1_u8( pui8_src1 );            \
                v8_b = vld1_u8( pui8_src2 );            \
                vst1_u8( pui8_dst, vrhadd_u8( v8_a, v8_b ) ); \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_a, v16_b;                \
                v16_a = vld1q_u8( pui8_src1 );          \
                v16_b = vld1q_u8( pui8_src2 );          \
                vst1q_u8( pui8_dst, vrhaddq_u8( v16_a, v16_b ) ); \
            }                                           \
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else if( hpelidx == 2 )								\
	{													\
		uint8_t *pui8_src1, *pui8_src2;					\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + i_src_stride;			\
														\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_a, v8_b;                   \
                v8_a = vld1_u8( pui8_src1 );            \
                v8_b = vld1_u8( pui8_src2 );            \
                vst1_u8( pui8_dst, vrhadd_u8( v8_a, v8_b ) ); \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_a, v16_b;                \
                v16_a = vld1q_u8( pui8_src1 );          \
                v16_b = vld1q_u8( pui8_src2 );          \
                vst1q_u8( pui8_dst, vrhaddq_u8( v16_a, v16_b ) ); \
            }                                           \
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else												\
	{													\
		uint8_t *pui8_src1, *pui8_src2, *pui8_src3, *pui8_src4;		\
        uint8x8_t v8_a, v8_b, v8_c, v8_d;               \
        uint8x16_t v16_a, v16_b, v16_c, v16_d;          \
        uint8x8_t v8_one = vmov_n_u8( 1 );              \
        uint8x16_t v16_one = vmovq_n_u8( 1 );           \
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + 1;						\
		pui8_src3 = pui8_src + i_src_stride;			\
		pui8_src4 = pui8_src + i_src_stride + 1;		\
														\
        if( i_width == 8 )                              \
        {                                               \
            v8_a = vld1_u8( pui8_src1 );                \
            v8_b = vld1_u8( pui8_src2 );                \
        }                                               \
        else if( i_width == 16 )                        \
        {                                               \
            v16_a = vld1q_u8( pui8_src1 );              \
            v16_b = vld1q_u8( pui8_src2 );              \
        }                                               \
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_carry0, v8_carry1;         \
                v8_c = vld1_u8( pui8_src3 );            \
                v8_d = vld1_u8( pui8_src4 );            \
                                                        \
                v8_carry0 = veor_u8( v8_a, v8_c);       \
                v8_carry1 = veor_u8( v8_b, v8_d);       \
                                                        \
                v8_a = vrhadd_u8( v8_a, v8_c );         \
                v8_b = vrhadd_u8( v8_b, v8_d );         \
                v8_carry0 = vorr_u8( v8_carry0, v8_carry1 ); \
                                                        \
                v8_carry1 = veor_u8( v8_a, v8_b);       \
                v8_carry0 = vand_u8( v8_carry0, v8_carry1 );    \
                v8_carry0 = vand_u8( v8_carry0, v8_one );   \
                                                        \
                v8_a = vrhadd_u8( v8_a, v8_b );         \
                v8_a = vsub_u8( v8_a, v8_carry0 );      \
                                                        \
                vst1_u8( pui8_dst, v8_a );              \
                                                        \
                v8_a = v8_c;                            \
                v8_b = v8_d;                            \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_carry0, v16_carry1;         \
                v16_c = vld1q_u8( pui8_src3 );            \
                v16_d = vld1q_u8( pui8_src4 );            \
                                                        \
                v16_carry0 = veorq_u8( v16_a, v16_c);       \
                v16_carry1 = veorq_u8( v16_b, v16_d);       \
                                                        \
                v16_a = vrhaddq_u8( v16_a, v16_c );         \
                v16_b = vrhaddq_u8( v16_b, v16_d );         \
                v16_carry0 = vorrq_u8( v16_carry0, v16_carry1 ); \
                                                        \
                v16_carry1 = veorq_u8( v16_a, v16_b);       \
                v16_carry0 = vandq_u8( v16_carry0, v16_carry1 );    \
                v16_carry0 = vandq_u8( v16_carry0, v16_one );   \
                                                        \
                v16_a = vrhaddq_u8( v16_a, v16_b );         \
                v16_a = vsubq_u8( v16_a, v16_carry0 );      \
                                                        \
                vst1q_u8( pui8_dst, v16_a );              \
                                                        \
                v16_a = v16_c;                            \
                v16_b = v16_d;                            \
            }                                           \
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_src3 += i_src_stride;					\
			pui8_src4 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
}														\
														\
														\
void y262dec_motcomp_##name##_avg_neon( uint8_t *pui8_src, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride )	\
{														\
	int32_t i_x, i_y;									\
														\
	if( hpelidx == 0 )									\
	{													\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_a, v8_z;                   \
                v8_a = vld1_u8( pui8_src );             \
                v8_z = vld1_u8( pui8_dst );             \
                vst1_u8( pui8_dst, vrhadd_u8( v8_a, v8_z ) ); \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_a, v16_z;                \
                v16_a = vld1q_u8( pui8_src );           \
                v16_z = vld1q_u8( pui8_dst );           \
                vst1q_u8( pui8_dst, vrhaddq_u8( v16_a, v16_z ) ); \
            }                                           \
			pui8_src += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else if( hpelidx == 1 )								\
	{													\
		uint8_t *pui8_src1, *pui8_src2;					\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + 1;						\
														\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_a, v8_b, v8_z;             \
                v8_a = vld1_u8( pui8_src1 );            \
                v8_b = vld1_u8( pui8_src2 );            \
                v8_z = vld1_u8( pui8_dst );             \
                v8_a = vrhadd_u8( v8_a, v8_b );         \
                vst1_u8( pui8_dst, vrhadd_u8( v8_a, v8_z ) ); \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_a, v16_b, v16_z;         \
                v16_a = vld1q_u8( pui8_src1 );          \
                v16_b = vld1q_u8( pui8_src2 );          \
                v16_z = vld1q_u8( pui8_dst );           \
                v16_a = vrhaddq_u8( v16_a, v16_b );     \
                vst1q_u8( pui8_dst, vrhaddq_u8( v16_a, v16_z ) ); \
            }                                           \
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else if( hpelidx == 2 )								\
	{													\
		uint8_t *pui8_src1, *pui8_src2;					\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + i_src_stride;			\
														\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_a, v8_b, v8_z;             \
                v8_a = vld1_u8( pui8_src1 );            \
                v8_b = vld1_u8( pui8_src2 );            \
                v8_z = vld1_u8( pui8_dst );             \
                v8_a = vrhadd_u8( v8_a, v8_b );         \
                vst1_u8( pui8_dst, vrhadd_u8( v8_a, v8_z ) ); \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_a, v16_b, v16_z;         \
                v16_a = vld1q_u8( pui8_src1 );          \
                v16_b = vld1q_u8( pui8_src2 );          \
                v16_z = vld1q_u8( pui8_dst );           \
                v16_a = vrhaddq_u8( v16_a, v16_b );     \
                vst1q_u8( pui8_dst, vrhaddq_u8( v16_a, v16_z ) ); \
            }                                           \
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else												\
	{													\
		uint8_t *pui8_src1, *pui8_src2, *pui8_src3, *pui8_src4;		\
        uint8x8_t v8_a, v8_b, v8_c, v8_d, v8_z;         \
        uint8x16_t v16_a, v16_b, v16_c, v16_d, v16_z;   \
        uint8x8_t v8_one = vmov_n_u8( 1 );              \
        uint8x16_t v16_one = vmovq_n_u8( 1 );           \
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + 1;						\
		pui8_src3 = pui8_src + i_src_stride;			\
		pui8_src4 = pui8_src + i_src_stride + 1;		\
														\
        if( i_width == 8 )                              \
        {                                               \
            v8_a = vld1_u8( pui8_src1 );                \
            v8_b = vld1_u8( pui8_src2 );                \
        }                                               \
        else if( i_width == 16 )                        \
        {                                               \
            v16_a = vld1q_u8( pui8_src1 );              \
            v16_b = vld1q_u8( pui8_src2 );              \
        }                                               \
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
            if( i_width == 8 )                          \
            {                                           \
                uint8x8_t v8_carry0, v8_carry1;         \
                v8_c = vld1_u8( pui8_src3 );            \
                v8_d = vld1_u8( pui8_src4 );            \
                v8_z = vld1_u8( pui8_dst );             \
                                                        \
                v8_carry0 = veor_u8( v8_a, v8_c);       \
                v8_carry1 = veor_u8( v8_b, v8_d);       \
                                                        \
                v8_a = vrhadd_u8( v8_a, v8_c );         \
                v8_b = vrhadd_u8( v8_b, v8_d );         \
                v8_carry0 = vorr_u8( v8_carry0, v8_carry1 ); \
                                                        \
                v8_carry1 = veor_u8( v8_a, v8_b);       \
                v8_carry0 = vand_u8( v8_carry0, v8_carry1 ); \
                v8_carry0 = vand_u8( v8_carry0, v8_one ); \
                                                        \
                v8_a = vrhadd_u8( v8_a, v8_b );         \
                v8_a = vsub_u8( v8_a, v8_carry0 );      \
                                                        \
                vst1_u8( pui8_dst, vrhadd_u8( v8_a, v8_z ) ); \
                                                        \
                v8_a = v8_c;                            \
                v8_b = v8_d;                            \
            }                                           \
            else if( i_width == 16 )                    \
            {                                           \
                uint8x16_t v16_carry0, v16_carry1;      \
                v16_c = vld1q_u8( pui8_src3 );          \
                v16_d = vld1q_u8( pui8_src4 );          \
                v16_z = vld1q_u8( pui8_dst );           \
                                                        \
                v16_carry0 = veorq_u8( v16_a, v16_c);   \
                v16_carry1 = veorq_u8( v16_b, v16_d);   \
                                                        \
                v16_a = vrhaddq_u8( v16_a, v16_c );     \
                v16_b = vrhaddq_u8( v16_b, v16_d );     \
                v16_carry0 = vorrq_u8( v16_carry0, v16_carry1 ); \
                                                        \
                v16_carry1 = veorq_u8( v16_a, v16_b);   \
                v16_carry0 = vandq_u8( v16_carry0, v16_carry1 ); \
                v16_carry0 = vandq_u8( v16_carry0, v16_one ); \
                                                        \
                v16_a = vrhaddq_u8( v16_a, v16_b );     \
                v16_a = vsubq_u8( v16_a, v16_carry0 );  \
                                                        \
                vst1q_u8( pui8_dst, vrhaddq_u8( v16_a, v16_z ) ); \
                                                        \
                v16_a = v16_c;                          \
                v16_b = v16_d;                          \
            }                                           \
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_src3 += i_src_stride;					\
			pui8_src4 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
}														\


MC_FUNC_NEON( 16x16_00, 16, 16, 0 );
MC_FUNC_NEON( 16x16_01, 16, 16, 1 );
MC_FUNC_NEON( 16x16_10, 16, 16, 2 );
MC_FUNC_NEON( 16x16_11, 16, 16, 3 );

MC_FUNC_NEON( 16x8_00, 16, 8, 0 );
MC_FUNC_NEON( 16x8_01, 16, 8, 1 );
MC_FUNC_NEON( 16x8_10, 16, 8, 2 );
MC_FUNC_NEON( 16x8_11, 16, 8, 3 );

MC_FUNC_NEON( 8x16_00, 8, 16, 0 );
MC_FUNC_NEON( 8x16_01, 8, 16, 1 );
MC_FUNC_NEON( 8x16_10, 8, 16, 2 );
MC_FUNC_NEON( 8x16_11, 8, 16, 3 );

MC_FUNC_NEON( 8x8_00, 8, 8, 0 );
MC_FUNC_NEON( 8x8_01, 8, 8, 1 );
MC_FUNC_NEON( 8x8_10, 8, 8, 2 );
MC_FUNC_NEON( 8x8_11, 8, 8, 3 );

MC_FUNC_NEON( 8x4_00, 8, 4, 0 );
MC_FUNC_NEON( 8x4_01, 8, 4, 1 );
MC_FUNC_NEON( 8x4_10, 8, 4, 2 );
MC_FUNC_NEON( 8x4_11, 8, 4, 3 );







