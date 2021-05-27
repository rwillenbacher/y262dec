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


void y262dec_decoder_output_sequence_parameters( y262dec_t *ps_dec )
{
	y262dec_decode_result_t s_result;
	y262dec_sequence_header_t *ps_sequence_header = &ps_dec->s_headers.s_sequence_header;
	y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;

	memset( &s_result, 0, sizeof( s_result ) );

	s_result.i_result_type = Y262DEC_DECODE_RESULT_TYPE_SEQUENCE_PARAMS;
	s_result.u_result.s_parameters.b_progressive = ps_sequence_extension->b_progressive_sequence;
	s_result.u_result.s_parameters.i_aspect_ratio_code = ps_sequence_header->i_aspect_ratio_information;
	s_result.u_result.s_parameters.i_chroma_format = ps_sequence_extension->i_chroma_format;
	s_result.u_result.s_parameters.i_frame_rate_code = ps_sequence_header->i_frame_rate_code;
	s_result.u_result.s_parameters.i_width = ps_sequence_header->i_horizontal_size + ( ps_sequence_extension->i_horizontal_size_extension << 12 );
	s_result.u_result.s_parameters.i_height = ps_sequence_header->i_vertical_size + ( ps_sequence_extension->i_vertical_size_extension << 12 );

	ps_dec->s_config.pf_callback( ps_dec->s_config.p_user, &s_result );

}



bool y262dec_decoder_init_buffers( y262dec_t *ps_dec )
{
	int32_t i_idx, i_chroma_scale = 0;
	int32_t i_size, i_luma_size, i_chroma_size, i_frame_size;
	uint32_t ui_remainder;

	y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	y262dec_sequence_header_t *ps_sequence_header = &ps_dec->s_headers.s_sequence_header;
	y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	y262dec_frame_picture_t *ps_frame_picture;

	ps_state->b_initialized = true;
	ps_state->i_horizontal_size = ps_sequence_header->i_horizontal_size + ( ps_sequence_extension->i_horizontal_size_extension << 12 );
	ps_state->i_vertical_size = ps_sequence_header->i_vertical_size + ( ps_sequence_extension->i_vertical_size_extension << 12 );

	ps_state->i_mb_width = ( ps_state->i_horizontal_size + 15 ) / 16;
	if( ps_sequence_extension->b_progressive_sequence )
	{
		ps_state->i_mb_height = ( ps_state->i_vertical_size + 15 ) / 16;
	}
	else
	{
		ps_state->i_mb_height = 2 * ( ( ps_state->i_vertical_size + 31 ) / 32 );
	}

	i_size = 0;
	i_luma_size = ps_state->i_mb_width * ps_state->i_mb_height * 16 * 16;
	i_size += i_luma_size;

	if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_420 )
	{
		i_chroma_size = ps_state->i_mb_width * ps_state->i_mb_height * 8 * 8;
		i_chroma_scale = 1;
		ps_state->i_mb_chroma_width = 8;
		ps_state->i_mb_chroma_height = 8;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_422 )
	{
		i_chroma_size = ps_state->i_mb_width * ps_state->i_mb_height * 8 * 16;
		i_chroma_scale = 1;
		ps_state->i_mb_chroma_width = 8;
		ps_state->i_mb_chroma_height = 16;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_444 )
	{
		i_chroma_size = ps_state->i_mb_width * ps_state->i_mb_height * 16 * 16;
		i_chroma_scale = 0;
		ps_state->i_mb_chroma_width = 16;
		ps_state->i_mb_chroma_height = 16;
	}
	else
	{
		i_chroma_size = 0;
		i_chroma_scale = 0;
		ps_state->i_mb_chroma_width = 0;
		ps_state->i_mb_chroma_height = 0;
		/* impossible */
		return false;
	}
	i_size += i_chroma_size * 2;
	i_frame_size = i_size;
	i_size = i_size * 3 * sizeof( uint8_t );

	if( !ps_state->b_buffer_init || ps_state->i_frame_buffer_size != i_size )
	{
		uint8_t *pui8_frame_buffer;

		if( ps_state->b_buffer_init )
		{
			free( ps_state->pui8_frame_buffer );
		}

		pui8_frame_buffer = ps_state->pui8_frame_buffer = malloc( ( size_t ) ( i_size + 32 ) );
		if( !pui8_frame_buffer )
		{
			return false;
		}
		memset( pui8_frame_buffer, 0, ( size_t ) ( i_size + 32 ) );

		ui_remainder = ( ( uint8_t ) pui8_frame_buffer ) & 0x1f;
		if( ui_remainder != 0 )
		{
			pui8_frame_buffer += 32 - ui_remainder;
		}
		ps_state->i_frame_buffer_size = i_size;

		for( i_idx = 0; i_idx < 3; i_idx++ )
		{
			ps_frame_picture = &ps_state->rgs_picture_buffer[ i_idx ];
			ps_frame_picture->i_width = ps_state->i_mb_width * 16;
			ps_frame_picture->i_stride = ps_frame_picture->i_width;
			ps_frame_picture->i_stride_chroma = ps_frame_picture->i_stride >> i_chroma_scale;

			ps_frame_picture->i_height = ps_state->i_mb_height * 16;

			ps_frame_picture->pui8_luma = pui8_frame_buffer + ( ( size_t )( i_idx * i_frame_size ) );
			ps_frame_picture->pui8_cb = ps_frame_picture->pui8_luma + i_luma_size;
			ps_frame_picture->pui8_cr = ps_frame_picture->pui8_cb + i_chroma_size;
		}
		ps_state->rgb_fields[ 0 ] = ps_state->rgb_fields[ 1 ] = false;

		ps_state->b_buffer_init = true;

		ps_state->s_decoded_picture_buffer.i_current_picture = 0;
		ps_state->s_decoded_picture_buffer.i_reference_picture_0 = 1;
		ps_state->s_decoded_picture_buffer.i_reference_picture_1 = 2;
		ps_state->s_decoded_picture_buffer.i_fill = 0;
	}

	return true;
}


