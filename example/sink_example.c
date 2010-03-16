/**
 * Simple example program that prints the source IP address of each Defintel sink nmsg to stdout
 *
 * gcc -o sink_example sink_example.c -lnmsg
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include <nmsg.h>

#include "sink_example.h"

/** Global Variables */
example_context_t ctx;

static void
signal_handler(int sig __attribute__((unused)), siginfo_t *info __attribute__((unused)), void *arg __attribute__((unused)))
{
	nmsg_io_breakloop(ctx.io);
}

static int 
setup_signals(void) {
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sigaddset(&sa.sa_mask, SIGINT);
    sigaddset(&sa.sa_mask, SIGTERM);
	sa.sa_flags |= SA_SIGINFO;
    sa.sa_sigaction = &signal_handler;
    if (sigaction(SIGINT, &sa, NULL) != 0) {
		return -1;
    }
    if (sigaction(SIGTERM, &sa, NULL) != 0) {
		return -1;
    }

	return 0;
}

static void *
example_thread(void *arg)
{
	static unsigned long prev;

	pthread_mutex_lock(&ctx.lock);
	while(ctx.running == 1)
	{
		fprintf(stderr, "nmsg/sec: %lu\n", ctx.count - prev);
		prev = ctx.count;
		pthread_mutex_unlock(&ctx.lock);
		// Ugly sleep for 1 second .. replace this with select()
		sleep(1);
		pthread_mutex_lock(&ctx.lock);
	}
}

/**
 * This callback is called from the nmsg io even loop each time a Defintel sink message is received.
 */
static void
callback(nmsg_message_t msg, void *user)
{
	nmsg_res res;
	uint8_t *source_ip;
	size_t len;
	int af;
	char addr_string[INET6_ADDRSTRLEN];

	pthread_mutex_lock(&ctx.lock);
	ctx.count++;
	pthread_mutex_unlock(&ctx.lock);

	/** Get the 'srcip' field from the message. The result is stored in source_ip and the length of the address is stored in len */
	res = nmsg_message_get_field(msg, "srcip", 0, (void **)&source_ip, &len);
	if(res != nmsg_res_success)
	{
		fprintf(stderr, "%s [%d]: nmsg_message_get_field failed to get field 'srcip'\n", __func__, __LINE__);
	}
	else
	{
		/** Determine if the address is IPv4 or IPv6 */
		switch(len)
		{
			case 4:
				af = AF_INET;
				break;
			case 16:
				af = AF_INET6;
				break;
			default:
				af = AF_UNSPEC;
		}

		if(af == AF_UNSPEC)
		{
			fprintf(stderr, "%s [%d]: srcip field length !=4 && !=16\n", __func__, __LINE__);
		}
		else
		{
			if(inet_ntop(af, (void *)source_ip, addr_string, INET6_ADDRSTRLEN) != NULL)
			{
				printf("srcip: %s\n", addr_string);
			}
		}
	}
}

int main(int argc, char **argv)
{
	int fd;
	struct sockaddr_in addr;
	nmsg_res res;
	int opt;
	unsigned long port;

	if(argc < 3)
	{
		fprintf(stderr, "Usage: %s <IP address> <port>\n", argv[0]);
		return 1;
	}

	port = strtoul(argv[2], NULL, 10);
	if(errno == EINVAL)
	{
		fprintf(stderr, "Failed to convert '%s' to integer.\n", argv[2]);
		return 1;
	}

	memset(&ctx, 0, sizeof(example_context_t));
	pthread_mutex_init(&ctx.lock, NULL);

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd < 0)
	{
		fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
		return 1;
	}

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	inet_aton(argv[1], &addr.sin_addr);
	addr.sin_port = htons(port);

	opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

#ifdef SO_REUSEPORT
	opt = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(int));
#endif

	if(bind(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
	{
		fprintf(stderr, "Failed to bind to %s port 8430\n", argv[1], strerror(errno));
		return 1;
	}
	
	/**
	 * libnmsg Initialization
	 */
	res = nmsg_init();
	if(res != nmsg_res_success)
	{
		fprintf(stderr, "Failed to initialize nmsg.\n");
		return 1;
	}

	/**
	 * Setup the signal handler
	 */
	if(setup_signals() != 0)
	{
		fprintf(stderr, "Failed to setup signals.\n");
		return 1;
	}

	/**
	 * Input Setup
	 */
	ctx.input = nmsg_input_open_sock(fd);
	if(ctx.input == NULL)
	{
		fprintf(stderr, "nmsg failed to open socket\n");
		return 1;
	}

	/**
	 * Set an input filter that will cause the io loop to only process messages
	 * that have the Vendor ID set to 'Defintel' and a message type of 'sink'.
	 */
	res = nmsg_input_set_filter_msgtype_byname(ctx.input, "Defintel", "sink");
	if(res != nmsg_res_success)
	{
		fprintf(stderr, "failed to set input filter on input\n");
		nmsg_input_close(&ctx.input);
		return 1;
	}


	/**
	 * Output Setup
	 */
	ctx.output = nmsg_output_open_callback(&callback, NULL);
	if(ctx.output == NULL)
	{
		fprintf(stderr, "nmsg failed to open callback output\n");
		nmsg_input_close(&ctx.input);
		return 1;
	}

	/**
	 * IO Setup
	 */
	ctx.io = nmsg_io_init();
	if(ctx.io == NULL)
	{
		fprintf(stderr, "nmsg failed to intialize IO\n");
		return 1;
	}

	nmsg_io_add_input(ctx.io, ctx.input, NULL);
	nmsg_io_add_output(ctx.io, ctx.output, NULL);

	ctx.running = 1;
	pthread_create(&ctx.example_tid, NULL, &example_thread, NULL);

	/**
	 * Run the io event loop
	 * This will not return until nmsg_io_breakloop() is called from the signal handler
	 */
	nmsg_io_loop(ctx.io);

	fprintf(stderr, "\nShutting down.\n");

	/** 
	 * Destroy the nmsg io
	 */
	nmsg_io_destroy(&ctx.io);

	pthread_mutex_lock(&ctx.lock);
	fprintf(stderr, "Stopping example thread.\n");
	ctx.running = 0;
	pthread_mutex_unlock(&ctx.lock);
	pthread_join(ctx.example_tid, NULL);

	fprintf(stderr, "Total messages processed: %lu\n", ctx.count);

	return 0;	
}
