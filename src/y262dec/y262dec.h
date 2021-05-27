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
#include <memory.h>


#ifndef MIN
#define MIN( x, y ) ( ( x ) < ( y ) ? ( x ) : ( y ) )
#endif

#ifndef MAX
#define MAX( x, y ) ( ( x ) < ( y ) ? ( y ) : ( x ) )
#endif


#include "y262decapi.h"

/* return codes */
#define Y262DEC_INT_OK     0
#define Y262DEC_INT_ERROR  1
#define Y262DEC_INT_MORE   2


/* startcodes */
#define Y262DEC_STARTCODE_PICTURE         0x00
#define Y262DEC_STARTCODE_SLICES_BEGIN    0x01
#define Y262DEC_STARTCODE_SLICES_END      0xAF
#define Y262DEC_STARTCODE_USER_DATA       0xB2
#define Y262DEC_STARTCODE_SEQUENCE_HEADER 0xB3
#define Y262DEC_STARTCODE_SEQUENCE_ERROR  0xB4
#define Y262DEC_STARTCODE_EXTENSION       0xB5
#define Y262DEC_STARTCODE_SEQUENCE_END    0xB7
#define Y262DEC_STARTCODE_GROUP           0xB8


#define Y262DEC_EXTENSION_SEQUENCE				1
#define Y262DEC_EXTENSION_SEQUENCE_DISPLAY		2
#define Y262DEC_EXTENSION_QUANT_MATRIX			3
#define Y262DEC_EXTENSION_PICTURE_DISPLAY		7
#define Y262DEC_EXTENSION_PICTURE_CODING		8


#define Y262DEC_PICTURE_CODING_TYPE_I 1
#define Y262DEC_PICTURE_CODING_TYPE_P 2
#define Y262DEC_PICTURE_CODING_TYPE_B 3

#define Y262DEC_PICTURE_CODING_STRUCTURE_TOP    1
#define Y262DEC_PICTURE_CODING_STRUCTURE_BOTTOM 2
#define Y262DEC_PICTURE_CODING_STRUCTURE_FRAME  3

#define Y262DEC_PICTURE_CODING_FORWARD    0
#define Y262DEC_PICTURE_CODING_BACKWARD   1
#define Y262DEC_PICTURE_CODING_HORIZONTAL 0
#define Y262DEC_PICTURE_CODING_VERTICAL   1

#define Y262DEC_STATE_WAIT_INIT                      0
#define Y262DEC_STATE_POST_SEQUENCE_HEADER           1
#define Y262DEC_STATE_POST_SEQUENCE_EXTENSION        2
#define Y262DEC_STATE_POST_GROUP_OF_PICTURES         3
#define Y262DEC_STATE_POST_PICTURE_HEADER            4
#define Y262DEC_STATE_POST_PICTURE_CODING_EXTENSION  5
#define Y262DEC_STATE_PROCESS_SLICES                 6

#define Y262DEC_REFERENCE_PFORWARD   0
#define Y262DEC_REFERENCE_BFORWARD   1
#define Y262DEC_REFERENCE_BBACKWARD  0
#define Y262DEC_REFERENCE_TOP        0
#define Y262DEC_REFERENCE_BOTTOM     1

#define Y262DEC_MACROBLOCK_QUANT			1
#define Y262DEC_MACROBLOCK_MOTION_FORWARD	2
#define Y262DEC_MACROBLOCK_MOTION_BACKWARD	4
#define Y262DEC_MACROBLOCK_PATTERN			8
#define Y262DEC_MACROBLOCK_INTRA			16
#define Y262DEC_MACROBLOCK_INTERLACED		32
#define Y262DEC_MACROBLOCK_MOTION_TYPE		64

#define Y262DEC_MOTION_TYPE_FRAME_FIELD  1
#define Y262DEC_MOTION_TYPE_FRAME_FRAME  2
#define Y262DEC_MOTION_TYPE_FRAME_DPRIME 3
#define Y262DEC_MOTION_TYPE_FIELD_FIELD  1
#define Y262DEC_MOTION_TYPE_FIELD_2X     2
#define Y262DEC_MOTION_TYPE_FIELD_DPRIME 3

#define Y262DEC_MV_FORMAT_FRAME	1
#define Y262DEC_MV_FORMAT_FIELD	2

#define Y262DEC_MOTION_VECTOR_FIRST  0
#define Y262DEC_MOTION_VECTOR_SECOND 1
#define Y262DEC_MOTION_VECTOR_THIRD  2
#define Y262DEC_MOTION_VECTOR_FOURTH 3
#define Y262DEC_MOTION_FORWARD       0
#define Y262DEC_MOTION_BACKWARD      1
#define Y262DEC_MOTION_HORIZONTAL    0
#define Y262DEC_MOTION_VERTICAL      1

