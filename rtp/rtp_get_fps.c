/* * 
 * This file is part of libnemesi
 *
 * Copyright (C) 2007 by LScube team <team@streaming.polito.it>
 * See AUTHORS for more details
 * 
 * libnemesi is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * NetEmbryo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libnemesi; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * */

#include <nemesi/rtp.h>

/**
 *  Guess the fps based on what is available on the buffer.
 *  @param stm_src an active ssrc
 *  @return fps
 */

int rtp_get_fps(rtp_ssrc * stm_src)
{
    int i, pt, fps = 0;
    unsigned len;
    rtp_pkt *pkt = rtp_get_pkt(stm_src, &len);
    long timestamp;

    for (i = 0; pkt ; pkt = rtp_get_n_pkt(stm_src, &len, ++i)) {
        pt = RTP_PKT_PT(pkt);
        timestamp = RTP_PKT_TS(pkt);

        fps = stm_src->rtp_sess->ptdefs[pt]->fps = 
        (double) stm_src->rtp_sess->ptdefs[pt]->rate/
            abs(timestamp - stm_src->rtp_sess->ptdefs[pt]->prev_timestamp);
        stm_src->rtp_sess->ptdefs[pt]->prev_timestamp = timestamp;
    }

    return fps;
}
