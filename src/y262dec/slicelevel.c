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


void y262dec_predictors_reset_intra( y262dec_slicedec_t *ps_slicedec )
{
	y262dec_predictors_t *ps_predictors = &ps_slicedec->s_state.s_predictors;
	ps_predictors->rgi16_dc_dct_pred[ 0 ] = 1 << 10;
	ps_predictors->rgi16_dc_dct_pred[ 1 ] = 1 << 10;
	ps_predictors->rgi16_dc_dct_pred[ 2 ] = 1 << 10;
}


void y262dec_predictors_reset_vector( y262dec_slicedec_t *ps_slicedec )
{
	y262dec_predictors_t *ps_predictors = &ps_slicedec->s_state.s_predictors;
	ps_predictors->rgi16_motion_vector_pred[ 0 ][ 0 ][ 0 ] = 0;
	ps_predictors->rgi16_motion_vector_pred[ 0 ][ 0 ][ 1 ] = 0;
	ps_predictors->rgi16_motion_vector_pred[ 0 ][ 1 ][ 0 ] = 0;
	ps_predictors->rgi16_motion_vector_pred[ 0 ][ 1 ][ 1 ] = 0;
	ps_predictors->rgi16_motion_vector_pred[ 1 ][ 0 ][ 0 ] = 0;
	ps_predictors->rgi16_motion_vector_pred[ 1 ][ 0 ][ 1 ] = 0;
	ps_predictors->rgi16_motion_vector_pred[ 1 ][ 1 ][ 0 ] = 0;
	ps_predictors->rgi16_motion_vector_pred[ 1 ][ 1 ][ 1 ] = 0;
}


void y262dec_macroblock_update_quantizer( y262dec_slicedec_t *ps_slicedec, int32_t i_quantizer )
{
	int32_t i_qscale_type;
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;
	y262dec_quantizer_matrices_t *ps_quantizer_matrices = &ps_dec->s_quantizer_matrices;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;

	i_qscale_type = ps_picture_coding_extension->b_q_scale_type;

	ps_macroblock->rgpui16_quantizer_matrices[ 0 ] = &ps_quantizer_matrices->rgui16_intra_quantizer_matrix[ i_qscale_type ][ i_quantizer ][ 0 ];
	ps_macroblock->rgpui16_quantizer_matrices[ 1 ] = &ps_quantizer_matrices->rgui16_non_intra_quantizer_matrix[ i_qscale_type ][ i_quantizer ][ 0 ];
	ps_macroblock->rgpui16_quantizer_matrices[ 2 ] = &ps_quantizer_matrices->rgui16_chroma_intra_quantizer_matrix[ i_qscale_type ][ i_quantizer ][ 0 ];
	ps_macroblock->rgpui16_quantizer_matrices[ 3 ] = &ps_quantizer_matrices->rgui16_chroma_non_intra_quantizer_matrix[ i_qscale_type ][ i_quantizer ][ 0 ];
}


int32_t y262dec_get_mb_address_increment( y262dec_slicedec_t *ps_slicedec )
{
	int32_t i_sum = 0;
	const y262dec_mb_addr_t *ps_mbaddr;
	y262dec_bitstream_t *ps_bitstream = &ps_slicedec->s_bitstream;

	while( 1 )
	{
		if( ps_bitstream->ui_bits >= 0x10000000 )
		{
			ps_mbaddr = &rgs_y262dec_macroblock_aincrement_5[ y262dec_bitstream_peek_small( ps_bitstream, 5 ) ];
			break;
		}
		else if( ps_bitstream->ui_bits >= 0x03000000 )
		{
			ps_mbaddr = &rgs_y262dec_macroblock_aincrement_11[ y262dec_bitstream_peek_small( ps_bitstream, 11 ) ];
			break;
		}
		else
		{
			if( y262dec_bitstream_peek_small( ps_bitstream, 11 ) == 8 )
			{
				i_sum = i_sum + 33;
				y262dec_bitstream_drop( ps_bitstream, 11 );
			}
			else
			{
				return -1;
			}
		}
	}
	y262dec_bitstream_drop( ps_bitstream, ps_mbaddr->ui8_len );
	i_sum = i_sum + ps_mbaddr->ui8_increment;

	return i_sum;
}


void y262dec_predict_skipped_macroblock( y262dec_slicedec_t *ps_slicedec )
{
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_predictors_t *ps_predictors = &ps_slicedec->s_state.s_predictors;
	y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;

	if( ps_picture_header->i_picture_coding_type == Y262DEC_PICTURE_CODING_TYPE_P )
	{
		if( ps_picture_coding_extension->i_picture_structure != Y262DEC_PICTURE_CODING_STRUCTURE_FRAME )
		{
			if( ps_picture_coding_extension->i_picture_structure == Y262DEC_PICTURE_CODING_STRUCTURE_TOP )
			{
				ps_macroblock->rgi_motion_vertical_field_select[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ] = Y262DEC_REFERENCE_TOP;
			}
			else
			{
				ps_macroblock->rgi_motion_vertical_field_select[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ] = Y262DEC_REFERENCE_BOTTOM;
			}
		}
	}
	else if( ps_picture_header->i_picture_coding_type == Y262DEC_PICTURE_CODING_TYPE_B )
	{
		if( ps_picture_coding_extension->i_picture_structure != Y262DEC_PICTURE_CODING_STRUCTURE_FRAME )
		{
			if( ps_picture_coding_extension->i_picture_structure == Y262DEC_PICTURE_CODING_STRUCTURE_TOP )
			{
				ps_macroblock->rgi_motion_vertical_field_select[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ] = Y262DEC_REFERENCE_TOP;
				ps_macroblock->rgi_motion_vertical_field_select[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_BACKWARD ] = Y262DEC_REFERENCE_TOP;
			}
			else
			{
				ps_macroblock->rgi_motion_vertical_field_select[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ] = Y262DEC_REFERENCE_BOTTOM;
				ps_macroblock->rgi_motion_vertical_field_select[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_BACKWARD ] = Y262DEC_REFERENCE_BOTTOM;
			}
		}
		else
		{
			ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ][ 1 ] = ps_predictors->rgi16_motion_vector_pred[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ][ 1 ];
			ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_BACKWARD ][ 1 ] = ps_predictors->rgi16_motion_vector_pred[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_BACKWARD ][ 1 ];
		}
	}
}



bool y262dec_slice_process( y262dec_slicedec_t *ps_slicedec )
{
	int32_t i_startcode, i_slice_vertical_position, i_mb_row, i_quantizer_scale_code, i_macroblock_address_increment, i_macroblock_address, i_max_macroblock_address, i_conceilment_motion_type, i_is_frame_vector;
	uint32_t ui_macroblock_type;
	bool b_intra_slice_flag, b_intra_slice, b_frame_pred_frame_dct, b_predictors_reset_vector;
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	y262dec_slice_state_t *ps_slice_state = &ps_slicedec->s_state;
	y262dec_macroblock_t *ps_macroblock;
	y262dec_bitstream_t *ps_bitstream = &ps_slicedec->s_bitstream;
	y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	y262dec_predictors_t *ps_predictors = &ps_slice_state->s_predictors;
	const y262dec_macroblock_motcomp_f ( *rgf_macroblock_motcomp_table );

	i_startcode = y262dec_bitstream_read_small( ps_bitstream, 8 );

	if( ps_state->i_vertical_size > 2800 )
	{
		i_slice_vertical_position = y262dec_bitstream_read_small( ps_bitstream, 3 ) << 7;
		i_slice_vertical_position += i_startcode;
		i_mb_row = i_slice_vertical_position - 1;
	}
	else
	{
		i_slice_vertical_position = i_startcode;
		i_mb_row = i_slice_vertical_position - 1;
	}

	i_quantizer_scale_code = y262dec_bitstream_read_small( ps_bitstream, 5 );

	b_intra_slice_flag = y262dec_bitstream_read_small( ps_bitstream, 1 );
	if( b_intra_slice_flag )
	{
		int32_t i_reserved, i_extra_information_slice;

		b_intra_slice = y262dec_bitstream_read_small( ps_bitstream, 1 );

		i_reserved = y262dec_bitstream_read_small( ps_bitstream, 7 );

		while( y262dec_bitstream_read_small( ps_bitstream, 1 ) )
		{
			i_extra_information_slice = y262dec_bitstream_read_small( ps_bitstream, 8 );
		}
	}
	i_max_macroblock_address = ( ps_state->i_picture_mb_width * ps_state->i_picture_mb_height ) - 1;

	y262dec_predictors_reset_intra( ps_slicedec );
	y262dec_predictors_reset_vector( ps_slicedec );
	b_predictors_reset_vector = false;

	b_frame_pred_frame_dct = ps_picture_coding_extension->b_frame_pred_frame_dct;
	if( ps_picture_coding_extension->i_picture_structure != Y262DEC_PICTURE_CODING_STRUCTURE_FRAME )
	{
		i_is_frame_vector = 0;
		i_conceilment_motion_type = Y262DEC_MOTION_TYPE_FIELD_FIELD;
		rgf_macroblock_motcomp_table = rgf_y262dec_motion_compensation_field;
	}
	else
	{
		i_is_frame_vector = 1;
		i_conceilment_motion_type = Y262DEC_MOTION_TYPE_FRAME_FRAME;
		rgf_macroblock_motcomp_table = rgf_y262dec_motion_compensation_frame;
	}

	ps_macroblock = &ps_slice_state->s_macroblock;
	y262dec_macroblock_update_quantizer( ps_slicedec, i_quantizer_scale_code );

	i_macroblock_address = ( ps_state->i_picture_mb_width * i_mb_row );
	i_macroblock_address_increment = y262dec_get_mb_address_increment( ps_slicedec );
	if( i_macroblock_address_increment < 0 )
	{
		return false;
	}
	i_macroblock_address += i_macroblock_address_increment;
	if( i_macroblock_address > i_max_macroblock_address )
	{
		return false;
	}
	y262dec_setup_macroblock( ps_slicedec, i_macroblock_address );

	do
	{
		if( ps_state->s_current_frame.i_don == 25 && ps_macroblock->i_macroblock_pel_x == 240 && ps_macroblock->i_macroblock_pel_y == 48 )
		{
			ps_macroblock = ps_macroblock;
		}
		ui_macroblock_type = y262dec_macroblock_parse_modes( ps_slicedec );
		if( !ui_macroblock_type )
		{
			return false;
		}
		ps_macroblock->ui_macroblock_type = ui_macroblock_type;

		if( ( ui_macroblock_type & Y262DEC_MACROBLOCK_INTRA ) )
		{
			if( ps_picture_coding_extension->b_concealment_motion_vectors )
			{
				if( b_predictors_reset_vector )
				{
					y262dec_predictors_reset_vector( ps_slicedec );
					b_predictors_reset_vector = false;
				}

				y262dec_vector_parse( ps_slicedec, !!i_is_frame_vector, i_conceilment_motion_type, 0 );

				if( !y262dec_bitstream_read_small( ps_bitstream, 1 ) ) /* marker */
				{
					return false;
				}
			}
			else
			{
				b_predictors_reset_vector = true;
			}

			if( !y262dec_macroblock_process_blocks_intra( ps_slicedec ) )
			{
				return false;
			}
		}
		else
		{
			int32_t i_motion_type;

			y262dec_predictors_reset_intra( ps_slicedec );
			if( b_predictors_reset_vector )
			{
				y262dec_predictors_reset_vector( ps_slicedec );
				b_predictors_reset_vector = false;
			}

			i_motion_type = ui_macroblock_type / Y262DEC_MACROBLOCK_MOTION_TYPE;

			if( ui_macroblock_type & ( Y262DEC_MACROBLOCK_MOTION_FORWARD | Y262DEC_MACROBLOCK_MOTION_BACKWARD ) )
			{
				if( ( ui_macroblock_type & Y262DEC_MACROBLOCK_MOTION_FORWARD ) )
				{
					y262dec_vector_parse( ps_slicedec, !!i_is_frame_vector, i_motion_type, 0 );
					rgf_macroblock_motcomp_table[ i_motion_type ]( ps_slicedec, 0, Y262DEC_MOTION_FORWARD );
				}

				if( ( ui_macroblock_type & Y262DEC_MACROBLOCK_MOTION_BACKWARD ) )
				{
					y262dec_vector_parse( ps_slicedec, !!i_is_frame_vector, i_motion_type, 1 );
					if( ui_macroblock_type & Y262DEC_MACROBLOCK_MOTION_FORWARD )
					{
						rgf_macroblock_motcomp_table[ i_motion_type ]( ps_slicedec, 1, Y262DEC_MOTION_BACKWARD );
					}
					else
					{
						rgf_macroblock_motcomp_table[ i_motion_type ]( ps_slicedec, 0, Y262DEC_MOTION_BACKWARD );
					}
				}
			}
			else
			{
				rgf_macroblock_motcomp_table[ 0 ]( ps_slicedec, 0, 0 );
				b_predictors_reset_vector = true;
			}

			if( ( ui_macroblock_type & Y262DEC_MACROBLOCK_PATTERN ) )
			{
				if( !y262dec_macroblock_process_blocks_inter( ps_slicedec ) )
				{
					return false;
				}
			}
		}

		if( y262dec_bitstream_peek_small( ps_bitstream, 11 ) )
		{
			i_macroblock_address_increment = y262dec_get_mb_address_increment( ps_slicedec );
			if( i_macroblock_address_increment < 0 )
			{
				break;
			}

			i_macroblock_address++;
			y262dec_setup_next_macroblock( ps_slicedec );

			if( i_macroblock_address_increment )
			{
				y262dec_predictors_reset_intra( ps_slicedec );
				y262dec_predict_skipped_macroblock( ps_slicedec );

				if( ps_picture_header->i_picture_coding_type != Y262DEC_PICTURE_CODING_TYPE_B )
				{
					b_predictors_reset_vector = true;
					while( i_macroblock_address_increment-- )
					{
						rgf_macroblock_motcomp_table[ 0 ]( ps_slicedec, 0, 0 );
						i_macroblock_address++;
						y262dec_setup_next_macroblock( ps_slicedec );
					}
				}
				else
				{
					while( i_macroblock_address_increment-- )
					{
						if( ui_macroblock_type & Y262DEC_MACROBLOCK_MOTION_FORWARD )
						{
							rgf_macroblock_motcomp_table[ i_conceilment_motion_type ]( ps_slicedec, 0, Y262DEC_MOTION_FORWARD );
						}
						if( ui_macroblock_type & Y262DEC_MACROBLOCK_MOTION_BACKWARD )
						{
							if( ui_macroblock_type & Y262DEC_MACROBLOCK_MOTION_FORWARD )
							{
								rgf_macroblock_motcomp_table[ i_conceilment_motion_type ]( ps_slicedec, 1, Y262DEC_MOTION_BACKWARD );
							}
							else
							{
								rgf_macroblock_motcomp_table[ i_conceilment_motion_type ]( ps_slicedec, 0, Y262DEC_MOTION_BACKWARD );
							}
						}
						i_macroblock_address++;
						y262dec_setup_next_macroblock( ps_slicedec );
					}
				}
			}
		}
		else
		{
			break;
		}
	} while( ps_bitstream->pui8_buffer_ptr < ps_bitstream->pui8_buffer_end );

	return y262dec_bitstream_not_past_end( ps_bitstream );
}


