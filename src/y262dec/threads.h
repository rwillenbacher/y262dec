/*
Copyright (c) 2013,2016, Ralf Willenbacher
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

typedef void ( y262dec_thread_f )( void *p_arg );

bool y262dec_can_do_threads( );

void *y262dec_create_thread( y262dec_t *ps_y262, y262dec_thread_f *pf_func, void *p_arg );
void y262dec_join_thread( y262dec_t *ps_y262, void *p_thread );

void *y262dec_create_mutex( y262dec_t *ps_y262 );
void y262dec_destroy_mutex( y262dec_t *ps_y262, void *p_cs );
void y262dec_mutex_lock( y262dec_t *ps_y262, void *p_cs );
void y262dec_mutex_unlock( y262dec_t *ps_y262, void *p_cs );

void *y262dec_create_event( y262dec_t *ps_y262 );
void y262dec_destroy_event( y262dec_t *ps_y262, void *p_event );
void y262dec_event_wait_( y262dec_t *ps_y262, void *p_event );
void y262dec_event_set_( y262dec_t *ps_y262, void *p_event );
void y262dec_event_wait_g( y262dec_t *ps_y262, void *p_event );
void y262dec_event_set_g( y262dec_t *ps_y262, void *p_event );


void y262dec_slicedec_dispatch( y262dec_t *ps_dec, int32_t i_slicedec );
void y262dec_slicedec_wait( y262dec_t *ps_dec, int32_t i_slicedec );
bool y262dec_slicedec_thread_create( y262dec_t *ps_dec, int32_t i_slicedec );
void y262dec_slicedec_thread_destroy( y262dec_t *ps_dec, int32_t i_slicedec );