#define Y262DEC_RUN_LEVEL_END_OF_BLOCK -1
#define Y262DEC_RUN_LEVEL_ESCAPE       -2

#define Y262DEC_MC_BLOCK_00    0
#define Y262DEC_MC_BLOCK_01    1
#define Y262DEC_MC_BLOCK_10    2
#define Y262DEC_MC_BLOCK_11    3

#define Y262DEC_MC_BLOCK_16x16  0
#define Y262DEC_MC_BLOCK_16x8   1
#define Y262DEC_MC_BLOCK_8x16   2
#define Y262DEC_MC_BLOCK_8x8    3
#define Y262DEC_MC_BLOCK_8x4    4

typedef void ( *y262dec_motcomp_f ) ( uint8_t *pui8_src, int32_t i_stride, uint8_t *pui8_dst, int32_t i_dst_stride );

typedef struct
{
	uint8_t *pui8_buffer;
	int32_t i_max_fill;
	int32_t i_write_position;
	int32_t i_read_position;
	uint32_t ui_possible_startcode;
	bool b_synced;
	uint64_t ui64_next_pts;
	uint64_t ui64_next_dts;

} y262dec_input_buffer_t;


typedef struct
{
	uint8_t *pui8_buffer_ptr;
	uint8_t *pui8_buffer_end;
	int32_t i_num_bits;
	uint32_t ui_bits;

} y262dec_bitstream_t;


typedef struct
{
	int32_t i_code;
	int32_t i_length;

} y262dec_vlc_t;


typedef struct
{
	uint8_t i8_length;
	uint8_t i8_run;
	uint8_t i8_level;

} y262dec_run_level_t;


typedef struct
{
	uint8_t i8_delta;
	uint8_t i8_len;

} y262dec_mvdelta_t;


typedef struct
{
	int8_t i8_vector;
	uint8_t i8_len;
} y262dec_dprime_vector_t;

typedef struct
{
	uint8_t i8_len;
	uint8_t i8_flags;

} y262dec_mbtype_t;


typedef struct
{
	uint8_t i8_len;
	uint8_t i8_cbp;

} y262dec_cbp_t;


typedef struct
{
	uint8_t ui8_len;
	int8_t ui8_increment;

} y262dec_mb_addr_t;


typedef struct
{
	uint8_t i8_len;
	uint8_t i8_size;

} y262dec_dcsize_t;


typedef struct
{
	int32_t i_width;
	int32_t i_stride;
	int32_t i_stride_chroma;
	int32_t i_height;
	uint8_t *pui8_luma;
	uint8_t *pui8_cb;
	uint8_t *pui8_cr;

} y262dec_field_picture_t;


typedef struct
{
	int16_t rgi16_dc_dct_pred[ 3 ];
	int16_t rgi16_motion_vector_pred[ 2 ][ 2 ][ 2 ];

} y262dec_predictors_t;


typedef struct
{
	int32_t i_macroblock_pel_x;
	int32_t i_macroblock_pel_y;
	int32_t i_macroblock_chroma_pel_x;
	int32_t i_macroblock_chroma_pel_y;
	uint8_t *pui8_destination_luma;
	uint8_t *pui8_destination_chroma_cb;
	uint8_t *pui8_destination_chroma_cr;

	int32_t i_macroblock_address;
	uint32_t ui_macroblock_type;
	int32_t i_mv_format;
	bool b_dmv;

	int32_t rgi_dmvector[ 2 ];
	int32_t rgi_motion_vertical_field_select[ 2 ][ 2 ];
	int16_t rgi16_motion_vector[ 4 ][ 2 ][ 2 ];
	int16_t *rgpui16_quantizer_matrices[ 4 ];
} y262dec_macroblock_t;