void y262dec_setup_macroblock( y262dec_slicedec_t *ps_slicedec, int32_t i_macroblock_address )
{
	int32_t i_x, i_y;
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;

	ps_macroblock->i_macroblock_address = i_macroblock_address;

	i_x = i_macroblock_address % ps_state->i_picture_mb_width;
	i_y = i_macroblock_address / ps_state->i_picture_mb_width;

	ps_macroblock->i_macroblock_pel_x = i_x << 4;
	ps_macroblock->i_macroblock_pel_y = i_y << 4;

	if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_420 )
	{
		ps_macroblock->i_macroblock_chroma_pel_x = ps_macroblock->i_macroblock_pel_x >> 1;
		ps_macroblock->i_macroblock_chroma_pel_y = ps_macroblock->i_macroblock_pel_y >> 1;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_422 )
	{
		ps_macroblock->i_macroblock_chroma_pel_x = ps_macroblock->i_macroblock_pel_x >> 1;
		ps_macroblock->i_macroblock_chroma_pel_y = ps_macroblock->i_macroblock_pel_y;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_444 )
	{
		ps_macroblock->i_macroblock_chroma_pel_x = ps_macroblock->i_macroblock_pel_x;
		ps_macroblock->i_macroblock_chroma_pel_y = ps_macroblock->i_macroblock_pel_y;
	}

	if( ps_picture_coding_extension->i_picture_structure == Y262DEC_PICTURE_CODING_STRUCTURE_FRAME )
	{
		y262dec_frame_picture_t *ps_frame = &ps_state->s_current_frame;
		ps_macroblock->pui8_destination_luma = ps_frame->pui8_luma + ps_macroblock->i_macroblock_pel_x + ps_macroblock->i_macroblock_pel_y * ps_frame->i_stride;
		ps_macroblock->pui8_destination_chroma_cb = ps_frame->pui8_cb + ps_macroblock->i_macroblock_chroma_pel_x + ps_macroblock->i_macroblock_chroma_pel_y * ps_frame->i_stride_chroma;
		ps_macroblock->pui8_destination_chroma_cr = ps_frame->pui8_cr + ps_macroblock->i_macroblock_chroma_pel_x + ps_macroblock->i_macroblock_chroma_pel_y * ps_frame->i_stride_chroma;
	}
	else
	{
		y262dec_field_picture_t *ps_field = &ps_state->s_current_field;
		ps_macroblock->pui8_destination_luma = ps_field->pui8_luma + ps_macroblock->i_macroblock_pel_x + ps_macroblock->i_macroblock_pel_y * ps_field->i_stride;
		ps_macroblock->pui8_destination_chroma_cb = ps_field->pui8_cb + ps_macroblock->i_macroblock_chroma_pel_x + ps_macroblock->i_macroblock_chroma_pel_y * ps_field->i_stride_chroma;
		ps_macroblock->pui8_destination_chroma_cr = ps_field->pui8_cr + ps_macroblock->i_macroblock_chroma_pel_x + ps_macroblock->i_macroblock_chroma_pel_y * ps_field->i_stride_chroma;
	}
}


void y262dec_setup_next_macroblock( y262dec_slicedec_t *ps_slicedec )
{
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;

	ps_macroblock->i_macroblock_address++;

	ps_macroblock->i_macroblock_pel_x += 16;
	ps_macroblock->i_macroblock_chroma_pel_x += ps_state->i_mb_chroma_width;
	ps_macroblock->pui8_destination_luma += 16;
	ps_macroblock->pui8_destination_chroma_cb += ps_state->i_mb_chroma_width;
	ps_macroblock->pui8_destination_chroma_cr += ps_state->i_mb_chroma_width;

	if( ps_macroblock->i_macroblock_pel_x >= ps_state->i_picture_width )
	{
		ps_macroblock->i_macroblock_pel_x = 0;
		ps_macroblock->i_macroblock_pel_y += 16;
		ps_macroblock->i_macroblock_chroma_pel_x = 0;
		ps_macroblock->i_macroblock_chroma_pel_y += ps_state->i_mb_chroma_height;

		if( ps_picture_coding_extension->i_picture_structure == Y262DEC_PICTURE_CODING_STRUCTURE_FRAME )
		{
			y262dec_frame_picture_t *ps_frame = &ps_state->s_current_frame;
			ps_macroblock->pui8_destination_luma = ps_frame->pui8_luma + ps_macroblock->i_macroblock_pel_y * ps_frame->i_stride;
			ps_macroblock->pui8_destination_chroma_cb = ps_frame->pui8_cb + ps_macroblock->i_macroblock_chroma_pel_y * ps_frame->i_stride_chroma;
			ps_macroblock->pui8_destination_chroma_cr = ps_frame->pui8_cr + ps_macroblock->i_macroblock_chroma_pel_y * ps_frame->i_stride_chroma;
		}
		else
		{
			y262dec_field_picture_t *ps_field = &ps_state->s_current_field;
			ps_macroblock->pui8_destination_luma = ps_field->pui8_luma + ps_macroblock->i_macroblock_pel_y * ps_field->i_stride;
			ps_macroblock->pui8_destination_chroma_cb = ps_field->pui8_cb + ps_macroblock->i_macroblock_chroma_pel_y * ps_field->i_stride_chroma;
			ps_macroblock->pui8_destination_chroma_cr = ps_field->pui8_cr + ps_macroblock->i_macroblock_chroma_pel_y * ps_field->i_stride_chroma;
		}
	}
}


uint32_t y262dec_macroblock_parse_modes_i( y262dec_slicedec_t *ps_slicedec )
{
	uint32_t ui_macroblock_type, ui_vlc;
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_bitstream_t *ps_bitstream = &ps_slicedec->s_bitstream;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	const y262dec_mbtype_t *ps_mbtype;

	ui_vlc = y262dec_bitstream_peek_small( ps_bitstream, 2 );
	ps_mbtype = &rgs_y262dec_macroblock_flags_i[ ui_vlc ];
	y262dec_bitstream_drop( ps_bitstream, ps_mbtype->i8_len );
	ui_macroblock_type = ps_mbtype->i8_flags;

	if( !ps_picture_coding_extension->b_frame_pred_frame_dct && ps_picture_coding_extension->i_picture_structure == Y262DEC_PICTURE_CODING_STRUCTURE_FRAME )
	{
		ui_macroblock_type |= y262dec_bitstream_read_small( ps_bitstream, 1 ) * Y262DEC_MACROBLOCK_INTERLACED;
	}

	return ui_macroblock_type;
}


uint32_t y262dec_macroblock_parse_modes_p( y262dec_slicedec_t *ps_slicedec )
{
	uint32_t ui_macroblock_type, ui_vlc;
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_bitstream_t *ps_bitstream = &ps_slicedec->s_bitstream;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	const y262dec_mbtype_t *ps_mbtype;

	ui_vlc = y262dec_bitstream_peek_small( ps_bitstream, 6 );
	ps_mbtype = &rgs_y262dec_macroblock_flags_p[ ui_vlc ];
	y262dec_bitstream_drop( ps_bitstream, ps_mbtype->i8_len );
	ui_macroblock_type = ps_mbtype->i8_flags;

	if( ps_picture_coding_extension->i_picture_structure != Y262DEC_PICTURE_CODING_STRUCTURE_FRAME )
	{
		if( ui_macroblock_type & Y262DEC_MACROBLOCK_MOTION_FORWARD )
		{
			ui_macroblock_type |= y262dec_bitstream_read_small( ps_bitstream, 2 ) * Y262DEC_MACROBLOCK_MOTION_TYPE;
		}
	}
	else if( ps_picture_coding_extension->b_frame_pred_frame_dct )
	{
		ui_macroblock_type |= Y262DEC_MOTION_TYPE_FRAME_FRAME * Y262DEC_MACROBLOCK_MOTION_TYPE;
	}
	else
	{
		if( ui_macroblock_type & Y262DEC_MACROBLOCK_MOTION_FORWARD )
		{
			ui_macroblock_type |= y262dec_bitstream_read_small( ps_bitstream, 2 ) * Y262DEC_MACROBLOCK_MOTION_TYPE;
		}
		if( ui_macroblock_type & ( Y262DEC_MACROBLOCK_INTRA | Y262DEC_MACROBLOCK_PATTERN ) )
		{
			ui_macroblock_type |= y262dec_bitstream_read_small( ps_bitstream, 1 ) * Y262DEC_MACROBLOCK_INTERLACED;
		}
	}

	return ui_macroblock_type;
}


uint32_t y262dec_macroblock_parse_modes_b( y262dec_slicedec_t *ps_slicedec )
{
	uint32_t ui_macroblock_type, ui_vlc;
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_bitstream_t *ps_bitstream = &ps_slicedec->s_bitstream;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	const y262dec_mbtype_t *ps_mbtype;

	ui_vlc = y262dec_bitstream_peek_small( ps_bitstream, 6 );
	ps_mbtype = &rgs_y262dec_macroblock_flags_b[ ui_vlc ];
	y262dec_bitstream_drop( ps_bitstream, ps_mbtype->i8_len );
	ui_macroblock_type = ps_mbtype->i8_flags;

	if( ps_picture_coding_extension->i_picture_structure != Y262DEC_PICTURE_CODING_STRUCTURE_FRAME )
	{
		if( !( ui_macroblock_type & Y262DEC_MACROBLOCK_INTRA ) )
		{
			ui_macroblock_type |= y262dec_bitstream_read_small( ps_bitstream, 2 ) * Y262DEC_MACROBLOCK_MOTION_TYPE;
		}
	}
	else if( ps_picture_coding_extension->b_frame_pred_frame_dct )
	{
		ui_macroblock_type |= Y262DEC_MOTION_TYPE_FRAME_FRAME * Y262DEC_MACROBLOCK_MOTION_TYPE;
	}
	else
	{
		if( !( ui_macroblock_type & Y262DEC_MACROBLOCK_INTRA ) )
		{
			ui_macroblock_type |= y262dec_bitstream_read_small( ps_bitstream, 2 ) * Y262DEC_MACROBLOCK_MOTION_TYPE;
		}

		if( ui_macroblock_type & ( Y262DEC_MACROBLOCK_INTRA | Y262DEC_MACROBLOCK_PATTERN ) )
		{
			ui_macroblock_type |= y262dec_bitstream_read_small( ps_bitstream, 1 ) * Y262DEC_MACROBLOCK_INTERLACED;
		}
	}

	return ui_macroblock_type;
}


uint32_t y262dec_macroblock_parse_modes( y262dec_slicedec_t *ps_slicedec )
{
	uint32_t ui_macroblock_type;
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_bitstream_t *ps_bitstream = &ps_slicedec->s_bitstream;
	y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;

	ui_macroblock_type = 0;
	if( ps_picture_header->i_picture_coding_type == Y262DEC_PICTURE_CODING_TYPE_I )
	{
		ui_macroblock_type = y262dec_macroblock_parse_modes_i( ps_slicedec );
	}
	else if( ps_picture_header->i_picture_coding_type == Y262DEC_PICTURE_CODING_TYPE_P )
	{
		ui_macroblock_type = y262dec_macroblock_parse_modes_p( ps_slicedec );
	}
	else if( ps_picture_header->i_picture_coding_type == Y262DEC_PICTURE_CODING_TYPE_B )
	{
		ui_macroblock_type = y262dec_macroblock_parse_modes_b( ps_slicedec );
	}

	if( ( ui_macroblock_type & Y262DEC_MACROBLOCK_QUANT ) )
	{
		int32_t i_quantizer_scale_code;
		i_quantizer_scale_code = y262dec_bitstream_read_small( ps_bitstream, 5 );
		y262dec_macroblock_update_quantizer( ps_slicedec, i_quantizer_scale_code );
	}

	return ui_macroblock_type;
}



int32_t y262dec_macroblock_get_motion_vector_delta( y262dec_bitstream_t *ps_bitstream, int32_t i_fcode_minus1 )
{
	const y262dec_mvdelta_t *ps_mvdelta;
	int32_t i_mv_delta, i_sign, i_residual;

	if( ps_bitstream->ui_bits & 0x80000000 )
	{
		y262dec_bitstream_drop( ps_bitstream, 1 );
		return 0;
	}
	else
	{
		if( ps_bitstream->ui_bits >= 0x0c000000 )
		{
			ps_mvdelta = &rgs_y262dec_mv_delta_4[ y262dec_bitstream_peek_small( ps_bitstream, 4 ) ];
			y262dec_bitstream_drop( ps_bitstream, ps_mvdelta->i8_len );
			i_sign = y262dec_bitstream_peek_sign( ps_bitstream );
			y262dec_bitstream_drop( ps_bitstream, 1 );
		}
		else
		{
			ps_mvdelta = &rgs_y262dec_mv_delta_10[ y262dec_bitstream_peek_small( ps_bitstream, 10 ) ];
			y262dec_bitstream_drop( ps_bitstream, ps_mvdelta->i8_len );
			i_sign = y262dec_bitstream_peek_sign( ps_bitstream );
			y262dec_bitstream_drop( ps_bitstream, 1 );
		}

		i_mv_delta = ps_mvdelta->i8_delta;
	}
	if( i_fcode_minus1 )
	{
		i_residual = y262dec_bitstream_read_small( ps_bitstream, i_fcode_minus1 );
		i_mv_delta = ( ( i_mv_delta - 1 ) << i_fcode_minus1 ) + i_residual + 1;
	}
	i_mv_delta = ( i_mv_delta ^ i_sign ) - i_sign;

	return i_mv_delta;
}



