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


bool y262dec_parse_sequence_header( y262dec_t *ps_dec )
{
	int32_t i_idx, i_scan_idx;

	y262dec_bitstream_t *ps_bitstream = &ps_dec->s_bitstream;
	y262dec_sequence_header_t *ps_sequence_header = &ps_dec->s_headers.s_sequence_header;
	y262dec_quantizer_matrices_t *ps_quantizer_matrices = &ps_dec->s_quantizer_matrices;

	ps_sequence_header->i_horizontal_size = y262dec_bitstream_read_small( ps_bitstream, 12 );
	ps_sequence_header->i_vertical_size = y262dec_bitstream_read_small( ps_bitstream, 12 );
	ps_sequence_header->i_aspect_ratio_information = y262dec_bitstream_read_small( ps_bitstream, 4 );
	ps_sequence_header->i_frame_rate_code = y262dec_bitstream_read_small( ps_bitstream, 4 );
	ps_sequence_header->i_bit_rate_value = y262dec_bitstream_read( ps_bitstream, 18 );
	if( !y262dec_bitstream_read_small( ps_bitstream, 1 ) ) /* marker */
	{
		return false;
	}
	ps_sequence_header->i_vbv_buffer_size_value = y262dec_bitstream_read_small( ps_bitstream, 10 );
	ps_sequence_header->b_constrained_parameters_flag = y262dec_bitstream_read_small( ps_bitstream, 1 );

	if( y262dec_bitstream_read_small( ps_bitstream, 1 ) )
	{
		for( i_idx = 0; i_idx < 64; i_idx++ )
		{
			i_scan_idx = rgui8_y262dec_scan_0_table[ i_idx ];
			ps_quantizer_matrices->rgui8_intra_quantizer_matrix[ i_scan_idx ] = ps_quantizer_matrices->rgui8_chroma_intra_quantizer_matrix[ i_scan_idx ] = y262dec_bitstream_read_small( ps_bitstream, 8 );
		}
		ps_quantizer_matrices->rgb_refresh_intra_quantizer_matrix[ 0 ] = ps_quantizer_matrices->rgb_refresh_intra_quantizer_matrix[ 1 ] = true;
	}
	else
	{
		for( i_idx = 0; i_idx < 64; i_idx++ )
		{
			i_scan_idx = rgui8_y262dec_scan_0_table[ i_idx ];
			ps_quantizer_matrices->rgui8_intra_quantizer_matrix[ i_scan_idx ] = ps_quantizer_matrices->rgui8_chroma_intra_quantizer_matrix[ i_scan_idx ] = rgui8_y262dec_default_intra_matrix[ i_idx ];
		}
	}

	if( y262dec_bitstream_read_small( ps_bitstream, 1 ) )
	{
		for( i_idx = 0; i_idx < 64; i_idx++ )
		{
			i_scan_idx = rgui8_y262dec_scan_0_table[ i_idx ];
			ps_quantizer_matrices->rgui8_non_intra_quantizer_matrix[ i_scan_idx ] = ps_quantizer_matrices->rgui8_chroma_non_intra_quantizer_matrix[ i_scan_idx ] = y262dec_bitstream_read_small( ps_bitstream, 8 );
		}
		ps_quantizer_matrices->rgb_refresh_non_intra_quantizer_matrix[ 0 ] = ps_quantizer_matrices->rgb_refresh_non_intra_quantizer_matrix[ 1 ] = true;
	}
	else
	{
		for( i_idx = 0; i_idx < 64; i_idx++ )
		{
			i_scan_idx = rgui8_y262dec_scan_0_table[ i_idx ];
			ps_quantizer_matrices->rgui8_non_intra_quantizer_matrix[ i_scan_idx ] = ps_quantizer_matrices->rgui8_chroma_non_intra_quantizer_matrix[ i_scan_idx ] = rgui8_y262dec_default_non_intra_matrix[ i_idx ];
		}
	}

	ps_quantizer_matrices->rgb_refresh_intra_quantizer_matrix[ 0 ] = ps_quantizer_matrices->rgb_refresh_intra_quantizer_matrix[ 1 ] = true;
	ps_quantizer_matrices->rgb_refresh_non_intra_quantizer_matrix[ 0 ] = ps_quantizer_matrices->rgb_refresh_non_intra_quantizer_matrix[ 1 ] = true;

	return y262dec_bitstream_not_past_end( ps_bitstream );
}