typedef struct
{
	int32_t i_next_don;
	int32_t i_next_pon;
	int32_t i_state;
	bool b_mpeg_2;

	bool b_initialized;

	int32_t i_horizontal_size;
	int32_t i_vertical_size;

	int32_t i_mb_width;
	int32_t i_mb_height;

	int32_t i_picture_mb_width;
	int32_t i_picture_mb_height;
	int32_t i_picture_width;
	int32_t i_picture_height;

	int32_t i_mb_chroma_width;
	int32_t i_mb_chroma_height;
	uint32_t ui_luma_block_y_stride_dct0;
	uint32_t ui_luma_stride_dct0;
	uint32_t ui_chroma_block_y_stride_dct0;
	uint32_t ui_chroma_stride_dct0;
	uint32_t ui_luma_block_y_stride_dct1;
	uint32_t ui_luma_stride_dct1;
	uint32_t ui_chroma_block_y_stride_dct1;
	uint32_t ui_chroma_stride_dct1;

	y262dec_frame_picture_t s_current_frame;
	y262dec_field_picture_t s_current_field;
	bool rgb_fields[ 2 ];
	bool b_top_field_first;

	y262dec_frame_picture_t rgs_picture_buffer[ 3 ];
	struct
	{
		int32_t i_current_picture;
		int32_t i_reference_picture_0;
		int32_t i_reference_picture_1;
		int32_t i_fill;
	} s_decoded_picture_buffer;
	y262dec_frame_picture_t rgs_reference_frames[ 2 ];
	y262dec_field_picture_t rgs_reference_fields[ 2 ][ 2 ];

	bool b_buffer_init;
	int32_t i_frame_buffer_size;
	uint8_t *pui8_frame_buffer;

	y262dec_frame_picture_t s_output_frame;
	bool b_output_frame;

	/* */
	const uint8_t *pui8_current_scan;
	uint32_t ui_intra_dc_shift;
	y262dec_frame_picture_t *rgps_reference_frames[ 2 ];
	y262dec_field_picture_t *rgps_reference_fields[ 2 ][ 2 ];
} y262dec_state_t;


typedef struct
{
	bool b_buffer_init;
	uint8_t *pui8_block_buffer;
	int16_t *pi16_block;
	y262dec_predictors_t s_predictors;
	y262dec_macroblock_t s_macroblock;
} y262dec_slice_state_t;



typedef struct
{
	int32_t i_horizontal_size;
	int32_t i_vertical_size;
	int32_t i_aspect_ratio_information;
	int32_t i_frame_rate_code;
	int32_t i_bit_rate_value;
	int32_t i_vbv_buffer_size_value;
	bool  b_constrained_parameters_flag;
	bool  b_load_intra_quantizer_matrix;
	uint8_t rgui8_intra_quantizer_matrix[ 64 ];
	bool  b_load_non_intra_quantizer_matrix;
	uint8_t rgui8_non_intra_quantizer_matrix[ 64 ];

} y262dec_sequence_header_t;


typedef struct
{
	int32_t i_time_code;
	bool  b_closed_gop;
	bool  b_broken_link;

} y262dec_group_of_pictures_header_t;

typedef struct
{
	int32_t	i_temporal_reference;
	int32_t	i_picture_coding_type;
	int32_t	i_vbv_delay;
	bool	b_full_pel_forward_vector;
	int32_t	i_forward_f_code;
	bool	b_full_pel_backward_vector;
	int32_t	i_backward_f_code;
	bool	b_extra_bit_picture;
#define Y262DEC_PICTURE_MAX_EXTRA_INFORMATION 32
	uint8_t	rgui8_extra_information_picture[ Y262DEC_PICTURE_MAX_EXTRA_INFORMATION ];

} y262dec_picture_header_t;


typedef struct
{
	int32_t i_profile_and_level_indication;
	bool  b_progressive_sequence;
	int32_t i_chroma_format;
	int32_t i_horizontal_size_extension;
	int32_t i_vertical_size_extension;
	int32_t i_bit_rate_extension;
	int32_t i_vbv_buffer_size_extension;
	bool  b_low_delay;
	int32_t i_frame_rate_extension_n;
	int32_t i_frame_rate_extension_d;

} y262dec_sequence_extension_t;


typedef struct
{
	int32_t i_video_format;
	bool  b_color_description;
	struct
	{
		int32_t i_color_primaries;
		int32_t i_transfer_characteristics;
		int32_t i_matrix_coefficients;
	} s_color_description;
	int32_t i_display_horizontal_size;
	int32_t i_display_vertical_size;

} y262dec_sequence_display_extension_t;


typedef struct
{
	int32_t i_num_frame_centre_offsets;
#define PICTURE_DISPLAY_EXTENSION_MAX_FRAME_CENTRE_OFFSETS 3
	struct
	{
		int32_t i_frame_centre_horizontal_offset;
		int32_t i_frame_centre_vertical_offset;
	} rgs_frame_centre_offsets[ 3 ];

} y262dec_picture_display_extension_t;


typedef struct
{
	int32_t rgi_f_code[ 2 ][ 2 ];
	int32_t i_intra_dc_precision;
	int32_t i_picture_structure;
	bool  b_top_field_first;
	bool  b_frame_pred_frame_dct;
	bool  b_concealment_motion_vectors;
	bool  b_q_scale_type;
	bool  b_intra_vlc_format;
	bool  b_alternate_scan;
	bool  b_repeat_first_field;
	bool  b_chroma_420_type;
	bool  b_progressive_frame;
	bool  b_composite_display_flag;
	struct
	{
		bool  b_v_axis;
		int32_t i_field_sequence;
		bool  b_sub_carrier;
		int32_t i_burst_amplitude;
		int32_t i_sub_carrier_phase;
	} s_composite_display;

} y262dec_picture_coding_extension_t;