void y262dec_vector_parse_internal( y262dec_slicedec_t *ps_slicedec, bool b_frame_vector, int32_t i_vector_format, int32_t i_num_mv, bool b_dual_prime, int32_t i_s )
{
	int32_t i_mv, i_scale, i_fsize_x, i_fsize_y, i_dmvector, i_delta, i_prediction;
	int32_t i_parity_pred, i_parity_ref, i_add_x, i_add_y, rgi_m[ 2 ][ 2 ], rgi_e[ 2 ][ 2 ];
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	y262dec_bitstream_t *ps_bitstream = &ps_slicedec->s_bitstream;
	y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;
	y262dec_predictors_t *ps_predictors = &ps_slicedec->s_state.s_predictors;
	const y262dec_dprime_vector_t *ps_dpv;

	i_fsize_x = ps_picture_coding_extension->rgi_f_code[ i_s ][ 0 ] - 1;
	i_fsize_y = ps_picture_coding_extension->rgi_f_code[ i_s ][ 1 ] - 1;

	if( i_num_mv == 1 )
	{
		if( i_vector_format == Y262DEC_MV_FORMAT_FIELD && !b_dual_prime )
		{
			ps_macroblock->rgi_motion_vertical_field_select[ 0 ][ i_s ] = y262dec_bitstream_read_small( ps_bitstream, 1 );
		}
		i_delta = y262dec_macroblock_get_motion_vector_delta( ps_bitstream, i_fsize_x );
		i_prediction = ps_predictors->rgi16_motion_vector_pred[ 0 ][ i_s ][ 0 ] ;
		i_mv = i_prediction + i_delta;
		i_mv = ( i_mv << ( 27 - i_fsize_x ) ) >> ( 27 - i_fsize_x );
		ps_macroblock->rgi16_motion_vector[ 0 ][ i_s ][ 0 ] = i_mv;
		ps_predictors->rgi16_motion_vector_pred[ 0 ][ i_s ][ 0 ] = i_mv;
		ps_predictors->rgi16_motion_vector_pred[ 1 ][ i_s ][ 0 ] = i_mv;
		if( b_dual_prime )
		{
			i_dmvector = y262dec_bitstream_peek_small( ps_bitstream, 2 );
			ps_dpv = &rgs_y262dec_dual_prime_vector[ i_dmvector ];
			ps_macroblock->rgi_dmvector[ 0 ] = ps_dpv->i8_vector;
			y262dec_bitstream_drop( ps_bitstream, ps_dpv->i8_len );
		}
		i_delta = y262dec_macroblock_get_motion_vector_delta( ps_bitstream, i_fsize_y );
		i_prediction = ps_predictors->rgi16_motion_vector_pred[ 0 ][ i_s ][ 1 ] ;
		if( i_vector_format == Y262DEC_MV_FORMAT_FIELD && b_frame_vector )
		{
			i_scale = 1;
		}
		else
		{
			i_scale = 0;
		}
		i_prediction = i_prediction >> i_scale;
		i_mv = i_prediction + i_delta;
		i_mv = ( i_mv << ( 27 - i_fsize_y ) ) >> ( 27 - i_fsize_y );
		ps_macroblock->rgi16_motion_vector[ 0 ][ i_s ][ 1 ] = i_mv;
		ps_predictors->rgi16_motion_vector_pred[ 0 ][ i_s ][ 1 ] = i_mv << i_scale;
		ps_predictors->rgi16_motion_vector_pred[ 1 ][ i_s ][ 1 ] = i_mv << i_scale;
		if( b_dual_prime )
		{
			i_dmvector = y262dec_bitstream_peek_small( ps_bitstream, 2 );
			ps_dpv = &rgs_y262dec_dual_prime_vector[ i_dmvector ];
			ps_macroblock->rgi_dmvector[ 1 ] = ps_dpv->i8_vector;
			y262dec_bitstream_drop( ps_bitstream, ps_dpv->i8_len );
		}
		if( b_dual_prime && b_frame_vector )
		{
			if( ps_picture_coding_extension->b_top_field_first )
			{
				rgi_m[ 1 ][ 0 ] = 1;
				rgi_m[ 0 ][ 1 ] = 3;
			}
			else
			{
				rgi_m[ 1 ][ 0 ] = 3;
				rgi_m[ 0 ][ 1 ] = 1;
			}
			rgi_e[ 0 ][ 1 ] = 1;
			rgi_e[ 1 ][ 0 ] = -1;
			i_add_x = ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ][ 0 ] > 0 ? 1 : 0;
			i_add_y = ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ][ 1 ] > 0 ? 1 : 0;
			i_parity_ref = Y262DEC_REFERENCE_BOTTOM;
			i_parity_pred = Y262DEC_REFERENCE_TOP;
			ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_THIRD ][ Y262DEC_MOTION_FORWARD ][ 0 ] =
				( ( ( ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ][ 0 ] * rgi_m[ i_parity_ref ][ i_parity_pred ] ) + i_add_x ) >> 1 ) + ps_macroblock->rgi_dmvector[ 0 ];
			ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_THIRD ][ Y262DEC_MOTION_FORWARD ][ 1 ] =
				( ( ( ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ][ 1 ] * rgi_m[ i_parity_ref ][ i_parity_pred ] ) + i_add_y ) >> 1 ) + rgi_e[ i_parity_ref ][ i_parity_pred ] + ps_macroblock->rgi_dmvector[ 1 ];
			i_parity_ref = Y262DEC_REFERENCE_TOP;
			i_parity_pred = Y262DEC_REFERENCE_BOTTOM;
			ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FOURTH ][ Y262DEC_MOTION_FORWARD ][ 0 ] =
				( ( ( ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ][ 0 ] * rgi_m[ i_parity_ref ][ i_parity_pred ] ) + i_add_x ) >> 1 ) + ps_macroblock->rgi_dmvector[ 0 ];
			ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FOURTH ][ Y262DEC_MOTION_FORWARD ][ 1 ] =
				( ( ( ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ][ 1 ] * rgi_m[ i_parity_ref ][ i_parity_pred ] ) + i_add_y ) >> 1 ) + rgi_e[ i_parity_ref ][ i_parity_pred ] + ps_macroblock->rgi_dmvector[ 1 ];
		}
		else if( b_dual_prime )
		{
			rgi_e[ 0 ][ 1 ] = 1;
			rgi_e[ 1 ][ 0 ] = -1;
			if( ps_picture_coding_extension->i_picture_structure == Y262DEC_PICTURE_CODING_STRUCTURE_TOP )
			{
				i_parity_ref = Y262DEC_REFERENCE_BOTTOM;
				i_parity_pred = Y262DEC_REFERENCE_TOP;
			}
			else
			{
				i_parity_ref = Y262DEC_REFERENCE_TOP;
				i_parity_pred = Y262DEC_REFERENCE_BOTTOM;
			}
			i_add_x = ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ][ 0 ] > 0 ? 1 : 0;
			i_add_y = ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ][ 1 ] > 0 ? 1 : 0;
			ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_THIRD ][ Y262DEC_MOTION_FORWARD ][ 0 ] =
				( ( ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ][ 0 ] + i_add_x ) >> 1 ) + ps_macroblock->rgi_dmvector[ 0 ];
			ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_THIRD ][ Y262DEC_MOTION_FORWARD ][ 1 ] =
				( ( ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ Y262DEC_MOTION_FORWARD ][ 1 ] + i_add_y ) >> 1 ) + rgi_e[ i_parity_ref ][ i_parity_pred ] + ps_macroblock->rgi_dmvector[ 1 ];
		}
	}
	else
	{
		ps_macroblock->rgi_motion_vertical_field_select[ 0 ][ i_s ] = y262dec_bitstream_read_small( ps_bitstream, 1 );
		i_delta = y262dec_macroblock_get_motion_vector_delta( ps_bitstream, i_fsize_x );
		i_prediction = ps_predictors->rgi16_motion_vector_pred[ 0 ][ i_s ][ 0 ];
		i_mv = i_prediction + i_delta;
		i_mv = ( i_mv << ( 27 - i_fsize_x ) ) >> ( 27 - i_fsize_x );
		ps_macroblock->rgi16_motion_vector[ 0 ][ i_s ][ 0 ] = i_mv;
		ps_predictors->rgi16_motion_vector_pred[ 0 ][ i_s ][ 0 ] = i_mv;

		i_delta = y262dec_macroblock_get_motion_vector_delta( ps_bitstream, i_fsize_y );
		i_prediction = ps_predictors->rgi16_motion_vector_pred[ 0 ][ i_s ][ 1 ];
		if( i_vector_format == Y262DEC_MV_FORMAT_FIELD && b_frame_vector )
		{
			i_scale = 1;
		}
		else
		{
			i_scale = 0;
		}
		i_prediction = i_prediction >> i_scale;
		i_mv = i_prediction + i_delta;
		i_mv = ( i_mv << ( 27 - i_fsize_y ) ) >> ( 27 - i_fsize_y );
		ps_macroblock->rgi16_motion_vector[ 0 ][ i_s ][ 1 ] = i_mv;
		ps_predictors->rgi16_motion_vector_pred[ 0 ][ i_s ][ 1 ] = i_mv << i_scale;
		ps_macroblock->rgi_motion_vertical_field_select[ 1 ][ i_s ] = y262dec_bitstream_read_small( ps_bitstream, 1 );
		i_delta = y262dec_macroblock_get_motion_vector_delta( ps_bitstream, i_fsize_x );
		i_prediction = ps_predictors->rgi16_motion_vector_pred[ 1 ][ i_s ][ 0 ];
		i_mv = i_prediction + i_delta;
		i_mv = ( i_mv << ( 27 - i_fsize_x ) ) >> ( 27 - i_fsize_x );
		ps_macroblock->rgi16_motion_vector[ 1 ][ i_s ][ 0 ] = i_mv;
		ps_predictors->rgi16_motion_vector_pred[ 1 ][ i_s ][ 0 ] = i_mv;
		i_delta = y262dec_macroblock_get_motion_vector_delta( ps_bitstream, i_fsize_y );
		i_prediction = ps_predictors->rgi16_motion_vector_pred[ 1 ][ i_s ][ 1 ];
		if( i_vector_format == Y262DEC_MV_FORMAT_FIELD && b_frame_vector )
		{
			i_scale = 1;
		}
		else
		{
			i_scale = 0;
		}
		i_prediction = i_prediction >> i_scale;
		i_mv = i_prediction + i_delta;
		i_mv = ( i_mv << ( 27 - i_fsize_y ) ) >> ( 27 - i_fsize_y );
		ps_macroblock->rgi16_motion_vector[ 1 ][ i_s ][ 1 ] = i_mv;
		ps_predictors->rgi16_motion_vector_pred[ 1 ][ i_s ][ 1 ] = i_mv << i_scale;
	}
}


void y262dec_vector_parse( y262dec_slicedec_t *ps_slicedec, int32_t i_is_frame, int32_t i_motion_type, int32_t i_s )
{
	if( i_is_frame )
	{
		if( i_motion_type == Y262DEC_MOTION_TYPE_FRAME_FIELD )
		{
			y262dec_vector_parse_internal( ps_slicedec, true, Y262DEC_MV_FORMAT_FIELD, 2, false, i_s );
		}
		else if( i_motion_type == Y262DEC_MOTION_TYPE_FRAME_FRAME )
		{
			y262dec_vector_parse_internal( ps_slicedec, true, Y262DEC_MV_FORMAT_FRAME, 1, false, i_s );
		}
		else if( i_motion_type == Y262DEC_MOTION_TYPE_FRAME_DPRIME )
		{
			y262dec_vector_parse_internal( ps_slicedec, true, Y262DEC_MV_FORMAT_FIELD, 1, true, i_s );
		}
	}
	else
	{
		if( i_motion_type == Y262DEC_MOTION_TYPE_FIELD_FIELD )
		{
			y262dec_vector_parse_internal( ps_slicedec, false, Y262DEC_MV_FORMAT_FIELD, 1, false, i_s );
		}
		else if( i_motion_type == Y262DEC_MOTION_TYPE_FIELD_2X )
		{
			y262dec_vector_parse_internal( ps_slicedec, false, Y262DEC_MV_FORMAT_FIELD, 2, false, i_s );
		}
		else if( i_motion_type == Y262DEC_MOTION_TYPE_FIELD_DPRIME )
		{
			y262dec_vector_parse_internal( ps_slicedec, false, Y262DEC_MV_FORMAT_FRAME, 1, true, i_s );
		}
	}
}




int32_t y262dec_macroblock_process_dct_luma_dc( y262dec_bitstream_t *ps_bitstream, uint32_t ui_intra_dc_shift )
{
	int32_t i_dct_dc_size, i_dct_differential, i_half_range;
	const y262dec_dcsize_t *ps_dct_size;

	if( ps_bitstream->ui_bits < 0xf8000000 )
	{
		ps_dct_size = &rgs_y262dec_dct_dc_size_luma_5[ y262dec_bitstream_peek_small( ps_bitstream, 5 ) ];
		i_dct_dc_size = ps_dct_size->i8_size;
		if( i_dct_dc_size > 0 )
		{
			y262dec_bitstream_drop( ps_bitstream, ps_dct_size->i8_len );
			i_dct_differential = y262dec_bitstream_read_small( ps_bitstream, i_dct_dc_size );
			i_half_range = 1 << ( i_dct_dc_size - 1 );
			if( i_dct_differential < i_half_range )
			{
				i_dct_differential = ( i_dct_differential + 1 ) - ( 2 * i_half_range );
			}
			return i_dct_differential << ui_intra_dc_shift;
		}
		else
		{
			y262dec_bitstream_drop( ps_bitstream, 3 );
			return 0;
		}
	}
	else
	{
		uint32_t ui_vlc;
		ui_vlc = y262dec_bitstream_peek_small( ps_bitstream, 9 ) - 0x1f0;
		ps_dct_size = &rgs_y262dec_dct_dc_size_luma_9[ ui_vlc ];
		y262dec_bitstream_drop( ps_bitstream, ps_dct_size->i8_len );
		i_dct_dc_size = ps_dct_size->i8_size;
		i_dct_differential = y262dec_bitstream_peek_small( ps_bitstream, i_dct_dc_size );
		i_half_range = 1 << ( i_dct_dc_size - 1 );
		if( i_dct_differential < i_half_range )
		{
			i_dct_differential = ( i_dct_differential + 1 ) - ( 2 * i_half_range );
		}
	}

	y262dec_bitstream_drop( ps_bitstream, i_dct_dc_size );

	return i_dct_differential << ui_intra_dc_shift;
}