bool y262dec_parse_group_header( y262dec_t *ps_dec )
{
	y262dec_bitstream_t *ps_bitstream = &ps_dec->s_bitstream;
	y262dec_group_of_pictures_header_t *ps_group_of_pictures_header = &ps_dec->s_headers.s_group_of_pictures_header;

	ps_group_of_pictures_header->i_time_code = y262dec_bitstream_read( ps_bitstream, 25 );
	ps_group_of_pictures_header->b_closed_gop = y262dec_bitstream_read( ps_bitstream, 1 );
	ps_group_of_pictures_header->b_broken_link = y262dec_bitstream_read( ps_bitstream, 1 );

	return y262dec_bitstream_not_past_end( ps_bitstream );
}


bool y262dec_parse_picture_header( y262dec_t *ps_dec )
{
	y262dec_bitstream_t *ps_bitstream = &ps_dec->s_bitstream;
	y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;

	ps_picture_header->i_temporal_reference = y262dec_bitstream_read_small( ps_bitstream, 10 );
	ps_picture_header->i_picture_coding_type = y262dec_bitstream_read_small( ps_bitstream, 3 );
	ps_picture_header->i_vbv_delay = y262dec_bitstream_read_small( ps_bitstream, 16 );
	if( ps_picture_header->i_picture_coding_type == Y262DEC_PICTURE_CODING_TYPE_P ||
		ps_picture_header->i_picture_coding_type == Y262DEC_PICTURE_CODING_TYPE_B )
	{
		ps_picture_header->b_full_pel_forward_vector = y262dec_bitstream_read_small( ps_bitstream, 1 );
		ps_picture_header->i_forward_f_code = y262dec_bitstream_read_small( ps_bitstream, 3 );
	}
	if( ps_picture_header->i_picture_coding_type == Y262DEC_PICTURE_CODING_TYPE_B )
	{
		ps_picture_header->b_full_pel_backward_vector = y262dec_bitstream_read_small( ps_bitstream, 1 );
		ps_picture_header->i_backward_f_code = y262dec_bitstream_read_small( ps_bitstream, 3 );
	}

	ps_picture_header->b_extra_bit_picture = y262dec_bitstream_read_small( ps_bitstream, 1 );
	while( ps_picture_header->b_extra_bit_picture )
	{
		y262dec_bitstream_read( ps_bitstream, 8 );
		ps_picture_header->b_extra_bit_picture = y262dec_bitstream_read_small( ps_bitstream, 1 );
	}

	return y262dec_bitstream_not_past_end( ps_bitstream );
}


void y262dec_mpeg1_set_extension_sequence( y262dec_t *ps_dec )
{
	y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;

	ps_sequence_extension->i_profile_and_level_indication = 0;
	ps_sequence_extension->b_progressive_sequence = true;
	ps_sequence_extension->i_chroma_format = Y262DEC_CHROMA_FORMAT_420;
	ps_sequence_extension->i_horizontal_size_extension = 0;
	ps_sequence_extension->i_vertical_size_extension = 0;
	ps_sequence_extension->i_bit_rate_extension = 0;
	ps_sequence_extension->i_vbv_buffer_size_extension = 0;
	ps_sequence_extension->b_low_delay = 0;
	ps_sequence_extension->i_frame_rate_extension_n = 0;
	ps_sequence_extension->i_frame_rate_extension_d = 0;
}


bool y262dec_parse_extension_sequence( y262dec_t *ps_dec )
{
	y262dec_bitstream_t *ps_bitstream = &ps_dec->s_bitstream;
	y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;

	ps_sequence_extension->i_profile_and_level_indication = y262dec_bitstream_read_small( ps_bitstream, 8 );
	ps_sequence_extension->b_progressive_sequence = y262dec_bitstream_read_small( ps_bitstream, 1 );
	ps_sequence_extension->i_chroma_format = y262dec_bitstream_read_small( ps_bitstream, 2 );
	if( ps_sequence_extension->i_chroma_format == 0 )
	{
		return false;
	}
	ps_sequence_extension->i_horizontal_size_extension = y262dec_bitstream_read_small( ps_bitstream, 2 );
	ps_sequence_extension->i_vertical_size_extension = y262dec_bitstream_read_small( ps_bitstream, 2 );
	ps_sequence_extension->i_bit_rate_extension = y262dec_bitstream_read_small( ps_bitstream, 12 );
	if( !y262dec_bitstream_read_small( ps_bitstream, 1 ) ) /* marker */
	{
		return false;
	}
	ps_sequence_extension->i_vbv_buffer_size_extension = y262dec_bitstream_read_small( ps_bitstream, 8 );
	ps_sequence_extension->b_low_delay = y262dec_bitstream_read_small( ps_bitstream, 1 );
	ps_sequence_extension->i_frame_rate_extension_n = y262dec_bitstream_read_small( ps_bitstream, 2 );
	ps_sequence_extension->i_frame_rate_extension_d = y262dec_bitstream_read_small( ps_bitstream, 5 );

	return y262dec_bitstream_not_past_end( ps_bitstream );
}