void y262dec_decoder_init_picture( y262dec_t *ps_dec )
{
	y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;
	y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;

	ps_state = &ps_dec->s_dec_state;
	ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;

	if( ps_picture_header->i_picture_coding_type == Y262DEC_PICTURE_CODING_TYPE_B )
	{
		ps_state->rgps_reference_frames[ 0 ] = &ps_state->rgs_reference_frames[ Y262DEC_REFERENCE_BFORWARD ];
		ps_state->rgps_reference_frames[ 1 ] = &ps_state->rgs_reference_frames[ Y262DEC_REFERENCE_BBACKWARD ];

		ps_state->rgps_reference_fields[ 0 ][ Y262DEC_REFERENCE_TOP ] = &ps_state->rgs_reference_fields[ Y262DEC_REFERENCE_BFORWARD ][ Y262DEC_REFERENCE_TOP ];
		ps_state->rgps_reference_fields[ 0 ][ Y262DEC_REFERENCE_BOTTOM ] = &ps_state->rgs_reference_fields[ Y262DEC_REFERENCE_BFORWARD ][ Y262DEC_REFERENCE_BOTTOM ];

		ps_state->rgps_reference_fields[ 1 ][ Y262DEC_REFERENCE_TOP ] = &ps_state->rgs_reference_fields[ Y262DEC_REFERENCE_BBACKWARD ][ Y262DEC_REFERENCE_TOP ];
		ps_state->rgps_reference_fields[ 1 ][ Y262DEC_REFERENCE_BOTTOM ] = &ps_state->rgs_reference_fields[ Y262DEC_REFERENCE_BBACKWARD ][ Y262DEC_REFERENCE_BOTTOM ];
	}
	else
	{
		ps_state->rgps_reference_frames[ 0 ] = &ps_state->rgs_reference_frames[ Y262DEC_REFERENCE_PFORWARD ];
		ps_state->rgps_reference_frames[ 1 ] = &ps_state->rgs_reference_frames[ Y262DEC_REFERENCE_PFORWARD ];

		ps_state->rgps_reference_fields[ 0 ][ Y262DEC_REFERENCE_TOP ] = &ps_state->rgs_reference_fields[ Y262DEC_REFERENCE_PFORWARD ][ Y262DEC_REFERENCE_TOP ];
		ps_state->rgps_reference_fields[ 0 ][ Y262DEC_REFERENCE_BOTTOM ] = &ps_state->rgs_reference_fields[ Y262DEC_REFERENCE_PFORWARD ][ Y262DEC_REFERENCE_BOTTOM ];

		ps_state->rgps_reference_fields[ 1 ][ Y262DEC_REFERENCE_TOP ] = &ps_state->rgs_reference_fields[ Y262DEC_REFERENCE_PFORWARD ][ Y262DEC_REFERENCE_TOP ];
		ps_state->rgps_reference_fields[ 1 ][ Y262DEC_REFERENCE_BOTTOM ] = &ps_state->rgs_reference_fields[ Y262DEC_REFERENCE_PFORWARD ][ Y262DEC_REFERENCE_BOTTOM ];
	}


	ps_state->i_picture_mb_width = ps_state->i_mb_width;
	if( ps_picture_coding_extension->i_picture_structure == Y262DEC_PICTURE_CODING_STRUCTURE_FRAME )
	{
		ps_state->i_picture_mb_height = ps_state->i_mb_height;
	}
	else
	{
		ps_state->i_picture_mb_height = ps_state->i_mb_height / 2;
	}

	ps_state->i_picture_width = ps_state->i_picture_mb_width << 4;
	ps_state->i_picture_height = ps_state->i_picture_mb_height << 4;

	if( ps_picture_coding_extension->b_alternate_scan )
	{
		ps_state->pui8_current_scan = &rgui8_y262dec_scan_1_table[ 0 ];
	}
	else
	{
		ps_state->pui8_current_scan = &rgui8_y262dec_scan_0_table[ 0 ];
	}

	ps_state->ui_intra_dc_shift = 3 - ps_picture_coding_extension->i_intra_dc_precision;
}

