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

#include "y262dec.h"


#define RND1BITS ( 11 )
#define RND2BITS ( 31 - RND1BITS )

static const int16_t rgi16_y262_idct_cs1[ 8 ][ 8 ] = {
        { 16383,  16383,  16383,  16383,  16383,  16383,  16383,  16383 },
        { 22724,  19265,  12872,   4520,  -4520, -12872, -19265, -22724 },
        { 21406,   8867,  -8867, -21406, -21406,  -8867,   8867,  21406 },
        { 19265,  -4520, -22724, -12872,  12872,  22724,   4520, -19265 },
        { 16383, -16383, -16383,  16383,  16383, -16383, -16383,  16383 },
        { 12872, -22724,   4520,  19265, -19265,  -4520,  22724, -12872 },
        {  8867, -21406,  21406,  -8867,  -8867,  21406, -21406,   8867 },
        {  4520, -12872,  19265, -22724,  22724, -19265,  12872,  -4520 },
};
static const int16_t rgi16_y262_idct_cs2[ 8 ][ 8 ] = {
        { 16385,  16385,  16385,  16385,  16385,  16385,  16385,  16385 },
        { 22726,  19266,  12873,   4521,  -4521, -12873, -19266, -22726 },
        { 21408,   8867,  -8867, -21408, -21408,  -8867,   8867,  21408 },
        { 19266,  -4521, -22726, -12873,  12873,  22726,   4521, -19266 },
        { 16385, -16385, -16385,  16385,  16385, -16385, -16385,  16385 },
        { 12873, -22726,   4521,  19266, -19266,  -4521,  22726, -12873 },
        {  8867, -21408,  21408,  -8867,  -8867,  21408, -21408,   8867 },
        {  4521, -12873,  19266, -22726,  22726, -19266,  12873,  -4521 },
};


