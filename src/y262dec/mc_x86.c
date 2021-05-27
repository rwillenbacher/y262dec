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

#include "y262dec.h"

#include <emmintrin.h>


#define MC_FUNC_REF( name, i_width, i_height, hpelidx )	\
void y262dec_motcomp_##name##_put_sse2( uint8_t *pui8_src_, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride )		\
{														\
	int32_t i_y;										\
	const uint8_t *pui8_src = pui8_src_;				\
														\
	if( hpelidx == 0 )									\
	{													\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
			if( i_width == 8 )							\
			{											\
				__m128i m_m0;							\
				m_m0 = _mm_loadl_epi64( ( const __m128i * ) pui8_src );	\
				_mm_storel_epi64( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			else if( i_width == 16 )					\
			{											\
				__m128i m_m0;							\
				m_m0 = _mm_loadu_si128( ( const __m128i * ) pui8_src );	\
				_mm_storeu_si128( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			pui8_src += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else if( hpelidx == 1 )								\
	{													\
		const uint8_t *pui8_src1, *pui8_src2;			\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + 1;						\
														\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
			if( i_width == 8 )							\
			{											\
				__m128i m_m0, m_m1;						\
				m_m0 = _mm_loadl_epi64( ( const __m128i * ) pui8_src1 );	\
				m_m1 = _mm_loadl_epi64( ( const __m128i * ) pui8_src2 );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				_mm_storel_epi64( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			else if( i_width == 16 )					\
			{											\
				__m128i m_m0, m_m1;						\
				m_m0 = _mm_loadu_si128( ( const __m128i * ) pui8_src1 );	\
				m_m1 = _mm_loadu_si128( ( const __m128i * ) pui8_src2 );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				_mm_storeu_si128( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else if( hpelidx == 2 )								\
	{													\
		const uint8_t *pui8_src1, *pui8_src2;			\
		__m128i m_m0, m_m1;								\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + i_src_stride;			\
														\
		if( i_width == 8 )								\
		{												\
			m_m0 = _mm_loadl_epi64( ( const __m128i * ) pui8_src1 );	\
		}												\
		else if( i_width == 16 )						\
		{												\
			m_m0 = _mm_loadu_si128( ( const __m128i * ) pui8_src1 );	\
		}												\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
			if( i_width == 8 )							\
			{											\
				m_m1 = _mm_loadl_epi64( ( const __m128i * ) pui8_src2 );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				_mm_storel_epi64( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			else if( i_width == 16 )					\
			{											\
				m_m1 = _mm_loadu_si128( ( const __m128i * ) pui8_src2 );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				_mm_storeu_si128( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			m_m0 = m_m1;								\
			pui8_src2 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else												\
	{													\
		__m128i m_m0, m_m1, m_m2, m_m3, m_m5, m_m6;		\
		__m128i m_1 = _mm_set1_epi8( 1 );				\
		const uint8_t *pui8_src1, *pui8_src2, *pui8_src3, *pui8_src4;	\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + 1;						\
		pui8_src3 = pui8_src + i_src_stride;			\
		pui8_src4 = pui8_src + i_src_stride + 1;		\
														\
		if( i_width == 8 )								\
		{												\
			m_m0 = _mm_loadl_epi64( ( const __m128i * ) pui8_src1 );	\
			m_m2 = _mm_loadl_epi64( ( const __m128i * ) pui8_src2 );	\
		}												\
		else if( i_width == 16 )						\
		{												\
			m_m0 = _mm_loadu_si128( ( const __m128i * ) pui8_src1 );	\
			m_m2 = _mm_loadu_si128( ( const __m128i * ) pui8_src2 );	\
		}												\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
			if( i_width == 8 )							\
			{											\
				m_m1 = _mm_loadl_epi64( ( const __m128i * ) pui8_src3 );	\
				m_m3 = _mm_loadl_epi64( ( const __m128i * ) pui8_src4 );	\
				m_m6 = _mm_xor_si128( m_m0, m_m1 );		\
				m_m5 = _mm_xor_si128( m_m2, m_m3 );		\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				m_m2 = _mm_avg_epu8( m_m2, m_m3 );		\
				m_m6 = _mm_or_si128( m_m6, m_m5 );		\
				m_m5 = _mm_xor_si128( m_m0, m_m2 );		\
				m_m6 = _mm_and_si128( m_m6, m_m5 );		\
				m_m6 = _mm_and_si128( m_m6, m_1 );		\
				m_m0 = _mm_avg_epu8( m_m0, m_m2 );		\
				m_m0 = _mm_subs_epu8( m_m0, m_m6 );		\
				_mm_storel_epi64( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			else if( i_width == 16 )					\
			{											\
				m_m1 = _mm_loadu_si128( ( const __m128i * ) pui8_src3 );	\
				m_m3 = _mm_loadu_si128( ( const __m128i * ) pui8_src4 );	\
				m_m6 = _mm_xor_si128( m_m0, m_m1 );		\
				m_m5 = _mm_xor_si128( m_m2, m_m3 );		\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				m_m2 = _mm_avg_epu8( m_m2, m_m3 );		\
				m_m6 = _mm_or_si128( m_m6, m_m5 );		\
				m_m5 = _mm_xor_si128( m_m0, m_m2 );		\
				m_m6 = _mm_and_si128( m_m6, m_m5 );		\
				m_m6 = _mm_and_si128( m_m6, m_1 );		\
				m_m0 = _mm_avg_epu8( m_m0, m_m2 );		\
				m_m0 = _mm_subs_epu8( m_m0, m_m6 );		\
				_mm_storeu_si128( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			m_m0 = m_m1;								\
			m_m2 = m_m3;								\
			pui8_src3 += i_src_stride;					\
			pui8_src4 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
}														\
														\
														\
void y262dec_motcomp_##name##_avg_sse2( uint8_t *pui8_src_, int32_t i_src_stride, uint8_t *pui8_dst, int32_t i_dst_stride )	\
{														\
	int32_t i_y;										\
	const uint8_t *pui8_src = pui8_src_;				\
														\
	if( hpelidx == 0 )									\
	{													\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
			if( i_width == 8 )							\
			{											\
				__m128i m_m0, m_m1;						\
				m_m0 = _mm_loadl_epi64( ( const __m128i * ) pui8_src );	\
				m_m1 = _mm_loadl_epi64( ( const __m128i * ) pui8_dst );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				_mm_storel_epi64( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			else if( i_width == 16 )					\
			{											\
				__m128i m_m0, m_m1;						\
				m_m0 = _mm_loadu_si128( ( const __m128i * ) pui8_src );	\
				m_m1 = _mm_loadu_si128( ( const __m128i * ) pui8_dst );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				_mm_storeu_si128( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			pui8_src += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else if( hpelidx == 1 )								\
	{													\
		const uint8_t *pui8_src1, *pui8_src2;			\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + 1;						\
														\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
			if( i_width == 8 )							\
			{											\
				__m128i m_m0, m_m1;						\
				m_m0 = _mm_loadl_epi64( ( const __m128i * ) pui8_src1 );	\
				m_m1 = _mm_loadl_epi64( ( const __m128i * ) pui8_src2 );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				m_m1 = _mm_loadl_epi64( ( const __m128i * ) pui8_dst );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				_mm_storel_epi64( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			else if( i_width == 16 )					\
			{											\
				__m128i m_m0, m_m1;						\
				m_m0 = _mm_loadu_si128( ( const __m128i * ) pui8_src1 );	\
				m_m1 = _mm_loadu_si128( ( const __m128i * ) pui8_src2 );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				m_m1 = _mm_loadu_si128( ( const __m128i * ) pui8_dst );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				_mm_storeu_si128( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			pui8_src1 += i_src_stride;					\
			pui8_src2 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else if( hpelidx == 2 )								\
	{													\
		const uint8_t *pui8_src1, *pui8_src2;			\
		__m128i m_m0, m_m1, m_m2;						\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + i_src_stride;			\
														\
		if( i_width == 8 )								\
		{												\
			m_m0 = _mm_loadl_epi64( ( const __m128i * ) pui8_src1 );	\
		}												\
		else if( i_width == 16 )						\
		{												\
			m_m0 = _mm_loadu_si128( ( const __m128i * ) pui8_src1 );	\
		}												\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
			if( i_width == 8 )							\
			{											\
				m_m1 = _mm_loadl_epi64( ( const __m128i * ) pui8_src2 );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				m_m2 = _mm_loadl_epi64( ( const __m128i * ) pui8_dst );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m2 );		\
				_mm_storel_epi64( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			else if( i_width == 16 )					\
			{											\
				m_m1 = _mm_loadu_si128( ( const __m128i * ) pui8_src2 );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				m_m2 = _mm_loadu_si128( ( const __m128i * ) pui8_dst );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m2 );		\
				_mm_storeu_si128( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			m_m0 = m_m1;								\
			pui8_src2 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
	else												\
	{													\
		__m128i m_m0, m_m1, m_m2, m_m3, m_m5, m_m6;		\
		__m128i m_1 = _mm_set1_epi8( 1 );				\
		const uint8_t *pui8_src1, *pui8_src2, *pui8_src3, *pui8_src4;	\
														\
		pui8_src1 = pui8_src;							\
		pui8_src2 = pui8_src + 1;						\
		pui8_src3 = pui8_src + i_src_stride;			\
		pui8_src4 = pui8_src + i_src_stride + 1;		\
														\
		if( i_width == 8 )								\
		{												\
			m_m0 = _mm_loadl_epi64( ( const __m128i * ) pui8_src1 );	\
			m_m2 = _mm_loadl_epi64( ( const __m128i * ) pui8_src2 );	\
		}												\
		else if( i_width == 16 )						\
		{												\
			m_m0 = _mm_loadu_si128( ( const __m128i * ) pui8_src1 );	\
			m_m2 = _mm_loadu_si128( ( const __m128i * ) pui8_src2 );	\
		}												\
		for( i_y = 0; i_y < i_height; i_y++ )			\
		{												\
			if( i_width == 8 )							\
			{											\
				m_m1 = _mm_loadl_epi64( ( const __m128i * ) pui8_src3 );	\
				m_m3 = _mm_loadl_epi64( ( const __m128i * ) pui8_src4 );	\
				m_m6 = _mm_xor_si128( m_m0, m_m1 );		\
				m_m5 = _mm_xor_si128( m_m2, m_m3 );		\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				m_m2 = _mm_avg_epu8( m_m2, m_m3 );		\
				m_m6 = _mm_or_si128( m_m6, m_m5 );		\
				m_m5 = _mm_xor_si128( m_m0, m_m2 );		\
				m_m6 = _mm_and_si128( m_m6, m_m5 );		\
				m_m6 = _mm_and_si128( m_m6, m_1 );		\
				m_m0 = _mm_avg_epu8( m_m0, m_m2 );		\
				m_m0 = _mm_subs_epu8( m_m0, m_m6 );		\
				m_m2 = _mm_loadl_epi64( ( const __m128i * ) pui8_dst );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m2 );		\
				_mm_storel_epi64( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			else if( i_width == 16 )					\
			{											\
				m_m1 = _mm_loadu_si128( ( const __m128i * ) pui8_src3 );	\
				m_m3 = _mm_loadu_si128( ( const __m128i * ) pui8_src4 );	\
				m_m6 = _mm_xor_si128( m_m0, m_m1 );		\
				m_m5 = _mm_xor_si128( m_m2, m_m3 );		\
				m_m0 = _mm_avg_epu8( m_m0, m_m1 );		\
				m_m2 = _mm_avg_epu8( m_m2, m_m3 );		\
				m_m6 = _mm_or_si128( m_m6, m_m5 );		\
				m_m5 = _mm_xor_si128( m_m0, m_m2 );		\
				m_m6 = _mm_and_si128( m_m6, m_m5 );		\
				m_m6 = _mm_and_si128( m_m6, m_1 );		\
				m_m0 = _mm_avg_epu8( m_m0, m_m2 );		\
				m_m0 = _mm_subs_epu8( m_m0, m_m6 );		\
				m_m2 = _mm_loadu_si128( ( const __m128i * ) pui8_dst );	\
				m_m0 = _mm_avg_epu8( m_m0, m_m2 );		\
				_mm_storeu_si128( ( __m128i * )pui8_dst, m_m0 );	\
			}											\
			m_m0 = m_m1;								\
			m_m2 = m_m3;								\
			pui8_src3 += i_src_stride;					\
			pui8_src4 += i_src_stride;					\
			pui8_dst += i_dst_stride;					\
		}												\
	}													\
}


MC_FUNC_REF( 16x16_00, 16, 16, 0 );
MC_FUNC_REF( 16x16_01, 16, 16, 1 );
MC_FUNC_REF( 16x16_10, 16, 16, 2 );
MC_FUNC_REF( 16x16_11, 16, 16, 3 );

MC_FUNC_REF( 16x8_00, 16, 8, 0 );
MC_FUNC_REF( 16x8_01, 16, 8, 1 );
MC_FUNC_REF( 16x8_10, 16, 8, 2 );
MC_FUNC_REF( 16x8_11, 16, 8, 3 );

MC_FUNC_REF( 8x16_00, 8, 16, 0 );
MC_FUNC_REF( 8x16_01, 8, 16, 1 );
MC_FUNC_REF( 8x16_10, 8, 16, 2 );
MC_FUNC_REF( 8x16_11, 8, 16, 3 );

MC_FUNC_REF( 8x8_00, 8, 8, 0 );
MC_FUNC_REF( 8x8_01, 8, 8, 1 );
MC_FUNC_REF( 8x8_10, 8, 8, 2 );
MC_FUNC_REF( 8x8_11, 8, 8, 3 );

MC_FUNC_REF( 8x4_00, 8, 4, 0 );
MC_FUNC_REF( 8x4_01, 8, 4, 1 );
MC_FUNC_REF( 8x4_10, 8, 4, 2 );
MC_FUNC_REF( 8x4_11, 8, 4, 3 );



