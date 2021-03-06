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
 * libnemesi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with libnemesi; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * */

#include "nemesi/rtp.h"
#include "bufferpool/bufferpool.h"

#define po_get_pkg(index) ((rtp_pkt *) (*(po->bufferpool) + index))
#define SEQ_MAX_JUMP 0xF000
#define SEQ_MAX_NUMBER 0x1000

/*!
* \brief Inserisce un elemento nel Buffer di Playout.
*
* L'inserimento e' ordinato secondo il numero di sequenza del pacchetto RTP.
* Si tratta di un normale inserimento in una lista doppio linkata con i
* collegameti effettuati tramite gli indici del vettore.
*
* \param po Il Buffer Pool corrente.
* \param index L'indice dello slot allocato dalla poget.
* \param cycles I cicli del campo \c SEQ dei pacchetti RTP.
* \return 0
* \see bpget
* \see podel
* \see bufferpool.h
* */
int poadd(playout_buff * po, int index, uint32_t cycles)
{
    int i;
    uint32_t cseq;
    uint32_t r_cycles;

    pthread_mutex_lock(&(po->po_mutex));

    r_cycles = po->cycles;
    i = po->pohead;
    cseq = ntohs(po_get_pkg(index)->seq) + cycles;

    while ( (i != -1) && ((ntohs(po_get_pkg(i)->seq) + r_cycles) > cseq) ) {
        rtp_pkt * current_pkg = po_get_pkg(i);
        rtp_pkt * next_pkg = po_get_pkg(po->pobuff[i].next);

        if( ntohs(next_pkg->seq) - ntohs(current_pkg->seq) >= SEQ_MAX_JUMP) {
           r_cycles -= SEQ_MAX_NUMBER;
        }

        i = po->pobuff[i].next;
    }

    if ( (i != -1) && ((ntohs(po_get_pkg(i)->seq) + r_cycles) == cseq) ) {
        pthread_mutex_unlock(&(po->po_mutex));
        return PKT_DUPLICATED;
    }

    if (i == po->pohead) {    /* head insert */
        po->pobuff[index].next = i;
        po->pohead = index;
        if (i == -1)
            po->potail = index;
        else
            po->pobuff[i].prev = index;
        po->pobuff[index].prev = -1;
        po->cycles = cycles;

        po->pocount++;
    } else {
        if (i == -1) {    /* tail insert */
            i = po->potail;
            po->potail = index;
            po->pobuff[index].next = po->pobuff[i].next;
            po->pobuff[i].next = index;
            po->pobuff[index].prev = i;
        } else {       /* Inserting before the current one (see the while loop) */
            po->pobuff[index].prev = po->pobuff[i].prev;
            po->pobuff[index].next = i;
            po->pobuff[po->pobuff[i].prev].next = index;
            po->pobuff[i].prev = index;
        }
        po->pocount++;
        pthread_mutex_unlock(&(po->po_mutex));
        return PKT_MISORDERED;
    }

//      pthread_cond_signal(&(po->cond_empty));

    pthread_mutex_unlock(&(po->po_mutex));

    return 0;
}
