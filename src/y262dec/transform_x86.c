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

const int16_t rgi16_y262dec_idct_x86_tab1[ ] = {
	22724, 19265, 22724, 19265, 22724, 19265, 22724, 19265,
	12872, 4520, 12872, 4520, 12872, 4520, 12872, 4520,
	19265, -4520, 19265, -4520, 19265, -4520, 19265, -4520,
	-22724, -12872, -22724, -12872, -22724, -12872, -22724, -12872,
	12872, -22724, 12872, -22724, 12872, -22724, 12872, -22724,
	4520, 19265, 4520, 19265, 4520, 19265, 4520, 19265,
	4520, -12872, 4520, -12872, 4520, -12872, 4520, -12872,
	19265, -22724, 19265, -22724, 19265, -22724, 19265, -22724,
	21406, 8867, 21406, 8867, 21406, 8867, 21406, 8867,
	16383, 16383, 16383, 16383, 16383, 16383, 16383, 16383,
	8867, -21406, 8867, -21406, 8867, -21406, 8867, -21406,
	16383, -16383, 16383, -16383, 16383, -16383, 16383, -16383
};

const int16_t rgi16_y262dec_idct_x86_tab2[ ] = {
	16385, 21408, 16385, 8867, 16385, -8867, 16385, -21408,
	16385, 8867, -16385, -21408, -16385, 21408, 16385, -8867,
	22726, 19266, 19266, -4521, 12873, -22726, 4521, -12873,
	12873, 4521, -22726, -12873, 4521, 19266, 19266, -22726
};

