/* * 
 *  $Id$
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

/*! \file rtsp.h
 * \brief Header contenente le definizioni della libreria \b rtsp.
 * */

/*! \defgroup RTSP RTSP Library
 *
 * \brief Implementazione del protocollo Real Time Streaming Protocol (RTSP) -
 * rfc 2326.
 *
 * Il modulo RTSP si occupa della richiesta degli stream multimediali e della
 * negoziazione dei parametri con il server.  Una volta superata la fase
 * iniziale, si comporta come un <em>telecomando remoto</em> per il controllo
 * del flusso multimediale.
 *
 * @{ */

#ifndef __RTSP_H
#define __RTSP_H

#include <config.h>

#include <stdio.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>

#include <nemesi/sdp.h>
#include <nemesi/cc.h>
#include <nemesi/rtp.h>
#include <nemesi/rtcp.h>
#include <nemesi/wsocket.h>
#include <nemesi/utils.h>
#include <nemesi/comm.h>

/*! Default RTSP port (default setting from rfc) if not explicitly scecified. */
#define RTSP_DEFAULT_PORT 554

/*! Version of implemented protocol. */
#define RTSP_VER "RTSP/1.0"

/*! RTSP End of Line. */
#define RTSP_EL "\r\n"

/*! Lower bound for first RTP port if specified by user. */
#define RTSP_MIN_RTP_PORT 1024

typedef struct {
	int32 first_rtp_port;
} nms_rtsp_hints;

/*! \enum Definition for possible states in RTSP state-machine
 * The last ("STATES_NUM") is used to know how many states are present in the machine.
 */
enum states { INIT, READY, PLAYING, RECORDING, STATES_NUM };
/*! \enum opcodes: enum which codify the available commands for the user.
* "COMMAND_NUM" is the total number of recognized commands.
* "NONE" is a special command, just for internal purpouses
* and not considered elsewhere.
*/
enum opcodes { OPEN, PLAY, PAUSE, STOP, CLOSE, COMMAND_NUM, NONE };

/*!
 * \brief Struttura per la comunicazione dei comandi tra la ui e il modulo
 * RTSP.
 *
 * All'immissione di un comando da console, il modulo di interfaccia utente
 * compila questa struttura impostando il codice del comando e, eventualmente,
 * i relativi parametri.  Il modulo che implementa il protocollo RTSP si occupa
 * di interpretare il comando e avviare la sequenza di procedure adatte, le
 * quali possono essere semplicemente un comando da mandare al sever oppure una
 * serie di operazioni pi� lunghe, come succede ad esempio nel caso di apertura
 * della connessione e impostazione dei parametri, che comporta pi� di un
 * comando RTSP.
 *
 * */
struct command {
	enum opcodes opcode;	/*!< Command code inserted by user. */
	char arg[256];		/*!< Possible command arguments. */
};

/*!
 * \brief Struttura di descrizione di un medium RTSP.
 *
 * Tale struttura � un elemento di una lista single-linked che identificano
 * tutti i media appartenenti ad una sessione RTSP.  Attraverso questa
 * struttura, tramite il puntatore \c rtp_sess, si accede alla sessione RTP a
 * cui appartiene il medium descritto.
 *
 * \note I metodi SETUP e TEARDOWN sono definiti ``a livello medium'', vanno
 * cio� mandati per ognuno dei medium e si deve attendere una risposta dal
 * server per ognuno di essi.
 *
 * \see Medium_info
 * \see rtsp_session
 * */
struct rtsp_medium {
	sdp_medium_info *medium_info;	/*!< Struttura delle informazioni
					  relative al medium. */
	struct rtp_session *rtp_sess;	/*!< Puntatore alla struttura della
					  sessione RTP alla quale appartiene il
					  medium. */
	struct rtsp_medium *next;	/*!< Puntatore al successivo medium
					  della sessione RTSP. */
	char *filename;			/*!< Identificare del medium. Utilizato
					  per i metodi ``a livello Medium''
					  (SETUP, TEARDOWN). */
};

/*!
 * \brief Struttura di descrizione della sessione RTSP.
 *
 * Elemento della lista delle sessioni RTSP.
 *
 * In questa struttura sono memorizzate tutte le informazioni utili per una
 * sessione RTSP.  Per ogni sessione c'� una coda di media che fanno parte
 * della stessa ``presentazione''.
 *
 * \note Imetodi PLAY, PAUSE, RECORD sono definiti ``a livello sessione'',
 * vanno cio� mandati per ogni sessione attiva. In caso di media aggregati,
 * essi faranno parte della stessa sessione, quindi un metodo spedito per una
 * sessione interesser� tutti i media ad essa appartenenti. Ci� � utilissimo,
 * ad esempio, nel caso di audio e video appartenenti ad un unico flusso e che
 * debbano essere sincronizzati.
 *
 * \see rtsp_session_info
 * \see rtsp_medium
 * */
