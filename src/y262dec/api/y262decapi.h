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


typedef struct
{
	bool b_progressive;
	int32_t i_width;
	int32_t i_height;
#define Y262DEC_CHROMA_FORMAT_420   1
#define Y262DEC_CHROMA_FORMAT_422   2
#define Y262DEC_CHROMA_FORMAT_444   3
	int32_t i_chroma_format;
	int32_t i_aspect_ratio_code;
	int32_t i_frame_rate_code;

} y262dec_sequence_parameters_t;


typedef struct
{
	int32_t i_width;
	int32_t i_stride;
	int32_t i_stride_chroma;
	int32_t i_height;
	bool  b_top_field_first;
	bool  b_repeat_first_field;
	int32_t i_repeat_frame_times;
	uint8_t *pui8_luma;
	uint8_t *pui8_cb;
	uint8_t *pui8_cr;
	int32_t i_don;
	int32_t i_pon;
} y262dec_frame_picture_t;


typedef struct
{
#define Y262DEC_DECODE_RESULT_TYPE_SEQUENCE_PARAMS 1
#define Y262DEC_DECODE_RESULT_TYPE_FRAME           2
	int32_t i_result_type;
	union
	{
		y262dec_sequence_parameters_t s_parameters;
		y262dec_frame_picture_t s_frame;
	}u_result;
} y262dec_decode_result_t;


typedef void ( *y262dec_callback_t )( void *p_user, y262dec_decode_result_t *ps_result );


typedef struct
{
	void *p_user;
	y262dec_callback_t pf_callback;
	bool b_multithreading;
} y262dec_config_t;

void *y262dec_create( y262dec_config_t *ps_config );

#define Y262DEC_OK     0
#define Y262DEC_ERROR  1
#define Y262DEC_MORE   2

int32_t y262dec_process( void *p_dec, uint8_t *pui8_data, int32_t i_data_size );

int32_t y262dec_flush( void *p_dec );

void y262dec_destroy( void *p_dec );


