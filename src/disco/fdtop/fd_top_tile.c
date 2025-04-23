/*#include "../topo/fd_topo.h"*/

/*#include "../metrics/fd_metrics.h"*/
#include "../../disco/keyguard/fd_keyload.h"
#include "../../disco/keyguard/fd_keyswitch.h"
#include "generated/fd_top_tile_seccomp.h"
#include "./fd_top.c"

#define MAX_IN (32UL)

/*typedef struct {*/
/* ulong            seq;*/
/* fd_frag_meta_t * mcache;*/
/* uchar *          data;*/
/*} fd_top_out_ctx_t;*/

typedef struct {
  fd_wksp_t * mem;
  ulong mtu;
  ulong chunk0;
  ulong wmark;
} fd_top_in_ctx_t;

typedef struct {
  fd_topo_t * topo;

  fd_top_t * top;

  char          version_string[ 16UL ];

  /*uchar const *    identity_key;*/
  fd_keyswitch_t * keyswitch;

  fd_top_in_ctx_t in [ MAX_IN ];
} fd_top_ctx_t;

FD_FN_CONST static inline ulong
scratch_align(){
  return 128UL;
}
static inline ulong
scratch_footprint( fd_topo_tile_t const * tile ){
  /*TODO*/
 ulong l = FD_LAYOUT_INIT;
 return FD_LAYOUT_FINI( l, scratch_align() );
}

static ulong
rlimit_file_cnt( fd_topo_t const * topo FD_PARAM_UNUSED,
                fd_topo_tile_t const * tile ){
 /*TODO*/
  return 5UL;
}

static ulong
populate_allowed_seccomp(
    fd_topo_t const * topo,
    fd_topo_tile_t const * tile,
    ulong out_cnt,
    int drain_output_fd, /*TODO: draining output will be impl later*/
    struct sock_filter * out ) {
 
  (void)topo;
  (void)tile;

  populate_sock_filter_policy_fd_top_tile(  out_cnt, out, (uint)fd_log_private_logfile_fd(), drain_output_fd );
  return sock_filter_policy_fd_top_tile_instr_cnt;
}

static ulong
populate_allowed_fds(
    fd_topo_t const * topo,
    fd_topo_tile_t const * tile,
    ulong out_fds_cnt,
    int * out_fds
    ){
  (void)topo;
  (void)tile;

  if( FD_UNLIKELY( out_fds_cnt<3UL ) ) FD_LOG_ERR(( "out_fds_cnt: %lu", out_fds_cnt ));
  ulong out_cnt = 0;
 out_fds[ out_cnt++ ] = 0 /* stdin */;
 out_fds[ out_cnt++ ] = 1 /* stdout */;
 out_fds[ out_cnt++ ] = 2 /* stderr */;
 if( FD_UNLIKELY( -1!=fd_log_private_logfile_fd() && 1!=fd_log_private_logfile_fd() ) ){
   out_fds[ out_cnt++ ] = fd_log_private_logfile_fd();
 }
 return out_cnt;
}

void
fd_top_set_identity( fd_top_ctx_t * ctx, uchar const * identity_key){
  memcpy( ctx->top->identity_key.uc, identity_key, 32UL );
  fd_base58_encode_32( identity_key, NULL, ctx->top->identity_key_base58 );
  ctx->top->identity_key_base58[ FD_BASE58_ENCODED_32_SZ-1UL] = '\0';

}
static inline void
during_housekeeping( fd_top_ctx_t * ctx ){
  if( FD_UNLIKELY( fd_keyswitch_state_query( ctx->keyswitch )==FD_KEYSWITCH_STATE_SWITCH_PENDING ) ){
    fd_top_set_identity( ctx, ctx->keyswitch->bytes );
    fd_keyswitch_state( ctx->keyswitch, FD_KEYSWITCH_STATE_COMPLETED );
  }
}

#define STEM_NAME "top_stem";
#define STEM_CALLBACK_CONTEXT_TYPE fd_top_t
#define STEM_CALLBACK_CONTEXT_ALIGN alignof(fd_top_ctx_t);
#define STEM_CALLBACK_DURING_HOUSEKEEPING during_housekeeping

#include "../../disco/stem/fd_stem.c"
fd_topo_run_tile_t fd_tile_top = {
  .name = "top",
  .rlimit_file_cnt = rlimit_file_cnt,
  .populate_allowed_seccomp = populate_allowed_seccomp,
  .populate_allowed_fds = populate_allowed_fds,
  .scratch_align = scratch_align,
  .scratch_footprint = scratch_footprint,
  .run = stem_run,
};