void y262dec_refresh_quantizer_matrices( y262dec_t *ps_dec )
{
	int32_t i_quantizer_scale, i_quantizer, i_idx, i_scale_idx;

	y262dec_quantizer_matrices_t *ps_quantizer_matrices = &ps_dec->s_quantizer_matrices;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;

	i_scale_idx = ps_picture_coding_extension->b_q_scale_type;

	if( ps_quantizer_matrices->rgb_refresh_intra_quantizer_matrix[ i_scale_idx ] )
	{
		for( i_quantizer = 0; i_quantizer < 32; i_quantizer++ )
		{
			i_quantizer_scale = rgi8_y262dec_quantizer_scale_table[ i_scale_idx ][ i_quantizer ];
			for( i_idx = 0; i_idx < 64; i_idx++ )
			{
				ps_quantizer_matrices->rgui16_intra_quantizer_matrix[ i_scale_idx ][ i_quantizer ][ i_idx ] = ps_quantizer_matrices->rgui16_chroma_intra_quantizer_matrix[ i_scale_idx ][ i_quantizer ][ i_idx ] =
					ps_quantizer_matrices->rgui8_intra_quantizer_matrix[ i_idx ] * i_quantizer_scale * 2;
			}
		}
		ps_quantizer_matrices->rgb_refresh_intra_quantizer_matrix[ i_scale_idx ] = false;
	}

	if( ps_quantizer_matrices->rgb_refresh_non_intra_quantizer_matrix[ i_scale_idx ] )
	{
		for( i_quantizer = 0; i_quantizer < 32; i_quantizer++ )
		{
			i_quantizer_scale = rgi8_y262dec_quantizer_scale_table[ i_scale_idx ][ i_quantizer ];
			for( i_idx = 0; i_idx < 64; i_idx++ )
			{
				ps_quantizer_matrices->rgui16_non_intra_quantizer_matrix[ i_scale_idx ][ i_quantizer ][ i_idx ] = ps_quantizer_matrices->rgui16_chroma_non_intra_quantizer_matrix[ i_scale_idx ][ i_quantizer ][ i_idx ] =
					ps_quantizer_matrices->rgui8_non_intra_quantizer_matrix[ i_idx ] * i_quantizer_scale;
			}
		}
		ps_quantizer_matrices->rgb_refresh_non_intra_quantizer_matrix[ i_scale_idx ] = false;
	}

	if( ps_quantizer_matrices->rgb_refresh_chroma_intra_quantizer_matrix[ i_scale_idx ] )
	{
		for( i_quantizer = 0; i_quantizer < 32; i_quantizer++ )
		{
			i_quantizer_scale = rgi8_y262dec_quantizer_scale_table[ i_scale_idx ][ i_quantizer ];
			for( i_idx = 0; i_idx < 64; i_idx++ )
			{
				ps_quantizer_matrices->rgui16_chroma_intra_quantizer_matrix[ i_scale_idx ][ i_quantizer ][ i_idx ] = ps_quantizer_matrices->rgui8_chroma_intra_quantizer_matrix[ i_idx ] * i_quantizer_scale * 2;
			}
		}
		ps_quantizer_matrices->rgb_refresh_chroma_intra_quantizer_matrix[ i_scale_idx ] = false;
	}

	if( ps_quantizer_matrices->rgb_refresh_chroma_non_intra_quantizer_matrix[ i_scale_idx ] )
	{
		for( i_quantizer = 0; i_quantizer < 32; i_quantizer++ )
		{
			i_quantizer_scale = rgi8_y262dec_quantizer_scale_table[ i_scale_idx ][ i_quantizer ];
			for( i_idx = 0; i_idx < 64; i_idx++ )
			{
				ps_quantizer_matrices->rgui16_chroma_non_intra_quantizer_matrix[ i_scale_idx ][ i_quantizer ][ i_idx ] = ps_quantizer_matrices->rgui8_chroma_non_intra_quantizer_matrix[ i_idx ] * i_quantizer_scale;
			}
		}
		ps_quantizer_matrices->rgb_refresh_chroma_non_intra_quantizer_matrix[ i_scale_idx ] = false;
	}
}


void y262dec_setup_field_picture( y262dec_t *ps_dec, y262dec_frame_picture_t *ps_frame, y262dec_field_picture_t *ps_field, bool b_top )
{
	ps_field->i_width = ps_frame->i_width;
	ps_field->i_height = ps_frame->i_height / 2;
	ps_field->i_stride = ps_frame->i_stride * 2;
	ps_field->i_stride_chroma = ps_frame->i_stride_chroma * 2;
	ps_field->pui8_luma = ps_frame->pui8_luma;
	ps_field->pui8_cb = ps_frame->pui8_cb;
	ps_field->pui8_cr = ps_frame->pui8_cr;
	if( !b_top )
	{
		ps_field->pui8_luma += ps_frame->i_stride;
		ps_field->pui8_cb += ps_frame->i_stride_chroma;
		ps_field->pui8_cr += ps_frame->i_stride_chroma;
	}
}