struct rtsp_session {
	uint64 Session_ID;	/*!< Identificatore della sessione RTSP. */
	int CSeq;		/*!< Numero di sequenza dell'ultimo pacchetto
				  RTSP spedito. */
	char *pathname;		/*!< Identificatore della sessione RTSP.
				  Utilizzato per i metodi ``a livello
				  sessione'' (PLAY, PAUSE, RECORD) */
	char *content_base;	/*!< Questo campo � diverso da \c NULL se �
				  stato trovato il campo Content-Base
				  nell'hearder del pacchetto di risposta al
				  metodo \em DESCRIBE. In tal caso il campo
				  <tt>\ref pathname</tt> della sessione e di
				  tutti gli <tt>\ref rtsp_medium</tt> ad essa
				  appartenenti sono relativi el percorso
				  memorizzato in \c content_base. */
	sdp_session_info *info;	/*!< Struttura delle informazioni
					  relative alla sessione */
	struct rtsp_medium *media_queue;	/*!< Puntatore alla code dei
						  media appertenenti alla
						  sessione */
	struct rtsp_session *next;	/*!< Puntatore alla sessione successiva
					  della coda */
	/*****************************/
	/* Da non usare. MAI         */
	/* SOLO PER INIZIALIZZAZIONE */
	/*****************************/
	char *body;		/*!< Puntatore alla zona di memoria dentro la
				  quale c'� il corpo della risposta alla
				  describe mandata dal server. Questa zona di
				  memoria non � MAI direttamente utilizzata. �
				  utilizzata solamente nella fase di settaggio
				  dei puntatori della struttura delle
				  informazioni come spiegato nella nota della
				  documentazione relativa alla struttura
				  <tt>\ref rtsp_session_info</tt>. */
};

/*!
 * \brief Buffer di input per i pacchetti RTSP.
 *
 * Questa struttura rappresenta il buffer in cui vengono immagazzinati i
 * pacchetti letti sulla connessione RTSP.  Essendo RTSP un protocollo
 * testuale, � possibile che un suo pacchetto sia frammentato in pi� pacchetti
 * del livello di trasporto sottostante (TCP). Questo ci costringe a
 * controllare, prima di poterlo processare, che il pacchetto RTSP sia arrivato
 * completamente. In caso contrario si deve memorizzare il frammento arrivato e
 * aspettare che il resto arrivi successivamente.
 *
 * */
struct rtsp_buffer {
	int size;		/*!< Dimensione totale del buffer. */
	int first_pkt_size;	/*!< Dimensione del primo pacchetto. */
	char *data;		/*!< Puntatore alla zona dati. */
};

#define RTSP_READY	0
#define RTSP_BUSY	1

/*!
 * \brief Definition of the common part for rtsp_thread and rtsp_ctrl structs
 */
#define RTSP_COMMON_IF \
			int pipefd[2]; \
			pthread_mutex_t comm_mutex; \
			struct command *comm; \
			enum states status;	/*!< Current RTSP state-machine status */ \
			unsigned char busy; /*!< Boolean value identifing if \
						the rtsp module is busy waiting reply from server*/ \
			pthread_t rtsp_tid; \
			char descr_fmt; /* Description format inside RTSP body */ \
			struct rtsp_session *rtsp_queue;/*!< List of active sessions. */

/*!
 * \brief Struttura al vertice della piramide del modulo RTSP.
 *
 * Questa struttura contiene i dati globali del modulo RTSP. Essi sono
 * principalmente lo stato corrente, la porta sulla quale � attiva la
 * connessione, il buffer di input dei pacchetti RTSP e la coda delle sessioni.
 *
 * \note � presente un campo che indica il tipo di flusso multimediale attivo.
 * Questo serve perch� in base al tipo di presentazione supportata dal server
 * il protocollo RTSP assume comporatamenti diversi.
 * 
 * \see rtsp_session
 * \see buffer
 * */
