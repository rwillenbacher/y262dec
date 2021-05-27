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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


#include <y262decapi.h>


typedef struct
{
	FILE *pf_out;
	y262dec_sequence_parameters_t s_sequence_parameters;
	y262dec_frame_picture_t s_frame;
} y262decapp_t;



void y262decapp_callback( void *p_user, y262dec_decode_result_t *ps_result )
{
	y262decapp_t *ps_app = ( y262decapp_t * ) p_user;

	if( ps_result->i_result_type == Y262DEC_DECODE_RESULT_TYPE_SEQUENCE_PARAMS )
	{
		ps_app->s_sequence_parameters = ps_result->u_result.s_parameters;
		fprintf( stderr, "SEQPARAM: %d %d ( %s )\n", ps_app->s_sequence_parameters.i_width, ps_app->s_sequence_parameters.i_height,
			ps_app->s_sequence_parameters.b_progressive ? "progressive" : "interlaced" );
	}
	else
	{
		ps_app->s_frame = ps_result->u_result.s_frame;
		fprintf( stderr, "FRAME PON %d DON %d\n", ps_app->s_frame.i_pon, ps_app->s_frame.i_don );
		if( ps_app->pf_out )
		{
			int32_t i_width, i_height;
			i_width = ps_app->s_frame.i_width;
			i_height = ps_app->s_frame.i_height;

			fwrite( ps_app->s_frame.pui8_luma, i_width * i_height, 1, ps_app->pf_out );

			if( ps_app->s_sequence_parameters.i_chroma_format == Y262DEC_CHROMA_FORMAT_420 )
			{
				i_width >>= 1;
				i_height >>= 1;
			}
			else if( ps_app->s_sequence_parameters.i_chroma_format == Y262DEC_CHROMA_FORMAT_422 )
			{
				i_width >>= 1;
			}

			fwrite( ps_app->s_frame.pui8_cb, i_width * i_height, 1, ps_app->pf_out );
			fwrite( ps_app->s_frame.pui8_cr, i_width * i_height, 1, ps_app->pf_out );
		}
	}
}


int32_t main( int32_t i_argc, char *rgpc_argv[ ] )
{
	void *p_dec;
	FILE *pf_in;
	int32_t i_ret;
	size_t i_read;
	uint8_t rgui8_read[ 1 << 12 ];
	y262dec_config_t s_config;
	y262decapp_t s_app;

	if( i_argc < 3 )
	{
		fprintf( stderr, "usage: y262decapp <infile> <outfile>\n");
		return 1;
	}

	pf_in = fopen( rgpc_argv[ 1 ], "rb" );
	if( !pf_in )
	{
		fprintf( stderr, "unable to open input file '%s'\n", rgpc_argv[ 1 ] );
		return 1;
	}

	s_app.pf_out = fopen( rgpc_argv[ 2 ], "wb" );
	if( !s_app.pf_out )
	{
		fprintf( stderr, "unable to open output file '%s'\n", rgpc_argv[ 2 ] );
		return 1;
	}

	s_config.pf_callback = y262decapp_callback;
	s_config.p_user = &s_app;
	s_config.b_multithreading = true;
	p_dec = y262dec_create( &s_config );

	while( 1 )
	{
		do
		{
			i_ret = y262dec_process( p_dec, NULL, 0 );
		} while( i_ret == Y262DEC_OK );

		if( i_ret == Y262DEC_MORE )
		{
			i_read = fread( rgui8_read, sizeof( uint8_t ), 1 << 12, pf_in );
			if( i_read == 0 )
			{
				fprintf( stderr, "eof.\n" );
				break;
			}
			y262dec_process( p_dec, rgui8_read, ( int32_t ) i_read );
		}
	}


	while( 1 )
	{
		i_ret = y262dec_flush( p_dec );
		if( i_ret == Y262DEC_MORE )
		{
			continue;
		}
		else if( i_ret == Y262DEC_OK )
		{
			break;
		}
		else
		{
			break;
		}
	}

	y262dec_destroy( p_dec );

	fclose( pf_in );
	fclose( s_app.pf_out );

	return 0;
}