bool y262dec_parse_extension_sequence_display( y262dec_t *ps_dec )
{
	y262dec_bitstream_t *ps_bitstream = &ps_dec->s_bitstream;
	y262dec_sequence_display_extension_t *ps_sequence_display_extension = &ps_dec->s_headers.s_sequence_display_extension;

	ps_sequence_display_extension->i_video_format = y262dec_bitstream_read_small( ps_bitstream, 3 );
	ps_sequence_display_extension->b_color_description = y262dec_bitstream_read_small( ps_bitstream, 1 );
	if( ps_sequence_display_extension->b_color_description )
	{
		ps_sequence_display_extension->s_color_description.i_color_primaries = y262dec_bitstream_read_small( ps_bitstream, 8 );
		ps_sequence_display_extension->s_color_description.i_transfer_characteristics = y262dec_bitstream_read_small( ps_bitstream, 8 );
		ps_sequence_display_extension->s_color_description.i_matrix_coefficients = y262dec_bitstream_read_small( ps_bitstream, 8 );
	}
	ps_sequence_display_extension->i_display_horizontal_size = y262dec_bitstream_read_small( ps_bitstream, 14 );
	if( !y262dec_bitstream_read_small( ps_bitstream, 1 ) ) /* marker */
	{
		return false;
	}
	ps_sequence_display_extension->i_display_vertical_size = y262dec_bitstream_read_small( ps_bitstream, 14 );

	return y262dec_bitstream_not_past_end( ps_bitstream );
}


bool y262dec_parse_extension_quant_matrix( y262dec_t *ps_dec )
{
	int32_t i_idx, i_scan_idx;
	y262dec_bitstream_t *ps_bitstream = &ps_dec->s_bitstream;
	y262dec_quantizer_matrices_t *ps_quantizer_matrices = &ps_dec->s_quantizer_matrices;

	if( y262dec_bitstream_read_small( ps_bitstream, 1 ) )
	{
		for( i_idx = 0; i_idx < 64; i_idx++ )
		{
			i_scan_idx = rgui8_y262dec_scan_0_table[ i_idx ];
			ps_quantizer_matrices->rgui8_intra_quantizer_matrix[ i_scan_idx ] = ps_quantizer_matrices->rgui8_chroma_intra_quantizer_matrix[ i_scan_idx ] = y262dec_bitstream_read_small( ps_bitstream, 8 );
		}
		ps_quantizer_matrices->rgb_refresh_intra_quantizer_matrix[ 0 ] = ps_quantizer_matrices->rgb_refresh_intra_quantizer_matrix[ 1 ] = true;
	}

	if( y262dec_bitstream_read_small( ps_bitstream, 1 ) )
	{
		for( i_idx = 0; i_idx < 64; i_idx++ )
		{
			i_scan_idx = rgui8_y262dec_scan_0_table[ i_idx ];
			ps_quantizer_matrices->rgui8_non_intra_quantizer_matrix[ i_scan_idx ] = ps_quantizer_matrices->rgui8_chroma_non_intra_quantizer_matrix[ i_scan_idx ] = y262dec_bitstream_read_small( ps_bitstream, 8 );
		}
		ps_quantizer_matrices->rgb_refresh_non_intra_quantizer_matrix[ 0 ] = ps_quantizer_matrices->rgb_refresh_non_intra_quantizer_matrix[ 1 ] = true;
	}

	if( y262dec_bitstream_read_small( ps_bitstream, 1 ) )
	{
		for( i_idx = 0; i_idx < 64; i_idx++ )
		{
			i_scan_idx = rgui8_y262dec_scan_0_table[ i_idx ];
			ps_quantizer_matrices->rgui8_chroma_intra_quantizer_matrix[ i_scan_idx ] = y262dec_bitstream_read_small( ps_bitstream, 8 );
		}
		ps_quantizer_matrices->rgb_refresh_chroma_intra_quantizer_matrix[ 0 ] = ps_quantizer_matrices->rgb_refresh_chroma_intra_quantizer_matrix[ 1 ] = true;
	}

	if( y262dec_bitstream_read_small( ps_bitstream, 1 ) )
	{
		for( i_idx = 0; i_idx < 64; i_idx++ )
		{
			i_scan_idx = rgui8_y262dec_scan_0_table[ i_idx ];
			ps_quantizer_matrices->rgui8_chroma_non_intra_quantizer_matrix[ i_scan_idx ] = y262dec_bitstream_read_small( ps_bitstream, 8 );
		}
		ps_quantizer_matrices->rgb_refresh_chroma_non_intra_quantizer_matrix[ 0 ] = ps_quantizer_matrices->rgb_refresh_chroma_non_intra_quantizer_matrix[ 1 ] = true;
	}

	return y262dec_bitstream_not_past_end( ps_bitstream );
}