int32_t y262dec_macroblock_process_dct_chroma_dc( y262dec_bitstream_t *ps_bitstream, uint32_t ui_intra_dc_shift )
{
	int32_t i_dct_dc_size, i_dct_differential, i_half_range;
	const y262dec_dcsize_t *ps_dct_size;

	if( ps_bitstream->ui_bits < 0xf8000000 )
	{
		ps_dct_size = &rgs_y262dec_dct_dc_size_chroma_5[ y262dec_bitstream_peek_small( ps_bitstream, 5 ) ];
		i_dct_dc_size = ps_dct_size->i8_size;
		if( i_dct_dc_size > 0 )
		{
			y262dec_bitstream_drop( ps_bitstream, ps_dct_size->i8_len );
			i_dct_differential = y262dec_bitstream_read_small( ps_bitstream, i_dct_dc_size );
			i_half_range = 1 << ( i_dct_dc_size - 1 );
			if( i_dct_differential < i_half_range )
			{
				i_dct_differential = ( i_dct_differential + 1 ) - ( 2 * i_half_range );
			}
			return i_dct_differential << ui_intra_dc_shift;
		}
		else
		{
			y262dec_bitstream_drop( ps_bitstream, 2 );
			return 0;
		}
	}
	else
	{
		ps_dct_size = &rgs_y262dec_dct_dc_size_chroma_10[ y262dec_bitstream_peek_small( ps_bitstream, 10 ) - 0x3e0 ];
		y262dec_bitstream_drop( ps_bitstream, ps_dct_size->i8_len );
		i_dct_dc_size = ps_dct_size->i8_size;
		i_dct_differential = y262dec_bitstream_peek_small( ps_bitstream, i_dct_dc_size );
		y262dec_bitstream_drop( ps_bitstream, i_dct_dc_size );
		i_half_range = 1 << ( i_dct_dc_size - 1 );
		if( i_dct_differential < i_half_range )
		{
			i_dct_differential = ( i_dct_differential + 1 ) - ( 2 * i_half_range );
		}
	}
	return i_dct_differential << ui_intra_dc_shift;
}


int32_t y262dec_parse_macroblock_block_intra_mpeg1_vlc0( y262dec_slicedec_t *ps_slicedec, uint16_t *pui16_quant_matrix )
{
	int32_t i_idx, i_level, i_sign, i_scan_idx;
	uint32_t ui_codeword;
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_bitstream_t *ps_bitstream = &ps_slicedec->s_bitstream;
	const uint8_t *pui8_scan = ps_dec->s_dec_state.pui8_current_scan;
	int16_t *pi16_block = ps_slicedec->s_state.pi16_block;
	
	i_idx = 0;

	while( 1 )
	{
		const y262dec_run_level_t *ps_run_level;

		ui_codeword = ps_bitstream->ui_bits;
		if( ui_codeword >= 0x28000000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_5[ y262dec_bitstream_peek_small( ps_bitstream, 5 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx >= 64 )
			{
				break;
			}

parse_macroblock_block_intra_vlc0_coeff:
			y262dec_bitstream_drop( ps_bitstream, ps_run_level->i8_length );
			i_scan_idx = pui8_scan[ i_idx ];
			i_level = ( ps_run_level->i8_level * pui16_quant_matrix[ i_scan_idx ] ) >> 5;
			i_sign = y262dec_bitstream_peek_sign( ps_bitstream );
			y262dec_bitstream_drop( ps_bitstream, 1 );
			if( i_sign )
			{
				i_level = -( ( i_level - 1 ) | 1 );
			}
			else
			{
				i_level = ( i_level - 1 ) | 1;
			}
			if( i_level > 2047 )
			{
				i_level = 2047;
			}
			else if( i_level < -2048 )
			{
				i_level = -2048;
			}
			pi16_block[ i_scan_idx ] = i_level;
			continue;
		}
		else if( ui_codeword >= 0x4000000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_8[ y262dec_bitstream_peek_small( ps_bitstream, 8 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_intra_vlc0_coeff;
			}
			y262dec_bitstream_drop( ps_bitstream, 6 );
			i_idx += y262dec_bitstream_read_small( ps_bitstream, 6 ) - 64;
			if( i_idx >= 64 )
			{
				break;
			}
			i_scan_idx = pui8_scan[ i_idx ];

			i_level = ( ( ( int32_t ) y262dec_bitstream_read_small( ps_bitstream, 8 ) ) << 24 ) >> 24;
			if( i_level == -128 )
			{
				i_level = -256 + ( ( int32_t ) y262dec_bitstream_read_small( ps_bitstream, 8 ) );
			}
			else if( i_level == 0 )
			{
				i_level = y262dec_bitstream_read_small( ps_bitstream, 8 );
			}
			i_level = ( i_level * pui16_quant_matrix[ i_scan_idx ] ) / 32;
			if( i_level < 0 )
			{
				i_level = -i_level;
				i_level = ( i_level - 1 ) | 1;
				i_level = -i_level;
			}
			else
			{
				i_level = ( i_level - 1 ) | 1;
			}
			if( i_level > 2047 )
			{
				i_level = 2047;
			}
			else if( i_level < -2048 )
			{
				i_level = -2048;
			}
			pi16_block[ i_scan_idx ] = i_level;
			continue;
		}
		else if( ui_codeword >= 0x2000000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_10[ y262dec_bitstream_peek_small( ps_bitstream, 10 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_intra_vlc0_coeff;
			}
		}
		else if( ui_codeword >= 0x00800000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_13[ y262dec_bitstream_peek_small( ps_bitstream, 13 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_intra_vlc0_coeff;
			}
		}
		else if( ui_codeword >= 0x00200000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_15[ y262dec_bitstream_peek_small( ps_bitstream, 15 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_intra_vlc0_coeff;
			}
		}
		else if( ui_codeword >= 0x00100000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_16[ y262dec_bitstream_peek_small( ps_bitstream, 16 ) ];
			y262dec_bitstream_drop( ps_bitstream, 16 );
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_intra_vlc0_coeff;
			}
		}
		break;
	}

	y262dec_bitstream_drop( ps_bitstream, 2 );
	return i_idx;
}


int32_t y262dec_parse_macroblock_block_intra_mpeg2_vlc0( y262dec_slicedec_t *ps_slicedec, uint16_t *pui16_quant_matrix )
{
	int32_t i_idx, i_level, i_sign, i_scan_idx, i_sum;
	uint32_t ui_codeword;
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_bitstream_t *ps_bitstream = &ps_slicedec->s_bitstream;
	const uint8_t *pui8_scan = ps_dec->s_dec_state.pui8_current_scan;
	int16_t *pi16_block = ps_slicedec->s_state.pi16_block;

	i_sum = pi16_block[ 0 ] + 1;
	i_idx = 0;

	while( 1 )
	{
		const y262dec_run_level_t *ps_run_level;

		ui_codeword = ps_bitstream->ui_bits;
		if( ui_codeword >= 0x28000000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_5[ y262dec_bitstream_peek_small( ps_bitstream, 5 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx >= 64 )
			{
				break;
			}

parse_macroblock_block_intra_vlc0_coeff:
			y262dec_bitstream_drop( ps_bitstream, ps_run_level->i8_length );
			i_scan_idx = pui8_scan[ i_idx ];
			i_level = ( ps_run_level->i8_level * pui16_quant_matrix[ i_scan_idx ] ) >> 5;
			i_sign = y262dec_bitstream_peek_sign( ps_bitstream );
			y262dec_bitstream_drop( ps_bitstream, 1 );
			i_level = ( i_level ^ i_sign ) - i_sign;
			if( i_level > 2047 )
			{
				i_level = 2047;
			}
			else if( i_level < -2048 )
			{
				i_level = -2048;
			}
			pi16_block[ i_scan_idx ] = i_level;
			i_sum ^= i_level;
			continue;
		}
		else if( ui_codeword >= 0x4000000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_8[ y262dec_bitstream_peek_small( ps_bitstream, 8 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_intra_vlc0_coeff;
			}
			y262dec_bitstream_drop( ps_bitstream, 6 );
			i_idx += y262dec_bitstream_read_small( ps_bitstream, 6 ) - 64;
			if( i_idx >= 64 )
			{
				break;
			}
			i_scan_idx = pui8_scan[ i_idx ];
			i_level = ( ( int32_t ) ps_bitstream->ui_bits ) >> ( 32 - 12 );
			i_level = ( i_level * pui16_quant_matrix[ i_scan_idx ] ) / 32;
			y262dec_bitstream_drop( ps_bitstream, 12 );
			if( i_level > 2047 )
			{
				i_level = 2047;
			}
			else if( i_level < -2048 )
			{
				i_level = -2048;
			}
			pi16_block[ i_scan_idx ] = i_level;
			i_sum ^= i_level;
			continue;
		}
		else if( ui_codeword >= 0x2000000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_10[ y262dec_bitstream_peek_small( ps_bitstream, 10 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_intra_vlc0_coeff;
			}
		}
		else if( ui_codeword >= 0x00800000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_13[ y262dec_bitstream_peek_small( ps_bitstream, 13 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_intra_vlc0_coeff;
			}
		}
		else if( ui_codeword >= 0x00200000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_15[ y262dec_bitstream_peek_small( ps_bitstream, 15 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_intra_vlc0_coeff;
			}
		}
		else if( ui_codeword >= 0x00100000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_16[ y262dec_bitstream_peek_small( ps_bitstream, 16 ) ];
			y262dec_bitstream_drop( ps_bitstream, 16 );
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_intra_vlc0_coeff;
			}
		}
		break;
	}

	pi16_block[ 63 ] ^= ( i_sum & 1 );
	y262dec_bitstream_drop( ps_bitstream, 2 );
	return i_idx;
}


int32_t y262dec_parse_macroblock_block_intra_vlc1( y262dec_slicedec_t *ps_slicedec, uint16_t *pui16_quant_matrix )
{
	int32_t i_idx, i_level, i_sign, i_scan_idx, i_sum;
	uint32_t ui_codeword;
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_bitstream_t *ps_bitstream = &ps_slicedec->s_bitstream;
	const uint8_t *pui8_scan = ps_dec->s_dec_state.pui8_current_scan;
	int16_t *pi16_block = ps_slicedec->s_state.pi16_block;

	i_sum = pi16_block[ 0 ] + 1;
	i_idx = 0;

	while( 1 )
	{
		const y262dec_run_level_t *ps_run_level;

		ui_codeword = ps_bitstream->ui_bits;
		if( ui_codeword >= 0x04000000 )
		{
			ps_run_level = &rgs_y262dec_dct_one_table_8f[ y262dec_bitstream_peek_small( ps_bitstream, 8 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
parse_macroblock_block_intra_vlc1_coeff:
				i_level = ps_run_level->i8_level;
				y262dec_bitstream_drop( ps_bitstream, ps_run_level->i8_length );

				i_sign = y262dec_bitstream_peek_sign( ps_bitstream );
				y262dec_bitstream_drop( ps_bitstream, 1 );

				i_scan_idx = pui8_scan[ i_idx ];
				i_level = ( i_level * pui16_quant_matrix[ i_scan_idx ] ) >> 5;
				i_level = ( i_level ^ i_sign ) - i_sign;

				if( i_level > 2047 )
				{
					i_level = 2047;
				}
				else if( i_level < -2048 )
				{
					i_level = -2048;
				}

				i_sum += i_level;
				pi16_block[ i_scan_idx ] = i_level;

				continue;
			}
			else
			{
				if( ps_run_level->i8_run == 191 )
				{
					break;
				}
				y262dec_bitstream_drop( ps_bitstream, 6 );
				i_idx += y262dec_bitstream_read_small( ps_bitstream, 6 ) - 64;
				if( i_idx > 63 )
				{
					break;
				}
				i_scan_idx = pui8_scan[ i_idx ];
				i_level = ( ( int32_t ) ps_bitstream->ui_bits ) >> ( 32 - 12 );
				i_level = ( i_level * pui16_quant_matrix[ i_scan_idx ] ) / 32;
				y262dec_bitstream_drop( ps_bitstream, 12 );
				if( i_level > 2047 )
				{
					i_level = 2047;
				}
				else if( i_level < -2048 )
				{
					i_level = -2048;
				}
				i_sum += i_level;
				pi16_block[ i_scan_idx ] = i_level;
				continue;
			}
		}
		else if( ui_codeword >= 0x02000000 )
		{
			ps_run_level = &rgs_y262dec_dct_one_table_10[ y262dec_bitstream_peek_small( ps_bitstream, 10 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_intra_vlc1_coeff;
			}
		}
		else if( ui_codeword >= 0x00800000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_13[ y262dec_bitstream_peek_small( ps_bitstream, 13 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_intra_vlc1_coeff;
			}
		}
		else if( ui_codeword >= 0x00200000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_15[ y262dec_bitstream_peek_small( ps_bitstream, 15 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_intra_vlc1_coeff;
			}
		}
		else if( ui_codeword >= 0x00100000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_16[ y262dec_bitstream_peek_small( ps_bitstream, 16 ) ];
			y262dec_bitstream_drop( ps_bitstream, 16 );
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_intra_vlc1_coeff;
			}
		}
		break;
	}

	pi16_block[ 63 ] ^= ( i_sum & 1 );
	y262dec_bitstream_drop( ps_bitstream, 4 );
	return i_idx;
}


int32_t y262dec_parse_macroblock_block_intra( y262dec_slicedec_t *ps_slicedec, int32_t i_plane_idx )
{
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	y262dec_predictors_t *ps_predictors = &ps_slicedec->s_state.s_predictors;
	y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;
	y262dec_bitstream_t *ps_bitstream = &ps_slicedec->s_bitstream;
	int16_t *pi16_block = ps_slicedec->s_state.pi16_block;
	uint16_t *pui16_quant_matrix;

	memset( pi16_block, 0, sizeof( int16_t ) * 64 );

	if( i_plane_idx == 0 )
	{
		pui16_quant_matrix = ps_macroblock->rgpui16_quantizer_matrices[ 0 ];
		ps_predictors->rgi16_dc_dct_pred[ i_plane_idx ] += y262dec_macroblock_process_dct_luma_dc( ps_bitstream, ps_dec->s_dec_state.ui_intra_dc_shift );
	}
	else
	{
		pui16_quant_matrix = ps_macroblock->rgpui16_quantizer_matrices[ 2 ];
		ps_predictors->rgi16_dc_dct_pred[ i_plane_idx ] += y262dec_macroblock_process_dct_chroma_dc( ps_bitstream, ps_dec->s_dec_state.ui_intra_dc_shift );
	}
	pi16_block[ 0 ] = ps_predictors->rgi16_dc_dct_pred[ i_plane_idx ];

	if( ps_picture_coding_extension->b_intra_vlc_format )
	{
		return y262dec_parse_macroblock_block_intra_vlc1( ps_slicedec, pui16_quant_matrix );
	}
	else
	{
		if( ps_dec->s_dec_state.b_mpeg_2 )
		{
			return y262dec_parse_macroblock_block_intra_mpeg2_vlc0( ps_slicedec, pui16_quant_matrix );
		}
		else
		{
			return y262dec_parse_macroblock_block_intra_mpeg1_vlc0( ps_slicedec, pui16_quant_matrix );
		}
		
	}
}



bool y262dec_macroblock_process_blocks_intra( y262dec_slicedec_t *ps_slicedec )
{
	int32_t i_idx;
	uint8_t *pui8_destination, *pui8_destination_cb, *pui8_destination_cr;
	int16_t *pi16_block = ps_slicedec->s_state.pi16_block;
	uint32_t ui_block_y_stride, ui_stride, ui_chroma_block_y_stride, ui_stride_chroma;
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;

	pui8_destination = ps_macroblock->pui8_destination_luma;
	if( ps_macroblock->ui_macroblock_type & Y262DEC_MACROBLOCK_INTERLACED )
	{
		ui_block_y_stride = ps_state->ui_luma_block_y_stride_dct1;
		ui_stride = ps_state->ui_luma_stride_dct1;
	}
	else
	{
		ui_block_y_stride = ps_state->ui_luma_block_y_stride_dct0;
		ui_stride = ps_state->ui_luma_stride_dct0;
	}
	
	i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 0 );
	ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination, ui_stride );
	i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 0 );
	ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination + 8, ui_stride );
	pui8_destination += ui_block_y_stride;
	i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 0 );
	ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination, ui_stride );
	i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 0 );
	ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination + 8, ui_stride );

	if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_420 )
	{
		pui8_destination_cb = ps_macroblock->pui8_destination_chroma_cb;
		pui8_destination_cr = ps_macroblock->pui8_destination_chroma_cr;
		ui_stride_chroma = ps_state->ui_chroma_stride_dct0;
		i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 1 );
		ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination_cb, ui_stride_chroma );
		i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 2 );
		ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination_cr, ui_stride_chroma );
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_422 )
	{
		pui8_destination_cb = ps_macroblock->pui8_destination_chroma_cb;
		pui8_destination_cr = ps_macroblock->pui8_destination_chroma_cr;
		ui_stride_chroma = ui_stride >> 1;
		ui_chroma_block_y_stride = ui_block_y_stride >> 1;
		i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 1 );
		ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination_cb, ui_stride_chroma );
		i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 2 );
		ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination_cr, ui_stride_chroma );
		pui8_destination_cb += ui_chroma_block_y_stride;
		pui8_destination_cr += ui_chroma_block_y_stride;
		i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 1 );
		ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination_cb, ui_stride_chroma );
		i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 2 );
		ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination_cr, ui_stride_chroma );
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_444 )
	{
		pui8_destination_cb = ps_macroblock->pui8_destination_chroma_cb;
		pui8_destination_cr = ps_macroblock->pui8_destination_chroma_cr;
		ui_stride_chroma = ui_stride;
		ui_chroma_block_y_stride = ui_block_y_stride;
		i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 1 );
		ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination_cb, ui_stride_chroma );
		i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 2 );
		ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination_cr, ui_stride_chroma );
		i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 1 );
		ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination_cb + ui_chroma_block_y_stride, ui_stride_chroma );
		i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 2 );
		ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination_cr + ui_chroma_block_y_stride, ui_stride_chroma );
		i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 1 );
		ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination_cb + 8, ui_stride_chroma );
		i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 2 );
		ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination_cr + 8, ui_stride_chroma );
		i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 1 );
		ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination_cb + 8 + ui_chroma_block_y_stride, ui_stride_chroma );
		i_idx = y262dec_parse_macroblock_block_intra( ps_slicedec, 2 );
		ps_dec->s_functions.pf_idct_put( pi16_block, i_idx, pui8_destination_cr + 8 + ui_chroma_block_y_stride, ui_stride_chroma );
	}

	return y262dec_bitstream_not_past_end( &ps_slicedec->s_bitstream );
}


