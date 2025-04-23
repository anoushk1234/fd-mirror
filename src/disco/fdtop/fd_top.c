#include "../../util/log/fd_log.h"
#include "../../flamenco/leaders/fd_leaders.h"

typedef struct {
  long polling_rate_ms;

  fd_pubkey_t identity_key;
  char identity_key_base58[ FD_BASE58_ENCODED_32_SZ ];
  ulong next_leader_slot;
  ulong current_slot;
  
  struct {
    ulong epoch;
    ulong tsstart;
    ulong tsend;

    ulong my_total_slots;
    ulong my_skipped_slots;
    
    ulong epoch_total_stake;
    ulong my_total_stake;
    fd_epoch_leaders_t * leader_sched;
  } epoch;

  struct {
    fd_pubkey_t vote_account [ 1 ];
    ulong last_vote;
    ulong epoch_credits;

    /* The vote credit data for last 32 slots. 
       TODO: Change this to a better number. */
    ulong tvc_historical [ 32 ];
    int delinquent;
  } vote_info;

  struct {
   ulong gossip_in_bytes;
   ulong gossip_out_bytes;

   ulong quic_conn_cnt;
   ulong net_in_rx_cnt;
   ulong net_out_tx_cnt;
  } stats;

  struct {
    
   int page_number;

   /* An integer where the first eight bits signify if the corresponding
     monitor at the respective index is enabled or disabled. */
   int monitors;
  } app_state;

} fd_top_t;

int
fd_top_poll_update( fd_top_t * top){
  int now = fd_log_wallclock();
  if( FD_UNLIKELY( now>=top->polling_rate_ms ) ){
    /* Update UI objects with data */    
    return 1;
     }
  return 0;
}