static void y262dec_idct_sse2( int16_t *pi16_block, int32_t i_idx, uint8_t *pui8_dst, uint32_t ui_stride, int32_t i_add )
{
	int16_t *pi16_work = pi16_block;
	__m128i m_1024 = _mm_set1_epi32( 1024 );
	__m128i m_524288 = _mm_set1_epi32( 524288 );
	__m128i m_m0, m_m1, m_m2, m_m3, m_m4, m_m5, m_m6, m_m7;

	for( i_idx = 0; i_idx < 2; i_idx++ )
	{
		m_m0 = _mm_loadl_epi64( ( const __m128i * )( pi16_work + 8 ) );
		m_m2 = _mm_loadl_epi64( ( const __m128i * )( pi16_work + 24 ) );
		m_m1 = _mm_loadl_epi64( ( const __m128i * )( pi16_work + 40 ) );
		m_m3 = _mm_loadl_epi64( ( const __m128i * )( pi16_work + 56 ) );
		m_m0 = _mm_unpacklo_epi16( m_m0, m_m2 );
		m_m1 = _mm_unpacklo_epi16( m_m1, m_m3 );

		m_m2 = _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab1[ 0 ] );
		m_m7 = _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab1[ 8 ] );
		m_m2 = _mm_madd_epi16( m_m2, m_m0 );
		m_m7 = _mm_madd_epi16( m_m7, m_m1 );
		m_m2 = _mm_add_epi32( m_m2, m_m7 );

		m_m3 = _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab1[ 16 ] );
		m_m7 = _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab1[ 24 ] );
		m_m3 = _mm_madd_epi16( m_m3, m_m0 );
		m_m7 = _mm_madd_epi16( m_m7, m_m1 );
		m_m3 = _mm_add_epi32( m_m3, m_m7 );

		m_m4 = _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab1[ 32 ] );
		m_m7 = _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab1[ 40 ] );
		m_m4 = _mm_madd_epi16( m_m4, m_m0 );
		m_m7 = _mm_madd_epi16( m_m7, m_m1 );
		m_m4 = _mm_add_epi32( m_m4, m_m7 );

		m_m5 = _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab1[ 48 ] );
		m_m7 = _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab1[ 56 ] );
		m_m5 = _mm_madd_epi16( m_m5, m_m0 );
		m_m7 = _mm_madd_epi16( m_m7, m_m1 );
		m_m5 = _mm_add_epi32( m_m5, m_m7 );

		m_m6 = _mm_loadl_epi64( ( const __m128i * )( pi16_work + 16 ) );
		m_m0 = _mm_loadl_epi64( ( const __m128i * )( pi16_work + 48 ) );
		m_m6 = _mm_unpacklo_epi16( m_m6, m_m0 );
		m_m6 = _mm_madd_epi16( m_m6, _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab1[ 64 ] ) );

		m_m7 = _mm_loadl_epi64( ( const __m128i * )( pi16_work + 0 ) );
		m_m0 = _mm_loadl_epi64( ( const __m128i * )( pi16_work + 32 ) );
		m_m7 = _mm_unpacklo_epi16( m_m7, m_m0 );
		m_m7 = _mm_madd_epi16( m_m7, _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab1[ 72 ] ) );


		m_m0 = _mm_add_epi32( m_m6, m_m7 );
		m_m7 = _mm_sub_epi32( m_m7, m_m6 );

		m_m1 = _mm_add_epi32( m_m2, m_m0 );
		m_m0 = _mm_sub_epi32( m_m0, m_m2 );

		m_m0 = _mm_add_epi32( m_m0, m_1024 );
		m_m1 = _mm_add_epi32( m_m1, m_1024 );

		m_m0 = _mm_srai_epi32( m_m0, 11 );
		m_m1 = _mm_srai_epi32( m_m1, 11 );

		m_m0 = _mm_packs_epi32( m_m0, m_m0 );
		m_m1 = _mm_packs_epi32( m_m1, m_m1 );

		_mm_storel_epi64( ( __m128i * ) ( pi16_work + 56 ), m_m0 );

		m_m2 = _mm_add_epi32( m_m5, m_m7 );
		m_m7 = _mm_sub_epi32( m_m7, m_m5 );

		m_m2 = _mm_add_epi32( m_m2, m_1024 );
		m_m7 = _mm_add_epi32( m_m7, m_1024 );

		m_m2 = _mm_srai_epi32( m_m2, 11 );
		m_m7 = _mm_srai_epi32( m_m7, 11 );

		m_m2 = _mm_packs_epi32( m_m2, m_m2 );
		m_m7 = _mm_packs_epi32( m_m7, m_m7 );

		_mm_storel_epi64( ( __m128i * ) ( pi16_work + 24 ), m_m2 );

		m_m6 = _mm_loadl_epi64( ( const __m128i * )( pi16_work + 16 ) );
		m_m0 = _mm_loadl_epi64( ( const __m128i * )( pi16_work + 48 ) );
		m_m6 = _mm_unpacklo_epi16( m_m6, m_m0 );
		m_m6 = _mm_madd_epi16( m_m6, _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab1[ 80 ] ) );

		m_m2 = _mm_loadl_epi64( ( const __m128i * )( pi16_work + 0 ) );
		m_m0 = _mm_loadl_epi64( ( const __m128i * )( pi16_work + 32 ) );
		_mm_storel_epi64( ( __m128i * ) ( pi16_work ), m_m1 );
		_mm_storel_epi64( ( __m128i * ) ( pi16_work + 32 ), m_m7 );
		m_m2 = _mm_unpacklo_epi16( m_m2, m_m0 );
		m_m2 = _mm_madd_epi16( m_m2, _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab1[ 88 ] ) );


		m_m0 = _mm_add_epi32( m_m6, m_m2 );
		m_m2 = _mm_sub_epi32( m_m2, m_m6 );

		m_m7 = _mm_add_epi32( m_m3, m_m0 );
		m_m0 = _mm_sub_epi32( m_m0, m_m3 );

		m_m7 = _mm_add_epi32( m_m7, m_1024 );
		m_m0 = _mm_add_epi32( m_m0, m_1024 );

		m_m7 = _mm_srai_epi32( m_m7, 11 );
		m_m0 = _mm_srai_epi32( m_m0, 11 );

		m_m7 = _mm_packs_epi32( m_m7, m_m7 );
		m_m0 = _mm_packs_epi32( m_m0, m_m0 );

		_mm_storel_epi64( ( __m128i * ) ( pi16_work + 8 ), m_m7 );
		_mm_storel_epi64( ( __m128i * ) ( pi16_work + 48 ), m_m0 );

		m_m1 = _mm_add_epi32( m_m4, m_m2 );
		m_m2 = _mm_sub_epi32( m_m2, m_m4 );

		m_m1 = _mm_add_epi32( m_m1, m_1024 );
		m_m2 = _mm_add_epi32( m_m2, m_1024 );

		m_m1 = _mm_srai_epi32( m_m1, 11 );
		m_m2 = _mm_srai_epi32( m_m2, 11 );

		m_m1 = _mm_packs_epi32( m_m1, m_m1 );
		m_m2 = _mm_packs_epi32( m_m2, m_m2 );

		_mm_storel_epi64( ( __m128i * ) ( pi16_work + 16 ), m_m1 );
		_mm_storel_epi64( ( __m128i * ) ( pi16_work + 40 ), m_m2 );

		pi16_work += 4;
	}

	pi16_work = pi16_block;
	for( i_idx = 0; i_idx < 8; i_idx++ )
	{
		m_m7 = _mm_loadu_si128( ( const __m128i * )( pi16_work ) );

		m_m0 = _mm_shufflelo_epi16( m_m7, 0x88 );
		m_m0 = _mm_unpacklo_epi32( m_m0, m_m0 );
		m_m1 = _mm_shufflehi_epi16( m_m7, 0x88 );
		m_m1 = _mm_unpackhi_epi32( m_m1, m_m1 );

		m_m2 = _mm_shufflelo_epi16( m_m7, 0xdd );
		m_m2 = _mm_unpacklo_epi32( m_m2, m_m2 );
		m_m3 = _mm_shufflehi_epi16( m_m7, 0xdd );
		m_m3 = _mm_unpackhi_epi32( m_m3, m_m3 );

		m_m0 = _mm_madd_epi16( m_m0, _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab2[ 0 ] ) );
		m_m1 = _mm_madd_epi16( m_m1, _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab2[ 8 ] ) );
		m_m2 = _mm_madd_epi16( m_m2, _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab2[ 16 ] ) );
		m_m3 = _mm_madd_epi16( m_m3, _mm_loadu_si128( ( const __m128i * ) & rgi16_y262dec_idct_x86_tab2[ 24 ] ) );

		m_m0 = _mm_add_epi32( m_m0, m_m1 );
		m_m2 = _mm_add_epi32( m_m2, m_m3 );
		m_m1 = _mm_sub_epi32( m_m0, m_m2 );
		m_m0 = _mm_add_epi32( m_m0, m_m2 );
		m_m1 = _mm_shuffle_epi32( m_m1, 0x1b );

		m_m0 = _mm_add_epi32( m_m0, m_524288 );
		m_m1 = _mm_add_epi32( m_m1, m_524288 );
		m_m0 = _mm_srai_epi32( m_m0, 20 );
		m_m1 = _mm_srai_epi32( m_m1, 20 );

		m_m0 = _mm_packs_epi32( m_m0, m_m1 );

		if( i_add )
		{
			m_m1 = _mm_loadl_epi64( ( __m128i * ) pui8_dst );
			m_m1 = _mm_unpacklo_epi8( m_m1, _mm_setzero_si128( ) );
			m_m0 = _mm_adds_epi16( m_m0, m_m1 );
			m_m0 = _mm_packus_epi16( m_m0, m_m0 );
			_mm_storel_epi64( ( __m128i * ) pui8_dst, m_m0 );

		}
		else
		{
			m_m0 = _mm_packus_epi16( m_m0, m_m0 );
			_mm_storel_epi64( ( __m128i * ) pui8_dst, m_m0 );
		}

		pui8_dst += ui_stride;
		pi16_work += 8;
	}
}


void y262dec_idct_sse2_put( int16_t *pi16_block, int32_t i_idx, uint8_t *pui8_dst, uint32_t ui_stride )
{
	y262dec_idct_sse2( pi16_block, i_idx, pui8_dst, ui_stride, 0 );
}


void y262dec_idct_sse2_add( int16_t *pi16_block, int32_t i_idx, uint8_t *pui8_dst, uint32_t ui_stride )
{
	y262dec_idct_sse2( pi16_block, i_idx, pui8_dst, ui_stride, 1 );
}