void y262dec_decoder_setup_reference_picture_buffers( y262dec_t *ps_dec )
{
	y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	y262dec_sequence_header_t *ps_sequence_header = &ps_dec->s_headers.s_sequence_header;
	y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	y262dec_field_picture_t *ps_field_picture;

	ps_state->s_current_frame = ps_state->rgs_picture_buffer[ ps_state->s_decoded_picture_buffer.i_current_picture ];
	ps_state->rgs_reference_frames[ 0 ] = ps_state->rgs_picture_buffer[ ps_state->s_decoded_picture_buffer.i_reference_picture_0 ];
	ps_state->rgs_reference_frames[ 1 ] = ps_state->rgs_picture_buffer[ ps_state->s_decoded_picture_buffer.i_reference_picture_1 ];

	if( ps_picture_coding_extension->i_picture_structure == Y262DEC_PICTURE_CODING_STRUCTURE_FRAME )
	{
		ps_field_picture = &ps_state->s_current_field;
		y262dec_setup_field_picture( ps_dec, &ps_state->s_current_frame, ps_field_picture, true );

		ps_field_picture = &ps_state->rgs_reference_fields[ 0 ][ Y262DEC_REFERENCE_TOP ];
		y262dec_setup_field_picture( ps_dec, &ps_state->rgs_reference_frames[ 0 ], ps_field_picture, true );
		ps_field_picture = &ps_state->rgs_reference_fields[ 0 ][ Y262DEC_REFERENCE_BOTTOM ];
		y262dec_setup_field_picture( ps_dec, &ps_state->rgs_reference_frames[ 0 ], ps_field_picture, false );

		ps_field_picture = &ps_state->rgs_reference_fields[ 1 ][ Y262DEC_REFERENCE_TOP ];
		y262dec_setup_field_picture( ps_dec, &ps_state->rgs_reference_frames[ 1 ], ps_field_picture, true );
		ps_field_picture = &ps_state->rgs_reference_fields[ 1 ][ Y262DEC_REFERENCE_BOTTOM ];
		y262dec_setup_field_picture( ps_dec, &ps_state->rgs_reference_frames[ 1 ], ps_field_picture, false );

		ps_state->rgb_fields[ Y262DEC_REFERENCE_TOP ] = ps_state->rgb_fields[ Y262DEC_REFERENCE_BOTTOM ] = true;

		if( ps_sequence_extension->b_progressive_sequence )
		{
			ps_state->b_top_field_first = true;
			ps_state->s_current_frame.b_repeat_first_field = false;
			ps_state->s_current_frame.i_repeat_frame_times = ps_picture_coding_extension->b_repeat_first_field == 0 ? 0 : ps_picture_coding_extension->b_top_field_first == 0 ? 1 : 2;

		}
		else
		{
			ps_state->b_top_field_first = ps_picture_coding_extension->b_top_field_first;
			ps_state->s_current_frame.i_repeat_frame_times = 0;

			if( ps_picture_coding_extension->b_progressive_frame == 0 )
			{
				ps_state->s_current_frame.b_repeat_first_field = false;
			}
			else
			{
				ps_state->s_current_frame.b_repeat_first_field = ps_picture_coding_extension->b_repeat_first_field;
			}
		}
		ps_state->s_current_frame.i_don = ps_state->i_next_don++;
		ps_state->s_current_frame.i_pon = -1;
		//ps_state->s_current_frame.ui64_dts = ps_state->ui64_next_dts;
		//ps_state->s_current_frame.ui64_pts = ps_state->ui64_next_pts;

		ps_state->ui_luma_block_y_stride_dct0 = ps_state->s_current_frame.i_stride * 8;
		ps_state->ui_luma_stride_dct0 = ps_state->s_current_frame.i_stride;
		ps_state->ui_chroma_block_y_stride_dct0 = ps_state->s_current_frame.i_stride_chroma * 8;
		ps_state->ui_chroma_stride_dct0 = ps_state->s_current_frame.i_stride_chroma;

		ps_state->ui_luma_block_y_stride_dct1 = ps_state->s_current_frame.i_stride;
		ps_state->ui_luma_stride_dct1 = ps_state->s_current_frame.i_stride * 2;
		if( ps_sequence_extension->i_chroma_format != Y262DEC_CHROMA_FORMAT_420 )
		{
			ps_state->ui_chroma_block_y_stride_dct1 = ps_state->s_current_frame.i_stride_chroma;
			ps_state->ui_chroma_stride_dct1 = ps_state->s_current_frame.i_stride_chroma * 2;
		}
		else
		{
			ps_state->ui_chroma_block_y_stride_dct1 = ps_state->ui_chroma_block_y_stride_dct0;
			ps_state->ui_chroma_stride_dct1 = ps_state->ui_chroma_stride_dct0;
		}
	}
	else
	{
		if( ps_picture_coding_extension->i_picture_structure == Y262DEC_PICTURE_CODING_STRUCTURE_TOP )
		{
			ps_field_picture = &ps_state->s_current_field;
			y262dec_setup_field_picture( ps_dec, &ps_state->s_current_frame, ps_field_picture, true );

			ps_field_picture = &ps_state->rgs_reference_fields[ 0 ][ Y262DEC_REFERENCE_TOP ];
			y262dec_setup_field_picture( ps_dec, &ps_state->rgs_reference_frames[ 0 ], ps_field_picture, true );

			ps_field_picture = &ps_state->rgs_reference_fields[ 0 ][ Y262DEC_REFERENCE_BOTTOM ];
			if( ps_state->rgb_fields[ Y262DEC_REFERENCE_BOTTOM ] && ps_picture_header->i_picture_coding_type != Y262DEC_PICTURE_CODING_TYPE_B )
			{
				y262dec_setup_field_picture( ps_dec, &ps_state->s_current_frame, ps_field_picture, false );
			}
			else
			{
				y262dec_setup_field_picture( ps_dec, &ps_state->rgs_reference_frames[ 0 ], ps_field_picture, false );
			}

			ps_field_picture = &ps_state->rgs_reference_fields[ 1 ][ Y262DEC_REFERENCE_TOP ];
			y262dec_setup_field_picture( ps_dec, &ps_state->rgs_reference_frames[ 1 ], ps_field_picture, true );
			ps_field_picture = &ps_state->rgs_reference_fields[ 1 ][ Y262DEC_REFERENCE_BOTTOM ];
			y262dec_setup_field_picture( ps_dec, &ps_state->rgs_reference_frames[ 1 ], ps_field_picture, false );

			ps_state->rgb_fields[ Y262DEC_REFERENCE_TOP ] = true;
			ps_state->b_top_field_first = ps_state->rgb_fields[ Y262DEC_REFERENCE_BOTTOM ] == false;
			if( ps_state->b_top_field_first )
			{
				ps_state->s_current_frame.i_don = ps_state->i_next_don++;
				ps_state->s_current_frame.i_pon = -1;
				//ps_state->s_current_frame.ui64_dts = ps_state->ui64_next_dts;
				//ps_state->s_current_frame.ui64_pts = ps_state->ui64_next_pts;
			}
		}
		else
		{
			ps_field_picture = &ps_state->s_current_field;
			y262dec_setup_field_picture( ps_dec, &ps_state->s_current_frame, ps_field_picture, false );

			ps_field_picture = &ps_state->rgs_reference_fields[ 0 ][ Y262DEC_REFERENCE_TOP ];
			if( ps_state->rgb_fields[ Y262DEC_REFERENCE_TOP ] && ps_picture_header->i_picture_coding_type != Y262DEC_PICTURE_CODING_TYPE_B )
			{
				y262dec_setup_field_picture( ps_dec, &ps_state->s_current_frame, ps_field_picture, true );
			}
			else
			{
				y262dec_setup_field_picture( ps_dec, &ps_state->rgs_reference_frames[ 0 ], ps_field_picture, true );
			}

			ps_field_picture = &ps_state->rgs_reference_fields[ 0 ][ Y262DEC_REFERENCE_BOTTOM ];
			y262dec_setup_field_picture( ps_dec, &ps_state->rgs_reference_frames[ 0 ], ps_field_picture, false );

			ps_field_picture = &ps_state->rgs_reference_fields[ 1 ][ Y262DEC_REFERENCE_TOP ];
			y262dec_setup_field_picture( ps_dec, &ps_state->rgs_reference_frames[ 1 ], ps_field_picture, true );
			ps_field_picture = &ps_state->rgs_reference_fields[ 1 ][ Y262DEC_REFERENCE_BOTTOM ];
			y262dec_setup_field_picture( ps_dec, &ps_state->rgs_reference_frames[ 1 ], ps_field_picture, false );

			ps_state->rgb_fields[ Y262DEC_REFERENCE_BOTTOM ] = true;
			ps_state->b_top_field_first = ps_state->rgb_fields[ Y262DEC_REFERENCE_TOP ] == true;
			if( !ps_state->b_top_field_first )
			{
				ps_state->s_current_frame.i_don = ps_state->i_next_don++;
				ps_state->s_current_frame.i_pon = -1;
				//ps_state->s_current_frame.ui64_dts = ps_state->ui64_next_dts;
				//ps_state->s_current_frame.ui64_pts = ps_state->ui64_next_pts;
			}
		}

		ps_state->ui_luma_block_y_stride_dct0 = ps_state->s_current_field.i_stride * 8;
		ps_state->ui_luma_stride_dct0 = ps_state->s_current_field.i_stride;
		ps_state->ui_chroma_block_y_stride_dct0 = ps_state->s_current_field.i_stride_chroma * 8;
		ps_state->ui_chroma_stride_dct0 = ps_state->s_current_field.i_stride_chroma;
		/* dct1 in field pictures should never be used */
		ps_state->ui_luma_block_y_stride_dct1 = 0;
		ps_state->ui_luma_stride_dct1 = 0;
		ps_state->ui_chroma_block_y_stride_dct1 = 0;
		ps_state->ui_chroma_stride_dct1 = 0;

	}
}


