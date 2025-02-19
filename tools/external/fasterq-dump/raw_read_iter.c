/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#include "raw_read_iter.h"

#ifndef _h_err_msg_
#include "err_msg.h"
#endif

#ifndef _h_file_printer_
#include "file_printer.h"
#endif

#ifndef _h_cmn_iter_
#include "cmn_iter.h"
#endif

typedef struct raw_read_iter_t {
    struct cmn_iter_t * cmn;
    uint32_t seq_spot_id, seq_read_id, read_id;
} raw_read_iter_t;


void destroy_raw_read_iter( struct raw_read_iter_t * iter ) {
    if ( NULL != iter ) {
        cmn_iter_release( iter -> cmn );
        free( ( void * ) iter );
    }
}

rc_t make_raw_read_iter( cmn_iter_params_t * params, struct raw_read_iter_t ** iter ) {

    rc_t rc = 0;
    raw_read_iter_t * i = calloc( 1, sizeof * i );
    if ( NULL == i ) {
        rc = RC( rcVDB, rcNoTarg, rcConstructing, rcMemory, rcExhausted );
        ErrMsg( "make_raw_read_iter.calloc( %d ) -> %R", ( sizeof * i ), rc );
    } else {
        rc = cmn_iter_make( params, "PRIMARY_ALIGNMENT", &i -> cmn );
        if ( 0 == rc ) {
            rc = cmn_iter_add_column( i -> cmn, "SEQ_SPOT_ID", &i -> seq_spot_id );
        }
        if ( 0 == rc ) {
            rc = cmn_iter_add_column( i -> cmn, "SEQ_READ_ID", &i -> seq_read_id );
        }
        if ( 0 == rc ) {
            rc = cmn_iter_add_column( i -> cmn, /*"(INSDC:4na:bin)RAW_READ"*/"READ", &i -> read_id );
        }
        if ( 0 == rc ) {
            rc = cmn_iter_detect_range( i -> cmn, i -> seq_spot_id );
        }
        if ( 0 != rc ) {
            destroy_raw_read_iter( i );
        } else {
            *iter = i;
        }
    }
    return rc;
}

bool get_from_raw_read_iter( struct raw_read_iter_t * iter, raw_read_rec_t * rec, rc_t * rc ) {
    bool res = cmn_iter_get_next( iter -> cmn, rc );
    if ( res ) {
        *rc = cmn_iter_read_uint64( iter -> cmn, iter -> seq_spot_id, &rec -> seq_spot_id );
        if ( 0 == *rc ) {
            *rc = cmn_iter_read_uint32( iter -> cmn, iter -> seq_read_id, &rec -> seq_read_id );
        }
        if ( 0 == *rc ) {
            *rc = cmn_iter_read_String( iter -> cmn, iter -> read_id, &rec -> read );
        }
    }
    return res;
}

uint64_t get_row_count_of_raw_read( struct raw_read_iter_t * iter ) {
    return cmn_iter_get_row_count( iter -> cmn );
}

rc_t write_out_prim( const KDirectory *dir, size_t buf_size, size_t cursor_cache,
                     const char * accession_short, const char * accession_path,
                     const char * output_file ) {
    rc_t rc;
    struct raw_read_iter_t * iter;
    cmn_iter_params_t params; /* helper.h */

    params . dir = dir;
    params . accession_short = accession_short;
    params . accession_path = accession_path;
    params . first_row = 0;
    params . row_count = 0;
    params . cursor_cache = cursor_cache;

    rc = make_raw_read_iter( &params, &iter ); /* raw_read_iter.c */
    if ( 0 == rc ) {
        struct file_printer_t * printer;
        rc = make_file_printer_from_filename( dir, &printer, buf_size, 1024, "%s", output_file );
        if ( 0 == rc ) {
            raw_read_rec_t rec;
            bool running = get_from_raw_read_iter( iter, &rec, &rc ); /* raw_read_iter.c */
            while( 0 == rc && running ) {
                uint64_t key = rec . seq_spot_id;

                key <<= 1;
                if ( rec. seq_read_id == 1 ) {
                    key &= 0xFFFFFFFFFFFFFFFE;
                } else {
                    key |= 1;
                }

                rc = file_print( printer, "%lu\n", key );
                if ( 0 == rc ) {
                    running = get_from_raw_read_iter( iter, &rec, &rc ); /* raw_read_iter.c */
                }
            }
            destroy_file_printer( printer );
        }
        destroy_raw_read_iter( iter );
    }
    return rc;
}