int32_t y262dec_parse_macroblock_block_inter_mpeg1( y262dec_slicedec_t *ps_slicedec, uint16_t *pui16_quant_matrix )
{
	int32_t i_idx, i_level, i_sign, i_scan_idx;
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	const uint8_t *pui8_scan = ps_dec->s_dec_state.pui8_current_scan;
	int16_t *pi16_block = ps_slicedec->s_state.pi16_block;
	y262dec_bitstream_t *ps_bitstream = &ps_slicedec->s_bitstream;
	const y262dec_run_level_t *ps_run_level;

	memset( pi16_block, 0, sizeof( int16_t ) * 64 );

	i_idx = -1;

	if( ps_bitstream->ui_bits >= 0x28000000 )
	{
		ps_run_level = &rgs_y262dec_dct_zero_table_5b[ y262dec_bitstream_peek_small( ps_bitstream, 5 ) ];
		goto dct_zero_table_5b_match;
	}
	goto dct_zero_table_5b_miss;

	while( 1 )
	{
		if( ps_bitstream->ui_bits >= 0x28000000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_5[ y262dec_bitstream_peek_small( ps_bitstream, 5 ) ];

dct_zero_table_5b_match:
			i_idx += ps_run_level->i8_run;
			if( i_idx >= 64 )
			{
				break;
			}

parse_macroblock_block_inter_coeff:
			y262dec_bitstream_drop( ps_bitstream, ps_run_level->i8_length );
			i_scan_idx = pui8_scan[ i_idx ];
			i_level = ( ( ( ps_run_level->i8_level * 2 ) + 1 ) * pui16_quant_matrix[ i_scan_idx ] ) >> 5;
			i_sign = y262dec_bitstream_peek_small( ps_bitstream, 1 );
			y262dec_bitstream_drop( ps_bitstream, 1 );
			if( i_sign )
			{
				i_level = -( ( i_level - 1 ) | 1 );
			}
			else
			{
				i_level = ( i_level - 1 ) | 1;
			}
			if( i_level > 2047 )
			{
				i_level = 2047;
			}
			else if( i_level < -2048 )
			{
				i_level = -2048;
			}
			pi16_block[ i_scan_idx ] = i_level;
			continue;
		}

dct_zero_table_5b_miss:
		if( ps_bitstream->ui_bits >= 0x4000000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_8[ y262dec_bitstream_peek_small( ps_bitstream, 8 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_inter_coeff;
			}
			y262dec_bitstream_drop( ps_bitstream, 6 );
			i_idx += y262dec_bitstream_read_small( ps_bitstream, 6 ) - 64;
			if( i_idx >= 64 )
			{
				break;
			}
			i_scan_idx = pui8_scan[ i_idx ];
			i_level = ( ( ( int32_t ) y262dec_bitstream_read_small( ps_bitstream, 8 ) ) << 24 ) >> 24;
			if( i_level == -128 )
			{
				i_level = -256 + ( ( int32_t ) y262dec_bitstream_read_small( ps_bitstream, 8 ) );
			}
			else if( i_level == 0 )
			{
				i_level = y262dec_bitstream_read_small( ps_bitstream, 8 );
			}
			if( i_level < 0 )
			{
				i_level = -i_level;
				i_level = ( ( ( i_level * 2 ) + 1 ) * pui16_quant_matrix[ i_scan_idx ] ) >> 5;
				i_level = ( i_level - 1 ) | 1;
				i_level = -i_level;
			}
			else
			{
				i_level = ( ( ( i_level * 2 ) + 1 ) * pui16_quant_matrix[ i_scan_idx ] ) >> 5;
				i_level = ( i_level - 1 ) | 1;
			}
			if( i_level > 2047 )
			{
				i_level = 2047;
			}
			else if( i_level < -2048 )
			{
				i_level = -2048;
			}
			pi16_block[ i_scan_idx ] = i_level;
			continue;
		}
		else if( ps_bitstream->ui_bits >= 0x2000000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_10[ y262dec_bitstream_peek_small( ps_bitstream, 10 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_inter_coeff;
			}
		}
		else if( ps_bitstream->ui_bits >= 0x00800000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_13[ y262dec_bitstream_peek_small( ps_bitstream, 13 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_inter_coeff;
			}
		}
		else if( ps_bitstream->ui_bits >= 0x00200000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_15[ y262dec_bitstream_peek_small( ps_bitstream, 15 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_inter_coeff;
			}
		}
		else if( ps_bitstream->ui_bits >= 0x00100000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_16[ y262dec_bitstream_peek_small( ps_bitstream, 16 ) ];
			y262dec_bitstream_drop( ps_bitstream, 16 );
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_inter_coeff;
			}
		}
		break;
	}

	y262dec_bitstream_drop( ps_bitstream, 2 );
	return i_idx;
}


int32_t y262dec_parse_macroblock_block_inter_mpeg2( y262dec_slicedec_t *ps_slicedec, uint16_t *pui16_quant_matrix )
{
	int32_t i_idx, i_level, i_sign, i_scan_idx, i_sum;
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	const uint8_t *pui8_scan = ps_dec->s_dec_state.pui8_current_scan;
	int16_t *pi16_block = ps_slicedec->s_state.pi16_block;
	y262dec_bitstream_t *ps_bitstream = &ps_slicedec->s_bitstream;
	const y262dec_run_level_t *ps_run_level;

	memset( pi16_block, 0, sizeof( int16_t ) * 64 );

	i_idx = -1;
	i_sum = -1;

	if( ps_bitstream->ui_bits >= 0x28000000 )
	{
		ps_run_level = &rgs_y262dec_dct_zero_table_5b[ y262dec_bitstream_peek_small( ps_bitstream, 5 ) ];
		goto dct_zero_table_5b_match;
	}
	goto dct_zero_table_5b_miss;

	while( 1 )
	{
		if( ps_bitstream->ui_bits >= 0x28000000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_5[ y262dec_bitstream_peek_small( ps_bitstream, 5 ) ];

dct_zero_table_5b_match:
			i_idx += ps_run_level->i8_run;
			if( i_idx >= 64 )
			{
				break;
			}

parse_macroblock_block_inter_coeff:
			y262dec_bitstream_drop( ps_bitstream, ps_run_level->i8_length );
			i_scan_idx = pui8_scan[ i_idx ];
			i_level = ( ( ( ps_run_level->i8_level * 2 ) + 1 ) * pui16_quant_matrix[ i_scan_idx ] ) >> 5;
			i_sign = y262dec_bitstream_peek_sign( ps_bitstream );
			i_level = ( i_level ^ i_sign ) - i_sign;
			y262dec_bitstream_drop( ps_bitstream, 1 );
			if( i_level > 2047 )
			{
				i_level = 2047;
			}
			else if( i_level < -2048 )
			{
				i_level = -2048;
			}
			pi16_block[ i_scan_idx ] = i_level;
			i_sum ^= i_level;
			continue;
		}

dct_zero_table_5b_miss:
		if( ps_bitstream->ui_bits >= 0x4000000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_8[ y262dec_bitstream_peek_small( ps_bitstream, 8 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_inter_coeff;
			}
			y262dec_bitstream_drop( ps_bitstream, 6 );
			i_idx += y262dec_bitstream_read_small( ps_bitstream, 6 ) - 64;
			if( i_idx >= 64 )
			{
				break;
			}
			i_scan_idx = pui8_scan[ i_idx ];
			i_level = 2 * ( ( ( ( int32_t ) ps_bitstream->ui_bits ) >> ( 32 - 12 ) ) + ( ( ( int32_t ) ps_bitstream->ui_bits ) >> 31 ) ) + 1;
			i_level = ( i_level * pui16_quant_matrix[ i_scan_idx ] ) / 32;
			if( i_level > 2047 )
			{
				i_level = 2047;
			}
			else if( i_level < -2048 )
			{
				i_level = -2048;
			}
			pi16_block[ i_scan_idx ] = i_level;
			i_sum ^= i_level;
			y262dec_bitstream_drop( ps_bitstream, 12 );
			continue;
		}
		else if( ps_bitstream->ui_bits >= 0x2000000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_10[ y262dec_bitstream_peek_small( ps_bitstream, 10 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_inter_coeff;
			}
		}
		else if( ps_bitstream->ui_bits >= 0x00800000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_13[ y262dec_bitstream_peek_small( ps_bitstream, 13 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_inter_coeff;
			}
		}
		else if( ps_bitstream->ui_bits >= 0x00200000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_15[ y262dec_bitstream_peek_small( ps_bitstream, 15 ) ];
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_inter_coeff;
			}
		}
		else if( ps_bitstream->ui_bits >= 0x00100000 )
		{
			ps_run_level = &rgs_y262dec_dct_zero_table_16[ y262dec_bitstream_peek_small( ps_bitstream, 16 ) ];
			y262dec_bitstream_drop( ps_bitstream, 16 );
			i_idx += ps_run_level->i8_run;
			if( i_idx < 64 )
			{
				goto parse_macroblock_block_inter_coeff;
			}
		}
		break;
	}

	pi16_block[ 63 ] ^= i_sum & 1;
	y262dec_bitstream_drop( ps_bitstream, 2 );
	return i_idx;
}


int32_t y262dec_parse_macroblock_block_inter( y262dec_slicedec_t *ps_slicedec, uint16_t *pui16_quant_matrix )
{
	y262dec_t *ps_dec = ps_slicedec->ps_dec;

	if( ps_dec->s_dec_state.b_mpeg_2 )
	{
		return y262dec_parse_macroblock_block_inter_mpeg2( ps_slicedec, pui16_quant_matrix );
	}
	else
	{
		return y262dec_parse_macroblock_block_inter_mpeg1( ps_slicedec, pui16_quant_matrix );
	}
}



bool y262dec_macroblock_process_blocks_inter( y262dec_slicedec_t *ps_slicedec )
{
	int32_t i_idx;
	uint8_t *pui8_destination, *pui8_destination_cb, *pui8_destination_cr;
	uint16_t *pui16_quant_matrix;
	uint32_t ui_cbp, ui_stride, ui_block_y_stride, ui_chroma_block_y_stride, ui_stride_chroma;
	int16_t *pi16_block = ps_slicedec->s_state.pi16_block;
	y262dec_t *ps_dec = ps_slicedec->ps_dec;
	y262dec_bitstream_t *ps_bitstream = &ps_slicedec->s_bitstream;
	y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;
	const y262dec_cbp_t *ps_cbp;

	if( ps_bitstream->ui_bits >= 0x20000000 )
	{
		ps_cbp = &rgs_y262dec_cbp_7[ y262dec_bitstream_peek_small( ps_bitstream, 7 ) - 16 ];
	}
	else
	{
		ps_cbp = &rgs_y262dec_cbp_9[ y262dec_bitstream_peek_small( ps_bitstream, 9 ) ];
		if( ps_cbp->i8_len == -1 )
		{
			return false;
		}
	}
	ui_cbp = ps_cbp->i8_cbp;
	y262dec_bitstream_drop( ps_bitstream, ps_cbp->i8_len );

	if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_420 )
	{
		pui16_quant_matrix = ps_macroblock->rgpui16_quantizer_matrices[ 1 ];
		pui8_destination = ps_macroblock->pui8_destination_luma;
		if( ps_macroblock->ui_macroblock_type & Y262DEC_MACROBLOCK_INTERLACED )
		{
			ui_stride = ps_state->ui_luma_stride_dct1;
			ui_block_y_stride = ps_state->ui_luma_block_y_stride_dct1;
		}
		else
		{
			ui_stride = ps_state->ui_luma_stride_dct0;
			ui_block_y_stride = ps_state->ui_luma_block_y_stride_dct0;
		}
		if( ui_cbp & ( 1 << 5 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination, ui_stride );
		}
		if( ui_cbp & ( 1 << 4 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination + 8, ui_stride );
		}
		pui8_destination += ui_block_y_stride;
		if( ui_cbp & ( 1 << 3 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination, ui_stride );
		}
		if( ui_cbp & ( 1 << 2 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination + 8, ui_stride );
		}

		pui8_destination_cb = ps_macroblock->pui8_destination_chroma_cb;
		pui8_destination_cr = ps_macroblock->pui8_destination_chroma_cr;
		ui_stride_chroma = ps_state->ui_chroma_stride_dct0;
		if( ui_cbp & ( 1 << 1 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination_cb, ui_stride_chroma );
		}
		if( ui_cbp & ( 1 << 0 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination_cr, ui_stride_chroma );
		}
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_422 )
	{
		pui16_quant_matrix = ps_macroblock->rgpui16_quantizer_matrices[ 1 ];
		pui8_destination = ps_macroblock->pui8_destination_luma;
		if( ps_macroblock->ui_macroblock_type & Y262DEC_MACROBLOCK_INTERLACED )
		{
			ui_stride = ps_state->ui_luma_stride_dct1;
			ui_stride_chroma = ps_state->ui_chroma_stride_dct1;
			ui_block_y_stride = ps_state->ui_luma_block_y_stride_dct1;
			ui_chroma_block_y_stride = ps_state->ui_chroma_block_y_stride_dct1;
		}
		else
		{
			ui_stride = ps_state->ui_luma_stride_dct0;
			ui_stride_chroma = ps_state->ui_chroma_stride_dct0;
			ui_block_y_stride = ps_state->ui_luma_block_y_stride_dct0;
			ui_chroma_block_y_stride = ps_state->ui_chroma_block_y_stride_dct0;
		}
		ui_cbp = ( ui_cbp << 2 ) + y262dec_bitstream_read_small( ps_bitstream, 2 );
		if( ui_cbp & ( 1 << 7 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination, ui_stride );
		}
		if( ui_cbp & ( 1 << 6 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination + 8, ui_stride );
		}
		pui8_destination += ui_block_y_stride;
		if( ui_cbp & ( 1 << 5 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination, ui_stride );
		}
		if( ui_cbp & ( 1 << 4 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination + 8, ui_stride );
		}

		pui16_quant_matrix = ps_macroblock->rgpui16_quantizer_matrices[ 3 ];
		pui8_destination_cb = ps_macroblock->pui8_destination_chroma_cb;
		pui8_destination_cr = ps_macroblock->pui8_destination_chroma_cr;
		if( ui_cbp & ( 1 << 3 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination_cb, ui_stride_chroma );
		}
		if( ui_cbp & ( 1 << 2 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination_cr, ui_stride_chroma );
		}
		pui8_destination_cb += ui_chroma_block_y_stride;
		pui8_destination_cr += ui_chroma_block_y_stride;
		if( ui_cbp & ( 1 << 1 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination_cb, ui_stride_chroma );
		}
		if( ui_cbp & ( 1 << 0 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination_cr, ui_stride_chroma );
		}
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_444 )
	{
		ui_cbp = ( ui_cbp << 6 ) + y262dec_bitstream_read_small( ps_bitstream, 6 );
		if( ps_macroblock->ui_macroblock_type & Y262DEC_MACROBLOCK_INTERLACED )
		{
			ui_stride = ps_state->ui_luma_stride_dct1;
			ui_stride_chroma = ps_state->ui_chroma_stride_dct1;
			ui_block_y_stride = ps_state->ui_luma_block_y_stride_dct1;
			ui_chroma_block_y_stride = ps_state->ui_chroma_block_y_stride_dct1;
		}
		else
		{
			ui_stride = ps_state->ui_luma_stride_dct0;
			ui_stride_chroma = ps_state->ui_chroma_stride_dct0;
			ui_block_y_stride = ps_state->ui_luma_block_y_stride_dct0;
			ui_chroma_block_y_stride = ps_state->ui_chroma_block_y_stride_dct0;
		}
		pui8_destination = ps_macroblock->pui8_destination_luma;
		pui16_quant_matrix = ps_macroblock->rgpui16_quantizer_matrices[ 1 ];
		if( ui_cbp & ( 1 << 11 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination, ui_stride );
		}
		if( ui_cbp & ( 1 << 10 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination + 8, ui_stride );
		}
		pui8_destination += ui_block_y_stride;
		if( ui_cbp & ( 1 << 9 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination, ui_stride );
		}
		if( ui_cbp & ( 1 << 8 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination + 8, ui_stride );
		}

		pui16_quant_matrix = ps_macroblock->rgpui16_quantizer_matrices[ 3 ];
		pui8_destination_cb = ps_macroblock->pui8_destination_chroma_cb;
		pui8_destination_cr = ps_macroblock->pui8_destination_chroma_cr;
		if( ui_cbp & ( 1 << 7 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination_cb, ui_stride_chroma );
		}
		if( ui_cbp & ( 1 << 6 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination_cr, ui_stride_chroma );
		}
		if( ui_cbp & ( 1 << 5 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination_cb + ui_chroma_block_y_stride, ui_stride_chroma );
		}
		if( ui_cbp & ( 1 << 4 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination_cr + ui_chroma_block_y_stride, ui_stride_chroma );
		}
		if( ui_cbp & ( 1 << 3 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination_cb + 8, ui_stride_chroma );
		}
		if( ui_cbp & ( 1 << 2 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination_cr + 8, ui_stride_chroma );
		}
		if( ui_cbp & ( 1 << 1 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination_cb + 8 + ui_chroma_block_y_stride, ui_stride_chroma );
		}
		if( ui_cbp & ( 1 << 0 ) )
		{
			i_idx = y262dec_parse_macroblock_block_inter( ps_slicedec, pui16_quant_matrix );
			ps_dec->s_functions.pf_idct_add( pi16_block, i_idx, pui8_destination_cr + 8 + ui_chroma_block_y_stride, ui_stride_chroma );
		}
	}

	return y262dec_bitstream_not_past_end( ps_bitstream );
}


void y262dec_motion_compensate_clamp_vector( int32_t *pel_x, int32_t *pel_y, int32_t *mv_x, int32_t *mv_y, const y262dec_macroblock_t *ps_macroblock, int32_t i_y_scale, int32_t i_mc_block, int32_t i_offset, int32_t i_pic_width, int32_t i_pic_height )
{
	const int32_t rgi_mc_block_height[ 5 ] = { 16, 8, 16, 8, 4 };
	int32_t i_limit, i_mb_x, i_mb_y;

	i_mb_x = ps_macroblock->i_macroblock_pel_x * 2;
	i_mb_y = ( ps_macroblock->i_macroblock_pel_y + i_offset ) * i_y_scale;
	i_limit = ( i_pic_width - 16 ) * 2;
	*pel_x = i_mb_x + *mv_x;
	if( ( ( uint32_t ) *pel_x ) > ( uint32_t ) i_limit )
	{
		if( ( *pel_x ) < 0 )
		{
			*mv_x = ps_macroblock->i_macroblock_pel_x * -2;
			*pel_x = 0;
		}
		else
		{
			*mv_x = i_limit - ( ps_macroblock->i_macroblock_pel_x * 2 );
			*pel_x = i_limit;
		}
	}
	i_limit = ( i_pic_height - rgi_mc_block_height[ i_mc_block ] ) * 2;
	*pel_y = i_mb_y + *mv_y;
	if( ( ( uint32_t ) *pel_y ) > ( uint32_t ) i_limit )
	{
		if( *pel_y < 0 )
		{
			*mv_y = -i_mb_y;
			*pel_y = 0;
		}
		else
		{
			*mv_y = i_limit - i_mb_y;
			*pel_y = i_limit;
		}
	}
}


#define Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( mv_x, mv_y, i_src_stride, i_src_offset )				\
do {																	\
	i_subpel_idx = ( mv_x & 1 ) + ( ( mv_y & 1 ) << 1 );						\
	i_src_offset = ( mv_x >> 1 ) + ( ( mv_y >> 1 ) * i_src_stride );	\
} while( 0 );


void y262dec_motion_compensate_macroblock_frame_null( y262dec_slicedec_t *ps_slicedec, int32_t i_avg, int32_t i_direction )
{
	int32_t i_stride, i_stride_chroma, i_dst_stride, i_dst_stride_chroma, i_chroma_mc_block, i_src_offset;
	uint8_t *pui8_src, *pui8_dst;
	const y262dec_t *ps_dec = ps_slicedec->ps_dec;
	const y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	const y262dec_frame_picture_t *ps_destination_frame = &ps_state->s_current_frame;
	const y262dec_frame_picture_t *ps_source_frame = ps_state->rgps_reference_frames[ Y262DEC_REFERENCE_PFORWARD ];;
	const y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;
	const y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	const y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;

	i_stride = ps_source_frame->i_stride;
	i_src_offset = ps_macroblock->i_macroblock_pel_x + ps_macroblock->i_macroblock_pel_y * i_stride;
	i_dst_stride = ps_destination_frame->i_stride;

	pui8_src = ps_source_frame->pui8_luma;
	pui8_dst = ps_macroblock->pui8_destination_luma;
	ps_dec->s_functions.rgf_motcomp_copy[ Y262DEC_MC_BLOCK_16x16 ][ 0 ]( pui8_src + i_src_offset, i_stride, pui8_dst, i_dst_stride );

	if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_420 )
	{
		i_chroma_mc_block = Y262DEC_MC_BLOCK_8x8;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_422 )
	{
		i_chroma_mc_block = Y262DEC_MC_BLOCK_8x16;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_444 )
	{
		i_chroma_mc_block = Y262DEC_MC_BLOCK_16x16;
	}

	i_stride_chroma = ps_source_frame->i_stride_chroma;
	i_dst_stride_chroma = ps_destination_frame->i_stride_chroma;
	i_src_offset = ps_macroblock->i_macroblock_chroma_pel_x + ps_macroblock->i_macroblock_chroma_pel_y * i_stride_chroma;

	pui8_src = ps_source_frame->pui8_cb;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cb;
	ps_dec->s_functions.rgf_motcomp_copy[ i_chroma_mc_block ][ 0 ]( pui8_src + i_src_offset, i_stride_chroma, pui8_dst, i_dst_stride_chroma );

	pui8_src = ps_source_frame->pui8_cr;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cr;
	ps_dec->s_functions.rgf_motcomp_copy[ i_chroma_mc_block ][ 0 ]( pui8_src + i_src_offset, i_stride_chroma, pui8_dst, i_dst_stride_chroma );
}



void y262dec_motion_compensate_macroblock_frame_frame( y262dec_slicedec_t *ps_slicedec, int32_t i_avg, int32_t i_direction )
{
	int32_t i_stride, i_stride_chroma, i_dst_stride, i_dst_stride_chroma, i_chroma_mc_block, i_subpel_idx, i_src_offset;
	int32_t i_pel_x, i_pel_y, i_mv_x, i_mv_y;
	uint8_t *pui8_src, *pui8_dst;
	const y262dec_t *ps_dec = ps_slicedec->ps_dec;
	const y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	const y262dec_frame_picture_t *ps_destination_frame = &ps_state->s_current_frame;
	const y262dec_frame_picture_t *ps_source_frame = ps_state->rgps_reference_frames[ i_direction ];
	const y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;
	const y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	const y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;
	const y262dec_motcomp_f( *pf_mc )[ 4 ];


	pf_mc = i_avg ? ps_dec->s_functions.rgf_motcomp_avg : ps_dec->s_functions.rgf_motcomp_copy;

	i_mv_x = ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ i_direction ][ 0 ];
	i_mv_y = ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ i_direction ][ 1 ];
	y262dec_motion_compensate_clamp_vector( &i_pel_x, &i_pel_y, &i_mv_x, &i_mv_y, ps_macroblock, 2, Y262DEC_MC_BLOCK_16x16, 0, ps_source_frame->i_width, ps_source_frame->i_height );

	i_stride = ps_source_frame->i_stride;
	i_dst_stride = ps_destination_frame->i_stride;
	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( i_pel_x, i_pel_y, i_stride, i_src_offset );

	pui8_src = ps_source_frame->pui8_luma;
	pui8_dst = ps_macroblock->pui8_destination_luma;
	pf_mc[ Y262DEC_MC_BLOCK_16x16 ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, i_dst_stride );

	if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_420 )
	{
		i_mv_x = i_mv_x / 2;
		i_mv_y = i_mv_y / 2;
		i_chroma_mc_block = Y262DEC_MC_BLOCK_8x8;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_422 )
	{
		i_mv_x = i_mv_x / 2;
		i_chroma_mc_block = Y262DEC_MC_BLOCK_8x16;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_444 )
	{
		i_chroma_mc_block = Y262DEC_MC_BLOCK_16x16;
	}

	i_pel_x = i_mv_x + ps_macroblock->i_macroblock_chroma_pel_x * 2;
	i_pel_y = i_mv_y + ps_macroblock->i_macroblock_chroma_pel_y * 2;

	i_stride_chroma = ps_source_frame->i_stride_chroma;
	i_dst_stride_chroma = ps_destination_frame->i_stride_chroma;
	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( i_pel_x, i_pel_y, i_stride_chroma, i_src_offset );

	pui8_src = ps_source_frame->pui8_cb;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cb;
	pf_mc[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride_chroma, pui8_dst, i_dst_stride_chroma );

	pui8_src = ps_source_frame->pui8_cr;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cr;
	pf_mc[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride_chroma, pui8_dst, i_dst_stride_chroma );
}