bool y262dec_parse_extension_picture_display( y262dec_t *ps_dec )
{
	int32_t i_idx;
	y262dec_bitstream_t *ps_bitstream;
	y262dec_sequence_extension_t *ps_sequence_extension;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension;
	y262dec_picture_display_extension_t *ps_picture_display_extension;

	ps_bitstream = &ps_dec->s_bitstream;
	ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	ps_picture_display_extension = &ps_dec->s_headers.s_picture_display_extension;

	if( ps_sequence_extension->b_progressive_sequence )
	{
		if( ps_picture_coding_extension->b_repeat_first_field )
		{
			if( ps_picture_coding_extension->b_top_field_first )
			{
				ps_picture_display_extension->i_num_frame_centre_offsets = 3;
			}
			else
			{
				ps_picture_display_extension->i_num_frame_centre_offsets = 2;
			}
		}
		else
		{
			ps_picture_display_extension->i_num_frame_centre_offsets = 1;
		}
	}
	else
	{
		if( ps_picture_coding_extension->i_picture_structure != Y262DEC_PICTURE_CODING_STRUCTURE_FRAME )
		{
			ps_picture_display_extension->i_num_frame_centre_offsets = 1;
		}
		else
		{
			if( ps_picture_coding_extension->b_repeat_first_field )
			{
				ps_picture_display_extension->i_num_frame_centre_offsets = 3;
			}
			else
			{
				ps_picture_display_extension->i_num_frame_centre_offsets = 2;
			}
		}
	}

	for( i_idx = 0; i_idx < ps_picture_display_extension->i_num_frame_centre_offsets; i_idx++ )
	{
		ps_picture_display_extension->rgs_frame_centre_offsets[ i_idx ].i_frame_centre_horizontal_offset = y262dec_bitstream_read_small( ps_bitstream, 16 );
		if( !y262dec_bitstream_read_small( ps_bitstream, 1 ) ) /* marker */
		{
			return false;
		}
		ps_picture_display_extension->rgs_frame_centre_offsets[ i_idx ].i_frame_centre_vertical_offset = y262dec_bitstream_read_small( ps_bitstream, 16 );
		if( !y262dec_bitstream_read_small( ps_bitstream, 1 ) ) /* marker */
		{
			return false;
		}
	}

	return y262dec_bitstream_not_past_end( ps_bitstream );
}


void y262dec_mpeg1_set_extension_picture_coding( y262dec_t *ps_dec )
{
	y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;

	ps_picture_coding_extension->rgi_f_code[ Y262DEC_PICTURE_CODING_FORWARD ][ Y262DEC_PICTURE_CODING_HORIZONTAL ] = ps_picture_header->i_forward_f_code;
	ps_picture_coding_extension->rgi_f_code[ Y262DEC_PICTURE_CODING_FORWARD ][ Y262DEC_PICTURE_CODING_VERTICAL ] = ps_picture_header->i_forward_f_code;
	ps_picture_coding_extension->rgi_f_code[ Y262DEC_PICTURE_CODING_BACKWARD ][ Y262DEC_PICTURE_CODING_HORIZONTAL ] = ps_picture_header->i_backward_f_code;
	ps_picture_coding_extension->rgi_f_code[ Y262DEC_PICTURE_CODING_BACKWARD ][ Y262DEC_PICTURE_CODING_VERTICAL ] = ps_picture_header->i_backward_f_code;
	ps_picture_coding_extension->i_intra_dc_precision = 0;
	ps_picture_coding_extension->i_picture_structure = Y262DEC_PICTURE_CODING_STRUCTURE_FRAME;
	ps_picture_coding_extension->b_top_field_first = true;
	ps_picture_coding_extension->b_frame_pred_frame_dct = true;
	ps_picture_coding_extension->b_concealment_motion_vectors = false;
	ps_picture_coding_extension->b_q_scale_type = false;
	ps_picture_coding_extension->b_intra_vlc_format = false;
	ps_picture_coding_extension->b_alternate_scan = false;
	ps_picture_coding_extension->b_repeat_first_field = false;
	ps_picture_coding_extension->b_chroma_420_type = false;
	ps_picture_coding_extension->b_progressive_frame = true;
	ps_picture_coding_extension->b_composite_display_flag = false;
}


