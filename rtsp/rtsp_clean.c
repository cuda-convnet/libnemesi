#include <nemesi/rtsp.h>
#include <nemesi/etui.h>

#include <fcntl.h>

void rtsp_clean(void *rtsp_arguments)
{
	struct RTSP_args *rtsp_args = (struct RTSP_args *)rtsp_arguments;
	struct RTSP_Thread *rtsp_th = ((struct RTSP_args *)rtsp_args)->rtsp_th;
	int n;
	char optstr[256];
	struct command *comm = ((struct RTSP_args *) rtsp_args)->comm;
	int command_fd = ((struct RTSP_args *) rtsp_args)->pipefd[0];
	char ch[1];

	if ( (n = fcntl(command_fd, F_GETFL, 0)) < 0 )
		uiprintf("fcntl F_GETFL error\n");
	n |= O_NONBLOCK;
	 if (fcntl(command_fd, F_SETFL, n) < 0)
		 uiprintf("fcntl F_SETFL error\n");
#if 1
	if((n=read(command_fd, ch, 1)) == 1)
		if(cmd[comm->opcode] (rtsp_th, comm->arg))
			return;
	if( (*(rtsp_th->waiting_for)) && (rtsp_th->fd != -1) ) {
		if ((n=rtsp_recv(rtsp_th)) < 0)
			uiprintf("No teardown response received\n");
		else if (n > 0){
			if (full_msg_rcvd(rtsp_th))
				if (handle_rtsp_pkt(rtsp_th))
					uiprintf("\nError!\n");
		} else {
			uiprintf("\nServer died prematurely!\n");
			rtsp_th->busy=0;
		}
	}
#endif
/*	reinit_rtsp(rtsp_th); */
	uiprintf("RTSP Thread R.I.P.\n");
#if 1
	fprintf(stderr, "\r"); /* TODO Da ottimizzare */
	while((n=read(UIINPUT_FILENO, optstr, 1)) > 0)
		write(STDERR_FILENO, optstr, n);
#endif
}