void y262dec_motion_compensate_macroblock_frame_field( y262dec_slicedec_t *ps_slicedec, int32_t i_avg, int32_t i_direction )
{
	int32_t i_stride, i_stride_chroma, i_dst_stride, i_dst_stride_chroma, i_chroma_mc_block, i_subpel_idx, i_src_offset, i_field, i_idx;
	int32_t i_motion_vector, i_pel_x, i_pel_y, i_mv_x, i_mv_y;
	uint8_t *pui8_src, *pui8_dst;
	const y262dec_t *ps_dec = ps_slicedec->ps_dec;
	const y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	const y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	const y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;
	const y262dec_frame_picture_t *ps_destination_frame = &ps_state->s_current_frame;
	const y262dec_field_picture_t *ps_src_field;
	const y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;
	const y262dec_motcomp_f( *pf_mc )[ 4 ];
	const int32_t rgi_motion_vectors[ 2 ] = { Y262DEC_MOTION_VECTOR_FIRST, Y262DEC_MOTION_VECTOR_SECOND };

	pf_mc = i_avg ? ps_dec->s_functions.rgf_motcomp_avg : ps_dec->s_functions.rgf_motcomp_copy;

	for( i_idx = 0; i_idx < 2; i_idx++ )
	{
		i_motion_vector = rgi_motion_vectors[ i_idx ];

		i_field = ps_macroblock->rgi_motion_vertical_field_select[ i_motion_vector ][ i_direction ];
		ps_src_field = ps_state->rgps_reference_fields[ i_direction ][ i_field ];

		i_mv_x = ps_macroblock->rgi16_motion_vector[ i_motion_vector ][ i_direction ][ 0 ];
		i_mv_y = ps_macroblock->rgi16_motion_vector[ i_motion_vector ][ i_direction ][ 1 ];
		y262dec_motion_compensate_clamp_vector( &i_pel_x, &i_pel_y, &i_mv_x, &i_mv_y, ps_macroblock, 1, Y262DEC_MC_BLOCK_16x8, 0, ps_src_field->i_width, ps_src_field->i_height );

		i_stride = ps_src_field->i_stride;
		i_dst_stride = ps_destination_frame->i_stride * 2;
		Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( i_pel_x, i_pel_y, i_stride, i_src_offset );

		pui8_src = ps_src_field->pui8_luma;
		pui8_dst = ps_macroblock->pui8_destination_luma;
		pui8_dst += ps_destination_frame->i_stride * i_idx;
		pf_mc[ Y262DEC_MC_BLOCK_16x8 ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, i_dst_stride );

		if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_420 )
		{
			i_mv_x = i_mv_x / 2;
			i_mv_y = i_mv_y / 2;
			i_chroma_mc_block = Y262DEC_MC_BLOCK_8x4;
		}
		else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_422 )
		{
			i_mv_x = i_mv_x / 2;
			i_chroma_mc_block = Y262DEC_MC_BLOCK_8x8;
		}
		else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_444 )
		{
			i_chroma_mc_block = Y262DEC_MC_BLOCK_16x8;
		}

		i_stride_chroma = ps_state->rgps_reference_fields[ i_direction ][ i_field ]->i_stride_chroma;
		i_dst_stride_chroma = ps_destination_frame->i_stride_chroma * 2;
		i_pel_x = i_mv_x + ps_macroblock->i_macroblock_chroma_pel_x * 2;
		i_pel_y = i_mv_y + ps_macroblock->i_macroblock_chroma_pel_y;
		Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( i_pel_x, i_pel_y, i_stride_chroma, i_src_offset );

		pui8_src = ps_state->rgps_reference_fields[ i_direction ][ i_field ]->pui8_cb;
		pui8_dst = ps_macroblock->pui8_destination_chroma_cb;
		pui8_dst += ps_destination_frame->i_stride_chroma * i_idx;
		pf_mc[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride_chroma, pui8_dst, i_dst_stride_chroma );

		pui8_src = ps_state->rgps_reference_fields[ i_direction ][ i_field ]->pui8_cr;
		pui8_dst = ps_macroblock->pui8_destination_chroma_cr;
		pui8_dst += ps_destination_frame->i_stride_chroma * i_idx;
		pf_mc[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride_chroma, pui8_dst, i_dst_stride_chroma );
	}
}