bool y262dec_parse_extension_picture_coding( y262dec_t *ps_dec )
{
	y262dec_bitstream_t *ps_bitstream = &ps_dec->s_bitstream;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;

	ps_picture_coding_extension->rgi_f_code[ Y262DEC_PICTURE_CODING_FORWARD ][ Y262DEC_PICTURE_CODING_HORIZONTAL ] = y262dec_bitstream_read_small( ps_bitstream, 4 );
	ps_picture_coding_extension->rgi_f_code[ Y262DEC_PICTURE_CODING_FORWARD ][ Y262DEC_PICTURE_CODING_VERTICAL ] = y262dec_bitstream_read_small( ps_bitstream, 4 );
	ps_picture_coding_extension->rgi_f_code[ Y262DEC_PICTURE_CODING_BACKWARD ][ Y262DEC_PICTURE_CODING_HORIZONTAL ] = y262dec_bitstream_read_small( ps_bitstream, 4 );
	ps_picture_coding_extension->rgi_f_code[ Y262DEC_PICTURE_CODING_BACKWARD ][ Y262DEC_PICTURE_CODING_VERTICAL ] = y262dec_bitstream_read_small( ps_bitstream, 4 );
	ps_picture_coding_extension->i_intra_dc_precision = y262dec_bitstream_read_small( ps_bitstream, 2 );
	ps_picture_coding_extension->i_picture_structure = y262dec_bitstream_read_small( ps_bitstream, 2 );
	ps_picture_coding_extension->b_top_field_first = y262dec_bitstream_read_small( ps_bitstream, 1 );
	ps_picture_coding_extension->b_frame_pred_frame_dct = y262dec_bitstream_read_small( ps_bitstream, 1 );
	ps_picture_coding_extension->b_concealment_motion_vectors = y262dec_bitstream_read_small( ps_bitstream, 1 );
	ps_picture_coding_extension->b_q_scale_type = y262dec_bitstream_read_small( ps_bitstream, 1 );
	ps_picture_coding_extension->b_intra_vlc_format = y262dec_bitstream_read_small( ps_bitstream, 1 );
	ps_picture_coding_extension->b_alternate_scan = y262dec_bitstream_read_small( ps_bitstream, 1 );
	ps_picture_coding_extension->b_repeat_first_field = y262dec_bitstream_read_small( ps_bitstream, 1 );
	ps_picture_coding_extension->b_chroma_420_type = y262dec_bitstream_read_small( ps_bitstream, 1 );
	ps_picture_coding_extension->b_progressive_frame = y262dec_bitstream_read_small( ps_bitstream, 1 );
	ps_picture_coding_extension->b_composite_display_flag = y262dec_bitstream_read_small( ps_bitstream, 1 );
	if( ps_picture_coding_extension->b_composite_display_flag )
	{
		ps_picture_coding_extension->s_composite_display.b_v_axis = y262dec_bitstream_read_small( ps_bitstream, 1 );
		ps_picture_coding_extension->s_composite_display.i_field_sequence = y262dec_bitstream_read_small( ps_bitstream, 3 );
		ps_picture_coding_extension->s_composite_display.b_sub_carrier = y262dec_bitstream_read_small( ps_bitstream, 1 );
		ps_picture_coding_extension->s_composite_display.i_burst_amplitude = y262dec_bitstream_read_small( ps_bitstream, 7 );
		ps_picture_coding_extension->s_composite_display.i_sub_carrier_phase = y262dec_bitstream_read_small( ps_bitstream, 8 );
	}

	return y262dec_bitstream_not_past_end( ps_bitstream );
}