typedef struct
{
	uint8_t rgui8_intra_quantizer_matrix[ 64 ];
	uint8_t rgui8_non_intra_quantizer_matrix[ 64 ];
	uint8_t rgui8_chroma_intra_quantizer_matrix[ 64 ];
	uint8_t rgui8_chroma_non_intra_quantizer_matrix[ 64 ];
	bool rgb_refresh_intra_quantizer_matrix[ 2 ];
	bool rgb_refresh_non_intra_quantizer_matrix[ 2 ];
	bool rgb_refresh_chroma_intra_quantizer_matrix[ 2 ];
	bool rgb_refresh_chroma_non_intra_quantizer_matrix[ 2 ];
	uint16_t rgui16_intra_quantizer_matrix[ 2 ][ 32 ][ 64 ];
	uint16_t rgui16_non_intra_quantizer_matrix[ 2 ][ 32 ][ 64 ];
	uint16_t rgui16_chroma_intra_quantizer_matrix[ 2 ][ 32 ][ 64 ];
	uint16_t rgui16_chroma_non_intra_quantizer_matrix[ 2 ][ 32 ][ 64 ];

} y262dec_quantizer_matrices_t;


typedef struct
{
	y262dec_motcomp_f rgf_motcomp_copy[ 5 ][ 4 ];
	y262dec_motcomp_f rgf_motcomp_avg[ 5 ][ 4 ];

	void ( *pf_idct_put )( int16_t *pi16_block, int32_t i_idx, uint8_t *pui8_dst, uint32_t ui_stride );
	void ( *pf_idct_add )( int16_t *pi16_block, int32_t i_idx, uint8_t *pui8_dst, uint32_t ui_stride );
} y262dec_functions_t;


typedef struct
{
	struct y262dec_s *ps_dec;
	int32_t i_slicedec_idx;

#define Y262DEC_MAX_SLICEDEC_SLICE_SIZE ( 1024 * 1024 * 2 ) /* 32k bits per MB @ 4k times 2 */
	uint8_t *pui8_slice_data;

	y262dec_bitstream_t s_bitstream;

	y262dec_slice_state_t s_state;

#define Y262DEC_SLICEDEC_THREAD_CMD_PROCESS 0
#define Y262DEC_SLICEDEC_THREAD_CMD_EXIT    1
	int32_t i_command;
	void *p_thread;
	void *p_event_start;
	void *p_event_finish;
} y262dec_slicedec_t;

typedef struct y262dec_s
{
	y262dec_config_t s_config;

	y262dec_input_buffer_t s_input_buffer;

	y262dec_bitstream_t s_bitstream;

	y262dec_state_t s_dec_state;

	y262dec_quantizer_matrices_t s_quantizer_matrices;

	struct
	{
		y262dec_sequence_header_t s_sequence_header;
		y262dec_group_of_pictures_header_t s_group_of_pictures_header;
		y262dec_picture_header_t s_picture_header;

		y262dec_sequence_extension_t s_sequence_extension;
		y262dec_sequence_display_extension_t s_sequence_display_extension;
		y262dec_picture_display_extension_t s_picture_display_extension;
		y262dec_picture_coding_extension_t s_picture_coding_extension;

	} s_headers;

	y262dec_functions_t s_functions;

	bool b_multithreading;
#define Y262DEC_MAX_SLICE_DECODERS 8
	int32_t i_num_slice_decoders;
	int32_t i_slice_dec_dispatch;
	int32_t i_slice_dec_collect;
	void *p_resource_mutex;
	y262dec_slicedec_t rgs_slice_decoders[ Y262DEC_MAX_SLICE_DECODERS ];

	bool b_flush_comitted;
} y262dec_t;


typedef void( *y262dec_macroblock_motcomp_f ) ( y262dec_slicedec_t *ps_slicedec, int32_t i_avg, int32_t i_direction );


bool y262dec_init( y262dec_t *ps_dec, y262dec_config_t *ps_config );
void y262dec_deinit( y262dec_t *ps_dec );
int32_t y262dec_process_internal( y262dec_t *ps_dec, uint8_t *pui8_data, int32_t i_data_size );
int32_t y262dec_process_flush( y262dec_t *ps_dec );


#include "tables.h"

#include "bitstream.h"

#include "aboveslicelevel.h"

#include "slicelevel.h"

#include "transform.h"

#include "threads.h"

#ifdef ASSEMBLY_X86
#include "transform_x86.h"
#include "mc_x86.h"
#endif

#ifdef ASSEMBLY_ARM64
#include "transform_arm64.h"
#include "mc_arm64.h"
#endif

void y262dec_init_motion_compensation( y262dec_t *ps_dec );