void y262dec_motion_compensate_macroblock_frame_dual_prime( y262dec_slicedec_t *ps_slicedec, int32_t i_avg, int32_t i_unused )
{
	int32_t i_idx, i_motion_vector, i_stride, i_chroma_mc_block, i_subpel_idx, i_src_offset;
	int32_t i_pel_x, i_pel_y, rgi_pel_x[ 3 ], rgi_pel_y[ 3 ], rgi_mv_x[ 3 ], rgi_mv_y[ 3 ];
	uint8_t *pui8_src, *pui8_dst;
	const y262dec_t *ps_dec = ps_slicedec->ps_dec;
	const y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	const y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;
	const y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	const y262dec_frame_picture_t *ps_destination_frame = &ps_state->s_current_frame;
	const y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;
	const int32_t rgi_motion_vectors[ 3 ] = { Y262DEC_MOTION_VECTOR_FIRST, Y262DEC_MOTION_VECTOR_THIRD, Y262DEC_MOTION_VECTOR_FOURTH };

	for( i_idx = 0; i_idx < 3; i_idx++ )
	{
		i_motion_vector = rgi_motion_vectors[ i_idx ];
		rgi_mv_x[ i_idx ] = ps_macroblock->rgi16_motion_vector[ i_motion_vector ][ Y262DEC_MOTION_FORWARD ][ 0 ];
		rgi_mv_y[ i_idx ] = ps_macroblock->rgi16_motion_vector[ i_motion_vector ][ Y262DEC_MOTION_FORWARD ][ 1 ];
		y262dec_motion_compensate_clamp_vector( &rgi_pel_x[ i_idx ], &rgi_pel_y[ i_idx ], &rgi_mv_x[ i_idx ], &rgi_mv_y[ i_idx ], ps_macroblock, 1, Y262DEC_MC_BLOCK_16x8, 0,
			ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_TOP ]->i_width, ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_TOP ]->i_height );
	}

	i_stride = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_TOP ]->i_stride;
	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( rgi_pel_x[ 0 ], rgi_pel_y[ 0 ], i_stride, i_src_offset );

	pui8_src = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_TOP ]->pui8_luma;
	pui8_dst = ps_macroblock->pui8_destination_luma;
	ps_dec->s_functions.rgf_motcomp_copy[ Y262DEC_MC_BLOCK_16x8 ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_destination_frame->i_stride * 2 );

	i_stride = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_BOTTOM ]->i_stride;
	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( rgi_pel_x[ 1 ], rgi_pel_y[ 1 ], i_stride, i_src_offset );

	pui8_src = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_BOTTOM ]->pui8_luma;
	ps_dec->s_functions.rgf_motcomp_avg[ Y262DEC_MC_BLOCK_16x8 ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_destination_frame->i_stride * 2 );

	i_stride = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_BOTTOM ]->i_stride;
	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( rgi_pel_x[ 0 ], rgi_pel_y[ 0 ], i_stride, i_src_offset );

	pui8_src = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_BOTTOM ]->pui8_luma;
	pui8_dst = ps_macroblock->pui8_destination_luma + ps_destination_frame->i_stride;
	ps_dec->s_functions.rgf_motcomp_copy[ Y262DEC_MC_BLOCK_16x8 ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_destination_frame->i_stride * 2 );

	i_stride = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_TOP ]->i_stride;
	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( rgi_pel_x[ 2 ], rgi_pel_y[ 2 ], i_stride, i_src_offset );

	pui8_src = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_TOP ]->pui8_luma;
	ps_dec->s_functions.rgf_motcomp_avg[ Y262DEC_MC_BLOCK_16x8 ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_destination_frame->i_stride * 2 );


	if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_420 )
	{
		for( i_idx = 0; i_idx < 3; i_idx++ )
		{
			rgi_mv_x[ i_idx ] = rgi_mv_x[ i_idx ] / 2;
			rgi_mv_y[ i_idx ] = rgi_mv_y[ i_idx ] / 2;
		}
		i_chroma_mc_block = Y262DEC_MC_BLOCK_8x4;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_422 )
	{
		for( i_idx = 0; i_idx < 3; i_idx++ )
		{
			rgi_mv_x[ i_idx ] = rgi_mv_x[ i_idx ] / 2;
		}
		i_chroma_mc_block = Y262DEC_MC_BLOCK_8x8;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_444 )
	{
		i_chroma_mc_block = Y262DEC_MC_BLOCK_16x8;
	}

	i_stride = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_TOP ]->i_stride_chroma;
	i_pel_x = rgi_mv_x[ 0 ] + ps_macroblock->i_macroblock_chroma_pel_x * 2;
	i_pel_y = rgi_mv_y[ 0 ] + ps_macroblock->i_macroblock_chroma_pel_y;
	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( i_pel_x, i_pel_y, i_stride, i_src_offset );

	pui8_src = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_TOP ]->pui8_cb;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cb;

	ps_dec->s_functions.rgf_motcomp_copy[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_destination_frame->i_stride_chroma * 2 );

	i_stride = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_BOTTOM ]->i_stride_chroma;

	pui8_src = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_BOTTOM ]->pui8_cb;
	pui8_dst += ps_destination_frame->i_stride_chroma;
	ps_dec->s_functions.rgf_motcomp_copy[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_destination_frame->i_stride_chroma * 2 );

	i_stride = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_TOP ]->i_stride_chroma;

	pui8_src = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_TOP ]->pui8_cr;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cr;
	ps_dec->s_functions.rgf_motcomp_copy[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_destination_frame->i_stride_chroma * 2 );

	i_stride = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_BOTTOM ]->i_stride_chroma;

	pui8_src = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_BOTTOM ]->pui8_cr;
	pui8_dst += ps_destination_frame->i_stride_chroma;
	ps_dec->s_functions.rgf_motcomp_copy[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_destination_frame->i_stride_chroma * 2 );

	i_stride = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_BOTTOM ]->i_stride_chroma;
	i_pel_x = rgi_mv_x[ 1 ] + ps_macroblock->i_macroblock_chroma_pel_x * 2;
	i_pel_y = rgi_mv_y[ 1 ] + ps_macroblock->i_macroblock_chroma_pel_y;
	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( i_pel_x, i_pel_y, i_stride, i_src_offset );

	pui8_src = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_BOTTOM ]->pui8_cb;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cb;
	ps_dec->s_functions.rgf_motcomp_avg[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_destination_frame->i_stride_chroma * 2 );

	i_stride = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_BOTTOM ]->i_stride_chroma;

	pui8_src = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_BOTTOM ]->pui8_cr;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cr;
	ps_dec->s_functions.rgf_motcomp_avg[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_destination_frame->i_stride_chroma * 2 );

	i_stride = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_TOP ]->i_stride_chroma;

	i_pel_x = rgi_mv_x[ 2 ] + ps_macroblock->i_macroblock_chroma_pel_x * 2;
	i_pel_y = rgi_mv_y[ 2 ] + ps_macroblock->i_macroblock_chroma_pel_y;
	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( i_pel_x, i_pel_y, i_stride, i_src_offset );

	pui8_src = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_TOP ]->pui8_cb;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cb + ps_destination_frame->i_stride_chroma;
	ps_dec->s_functions.rgf_motcomp_avg[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_destination_frame->i_stride_chroma * 2 );

	i_stride = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_TOP ]->i_stride_chroma;

	pui8_src = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ Y262DEC_REFERENCE_TOP ]->pui8_cr;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cr + ps_destination_frame->i_stride_chroma;
	ps_dec->s_functions.rgf_motcomp_avg[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_destination_frame->i_stride_chroma * 2 );

}


void y262dec_motion_compensate_macroblock_field_null( y262dec_slicedec_t *ps_slicedec, int32_t i_avg, int32_t i_direction )
{
	int32_t i_stride, i_src_offset, i_chroma_mc_block;
	uint8_t *pui8_src, *pui8_dst;
	const y262dec_t *ps_dec = ps_slicedec->ps_dec;
	const y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	const y262dec_field_picture_t *ps_field_picture = &ps_state->s_current_field;
	const y262dec_field_picture_t *ps_src_field;
	const y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	const y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;
	const y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	const y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;

	if( ps_picture_coding_extension->i_picture_structure == Y262DEC_PICTURE_CODING_STRUCTURE_TOP )
	{
		ps_src_field = ps_state->rgps_reference_fields[ 0 ][ Y262DEC_REFERENCE_TOP ];
	}
	else
	{
		ps_src_field = ps_state->rgps_reference_fields[ 0 ][ Y262DEC_REFERENCE_BOTTOM ];
	}

	pui8_src = ps_src_field->pui8_luma;
	pui8_dst = ps_macroblock->pui8_destination_luma;
	i_stride = ps_src_field->i_stride;
	i_src_offset = ps_macroblock->i_macroblock_pel_x + ps_macroblock->i_macroblock_pel_y * ps_src_field->i_stride;

	ps_dec->s_functions.rgf_motcomp_copy[ Y262DEC_MC_BLOCK_16x16 ][ 0 ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_field_picture->i_stride );

	if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_420 )
	{
		i_chroma_mc_block = Y262DEC_MC_BLOCK_8x8;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_422 )
	{
		i_chroma_mc_block = Y262DEC_MC_BLOCK_8x16;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_444 )
	{
		i_chroma_mc_block = Y262DEC_MC_BLOCK_16x16;
	}

	i_stride = ps_src_field->i_stride_chroma;
	i_src_offset = ps_macroblock->i_macroblock_chroma_pel_x + ps_macroblock->i_macroblock_chroma_pel_y * i_stride;

	pui8_src = ps_src_field->pui8_cb;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cb;
	ps_dec->s_functions.rgf_motcomp_copy[ i_chroma_mc_block ][ 0 ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_field_picture->i_stride_chroma );

	pui8_src = ps_src_field->pui8_cr;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cr;
	ps_dec->s_functions.rgf_motcomp_copy[ i_chroma_mc_block ][ 0 ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_field_picture->i_stride_chroma );
}


