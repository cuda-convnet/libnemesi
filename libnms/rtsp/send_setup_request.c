/* * 
 *  $Id:send_setup_request.c 267 2006-01-12 17:19:45Z shawill $
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

#include <nemesi/rtsp.h>
#include <nemesi/methods.h>

int send_setup_request(rtsp_thread * rtsp_th)
{

	char b[256 + strlen(rtsp_th->urlname)]; //XXX doublecheck
	char *options = NULL;
	rtsp_session *rtsp_sess;
	rtsp_medium *rtsp_med;
	struct sockaddr_storage rtpaddr, rtcpaddr;
	socklen_t rtplen = sizeof(rtpaddr), rtcplen = sizeof(rtcpaddr);
	nms_rtsp_interleaved *p;
	int sock_pair[2];
	unsigned int rnd;

	memset(b, 0, 256 + strlen(rtsp_th->urlname));

	// if ( get_curr_sess(NULL, &rtsp_sess, &rtsp_med))
	if (!(rtsp_sess = get_curr_sess(GCS_CUR_SESS))
	    || !(rtsp_med = get_curr_sess(GCS_CUR_MED)))
		return 1;

	if (!rtsp_th->force_rtp_port) {
		// default behaviour: random port number generation.
		rnd = (rand() % ((2 << 15) - 1 - 5001)) + 5001;

		if ((rnd % 2))
			rnd++;
	} else {
		// RTP port number partially specified by user.
		if (rtsp_th->force_rtp_port % 2) {
			rtsp_th->force_rtp_port++;
			nms_printf(NMSML_WARN,
				   "First RTP port specified was odd number => corrected to %u\n",
				   rtsp_th->force_rtp_port);
		}
		rnd = rtsp_th->force_rtp_port;
	}

	rtsp_med->rtp_sess->transport.type = rtsp_th->default_rtp_proto;
	switch (rtsp_med->rtp_sess->transport.type) {
	case UDP:

		sprintf(b, "%d", rnd);
		server_create(NULL, b, &(rtsp_med->rtp_sess->transport.RTP.fd));

		sprintf(b, "%d", rnd + 1);
		server_create(NULL, b, &(rtsp_med->rtp_sess->transport.RTCP.fd));

		/* per sapere il numero di porta assegnato */
		/* assigned ports */
		getsockname(rtsp_med->rtp_sess->transport.RTP.fd,
			    (struct sockaddr *) &rtpaddr, &rtplen);
		getsockname(rtsp_med->rtp_sess->transport.RTCP.fd,
			    (struct sockaddr *) &rtcpaddr, &rtcplen);

		rtsp_med->rtp_sess->transport.RTP.local_port =
		    ntohs(sock_get_port((struct sockaddr *) &rtpaddr));
		rtsp_med->rtp_sess->transport.RTCP.local_port =
		    ntohs(sock_get_port((struct sockaddr *) &rtcpaddr));

		if (set_transport_str(rtsp_med->rtp_sess, &options))
			return 1;

		// next rtp port forces
		if (rtsp_th->force_rtp_port) {
			rtsp_th->force_rtp_port += 2;
			nms_printf(NMSML_DBG2, "Next client ports will be %u-%u\n",
				   rtsp_th->force_rtp_port,
				   rtsp_th->force_rtp_port + 1);
		}
		break;
	case TCP:
		rtsp_med->rtp_sess->transport.RTP.u.tcp.ilvd =
		    (rtsp_th->next_ilvd_ch)++;

		rtsp_med->rtp_sess->transport.RTCP.u.tcp.ilvd =
		    (rtsp_th->next_ilvd_ch)++;

		if (set_transport_str(rtsp_med->rtp_sess, &options))
			return 1;

		if (!(p = calloc(1, sizeof(nms_rtsp_interleaved)))) {
			nms_printf(NMSML_ERR,
				   "Unable to allocate memory for interleaved struct!\n");
			return 1;
		}
		p->proto.tcp.rtp_ch = rtsp_med->rtp_sess->transport.RTP.u.tcp.ilvd;
		p->proto.tcp.rtcp_ch = rtsp_med->rtp_sess->transport.RTCP.u.tcp.ilvd;
		
		if (socketpair(PF_UNIX, SOCK_DGRAM, 0, sock_pair) < 0) {
			nms_printf(NMSML_ERR,
				   "Unable to allocate memory for interleaved struct!\n");
			free(p);
			return 1;
		}
		rtsp_med->rtp_sess->transport.RTP.fd = sock_pair[0];
		p->rtp_fd = sock_pair[1];

		if (socketpair(PF_UNIX, SOCK_DGRAM, 0, sock_pair) < 0) {
			nms_printf(NMSML_ERR,
				   "Unable to allocate memory for interleaved struct!\n");
			close(rtsp_med->rtp_sess->transport.RTP.fd);
			close(p->rtp_fd);
			free(p);
			return 1;
		}
		rtsp_med->rtp_sess->transport.RTCP.fd = sock_pair[0];
		p->rtcp_fd = sock_pair[1];

		nms_printf(NMSML_DBG1, "Interleaved RTP local sockets: %d <-> %d\n",
			   rtsp_med->rtp_sess->transport.RTP.fd, p->rtp_fd);

		nms_printf(NMSML_DBG1, "Interleaved RTCP local sockets: %d <-> %d\n",
			   rtsp_med->rtp_sess->transport.RTCP.fd, p->rtcp_fd);

		p->next = rtsp_th->interleaved;
		rtsp_th->interleaved = p;

		break;
	case SCTP:
		rtsp_med->rtp_sess->transport.RTP.u.sctp.stream =
		    ++(rtsp_th->next_ilvd_ch);

		rtsp_med->rtp_sess->transport.RTCP.u.sctp.stream =
		    ++(rtsp_th->next_ilvd_ch);

		if (set_transport_str(rtsp_med->rtp_sess, &options))
			return 1;

		if (!(p = calloc(1, sizeof(nms_rtsp_interleaved)))) {
			nms_printf(NMSML_ERR,
				   "Unable to allocate memory for interleaved struct!\n");
			return 1;
		}
		p->proto.sctp.rtp_st = rtsp_med->rtp_sess->transport.RTP.u.sctp.stream;
		p->proto.sctp.rtcp_st = rtsp_med->rtp_sess->transport.RTCP.u.sctp.stream;

		if (socketpair(PF_UNIX, SOCK_DGRAM, 0, sock_pair) < 0) {
			nms_printf(NMSML_ERR,
				   "Unable to allocate memory for interleaved struct!\n");
			free(p);
			return 1;
		}
		rtsp_med->rtp_sess->transport.RTP.fd = sock_pair[0];
		p->rtp_fd = sock_pair[1];

		if (socketpair(PF_UNIX, SOCK_DGRAM, 0, sock_pair) < 0) {
			nms_printf(NMSML_ERR,
				   "Unable to allocate memory for interleaved struct!\n");
			close(rtsp_med->rtp_sess->transport.RTP.fd);
			close(p->rtp_fd);
			free(p);
			return 1;
		}
		rtsp_med->rtp_sess->transport.RTCP.fd = sock_pair[0];
		p->rtcp_fd = sock_pair[1];

		p->next = rtsp_th->interleaved;
		rtsp_th->interleaved = p;

		break;
	default:
		return 1;
	}

	if (rtsp_sess->content_base != NULL)
		sprintf(b, "%s %s/%s %s" RTSP_EL, SETUP_TKN,
			rtsp_sess->content_base, rtsp_med->filename, RTSP_VER);
	else
		sprintf(b, "%s %s %s" RTSP_EL, SETUP_TKN, rtsp_med->filename,
			RTSP_VER);
	sprintf(b + strlen(b), "CSeq: %d" RTSP_EL, ++(rtsp_sess->CSeq));
	sprintf(b + strlen(b), "Transport: %s" RTSP_EL, options);

	if (rtsp_sess->Session_ID)	//Caso di controllo aggregato: � gi� stato definito un numero per la sessione corrente.
		sprintf(b + strlen(b), "Session: %llu" RTSP_EL,
		rtsp_sess->Session_ID);

	strcat(b, RTSP_EL);

	if (!nmst_write(&rtsp_th->transport, b, strlen(b), NULL)) {
		nms_printf(NMSML_ERR, "Cannot send SETUP request...\n");
		return 1;
	}


	sprintf(rtsp_th->waiting_for, "%d.%d", RTSP_SETUP_RESPONSE,
		rtsp_sess->CSeq);

	free(options);

	return 0;
}
