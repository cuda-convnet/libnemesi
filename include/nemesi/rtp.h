/* * 
 *  ./include/nemesi/rtp.h: $Revision: 1.2 $ -- $Date: 2002/11/07 12:12:10 $
 *  
 *  This file is part of NeMeSI
 *
 *  NeMeSI -- NEtwork MEdia Streamer I
 *
 *  Copyright (C) 2001 by
 *  	
 *  	Giampaolo "mancho" Mancini - manchoz@inwind.it
 *	Francesco "shawill" Varano - shawill@infinto.it
 *
 *  NeMeSI is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NeMeSI is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NeMeSI; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *  
 * */

#ifndef __RTP_H
#define __RTP_H

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <pthread.h>

#include <nemesi/comm.h>
#include <nemesi/bufferpool.h>
#include <nemesi/types.h>
#include <nemesi/utils.h>
#include <nemesi/wsocket.h>

#define RTP_VERSION 2

#define RTP_AVP_UDP "RTP/AVP/UDP"
#define RTP_SEQ_MOD (1<<16)
#define MIN_SEQUENTIAL 2
#define MAX_DROPOUT 3000
#define MAX_MISORDER 100

#define BANDWIDTH 16000 /* bytes-per-second */

typedef struct {
#ifdef WORDS_BIGENDIAN
	uint32 ver:2;
	uint32 pad:1;
	uint32 ext:1;
	uint32 cc:4;
#else
	uint32 cc:4;	/* source count */
	uint32 ext:1;	/* extension flag */
	uint32 pad:1;	/* padding flag */
	uint32 ver:2;	/* version */
#endif
#ifdef WORDS_BIGENDIAN
	uint32 mark:1;
	uint32 pt:7;
#else
	uint32 pt:7;	/* payload type */
	uint32 mark:1;	/* marker flag */
#endif
	uint32 seq:16;	/* sequence number */
	uint32 time;	/* timestamp */
	uint32 ssrc;	/* synchronization source identifier */
	uint8 data[1];	/* beginning of RTP data */
} rtp_pkt;

struct Transport {
	char *spec;
	enum deliveries { unicast, multicast } delivery;
	struct in_addr dstaddr;
	struct in_addr srcaddr;
	int layers;
	enum modes { play, record } mode;
	int append;
	int ttl;
	uint16 mcs_ports[2];
	uint16 cli_ports[2];
	uint16 srv_ports[2];
	uint32 ssrc;
};

struct SSRC_Stats {
	uint16 max_seq;		/* highest seq number seen */
	uint32 cycles;		/* shifted count of seq number cycles */
	uint32 base_seq;	/* base seq number */
	uint32 bad_seq;		/* last 'bad' seq number + 1 */
	uint32 probation;	/* sequ. pkts till source is valid */
	uint32 received;	/* RTP pkts received */
	uint32 expected_prior;	/* pkt expected at last interval */
	uint32 received_prior;	/* pkt received al last interval */
	uint32 transit;		/* relative trans time for prev pkt */
	double jitter;		/* extimated jitter */
	struct timeval lastrtp;	/* last RTP pkt reception time */
	struct timeval lastsr;	/* last RTCP SR pkt reception time */
	uint32 ntplastsr[2];	/* last RTCP SR pkt NTP reception time */
	uint32 firstts;		/* first pkt timestamp */
	struct timeval firsttv;	/* first pkt timeval */
};

struct SSRC_Description {
	char *end;
	char *cname;
	char *name;
	char *email;
	char *phone;
	char *loc;
	char *tool;
	char *note;
	char *priv;
};

struct Session_Stats {
	struct timeval tp;	/* the last time an RTCP pkt was transmitted */
	struct timeval tn;	/* the next scheduled transmission time of an RTCP pkt */
	uint32 pmembers;	/* the estimated number of session members at time tm was last recomputed */
	uint32 members;		/* the most currente estimate for the number of the session members */
	uint32 senders;		/* the most currente estimate for the number of senders in the session */
	double rtcp_bw;		/* the target RTCP bandwidht */
	uint8 we_sent;		/* flag that is true if the app has sent data since the second previous RTCP Report was transmitted */
	double avg_rtcp_size;	/* the average Compound RTCP pkt size, in octets, over all RTCP pkts sent and received by this partecipant */
	uint8 initial;		/* the flag that is true if the app has not yet sent an RTCP pkt */
};

struct Stream_Source {
	uint32 ssrc;
	struct sockaddr_in rtp_from;
	struct sockaddr_in rtcp_from;
	struct sockaddr_in rtcp_to;
	int rtcptofd;
	struct SSRC_Stats ssrc_stats;
	struct SSRC_Description ssrc_sdes;
	playout_buff po;
	struct Stream_Source *next;
};

struct Conflict {
	struct sockaddr_in *transaddr;
	time_t time;
	struct Conflict *next;
};

struct RTP_Session {
	pthread_t rtp_tid;
	pthread_t rtcp_tid;
	pthread_t dec_tid;
	uint32 local_ssrc;
	int rtpfd;
	int rtcpfd;
	struct Transport transport;
	struct Session_Stats sess_stats;
	struct Stream_Source *ssrc_queue;
	struct Conflict *conf_queue;
	buffer_pool bp;
	struct RTP_Session *next;
};

struct Dec_args {
	struct RTP_Session *rtp_sess_head;
	struct timeval startime;
	pthread_mutex_t syn;
};

enum proto_types {
	RTP,
	RTCP	
};

void *rtp(void *);

struct RTP_Session *init_rtp_sess(struct sockaddr, struct sockaddr);
int rtp_thread_create(struct RTP_Session *);

int rtp_recv(struct RTP_Session *);
int rtp_hdr_val_chk(rtp_pkt *, int);
int ssrc_check(struct RTP_Session *, uint32, struct Stream_Source **, struct sockaddr_in, enum proto_types);
int set_stm_src(struct RTP_Session *, struct Stream_Source **, uint32, struct sockaddr_in, enum proto_types);
void init_seq(struct Stream_Source *, uint16);
void update_seq(struct Stream_Source *, uint16);

void rtp_clean(void *);
#endif