void y262dec_motion_compensate_macroblock_field_field( y262dec_slicedec_t *ps_slicedec, int32_t i_avg, int32_t i_direction )
{
	int32_t i_stride, i_src_offset, i_subpel_idx, i_chroma_mc_block, i_stride_chroma, i_dst_stride_chroma;
	int32_t i_field, i_pel_x, i_pel_y, i_mv_x, i_mv_y;
	uint8_t *pui8_src, *pui8_dst;
	const y262dec_t *ps_dec = ps_slicedec->ps_dec;
	const y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	const y262dec_field_picture_t *ps_field_picture = &ps_state->s_current_field;
	const y262dec_field_picture_t *ps_src_field;
	const y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	const y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;
	const y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;
	const y262dec_motcomp_f( *pf_mc )[ 4 ];

	pf_mc = i_avg ? ps_dec->s_functions.rgf_motcomp_avg : ps_dec->s_functions.rgf_motcomp_copy;

	i_field = ps_macroblock->rgi_motion_vertical_field_select[ Y262DEC_MOTION_VECTOR_FIRST ][ i_direction ];
	ps_src_field = ps_state->rgps_reference_fields[ i_direction ][ i_field ];

	i_mv_x = ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ i_direction ][ 0 ];
	i_mv_y = ps_macroblock->rgi16_motion_vector[ Y262DEC_MOTION_VECTOR_FIRST ][ i_direction ][ 1 ];
	y262dec_motion_compensate_clamp_vector( &i_pel_x, &i_pel_y, &i_mv_x, &i_mv_y, ps_macroblock, 2, Y262DEC_MC_BLOCK_16x16, 0, ps_src_field->i_width, ps_src_field->i_height );

	pui8_src = ps_src_field->pui8_luma;
	pui8_dst = ps_macroblock->pui8_destination_luma;
	i_stride = ps_src_field->i_stride;
	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( i_pel_x, i_pel_y, i_stride, i_src_offset );

	pf_mc[ Y262DEC_MC_BLOCK_16x16 ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, ps_field_picture->i_stride );

	if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_420 )
	{
		i_mv_x = i_mv_x / 2;
		i_mv_y = i_mv_y / 2;
		i_chroma_mc_block = Y262DEC_MC_BLOCK_8x8;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_422 )
	{
		i_mv_x = i_mv_x / 2;
		i_chroma_mc_block = Y262DEC_MC_BLOCK_8x16;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_444 )
	{
		i_chroma_mc_block = Y262DEC_MC_BLOCK_16x16;
	}

	i_stride_chroma = ps_src_field->i_stride_chroma;
	i_dst_stride_chroma = ps_field_picture->i_stride_chroma;
	i_pel_x = ps_macroblock->i_macroblock_chroma_pel_x * 2 + i_mv_x;
	i_pel_y = ps_macroblock->i_macroblock_chroma_pel_y * 2 + i_mv_y;
	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( i_pel_x, i_pel_y, i_stride_chroma, i_src_offset );

	pui8_src = ps_src_field->pui8_cb;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cb;
	pf_mc[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride_chroma, pui8_dst, i_dst_stride_chroma );

	pui8_src = ps_src_field->pui8_cr;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cr;
	pf_mc[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride_chroma, pui8_dst, i_dst_stride_chroma );
}


void y262dec_motion_compensate_macroblock_field_16x8( y262dec_slicedec_t *ps_slicedec, int32_t i_avg, int32_t i_direction )
{
	int32_t i_idx, i_height, i_stride, i_stride_chroma, i_dst_stride_chroma, i_chroma_mc_block, i_src_offset, i_subpel_idx, i_motion_vector, i_dst_chroma_offset;
	int32_t i_field, i_pel_x, i_pel_y, i_mv_x, i_mv_y;
	uint8_t *pui8_src, *pui8_dst;
	const y262dec_t *ps_dec = ps_slicedec->ps_dec;
	const y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	const y262dec_field_picture_t *ps_field_picture = &ps_state->s_current_field;
	const y262dec_field_picture_t *ps_src_field;
	const y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	const y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;
	const y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;
	const y262dec_motcomp_f( *pf_mc )[ 4 ];
	const int32_t rgi_motion_vectors[ 2 ] = { Y262DEC_MOTION_VECTOR_FIRST, Y262DEC_MOTION_VECTOR_SECOND };

	pf_mc = i_avg ? ps_dec->s_functions.rgf_motcomp_avg : ps_dec->s_functions.rgf_motcomp_copy;

	for( i_idx = 0; i_idx < 2; i_idx++ )
	{
		i_motion_vector = rgi_motion_vectors[ i_idx ];
		i_field = ps_macroblock->rgi_motion_vertical_field_select[ i_motion_vector ][ i_direction ];
		ps_src_field = ps_state->rgps_reference_fields[ i_direction ][ i_field ];
		i_mv_x = ps_macroblock->rgi16_motion_vector[ i_motion_vector ][ i_direction ][ 0 ];
		i_mv_y = ps_macroblock->rgi16_motion_vector[ i_motion_vector ][ i_direction ][ 1 ];
		y262dec_motion_compensate_clamp_vector( &i_pel_x, &i_pel_y, &i_mv_x, &i_mv_y, ps_macroblock, 2, Y262DEC_MC_BLOCK_16x8, i_idx << 3, ps_src_field->i_width, ps_src_field->i_height );

		i_stride = ps_src_field->i_stride;
		Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( i_pel_x, i_pel_y, i_stride, i_src_offset );

		pui8_src = ps_src_field->pui8_luma;
		pui8_dst = ps_macroblock->pui8_destination_luma + ( i_stride * ( i_idx << 3 ) );
		pf_mc[ Y262DEC_MC_BLOCK_16x8 ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, i_stride );

		if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_420 )
		{
			i_mv_x = i_mv_x / 2;
			i_mv_y = i_mv_y / 2;
			i_chroma_mc_block = Y262DEC_MC_BLOCK_8x4;
			i_height = 8;
		}
		else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_422 )
		{
			i_mv_x = i_mv_x / 2;
			i_chroma_mc_block = Y262DEC_MC_BLOCK_8x8;
			i_height = 16;
		}
		else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_444 )
		{
			i_chroma_mc_block = Y262DEC_MC_BLOCK_16x8;
			i_height = 16;
		}

		i_stride_chroma = ps_src_field->i_stride_chroma;
		i_dst_stride_chroma = ps_field_picture->i_stride_chroma;
		i_dst_chroma_offset = ( ( i_height / 2 ) * i_dst_stride_chroma * i_idx );
		i_pel_x = ps_macroblock->i_macroblock_chroma_pel_x * 2 + i_mv_x;
		i_pel_y = ps_macroblock->i_macroblock_chroma_pel_y * 2 + i_mv_y + ( i_height * i_idx );
		Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( i_pel_x, i_pel_y, i_stride_chroma, i_src_offset );

		pui8_src = ps_src_field->pui8_cb;
		pui8_dst = ps_macroblock->pui8_destination_chroma_cb + i_dst_chroma_offset;
		pf_mc[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride_chroma, pui8_dst, i_dst_stride_chroma );

		pui8_src = ps_src_field->pui8_cr;
		pui8_dst = ps_macroblock->pui8_destination_chroma_cr + i_dst_chroma_offset;
		pf_mc[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride_chroma, pui8_dst, i_dst_stride_chroma );
	}
}


void y262dec_motion_compensate_macroblock_field_dual_prime( y262dec_slicedec_t *ps_slicedec, int32_t i_avg, int32_t i_direction )
{
	int32_t i_idx, i_motion_vetor, i_stride, i_stride_chroma, i_dst_stride, i_dst_stride_chroma, i_src_offset, i_subpel_idx, i_chroma_mc_block;
	int32_t rgi_pel_x[ 2 ], rgi_pel_y[ 2 ], rgi_mv_x[ 2 ], rgi_mv_y[ 2 ];
	uint8_t *pui8_src, *pui8_dst;
	bool b_field;
	const y262dec_t *ps_dec = ps_slicedec->ps_dec;
	const y262dec_state_t *ps_state = &ps_dec->s_dec_state;
	const y262dec_field_picture_t *ps_field_picture = &ps_state->s_current_field;
	const y262dec_field_picture_t *ps_src_field[ 2 ];
	const y262dec_sequence_extension_t *ps_sequence_extension = &ps_dec->s_headers.s_sequence_extension;
	const y262dec_picture_header_t *ps_picture_header = &ps_dec->s_headers.s_picture_header;
	const y262dec_picture_coding_extension_t *ps_picture_coding_extension = &ps_dec->s_headers.s_picture_coding_extension;
	const y262dec_macroblock_t *ps_macroblock = &ps_slicedec->s_state.s_macroblock;
	const int32_t rgi_motion_vectors[ 2 ] = { Y262DEC_MOTION_VECTOR_FIRST, Y262DEC_MOTION_VECTOR_THIRD };

	b_field = ps_picture_coding_extension->i_picture_structure == Y262DEC_PICTURE_CODING_STRUCTURE_BOTTOM;

	ps_field_picture = &ps_state->s_current_field;

	ps_src_field[ 0 ] = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ b_field ];
	ps_src_field[ 1 ] = ps_state->rgps_reference_fields[ Y262DEC_MOTION_FORWARD ][ !b_field ];

	for( i_idx = 0; i_idx < 2; i_idx++ )
	{
		i_motion_vetor = rgi_motion_vectors[ i_idx ];
		rgi_mv_x[ i_idx ] = ps_macroblock->rgi16_motion_vector[ i_motion_vetor ][ Y262DEC_MOTION_FORWARD ][ 0 ];
		rgi_mv_y[ i_idx ] = ps_macroblock->rgi16_motion_vector[ i_motion_vetor ][ Y262DEC_MOTION_FORWARD ][ 1 ];
		y262dec_motion_compensate_clamp_vector( &rgi_pel_x[ i_idx ], &rgi_pel_y[ i_idx ], &rgi_mv_x[ i_idx ], &rgi_mv_y[ i_idx ], ps_macroblock, 2, Y262DEC_MC_BLOCK_16x16, 0, ps_src_field[ 0 ]->i_width, ps_src_field[ 0 ]->i_height );
	}

	i_stride = ps_src_field[ 0 ]->i_stride;
	i_dst_stride = ps_field_picture->i_stride;
	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( rgi_pel_x[ 0 ], rgi_pel_y[ 0 ], i_stride, i_src_offset );

	pui8_src = ps_src_field[ 0 ]->pui8_luma;
	pui8_dst = ps_macroblock->pui8_destination_luma;
	ps_dec->s_functions.rgf_motcomp_copy[ Y262DEC_MC_BLOCK_16x16 ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, i_dst_stride );

	i_stride = ps_src_field[ 1 ]->i_stride;
	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( rgi_pel_x[ 1 ], rgi_pel_y[ 1 ], i_stride, i_src_offset );

	pui8_src = ps_src_field[ 1 ]->pui8_luma;
	ps_dec->s_functions.rgf_motcomp_avg[ Y262DEC_MC_BLOCK_16x16 ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride, pui8_dst, i_dst_stride );

	if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_420 )
	{
		for( i_idx = 0; i_idx < 2; i_idx++ )
		{
			rgi_mv_x[ i_idx ] = rgi_mv_x[ i_idx ] / 2;
			rgi_mv_y[ i_idx ] = rgi_mv_y[ i_idx ] / 2;
		}
		i_chroma_mc_block = Y262DEC_MC_BLOCK_8x8;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_422 )
	{
		for( i_idx = 0; i_idx < 2; i_idx++ )
		{
			rgi_mv_x[ i_idx ] = rgi_mv_x[ i_idx ] / 2;
		}
		i_chroma_mc_block = Y262DEC_MC_BLOCK_8x16;
	}
	else if( ps_sequence_extension->i_chroma_format == Y262DEC_CHROMA_FORMAT_444 )
	{
		i_chroma_mc_block = Y262DEC_MC_BLOCK_16x16;
	}

	for( i_idx = 0; i_idx < 2; i_idx++ )
	{
		rgi_pel_x[ i_idx ] = ps_macroblock->i_macroblock_chroma_pel_x * 2 + rgi_mv_x[ i_idx ];
		rgi_pel_y[ i_idx ] = ps_macroblock->i_macroblock_chroma_pel_y * 2 + rgi_mv_y[ i_idx ];
	}

	i_stride_chroma = ps_src_field[ 0 ]->i_stride_chroma;
	i_dst_stride_chroma = ps_field_picture->i_stride_chroma;
	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( rgi_pel_x[ 0 ], rgi_pel_y[ 0 ], i_stride_chroma, i_src_offset );

	pui8_src = ps_src_field[ 0 ]->pui8_cb;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cb;
	ps_dec->s_functions.rgf_motcomp_copy[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride_chroma, pui8_dst, i_dst_stride_chroma );

	pui8_src = ps_src_field[ 0 ]->pui8_cr;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cr;
	ps_dec->s_functions.rgf_motcomp_copy[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride_chroma, pui8_dst, i_dst_stride_chroma );

	Y262DEC_SOURCE_OFFSET_AND_MOTCOMP_IDX( rgi_pel_x[ 1 ], rgi_pel_y[ 1 ], i_stride_chroma, i_src_offset );

	pui8_src = ps_src_field[ 1 ]->pui8_cb;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cb;
	ps_dec->s_functions.rgf_motcomp_avg[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride_chroma, pui8_dst, i_dst_stride_chroma );

	pui8_src = ps_src_field[ 1 ]->pui8_cr;
	pui8_dst = ps_macroblock->pui8_destination_chroma_cr;
	ps_dec->s_functions.rgf_motcomp_avg[ i_chroma_mc_block ][ i_subpel_idx ]( pui8_src + i_src_offset, i_stride_chroma, pui8_dst, i_dst_stride_chroma );
}



const y262dec_macroblock_motcomp_f rgf_y262dec_motion_compensation_frame[ 4 ] = {
	y262dec_motion_compensate_macroblock_frame_null,
	y262dec_motion_compensate_macroblock_frame_field,
	y262dec_motion_compensate_macroblock_frame_frame,
	y262dec_motion_compensate_macroblock_frame_dual_prime
};

const y262dec_macroblock_motcomp_f rgf_y262dec_motion_compensation_field[ 4 ] = {
	y262dec_motion_compensate_macroblock_field_null,
	y262dec_motion_compensate_macroblock_field_field,
	y262dec_motion_compensate_macroblock_field_16x8,
	y262dec_motion_compensate_macroblock_field_dual_prime
};

