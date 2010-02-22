#ifndef SINK_EXAMPLE_H_
#define SINK_EXAMPLE_H_

typedef struct _example_context
{
	nmsg_io_t io;
	nmsg_input_t input;
	nmsg_output_t output;	
	unsigned long count;
	pthread_mutex_t lock;
	pthread_t example_tid;
	int running;
} example_context_t;

#endif