void y262dec_decoder_finish_picture( y262dec_t *ps_dec )
{
	int32_t i_current_picture_idx;
	bool b_frame = false;
	y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;

	if( ps_picture_coding_extension->i_picture_structure == Y262DEC_PICTURE_CODING_STRUCTURE_TOP ||
		ps_picture_coding_extension->i_picture_structure == Y262DEC_PICTURE_CODING_STRUCTURE_BOTTOM )
	{
		if( ps_state->rgb_fields[ Y262DEC_REFERENCE_TOP ] && ps_state->rgb_fields[ Y262DEC_REFERENCE_BOTTOM ] )
		{
			b_frame = true;
		}
	}
	else if( ps_picture_coding_extension->i_picture_structure == Y262DEC_PICTURE_CODING_STRUCTURE_FRAME )
	{
		b_frame = true;
	}

	ps_state->b_output_frame = false;
	if( b_frame )
	{
		ps_state->rgs_picture_buffer[ ps_state->s_decoded_picture_buffer.i_current_picture ] = ps_state->s_current_frame;
		ps_state->rgb_fields[ Y262DEC_REFERENCE_TOP ] = ps_state->rgb_fields[ Y262DEC_REFERENCE_BOTTOM ] = false;

		if( ps_picture_header->i_picture_coding_type == Y262DEC_PICTURE_CODING_TYPE_B )
		{
			ps_state->s_output_frame = ps_dec->s_dec_state.s_current_frame;
			ps_state->b_output_frame = true;
		}
		else
		{
			if( ps_state->s_decoded_picture_buffer.i_fill >= 1 )
			{
				ps_state->s_output_frame = ps_state->rgs_picture_buffer[ ps_state->s_decoded_picture_buffer.i_reference_picture_0 ];
				ps_state->s_decoded_picture_buffer.i_fill--;
				ps_state->b_output_frame = true;
			}
			/* bump */
			i_current_picture_idx = ps_state->s_decoded_picture_buffer.i_current_picture;
			ps_state->s_decoded_picture_buffer.i_current_picture = ps_state->s_decoded_picture_buffer.i_reference_picture_1;
			ps_state->s_decoded_picture_buffer.i_reference_picture_1 = ps_state->s_decoded_picture_buffer.i_reference_picture_0;
			ps_state->s_decoded_picture_buffer.i_reference_picture_0 = i_current_picture_idx;
			ps_state->s_decoded_picture_buffer.i_fill++;
		}
	}

	if( ps_state->b_output_frame )
	{
		y262dec_decode_result_t s_result;
		memset( &s_result, 0, sizeof( s_result ) );
		s_result.i_result_type = Y262DEC_DECODE_RESULT_TYPE_FRAME;
		s_result.u_result.s_frame = ps_state->s_output_frame;
		s_result.u_result.s_frame.i_pon = ps_state->i_next_pon++;
		ps_dec->s_config.pf_callback( ps_dec->s_config.p_user, &s_result );
	}
}


void y262dec_decoder_flush_dpb( y262dec_t *ps_dec )
{
	y262dec_state_t *ps_state = &ps_dec->s_dec_state;

	ps_state->b_output_frame = false;
	if( ps_state->s_decoded_picture_buffer.i_fill >= 1 )
	{
		ps_state->s_output_frame = ps_state->rgs_picture_buffer[ ps_state->s_decoded_picture_buffer.i_reference_picture_0 ];
		ps_state->s_decoded_picture_buffer.i_fill--;
		ps_state->b_output_frame = true;
	}
	if( ps_state->b_output_frame )
	{
		y262dec_decode_result_t s_result;
		memset( &s_result, 0, sizeof( s_result ) );
		s_result.i_result_type = Y262DEC_DECODE_RESULT_TYPE_FRAME;
		s_result.u_result.s_frame = ps_state->s_output_frame;
		s_result.u_result.s_frame.i_pon = ps_state->i_next_pon++;
		ps_dec->s_config.pf_callback( ps_dec->s_config.p_user, &s_result );
	}
}

