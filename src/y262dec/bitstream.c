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


void y262dec_bitstream_reset( y262dec_bitstream_t *ps_bitstream )
{
	ps_bitstream->ui_bits = 0;
	ps_bitstream->pui8_buffer_ptr = NULL;
	ps_bitstream->pui8_buffer_end = ps_bitstream->pui8_buffer_ptr;
}


void y262dec_bitstream_init( y262dec_bitstream_t *ps_bitstream, uint8_t *pui8_bitstream, int32_t i_length )
{
	uint8_t *pui8_buffer;

	pui8_buffer = pui8_bitstream;
	ps_bitstream->pui8_buffer_ptr = pui8_buffer + 4;

	ps_bitstream->ui_bits = ( pui8_buffer[ 0 ] << 24 ) | ( pui8_buffer[ 1 ] << 16 ) | ( pui8_buffer[ 2 ] << 8 ) | ( pui8_buffer[ 3 ] );
	ps_bitstream->i_num_bits = -16;

	ps_bitstream->pui8_buffer_end = ps_bitstream->pui8_buffer_ptr + i_length;
}


uint32_t y262dec_bitstream_read_small( y262dec_bitstream_t *ps_bitstream, int32_t i_symbol_length )
{
	uint32_t ui_bits;

	uint8_t *pui8_buffer;

	ui_bits = ps_bitstream->ui_bits >> ( 32 - i_symbol_length );
	ps_bitstream->ui_bits <<= i_symbol_length;
	ps_bitstream->i_num_bits += i_symbol_length;

	if( ps_bitstream->i_num_bits > 0 )
	{
		pui8_buffer = ps_bitstream->pui8_buffer_ptr;
		ps_bitstream->ui_bits |= ( pui8_buffer[ 0 ] << 8 | pui8_buffer[ 1 ] ) << ps_bitstream->i_num_bits;
		ps_bitstream->i_num_bits -= 16;

		ps_bitstream->pui8_buffer_ptr = MIN( ps_bitstream->pui8_buffer_ptr + 2, ps_bitstream->pui8_buffer_end );
	}

	return ui_bits;
}


uint32_t y262dec_bitstream_read( y262dec_bitstream_t *ps_bitstream, int32_t i_symbol_length )
{
	uint32_t ui_integer;
	int32_t i_read_bits, i_bits_remain, i_bits;

	i_bits_remain = i_symbol_length;
	ui_integer = 0;

	while( i_bits_remain )
	{
		i_read_bits = MIN( i_bits_remain, 16 );
		i_bits = y262dec_bitstream_read_small( ps_bitstream, i_read_bits );
		i_bits_remain -= i_read_bits;

		ui_integer = ( ui_integer << i_read_bits ) | i_bits;
	}

	return ui_integer;
}


uint32_t y262dec_bitstream_peek_small( y262dec_bitstream_t *ps_bitstream, int32_t i_symbol_length )
{
	return ( ps_bitstream->ui_bits >> ( 32 - ( i_symbol_length ) ) );
}


uint32_t y262dec_bitstream_peek( y262dec_bitstream_t *ps_bitstream, int32_t i_symbol_length )
{
	uint8_t *pui8_buffer;

	if( ps_bitstream->i_num_bits > -8 )
	{
		pui8_buffer = ps_bitstream->pui8_buffer_ptr;
		ps_bitstream->pui8_buffer_ptr++;
		ps_bitstream->ui_bits |= pui8_buffer[ 0 ] << ( ps_bitstream->i_num_bits + 8 );
		ps_bitstream->i_num_bits -= 8;
	}

	return ps_bitstream->ui_bits >> ( 32 - i_symbol_length );
}


int32_t y262dec_bitstream_peek_sign( y262dec_bitstream_t *ps_bitstream )
{
	return ( ( ( int32_t ) ps_bitstream->ui_bits ) >> 31 );
}


void y262dec_bitstream_drop( y262dec_bitstream_t *ps_bitstream, int32_t i_symbol_length )
{
	uint8_t *pui8_buffer;

	ps_bitstream->ui_bits <<= ( i_symbol_length );
	ps_bitstream->i_num_bits += ( i_symbol_length );
	if( ps_bitstream->i_num_bits > 0 )
	{
		pui8_buffer = ps_bitstream->pui8_buffer_ptr;
		ps_bitstream->ui_bits |= ( pui8_buffer[ 0 ] << 8 | pui8_buffer[ 1 ] ) << ps_bitstream->i_num_bits;
		ps_bitstream->i_num_bits -= 16;
		ps_bitstream->pui8_buffer_ptr = MIN( ps_bitstream->pui8_buffer_ptr + 2, ps_bitstream->pui8_buffer_end );
	}
}


bool y262dec_bitstream_not_past_end( y262dec_bitstream_t *ps_bitstream )
{
	if( ps_bitstream->pui8_buffer_ptr < ps_bitstream->pui8_buffer_end )
	{
		return true;
	}
	return false;
}


void y262dec_bitstream_bytealign( y262dec_bitstream_t *ps_bitstream )
{
	int32_t i_bits, i_drop;

	i_bits = -ps_bitstream->i_num_bits;
	i_drop = i_bits & 0x7;
	y262dec_bitstream_drop( ps_bitstream, i_drop );
}