void y262dec_idct_put( int16_t *pi16_block, int32_t i_idx, uint8_t *pui8_dst, uint32_t ui_stride )
{
	int i_j, i_k;
	int16_t rgi16_tmp[ 64 ];
	int32_t rgi_e[ 4 ], rgi_o[ 4 ];
	int32_t rgi_ee[ 2 ], rgi_eo[ 2 ];


#define RND( x, y ) ( ( ( x ) + ( ( y ) ? ( 1 << ( y - 1 ) ) : 0 ) ) >> ( y ) )
#define MUL( x, m ) ( ( x ) * ( m ) )

	for( i_j = 0; i_j < 8; i_j++ )
	{
		rgi_o[ 0 ] = rgi16_y262_idct_cs1[ 1 ][ 0 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_y262_idct_cs1[ 3 ][ 0 ] * pi16_block[ i_j + 8 * 3 ] +
			rgi16_y262_idct_cs1[ 5 ][ 0 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_y262_idct_cs1[ 7 ][ 0 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_o[ 1 ] = rgi16_y262_idct_cs1[ 1 ][ 1 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_y262_idct_cs1[ 3 ][ 1 ] * pi16_block[ i_j + 8 * 3 ] +
			rgi16_y262_idct_cs1[ 5 ][ 1 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_y262_idct_cs1[ 7 ][ 1 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_o[ 2 ] = rgi16_y262_idct_cs1[ 1 ][ 2 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_y262_idct_cs1[ 3 ][ 2 ] * pi16_block[ i_j + 8 * 3 ] +
			rgi16_y262_idct_cs1[ 5 ][ 2 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_y262_idct_cs1[ 7 ][ 2 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_o[ 3 ] = rgi16_y262_idct_cs1[ 1 ][ 3 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_y262_idct_cs1[ 3 ][ 3 ] * pi16_block[ i_j + 8 * 3 ] +
			rgi16_y262_idct_cs1[ 5 ][ 3 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_y262_idct_cs1[ 7 ][ 3 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_eo[ 0 ] = rgi16_y262_idct_cs1[ 2 ][ 0 ] * pi16_block[ i_j + 8 * 2 ] + rgi16_y262_idct_cs1[ 6 ][ 0 ] * pi16_block[ i_j + 8 * 6 ];
		rgi_eo[ 1 ] = rgi16_y262_idct_cs1[ 2 ][ 1 ] * pi16_block[ i_j + 8 * 2 ] + rgi16_y262_idct_cs1[ 6 ][ 1 ] * pi16_block[ i_j + 8 * 6 ];
		rgi_ee[ 0 ] = rgi16_y262_idct_cs1[ 0 ][ 0 ] * pi16_block[ i_j + 8 * 0 ] + rgi16_y262_idct_cs1[ 4 ][ 0 ] * pi16_block[ i_j + 8 * 4 ];
		rgi_ee[ 1 ] = rgi16_y262_idct_cs1[ 0 ][ 1 ] * pi16_block[ i_j + 8 * 0 ] + rgi16_y262_idct_cs1[ 4 ][ 1 ] * pi16_block[ i_j + 8 * 4 ];

		rgi_e[ 0 ] = rgi_ee[ 0 ] + rgi_eo[ 0 ];
		rgi_e[ 1 ] = rgi_ee[ 1 ] + rgi_eo[ 1 ];
		rgi_e[ 2 ] = rgi_ee[ 1 ] - rgi_eo[ 1 ];
		rgi_e[ 3 ] = rgi_ee[ 0 ] - rgi_eo[ 0 ];

		for( i_k = 0; i_k < 4; i_k++ )
		{
			rgi16_tmp[ i_j + 8 * i_k ] = RND( rgi_e[ i_k ] + rgi_o[ i_k ], RND1BITS );
			rgi16_tmp[ i_j + 8 * ( i_k + 4 ) ] = RND( rgi_e[ 3 - i_k ] - rgi_o[ 3 - i_k ], RND1BITS );
		}
	}

	for( i_j = 0; i_j < 8; i_j++ )
	{
		rgi_e[ 0 ] = rgi16_y262_idct_cs2[ 0 ][ 0 ] * rgi16_tmp[ i_j * 8 + 0 ] + rgi16_y262_idct_cs2[ 2 ][ 0 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_y262_idct_cs2[ 4 ][ 0 ] * rgi16_tmp[ i_j * 8 + 4 ] + rgi16_y262_idct_cs2[ 6 ][ 0 ] * rgi16_tmp[ i_j * 8 + 6 ];
		rgi_e[ 1 ] = rgi16_y262_idct_cs2[ 0 ][ 1 ] * rgi16_tmp[ i_j * 8 + 0 ] + rgi16_y262_idct_cs2[ 2 ][ 1 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_y262_idct_cs2[ 4 ][ 1 ] * rgi16_tmp[ i_j * 8 + 4 ] + rgi16_y262_idct_cs2[ 6 ][ 1 ] * rgi16_tmp[ i_j * 8 + 6 ];
		rgi_e[ 2 ] = rgi16_y262_idct_cs2[ 0 ][ 1 ] * rgi16_tmp[ i_j * 8 + 0 ] + -rgi16_y262_idct_cs2[ 2 ][ 1 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_y262_idct_cs2[ 4 ][ 1 ] * rgi16_tmp[ i_j * 8 + 4 ] + -rgi16_y262_idct_cs2[ 6 ][ 1 ] * rgi16_tmp[ i_j * 8 + 6 ];
		rgi_e[ 3 ] = rgi16_y262_idct_cs2[ 0 ][ 0 ] * rgi16_tmp[ i_j * 8 + 0 ] + -rgi16_y262_idct_cs2[ 2 ][ 0 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_y262_idct_cs2[ 4 ][ 0 ] * rgi16_tmp[ i_j * 8 + 4 ] + -rgi16_y262_idct_cs2[ 6 ][ 0 ] * rgi16_tmp[ i_j * 8 + 6 ];

		rgi_o[ 0 ] = rgi16_y262_idct_cs2[ 1 ][ 0 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_y262_idct_cs2[ 3 ][ 0 ] * rgi16_tmp[ i_j * 8 + 3 ] +
			rgi16_y262_idct_cs2[ 5 ][ 0 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_y262_idct_cs2[ 7 ][ 0 ] * rgi16_tmp[ i_j * 8 + 7 ];
		rgi_o[ 1 ] = rgi16_y262_idct_cs2[ 1 ][ 1 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_y262_idct_cs2[ 3 ][ 1 ] * rgi16_tmp[ i_j * 8 + 3 ] +
			rgi16_y262_idct_cs2[ 5 ][ 1 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_y262_idct_cs2[ 7 ][ 1 ] * rgi16_tmp[ i_j * 8 + 7 ];
		rgi_o[ 2 ] = rgi16_y262_idct_cs2[ 1 ][ 2 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_y262_idct_cs2[ 3 ][ 2 ] * rgi16_tmp[ i_j * 8 + 3 ] +
			rgi16_y262_idct_cs2[ 5 ][ 2 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_y262_idct_cs2[ 7 ][ 2 ] * rgi16_tmp[ i_j * 8 + 7 ];
		rgi_o[ 3 ] = rgi16_y262_idct_cs2[ 1 ][ 3 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_y262_idct_cs2[ 3 ][ 3 ] * rgi16_tmp[ i_j * 8 + 3 ] +
			rgi16_y262_idct_cs2[ 5 ][ 3 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_y262_idct_cs2[ 7 ][ 3 ] * rgi16_tmp[ i_j * 8 + 7 ];

#define CLAMP256( x ) ( ( x ) < 0 ? 0 : ( ( x ) > 255 ? 255 : ( x ) ) )
		for( i_k = 0; i_k < 4; i_k++ )
		{
			int32_t i_pel0, i_pel1;
			i_pel0 = RND( rgi_e[ i_k ] + rgi_o[ i_k ], RND2BITS );
			i_pel1 = RND( rgi_e[ 3 - i_k ] - rgi_o[ 3 - i_k ], RND2BITS );
			pui8_dst[ i_j * ui_stride + i_k ] = CLAMP256( i_pel0 );
			pui8_dst[ i_j * ui_stride + ( i_k + 4 ) ] = CLAMP256( i_pel1 );
		}
	}
}


void y262dec_idct_add( int16_t *pi16_block, int32_t i_idx, uint8_t *pui8_dst, uint32_t ui_stride )
{
	int i_j, i_k;
	int16_t rgi16_tmp[ 64 ];
	int32_t rgi_e[ 4 ], rgi_o[ 4 ];
	int32_t rgi_ee[ 2 ], rgi_eo[ 2 ];


#define RND( x, y ) ( ( ( x ) + ( ( y ) ? ( 1 << ( y - 1 ) ) : 0 ) ) >> ( y ) )
#define MUL( x, m ) ( ( x ) * ( m ) )

	for( i_j = 0; i_j < 8; i_j++ )
	{
		rgi_o[ 0 ] = rgi16_y262_idct_cs1[ 1 ][ 0 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_y262_idct_cs1[ 3 ][ 0 ] * pi16_block[ i_j + 8 * 3 ] +
			rgi16_y262_idct_cs1[ 5 ][ 0 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_y262_idct_cs1[ 7 ][ 0 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_o[ 1 ] = rgi16_y262_idct_cs1[ 1 ][ 1 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_y262_idct_cs1[ 3 ][ 1 ] * pi16_block[ i_j + 8 * 3 ] +
			rgi16_y262_idct_cs1[ 5 ][ 1 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_y262_idct_cs1[ 7 ][ 1 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_o[ 2 ] = rgi16_y262_idct_cs1[ 1 ][ 2 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_y262_idct_cs1[ 3 ][ 2 ] * pi16_block[ i_j + 8 * 3 ] +
			rgi16_y262_idct_cs1[ 5 ][ 2 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_y262_idct_cs1[ 7 ][ 2 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_o[ 3 ] = rgi16_y262_idct_cs1[ 1 ][ 3 ] * pi16_block[ i_j + 8 * 1 ] + rgi16_y262_idct_cs1[ 3 ][ 3 ] * pi16_block[ i_j + 8 * 3 ] +
			rgi16_y262_idct_cs1[ 5 ][ 3 ] * pi16_block[ i_j + 8 * 5 ] + rgi16_y262_idct_cs1[ 7 ][ 3 ] * pi16_block[ i_j + 8 * 7 ];

		rgi_eo[ 0 ] = rgi16_y262_idct_cs1[ 2 ][ 0 ] * pi16_block[ i_j + 8 * 2 ] + rgi16_y262_idct_cs1[ 6 ][ 0 ] * pi16_block[ i_j + 8 * 6 ];
		rgi_eo[ 1 ] = rgi16_y262_idct_cs1[ 2 ][ 1 ] * pi16_block[ i_j + 8 * 2 ] + rgi16_y262_idct_cs1[ 6 ][ 1 ] * pi16_block[ i_j + 8 * 6 ];
		rgi_ee[ 0 ] = rgi16_y262_idct_cs1[ 0 ][ 0 ] * pi16_block[ i_j + 8 * 0 ] + rgi16_y262_idct_cs1[ 4 ][ 0 ] * pi16_block[ i_j + 8 * 4 ];
		rgi_ee[ 1 ] = rgi16_y262_idct_cs1[ 0 ][ 1 ] * pi16_block[ i_j + 8 * 0 ] + rgi16_y262_idct_cs1[ 4 ][ 1 ] * pi16_block[ i_j + 8 * 4 ];

		rgi_e[ 0 ] = rgi_ee[ 0 ] + rgi_eo[ 0 ];
		rgi_e[ 1 ] = rgi_ee[ 1 ] + rgi_eo[ 1 ];
		rgi_e[ 2 ] = rgi_ee[ 1 ] - rgi_eo[ 1 ];
		rgi_e[ 3 ] = rgi_ee[ 0 ] - rgi_eo[ 0 ];

		for( i_k = 0; i_k < 4; i_k++ )
		{
			rgi16_tmp[ i_j + 8 * i_k ] = RND( rgi_e[ i_k ] + rgi_o[ i_k ], RND1BITS );
			rgi16_tmp[ i_j + 8 * ( i_k + 4 ) ] = RND( rgi_e[ 3 - i_k ] - rgi_o[ 3 - i_k ], RND1BITS );
		}
	}

	for( i_j = 0; i_j < 8; i_j++ )
	{
		rgi_e[ 0 ] = rgi16_y262_idct_cs2[ 0 ][ 0 ] * rgi16_tmp[ i_j * 8 + 0 ] + rgi16_y262_idct_cs2[ 2 ][ 0 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_y262_idct_cs2[ 4 ][ 0 ] * rgi16_tmp[ i_j * 8 + 4 ] + rgi16_y262_idct_cs2[ 6 ][ 0 ] * rgi16_tmp[ i_j * 8 + 6 ];
		rgi_e[ 1 ] = rgi16_y262_idct_cs2[ 0 ][ 1 ] * rgi16_tmp[ i_j * 8 + 0 ] + rgi16_y262_idct_cs2[ 2 ][ 1 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_y262_idct_cs2[ 4 ][ 1 ] * rgi16_tmp[ i_j * 8 + 4 ] + rgi16_y262_idct_cs2[ 6 ][ 1 ] * rgi16_tmp[ i_j * 8 + 6 ];
		rgi_e[ 2 ] = rgi16_y262_idct_cs2[ 0 ][ 1 ] * rgi16_tmp[ i_j * 8 + 0 ] + -rgi16_y262_idct_cs2[ 2 ][ 1 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_y262_idct_cs2[ 4 ][ 1 ] * rgi16_tmp[ i_j * 8 + 4 ] + -rgi16_y262_idct_cs2[ 6 ][ 1 ] * rgi16_tmp[ i_j * 8 + 6 ];
		rgi_e[ 3 ] = rgi16_y262_idct_cs2[ 0 ][ 0 ] * rgi16_tmp[ i_j * 8 + 0 ] + -rgi16_y262_idct_cs2[ 2 ][ 0 ] * rgi16_tmp[ i_j * 8 + 2 ] +
			rgi16_y262_idct_cs2[ 4 ][ 0 ] * rgi16_tmp[ i_j * 8 + 4 ] + -rgi16_y262_idct_cs2[ 6 ][ 0 ] * rgi16_tmp[ i_j * 8 + 6 ];

		rgi_o[ 0 ] = rgi16_y262_idct_cs2[ 1 ][ 0 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_y262_idct_cs2[ 3 ][ 0 ] * rgi16_tmp[ i_j * 8 + 3 ] +
			rgi16_y262_idct_cs2[ 5 ][ 0 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_y262_idct_cs2[ 7 ][ 0 ] * rgi16_tmp[ i_j * 8 + 7 ];
		rgi_o[ 1 ] = rgi16_y262_idct_cs2[ 1 ][ 1 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_y262_idct_cs2[ 3 ][ 1 ] * rgi16_tmp[ i_j * 8 + 3 ] +
			rgi16_y262_idct_cs2[ 5 ][ 1 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_y262_idct_cs2[ 7 ][ 1 ] * rgi16_tmp[ i_j * 8 + 7 ];
		rgi_o[ 2 ] = rgi16_y262_idct_cs2[ 1 ][ 2 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_y262_idct_cs2[ 3 ][ 2 ] * rgi16_tmp[ i_j * 8 + 3 ] +
			rgi16_y262_idct_cs2[ 5 ][ 2 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_y262_idct_cs2[ 7 ][ 2 ] * rgi16_tmp[ i_j * 8 + 7 ];
		rgi_o[ 3 ] = rgi16_y262_idct_cs2[ 1 ][ 3 ] * rgi16_tmp[ i_j * 8 + 1 ] + rgi16_y262_idct_cs2[ 3 ][ 3 ] * rgi16_tmp[ i_j * 8 + 3 ] +
			rgi16_y262_idct_cs2[ 5 ][ 3 ] * rgi16_tmp[ i_j * 8 + 5 ] + rgi16_y262_idct_cs2[ 7 ][ 3 ] * rgi16_tmp[ i_j * 8 + 7 ];

#define CLAMP256( x ) ( ( x ) < 0 ? 0 : ( ( x ) > 255 ? 255 : ( x ) ) )
		for( i_k = 0; i_k < 4; i_k++ )
		{
			int32_t i_pel0, i_pel1;
			i_pel0 = pui8_dst[ i_j * ui_stride + i_k ] + RND( rgi_e[ i_k ] + rgi_o[ i_k ], RND2BITS );
			i_pel1 = pui8_dst[ i_j * ui_stride + ( i_k + 4 ) ] + RND( rgi_e[ 3 - i_k ] - rgi_o[ 3 - i_k ], RND2BITS );
			pui8_dst[ i_j * ui_stride + i_k ] = CLAMP256( i_pel0 );
			pui8_dst[ i_j * ui_stride + ( i_k + 4 ) ] = CLAMP256( i_pel1 );
		}
	}
}



