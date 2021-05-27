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

bool y262dec_slice_process( y262dec_slicedec_t *ps_slicedec );

void y262dec_setup_macroblock( y262dec_slicedec_t *ps_slicedec, int32_t i_macroblock_address );

void y262dec_setup_next_macroblock( y262dec_slicedec_t *ps_slicedec );

uint32_t y262dec_macroblock_parse_modes( y262dec_slicedec_t *ps_slicedec );

void y262dec_vector_parse( y262dec_slicedec_t *ps_slicedec, int32_t i_is_frame, int32_t i_motion_type, int32_t i_s );

bool y262dec_macroblock_process_blocks_intra( y262dec_slicedec_t *ps_slicedec );

bool y262dec_macroblock_process_blocks_inter( y262dec_slicedec_t *ps_slicedec );

