/* * 
 *  $Id$
 *  
 *  This file is part of NeMeSI
 *
 *  NeMeSI -- NEtwork MEdia Streamer I
 *
 *  Copyright (C) 2006 by
 *  	
 *  	Giampaolo "mancho" Mancini - giampaolo.mancini@polito.it
 *	Francesco "shawill" Varano - francesco.varano@polito.it
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
#ifdef ENABLE_DECODER_2

pthread_t decoder_tid; // decoder thread ID
int (*decoders[128])(char *, int, nms_output *);

void *decoder(void *args)
{
	struct rtp_thread *rtp_th=(struct rtp_thread *)args;
	rtp_session *rtp_sess_head; // =rtp_th->rtp_sess_head;
	rtp_ssrc *stm_src;
	rtp_frame fr;
	// time vars
	struct timeval startime;
	struct timeval tvstart, tvstop, tvsleep;
	struct timeval tv_elapsed;
	struct timeval tv_min_next;
	// timestamp in ms vars
	double ts_min_next = 0, ts_next;
	// audio/video vars
	char buffering_audio=1;
	float audio_sysbuff=0;
	float video_sysbuff=0;
	
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	/* pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); */

	pthread_cleanup_push(dec_clean, (void *)nms_outc /*audio_buffer */);
	
	pthread_mutex_lock(&(rtp_th->syn));
	pthread_mutex_unlock(&(rtp_th->syn));
	rtp_sess_head=rtp_th->rtp_sess_head;

	gettimeofday(&startime, NULL);
	
	while(!pthread_mutex_trylock(&(rtp_th->syn))) {
		gettimeofday(&tvstart, NULL);
		for (stm_src=rtp_active_ssrc_queue(rtp_sess_head); stm_src; stm_src=rtp_next_active_ssrc(stm_src)) {
			if ( (ts_next=rtp_get_next_ts(stm_src)) < 0 )
				continue;
			f2time(ts_next, &tv_elapsed);
			timeval_add(&tv_elapsed, &tv_elapsed, &startime);
			
			if ( timeval_subtract(NULL, &tv_elapsed, &tvstart) ) {
				if ( !rtp_fill_buffer(stm_src, &fr) && decoders[fr.pt] ) {
					nms_outc->elapsed = ts_next * 1000;
					decoders[fr.pt](fr.data, fr.len, nms_outc);
					if (nms_outc->audio)
						nms_outc->audio->functions->control(ACTRL_GET_SYSBUF, &audio_sysbuff);
					if (nms_outc->video)
						nms_outc->video->functions->control(VCTRL_GET_SYSBUF, &video_sysbuff);

					// AUDIO
					if(buffering_audio) {
						if (audio_sysbuff > /*0.1*/ AUDIO_SYS_BUFF /*0.99*/ ) {
							buffering_audio = 0;
							// start playing audio
							nms_outc->audio->functions->resume();
						}
					}
					// VIDEO
					if((nms_outc->video) && (nms_outc->video->init) && (!nms_outc->video->tid))
						video_th_start(nms_outc);
						
					// status print on screen	
					nms_statusprintf(BUFFERS_STATUS, "Buffers: Net: %4.1f %% - A: %4.1f %% - V: %4.1f ",\
							(((float)((stm_src->rtp_sess->bp).flcount)/(float)BP_SLOT_NUM)*100.0), audio_sysbuff*100.0, video_sysbuff*100.0);
				}
				ts_next=rtp_get_next_ts(stm_src);
			}
			if ( ts_next >= 0 ) {
				if ( ts_min_next ) /* min between stored ts and next pkt's */
					ts_min_next = min(ts_min_next, ts_next);
				else
					ts_min_next = ts_next;
			}
				
		}
		gettimeofday(&tvstop, NULL);
		
		if ( ts_min_next ) { // is there next packet?
			f2time(ts_min_next, &tv_min_next);
//			tv_min_next.tv_sec=(long)ts_min_next;
//			tv_min_next.tv_usec=(long)((ts_min_next-tv_min_next.tv_sec)*1000000);

			timeval_add(&tv_min_next, &tv_min_next, &startime);
			   // timeval_subtract(&tv_min_next, &tv_min_next, &tv_sys_buff);

			if ( !timeval_subtract(&tvsleep, &tv_min_next, &tvstop) && (tvsleep.tv_usec > 10000) ) {
				nms_printf(NMSML_DBG3, "\n\tWe sleep for: %lds e %ldus\n", tvsleep.tv_sec, tvsleep.tv_usec);
				select(0, NULL, NULL, NULL, &tvsleep);
			}
			ts_min_next = 0;

		} else // Empty network playout buffer => take a sleep
			dec_idle();

		pthread_mutex_unlock(&(rtp_th->syn));
	}
	
	pthread_cleanup_pop(1);
	
	return NULL;
}
#endif // ENABLE_DECODER_2