struct rtsp_thread {
	RTSP_COMMON_IF
	nms_rtsp_hints *hints;
	uint16 force_rtp_port;
	pthread_cond_t cond_busy;
	int fd;		/*!< descrittore sul quale � attiva la connessione con
			  il server, dal quale cio�, verranno letti i dati
			  provenienti dal server */
		/*! \enum types enum dei possibili tipi di flussi. */
	enum types { M_ON_DEMAND, CONTAINER } type;	/*!< Tipo di flusso
							  multimediale attivo:
							  Media On Demand o
							  Container. */
	char waiting_for[64];	/*!< Stringa che contiene, eventualmente, la
				  descrizione della risposta che si sta
				  aspettando dal server. */
	char *server_port;	/*!< Porta sulla quale � in ascolto il server.
				 */
	char *urlname;		/*!< URL della richiesta. */
	struct rtsp_buffer in_buffer;	/*!< Buffer di input dei dati. */
	// struct rtsp_session *rtsp_queue;/*!< Lista delle sessioni attive. */
	struct nms_rtp_th *rtp_th;
};

struct rtsp_ctrl {
	RTSP_COMMON_IF
};

//******** interface functions ********************

	// old init function definitions: -|
	// /-------------------------------/
	// |- int init_rtsp(void);
	// \- struct rtsp_ctrl *init_rtsp(void);
struct rtsp_ctrl *rtsp_init(nms_rtsp_hints *);
int rtsp_is_busy(struct rtsp_ctrl *);
void rtsp_wait(struct rtsp_ctrl *);
int rtsp_close(struct rtsp_ctrl *);
int rtsp_open(struct rtsp_ctrl *, char *);
int rtsp_pause(struct rtsp_ctrl *, char);
int rtsp_play(struct rtsp_ctrl *, char *);
int rtsp_uninit(struct rtsp_ctrl *);
// enum states rtsp_status(struct rtsp_ctrl *);
#define rtsp_status(ctrl) ctrl->status
void rtsp_info_print(struct rtsp_ctrl *);
//
//***** ENDOF interface functions ******************

void rtsp_unbusy(struct rtsp_thread *);
int rtsp_reinit(struct rtsp_thread *);
void rtsp_clean(void *);

/*
extern int (*cmd[COMMAND_NUM]) (struct rtsp_thread *, char *);
*/
extern int (*cmd[COMMAND_NUM]) (struct rtsp_thread *, ...);

extern int (*state_machine[STATES_NUM]) (struct rtsp_thread *, short);

void *rtsp(void *);

int send_get_request(struct rtsp_thread *);
int send_pause_request(struct rtsp_thread *, char *);
int send_play_request(struct rtsp_thread *, char *);
int send_setup_request(struct rtsp_thread *);
int send_teardown_request(struct rtsp_thread *);

/*
int play_cmd(struct rtsp_thread *, char *);
int pause_cmd(struct rtsp_thread *, char *);
int stop_cmd(struct rtsp_thread *, char *);
int open_cmd(struct rtsp_thread *, char *);
int close_cmd(struct rtsp_thread *, char *);
*/

int play_cmd(struct rtsp_thread *, ...);
int pause_cmd(struct rtsp_thread *, ...);
int stop_cmd(struct rtsp_thread *, ...);
int open_cmd(struct rtsp_thread *, ...);
int close_cmd(struct rtsp_thread *, ...);

// RTSP packet handling/creation funcs.
int seturlname(struct rtsp_thread *, char *);
int handle_rtsp_pkt(struct rtsp_thread *);
int full_msg_rcvd( struct rtsp_thread *);
int rtsp_recv(struct rtsp_thread *);
int body_exists(char *);
int check_response(struct rtsp_thread *);
int check_status(char *, struct rtsp_thread *);
int set_transport_str(struct rtp_session *, char **);
int get_transport_str(struct rtp_session *, char *);

#define GCS_INIT 0
#define GCS_NXT_SESS 1
#define GCS_NXT_MED 2
#define GCS_CUR_SESS 3
#define GCS_CUR_MED 4
#define GCS_UNINIT 5
void *get_curr_sess(int cmd, ...);
// int get_curr_sess(struct rtsp_thread *, struct rtsp_session **, struct rtsp_medium **);

int set_rtsp_sessions(struct rtsp_thread *, int, char *, char *);
int set_rtsp_media(struct rtsp_thread *);
struct rtsp_session *rtsp_sess_dup(struct rtsp_session *);
struct rtsp_session *rtsp_sess_create(char *, char *);
struct rtsp_medium *rtsp_med_create(int);
int remove_pkt(struct rtsp_thread *);

int init_state(struct rtsp_thread *, short);
int ready_state(struct rtsp_thread *, short);
int playing_state(struct rtsp_thread *, short);
int recording_state(struct rtsp_thread *, short);

int handle_get_response(struct rtsp_thread *);
int handle_setup_response(struct rtsp_thread *);
int handle_play_response(struct rtsp_thread *);
int handle_pause_response(struct rtsp_thread *);
int handle_teardown_response(struct rtsp_thread *);


#endif
/* @} */