void y262dec_input_buffer_reset( y262dec_t *ps_dec )
{
	y262dec_input_buffer_t *ps_input_buffer = &ps_dec->s_input_buffer;

	ps_input_buffer->b_synced = false;
	ps_input_buffer->i_read_position = 0;
	ps_input_buffer->i_write_position = 0;
	ps_input_buffer->ui64_next_dts = 0;
	ps_input_buffer->ui64_next_pts = 0;
	ps_input_buffer->ui_possible_startcode = 0xffffff00;
}


bool y262dec_input_buffer_init( y262dec_t *ps_dec, int32_t i_max_fill )
{
	y262dec_input_buffer_t *ps_input_buffer = &ps_dec->s_input_buffer;

	ps_input_buffer->i_max_fill = i_max_fill;
	ps_input_buffer->pui8_buffer = malloc( sizeof( uint8_t ) * ps_input_buffer->i_max_fill );

	y262dec_input_buffer_reset( ps_dec );

	return true;
}


void y262dec_input_buffer_deinit( y262dec_t *ps_dec )
{
	y262dec_input_buffer_t *ps_input_buffer = &ps_dec->s_input_buffer;

	if( ps_input_buffer->pui8_buffer )
	{
		free( ps_input_buffer->pui8_buffer );
		ps_input_buffer->pui8_buffer = NULL;
	}
}


bool y262dec_input_buffer_push( y262dec_t *ps_dec, uint8_t *pui8_data, int32_t i_data_length )
{
	int32_t i_copy_size;

	y262dec_input_buffer_t *ps_input_buffer = &ps_dec->s_input_buffer;

	i_copy_size = ps_input_buffer->i_max_fill - ps_input_buffer->i_write_position;
	i_copy_size = MIN( i_data_length, i_copy_size );

	if( i_copy_size != i_data_length )
	{
		return false;
	}

	memcpy( ps_input_buffer->pui8_buffer + ps_input_buffer->i_write_position, pui8_data, i_copy_size );
	ps_input_buffer->i_write_position += i_copy_size;

	return true;
}


void y262dec_input_buffer_advance( y262dec_t *ps_dec, int32_t i_num_bytes )
{
	int32_t i_idx, i_i, i_j;
	y262dec_input_buffer_t *ps_input_buffer = &ps_dec->s_input_buffer;
	uint8_t *pui8_buffer = ps_input_buffer->pui8_buffer;

	i_i = 0;
	i_j = i_num_bytes;

	for( i_idx = i_num_bytes; i_idx < ps_input_buffer->i_write_position; i_idx++ )
	{
		pui8_buffer[ i_i++ ] = pui8_buffer[ i_j++ ];
	}

	ps_input_buffer->i_read_position -= i_num_bytes;
	ps_input_buffer->i_write_position -= i_num_bytes;
}




void y262dec_dispatch_slice_to_slicedec( y262dec_t *ps_dec, uint8_t *pui8_slice, int32_t i_slice_size )
{
	int32_t i_slicedec;
	y262dec_slicedec_t *ps_slicedec;

	while( ( ps_dec->i_slice_dec_dispatch - ps_dec->i_slice_dec_collect ) >= ps_dec->i_num_slice_decoders )
	{
		y262dec_slicedec_wait( ps_dec, ps_dec->i_slice_dec_collect );
		ps_dec->i_slice_dec_collect++;
	}

	i_slicedec = ps_dec->i_slice_dec_dispatch++;
	ps_slicedec = &ps_dec->rgs_slice_decoders[ i_slicedec % ps_dec->i_num_slice_decoders ];

	memcpy( ps_slicedec->pui8_slice_data, pui8_slice, i_slice_size * sizeof( uint8_t ) );

	y262dec_bitstream_init( &ps_slicedec->s_bitstream, ps_slicedec->pui8_slice_data, i_slice_size );

	if( !ps_dec->b_multithreading )
	{
		y262dec_slice_process( ps_slicedec );
		ps_dec->i_slice_dec_collect++;
	}
	else
	{
		y262dec_slicedec_dispatch( ps_dec, i_slicedec );
	}
}


void y262dec_wait_slices_from_slicedec( y262dec_t *ps_dec )
{
	while( ps_dec->i_slice_dec_dispatch > ps_dec->i_slice_dec_collect )
	{
		y262dec_slicedec_wait( ps_dec, ps_dec->i_slice_dec_collect );
		ps_dec->i_slice_dec_collect++;
	}

	ps_dec->i_slice_dec_dispatch = 0;
	ps_dec->i_slice_dec_collect = 0;
}

