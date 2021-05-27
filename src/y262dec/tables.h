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

extern const uint8_t rgui8_y262dec_default_intra_matrix[ 64 ];

extern const uint8_t rgui8_y262dec_default_non_intra_matrix[ 64 ];

extern const uint8_t rgui8_y262dec_scan_0_table[ 64 ];

extern const uint8_t rgui8_y262dec_scan_1_table[ 64 ];

extern const int8_t rgi8_y262dec_quantizer_scale_table[ 2 ][ 32 ];

extern const y262dec_mb_addr_t rgs_y262dec_macroblock_aincrement_5[ 32 ];

extern const y262dec_mb_addr_t rgs_y262dec_macroblock_aincrement_11[ 128 ];

extern const y262dec_mbtype_t rgs_y262dec_macroblock_flags_i[ 4 ];

extern const y262dec_mbtype_t rgs_y262dec_macroblock_flags_p[ 64 ];

extern const y262dec_mbtype_t rgs_y262dec_macroblock_flags_b[ 64 ];

extern const y262dec_dprime_vector_t rgs_y262dec_dual_prime_vector[ 4 ];

extern const y262dec_mvdelta_t rgs_y262dec_mv_delta_4[ 8 ];

extern const y262dec_mvdelta_t rgs_y262dec_mv_delta_10[ 48 ];

extern const y262dec_dcsize_t rgs_y262dec_dct_dc_size_luma_5[ 31 ];

extern const y262dec_dcsize_t rgs_y262dec_dct_dc_size_luma_9[ 16 ];

extern const y262dec_dcsize_t rgs_y262dec_dct_dc_size_chroma_5[ 31 ];

extern const y262dec_dcsize_t rgs_y262dec_dct_dc_size_chroma_10[ 32 ];

extern const y262dec_cbp_t rgs_y262dec_cbp_9[ 64 ];

extern const y262dec_cbp_t rgs_y262dec_cbp_7[ 112 ];

extern const y262dec_run_level_t rgs_y262dec_dct_zero_table_16[ 32 ];

extern const y262dec_run_level_t rgs_y262dec_dct_zero_table_15[ 64 ];

extern const y262dec_run_level_t rgs_y262dec_dct_zero_table_13[ 64 ];

extern const y262dec_run_level_t rgs_y262dec_dct_zero_table_10[ 16 ];

extern const y262dec_run_level_t rgs_y262dec_dct_zero_table_8[ 40 ];

extern const y262dec_run_level_t rgs_y262dec_dct_zero_table_5[ 32 ];

extern const y262dec_run_level_t rgs_y262dec_dct_zero_table_5b[ 32 ];

extern const y262dec_run_level_t rgs_y262dec_dct_one_table_13[ 48 ];

extern const y262dec_run_level_t rgs_y262dec_dct_one_table_10[ 16 ];

extern const y262dec_run_level_t rgs_y262dec_dct_one_table_8[ 40 ];

extern const y262dec_run_level_t rgs_y262dec_dct_one_table_5[ 32 ];

extern const y262dec_run_level_t rgs_y262dec_dct_one_table_m8[ 16 ];

extern const y262dec_run_level_t rgs_y262dec_dct_one_table_8f[ 256 ];

extern const y262dec_macroblock_motcomp_f rgf_y262dec_motion_compensation_frame[ 4 ];

extern const y262dec_macroblock_motcomp_f rgf_y262dec_motion_compensation_field[ 4 ];


