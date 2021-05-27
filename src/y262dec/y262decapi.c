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


void *y262dec_create( y262dec_config_t *ps_config )
{
	y262dec_t *ps_dec;

	if( !ps_config || ps_config->pf_callback == NULL )
	{
		return NULL;
	}

	ps_dec = malloc( sizeof( y262dec_t ) );
	if( !ps_dec )
	{
		return NULL;
	}

	memset( ps_dec, 0, sizeof( y262dec_t ) );

	if( !y262dec_init( ps_dec, ps_config ) )
	{
		y262dec_destroy( ps_dec );

		return NULL;
	}

	return ( void * ) ps_dec;
}


int32_t y262dec_process( void *p_dec, uint8_t *pui8_data, int32_t i_data_size )
{
	y262dec_t *ps_dec = ( y262dec_t * ) p_dec;

	return y262dec_process_internal( ps_dec, pui8_data, i_data_size );
}

int32_t y262dec_flush( void *p_dec )
{
	y262dec_t *ps_dec = ( y262dec_t * ) p_dec;

	return y262dec_process_flush( ps_dec );
}

void y262dec_destroy( void *p_dec )
{
	y262dec_t *ps_dec = ( y262dec_t * ) p_dec;

	y262dec_deinit( ps_dec );

	free( ps_dec );
}