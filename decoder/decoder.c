/* * 
 *  ./decoder/decoder.c: $Revision: 1.2 $ -- $Date: 2002/11/07 12:12:08 $
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

#include <nemesi/decoder.h>
#include <nemesi/rtpptdefs.h>

#define SYS_BUFF 5 /*system buffer in num of packets*/
#define GRAIN 20

void *decoder(void *args)
{
	struct Dec_args *dec_args=(struct Dec_args *)args;
	struct RTP_Session *rtp_sess_head=dec_args->rtp_sess_head;
	struct RTP_Session *rtp_sess;
	struct timeval tvsleep, tvdiff; 
	struct timeval tvstart, tvcheck, tvstop;
	struct timeval tvsel, tvbody;
	struct timeval tv_elapsed;
	struct timeval tv_sys_buff;
	double ts_elapsed;
	long int select_usec, body_usec, diff_usec, offset_usec=0;
	unsigned short cycles=SYS_BUFF;
	struct Stream_Source *stm_src;
	rtp_pkt *pkt;
	int len=0;
	int sys_buff=SYS_BUFF;
	struct audio_buff *audio_buffer = global_audio_buffer;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
/*	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); */
	pthread_cleanup_push(dec_clean, (void *)audio_buffer);
	
	tvdiff.tv_sec=tvsleep.tv_sec=dec_args->startime.tv_sec;
	tvdiff.tv_usec=tvsleep.tv_usec=dec_args->startime.tv_usec;

	tv_sys_buff.tv_sec=0;
	tv_sys_buff.tv_usec=SYS_BUFF*GRAIN*1000;

	pthread_mutex_lock(&(dec_args->syn));
	pthread_mutex_unlock(&(dec_args->syn));
	
	select(0, NULL, NULL, NULL, &tvsleep);

	while(1) {

		gettimeofday(&tvstart, NULL);
		
/*		printf("  sum: %7ld - select: %7ld - body: %7ld - diff: %7ld - offset: %7ld - sleep %7ld - cycles: %3hu \n",\
				select_usec + body_usec, select_usec, body_usec, diff_usec, offset_usec, tvdiff.tv_usec, cycles);
*/
		do { 
			for (rtp_sess=rtp_sess_head; rtp_sess; rtp_sess=rtp_sess->next)
			for (stm_src=rtp_sess->ssrc_queue; stm_src; stm_src=stm_src->next){
				if(stm_src->po.potail >= 0){

					pkt=(rtp_pkt *)(*(stm_src->po.bufferpool)+stm_src->po.potail);
				/*	
					uiprintf("Version Number:%d\n", pkt->ver);
					uiprintf("Payload Type:%d\n", pkt->pt);
					uiprintf("Sequence Number:%d\n", ntohs(pkt->seq));
					uiprintf("SSRC Number:%lu\n", ntohl(pkt->ssrc));
					uiprintf("RTP Timestamp:%lu\n", ntohl(pkt->time));
				*/	
					ts_elapsed=((double)(ntohl(pkt->time) - stm_src->ssrc_stats.firstts))/(double)rtp_pt_defs[pkt->pt].rate;
					tv_elapsed.tv_sec=(long)ts_elapsed;
					tv_elapsed.tv_usec=(long)((ts_elapsed-tv_elapsed.tv_sec)*1000000);

					timeval_add(&tv_elapsed, &(stm_src->ssrc_stats.firsttv), &tv_elapsed);
					timeval_add(&tv_elapsed, &tv_elapsed, &(dec_args->startime));
					timeval_subtract(&tv_elapsed, &tv_elapsed, &tv_sys_buff);
					
					if(cycles || timeval_subtract(NULL, &tv_elapsed, &tvstart) ){
					
						len= (stm_src->po.pobuff[stm_src->po.potail]).pktlen -\
							((void *)(pkt->data)-(void *)pkt) - pkt->cc - ((*(((uint8 *)pkt)+len-1)) * pkt->pad);
						if ((len != 0) && (decoders[pkt->pt] != NULL)) {
							decoders[pkt->pt](((uint8 *)pkt->data + pkt->cc), len, (uint8 *(*)(uint32))ab_get);
#ifndef HAVE_SDL
							oss_play();
#endif
							if(!sys_buff) {
								audio_play();
								sys_buff--;
							}
							else if(sys_buff > 0){
								sys_buff--;
							}
						}
/*
				 		uiprintf("\rPlayout Buffer Status: %4.1f %% full - pkt data len: %d   ",\
								(((float)((rtp_sess->bp).flcount)/(float)BP_SLOT_NUM)*100.0), len);
*/				
						bprmv(&(rtp_sess->bp), &(stm_src->po), stm_src->po.potail);
					}
				}
			}
		} while( cycles-- > 0);
		cycles=0;
		
		gettimeofday(&tvstop, NULL);
		
		timeval_subtract(&tvbody, &tvstop, &tvstart);
		
		if((body_usec=tvbody.tv_sec*1000000+tvbody.tv_usec) > (GRAIN*1000-offset_usec)){
			cycles=(body_usec+offset_usec)/(GRAIN*1000);
			offset_usec=(body_usec+offset_usec)%(GRAIN*1000);
		} else {
			
			tvdiff.tv_sec=tvsleep.tv_sec=tvbody.tv_sec;
			tvdiff.tv_usec=tvsleep.tv_usec=GRAIN*1000-tvbody.tv_usec-offset_usec;
			if (tvdiff.tv_usec < 0){
				tvdiff.tv_usec+=1000000;
				tvdiff.tv_sec--;
			}
			diff_usec=tvdiff.tv_sec*1000000+tvdiff.tv_usec;
			if ( tvsleep.tv_usec > 10000 )
				select(0, NULL, NULL, NULL, &tvsleep);
			gettimeofday(&tvcheck, NULL);

			timeval_subtract(&tvsel, &tvcheck, &tvstop);
			if((select_usec=tvsel.tv_sec*1000000+tvsel.tv_usec) > diff_usec){
				cycles=(select_usec-diff_usec)/(GRAIN*1000);
			}
				offset_usec=(select_usec-diff_usec)%(GRAIN*1000);
		}
		cycles+=get_sys_buff();

		
	}
	
	pthread_cleanup_pop(1);
}