void y262dec_input_buffer_push_unit( y262dec_t *ps_dec, uint8_t *pui8_unit, int32_t i_unit_size )
{
	int32_t i_startcode;
	int32_t i_extension_startcode;
	y262dec_bitstream_t *ps_bitstream = &ps_dec->s_bitstream;
	y262dec_state_t *ps_state = &ps_dec->s_dec_state;

	y262dec_bitstream_reset( ps_bitstream );
	y262dec_bitstream_init( ps_bitstream, pui8_unit, i_unit_size );

	i_startcode = y262dec_bitstream_read_small( ps_bitstream, 8 );
	if( i_startcode == Y262DEC_STARTCODE_EXTENSION )
	{
		i_extension_startcode = y262dec_bitstream_read_small( ps_bitstream, 4 );
	}
	else
	{
		i_extension_startcode = -1;
	}

	if( i_startcode >= Y262DEC_STARTCODE_SLICES_BEGIN && i_startcode <= Y262DEC_STARTCODE_SLICES_END )
	{
		if( ps_state->i_state != Y262DEC_STATE_PROCESS_SLICES )
		{
			y262dec_decoder_output_sequence_parameters( ps_dec );

			y262dec_decoder_init_buffers( ps_dec );

			y262dec_decoder_setup_reference_picture_buffers( ps_dec );

			y262dec_decoder_init_picture( ps_dec );

			y262dec_refresh_quantizer_matrices( ps_dec );

			ps_state->i_state = Y262DEC_STATE_PROCESS_SLICES;
		}

		y262dec_dispatch_slice_to_slicedec( ps_dec, pui8_unit, i_unit_size );
	}
	else
	{
		if( ps_state->i_state == Y262DEC_STATE_PROCESS_SLICES )
		{
			ps_state->i_state = Y262DEC_STATE_WAIT_INIT;

			y262dec_wait_slices_from_slicedec( ps_dec );

			y262dec_decoder_finish_picture( ps_dec );
		}

		if( i_startcode == 0xfe )
		{

		}
		else if( i_startcode == 0xff )
		{
			y262dec_decoder_flush_dpb( ps_dec );
		}
		else if( i_startcode == Y262DEC_STARTCODE_SEQUENCE_HEADER )
		{
			y262dec_parse_sequence_header( ps_dec );
			ps_state->b_mpeg_2 = false;
			y262dec_mpeg1_set_extension_sequence( ps_dec );
		}
		else if( i_startcode == Y262DEC_STARTCODE_USER_DATA )
		{
		}
		else if( i_startcode == Y262DEC_STARTCODE_GROUP )
		{
			y262dec_parse_group_header( ps_dec );
		}
		else if( i_startcode == Y262DEC_STARTCODE_PICTURE )
		{
			y262dec_parse_picture_header( ps_dec );
			y262dec_mpeg1_set_extension_picture_coding( ps_dec );
		}
		else if( i_startcode == Y262DEC_STARTCODE_EXTENSION )
		{
			if( i_extension_startcode == Y262DEC_EXTENSION_SEQUENCE )
			{
				y262dec_parse_extension_sequence( ps_dec );
				ps_state->b_mpeg_2 = true;
			}
			else if( i_extension_startcode == Y262DEC_EXTENSION_SEQUENCE_DISPLAY )
			{
				y262dec_parse_extension_sequence_display( ps_dec );
			}
			else if( i_extension_startcode == Y262DEC_EXTENSION_QUANT_MATRIX )
			{
				y262dec_parse_extension_quant_matrix( ps_dec );
			}
			else if( i_extension_startcode == Y262DEC_EXTENSION_PICTURE_DISPLAY )
			{
				y262dec_parse_extension_picture_display( ps_dec );
			}
			else if( i_extension_startcode == Y262DEC_EXTENSION_PICTURE_CODING )
			{
				y262dec_parse_extension_picture_coding( ps_dec );
			}
		}
	}
}


int32_t y262dec_input_buffer_process( y262dec_t *ps_dec, bool b_flush )
{
	uint8_t ui8_byte;
	uint32_t ui_possible_startcode;
	y262dec_input_buffer_t *ps_input_buffer = &ps_dec->s_input_buffer;

	ui_possible_startcode = ps_input_buffer->ui_possible_startcode;
	while( ps_input_buffer->i_read_position < ps_input_buffer->i_write_position )
	{
		ui8_byte = ps_input_buffer->pui8_buffer[ ps_input_buffer->i_read_position++ ];
		ui_possible_startcode = ( ui_possible_startcode | ui8_byte ) << 8;
		if( ui_possible_startcode == 0x00000100 )
		{
			if( ps_input_buffer->b_synced )
			{
				y262dec_input_buffer_push_unit( ps_dec, ps_input_buffer->pui8_buffer, ps_input_buffer->i_read_position );
				y262dec_input_buffer_advance( ps_dec, ps_input_buffer->i_read_position );
				if( ps_dec->s_dec_state.b_output_frame )
				{
					ps_dec->s_dec_state.b_output_frame = false;
					ps_input_buffer->ui_possible_startcode = ui_possible_startcode;
					return Y262DEC_INT_OK;
				}
			}
			else
			{
				ps_input_buffer->b_synced = true;
				y262dec_input_buffer_advance( ps_dec, ps_input_buffer->i_read_position );
			}
		}
	}
	ps_input_buffer->ui_possible_startcode = ui_possible_startcode;

	if( b_flush )
	{
		if( ps_input_buffer->b_synced )
		{
			y262dec_input_buffer_push_unit( ps_dec, ps_input_buffer->pui8_buffer, ps_input_buffer->i_write_position );
			y262dec_input_buffer_advance( ps_dec, ps_input_buffer->i_write_position );
			ps_input_buffer->b_synced = false;
		}
	}
	return Y262DEC_INT_MORE;
}


bool y262dec_decoder_init_slicedec( y262dec_t *ps_dec, int32_t i_slicedec_idx )
{
	int32_t i_block_size;
	uint32_t ui_remainder;
	uint8_t *pui8_block_buffer;

	y262dec_slicedec_t *ps_slicedec = &ps_dec->rgs_slice_decoders[ i_slicedec_idx ];

	memset( ps_slicedec, 0, sizeof( y262dec_slicedec_t ) );

	ps_slicedec->i_slicedec_idx = i_slicedec_idx;
	ps_slicedec->ps_dec = ps_dec;

	ps_slicedec->pui8_slice_data = malloc( sizeof( uint8_t ) * Y262DEC_MAX_SLICEDEC_SLICE_SIZE );
	if( !ps_slicedec->pui8_slice_data )
	{
		return false;
	}

	i_block_size = sizeof( int16_t ) * 8 * 8;
	pui8_block_buffer = ps_slicedec->s_state.pui8_block_buffer = malloc( i_block_size + 64 );
	if( !pui8_block_buffer )
	{
		free( ps_slicedec->pui8_slice_data );
		return false;
	}
	ui_remainder = ( ( uint8_t ) ps_slicedec->s_state.pui8_block_buffer ) & 0x3f;
	if( ui_remainder != 0 )
	{
		pui8_block_buffer += 64 - ui_remainder;
	}
	memset( pui8_block_buffer, 0, i_block_size );
	ps_slicedec->s_state.pi16_block = ( int16_t * ) pui8_block_buffer;

	if( ps_dec->b_multithreading )
	{
		y262dec_slicedec_thread_create( ps_dec, ps_slicedec->i_slicedec_idx );
	}

	return true;
}


