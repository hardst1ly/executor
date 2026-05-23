#pragma once
#include "helper/helper.h"

#define proto_lineinfo_enc vmval1
#define proto_abslineinfo_enc vmval3
#define proto_locvars_enc vmval3
#define proto_upvalues_enc vmval1
#define proto_source_enc vmval2

#define proto_debugname_enc vmval4
#define proto_debuginsn_enc vmval1
#define proto_typeinfo_enc vmval1
#define proto_userdata_enc vmval3

#define closure_cont_enc vmval2
#define closure_debugname_enc vmval4

#define tstring_hash_enc vmval4

#define udata_meta_enc vmval2

#define lstate_stacksize_enc vmval3