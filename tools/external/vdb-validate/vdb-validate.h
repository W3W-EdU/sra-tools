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

#include <kdb/manager.h>
#include <vdb/manager.h>

/// startup parameters and top level state
extern bool exhaustive;
extern bool md5_required;
extern bool ref_int_check;
extern bool s_IndexOnly;

typedef struct vdb_validate_params vdb_validate_params;
struct vdb_validate_params
{
    struct KDirectory const *wd;
    struct KDBManager const *kmgr;
    struct VDBManager const *vmgr;

    bool md5_chk;
    bool md5_chk_explicit;
    bool blob_crc;
    bool index_chk;
    bool consist_check;
    bool exhaustive;

    // data integrity checks parameters
    bool sdc_enabled;
    bool sdc_sec_rows_in_percent;
    union
    {
        double percent;
        uint64_t number;
    } sdc_sec_rows;

    bool sdc_seq_rows_in_percent;
    union
    {
        double percent;
        uint64_t number;
    } sdc_seq_rows;

    bool sdc_pa_len_thold_in_percent;
    union
    {
        double percent;
        uint64_t number;
    } sdc_pa_len_thold;
};

rc_t vdb_validate(const vdb_validate_params *pb, const char *aPath);