void y262dec_decoder_deinit_slicedec( y262dec_t *ps_dec, int32_t i_slicedec_idx )
{
	y262dec_slicedec_t *ps_slicedec = &ps_dec->rgs_slice_decoders[ i_slicedec_idx ];

	if( ps_dec->b_multithreading && ps_slicedec->p_thread )
	{
		y262dec_slicedec_thread_destroy( ps_dec, ps_slicedec->i_slicedec_idx );
		ps_slicedec->p_thread = NULL;
	}

	if( ps_slicedec->pui8_slice_data )
	{
		free( ps_slicedec->pui8_slice_data );
		ps_slicedec->pui8_slice_data = NULL;
	}
	if( ps_slicedec->s_state.pui8_block_buffer )
	{
		free( ps_slicedec->s_state.pui8_block_buffer );
		ps_slicedec->s_state.pui8_block_buffer = NULL;
	}
}


bool y262dec_init( y262dec_t *ps_dec, y262dec_config_t *ps_config )
{
	int32_t i_idx;

	ps_dec->s_config = *ps_config;
	ps_dec->b_multithreading = ps_config->b_multithreading;

	if( ps_dec->b_multithreading )
	{
		ps_dec->p_resource_mutex = y262dec_create_mutex( ps_dec );
	}

	if( !y262dec_input_buffer_init( ps_dec, 200000000 / 8 ) ) /* FIXME: size by profile/level */
	{
		return false;
	}

	y262dec_init_motion_compensation( ps_dec );

	ps_dec->s_functions.pf_idct_put = y262dec_idct_put;
	ps_dec->s_functions.pf_idct_add = y262dec_idct_add;
#ifdef ASSEMBLY_X86
	ps_dec->s_functions.pf_idct_put = y262dec_idct_sse2_put;
	ps_dec->s_functions.pf_idct_add = y262dec_idct_sse2_add;
#endif
#ifdef ASSEMBLY_ARM64
	ps_dec->s_functions.pf_idct_put = y262dec_idct_neon_put;
	ps_dec->s_functions.pf_idct_add = y262dec_idct_neon_add;
#endif

	ps_dec->i_num_slice_decoders = Y262DEC_MAX_SLICE_DECODERS;
	for( i_idx = 0; i_idx < ps_dec->i_num_slice_decoders; i_idx++ )
	{
		if( !y262dec_decoder_init_slicedec( ps_dec, i_idx ) )
		{
			return false;
		}
	}

	return true;
}


void y262dec_deinit( y262dec_t *ps_dec )
{
	int32_t i_idx;

	for( i_idx = 0; i_idx < ps_dec->i_num_slice_decoders; i_idx++ )
	{
		y262dec_decoder_deinit_slicedec( ps_dec, i_idx );
	}

	y262dec_input_buffer_deinit( ps_dec );

	if( ps_dec->b_multithreading )
	{
		y262dec_destroy_mutex( ps_dec, ps_dec->p_resource_mutex );
	}

	if( ps_dec->s_dec_state.b_buffer_init )
	{
		free( ps_dec->s_dec_state.pui8_frame_buffer );
		ps_dec->s_dec_state.pui8_frame_buffer = NULL;
	}
}


int32_t y262dec_process_internal2( y262dec_t *ps_dec, bool b_flush )
{
	int32_t i_ret;

	while( 1 )
	{
		i_ret = y262dec_input_buffer_process( ps_dec, b_flush );
		if( i_ret == Y262DEC_INT_OK )
		{
			return Y262DEC_INT_OK;
		}
		else if( i_ret == Y262DEC_INT_MORE )
		{
			return Y262DEC_INT_MORE;
		}
		else if( i_ret == Y262DEC_INT_ERROR )
		{
			return Y262DEC_INT_ERROR;
		}
	}

	return Y262DEC_INT_MORE;
}


int32_t y262dec_process_internal( y262dec_t *ps_dec, uint8_t *pui8_data, int32_t i_data_size )
{
	if( pui8_data )
	{
		y262dec_input_buffer_push( ps_dec, pui8_data, i_data_size );
	}

	return y262dec_process_internal2( ps_dec, false );
}


int32_t y262dec_process_flush( y262dec_t *ps_dec )
{
	int32_t i_ret;
	uint8_t rgui8_flush_cmd[ 10 ] = { 0, 0, 1, 0xfe, 0xfe, 0, 0, 1, 0xff, 0xff };

	if( !ps_dec->b_flush_comitted )
	{
		y262dec_input_buffer_push( ps_dec, rgui8_flush_cmd, sizeof( rgui8_flush_cmd ) );
		y262dec_input_buffer_push( ps_dec, rgui8_flush_cmd, sizeof( rgui8_flush_cmd ) );
		y262dec_input_buffer_push( ps_dec, rgui8_flush_cmd, sizeof( rgui8_flush_cmd ) );
		y262dec_input_buffer_push( ps_dec, rgui8_flush_cmd, sizeof( rgui8_flush_cmd ) );
		ps_dec->b_flush_comitted = true;
	}
	
	i_ret = y262dec_process_internal2( ps_dec, true );
	if( i_ret == Y262DEC_INT_OK )
	{
		return Y262DEC_INT_MORE;
	}
	else if( i_ret == Y262DEC_INT_MORE )
	{
		return Y262DEC_INT_OK;
	}
	else
	{
		return i_ret;
	}
}






