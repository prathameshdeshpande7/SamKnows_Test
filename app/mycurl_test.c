#include "mycurl.h"
#include <stdlib.h>	/* exit */
#include <string.h>	/* memset */
#include <getopt.h>	/* getopt_long */
#include <sys/time.h>	/* gettimeofday */
#include <signal.h>


/* Flag set by ‘--verbose’. */
static int verbose_flag;

/* global variables to deal with incoming signals */
static int end_of_game_pipe[2];

double tv_to_double( struct timeval tv )
{
	return tv.tv_sec + tv.tv_usec * 0.000001;
}

/*
 * realt
 *
 * A little function to calculate the current time (in second since some
 * random date) as a double.
 */

double realt(void)
{
	struct timeval  tv;
	struct timezone ov;

	gettimeofday(&tv, &ov);

	return tv_to_double(tv);
}

/*
 * signal handler
 */
static void control_c_handler(int signum, siginfo_t *si, void *uc_)
{
	static double last_time = 0.0;

	logmsg("pid %d received signal %d", getpid(), signum);

	/* ^C^C */
	if (realt() - last_time < 2.0)
		exit(EXIT_SUCCESS);

	last_time = realt();

	char c = 'x';
	if (end_of_game_pipe[1] >= 0)
		write(end_of_game_pipe[1], &c, 1);
}

static struct header_list *alloc_header(char *optarg)
{
	/* allocate memory for new node */
	struct header_list *node = (struct header_list *)malloc(sizeof(struct header_list));
	if (node == NULL)
	{
		fatal("Out of memory\n");
	}
	return node;
}

/* Add a new header to the front of the list */
static int add_header_to_list(char *optarg, struct header_list **head_ref)
{
	struct header_list *new_header = alloc_header(optarg);
	if (new_header == NULL)
		return -1;

	new_header->val = optarg;
	new_header->next = (*head_ref);
	(*head_ref) = new_header;

	return 0;
}

static void print(struct header_list **head_ref)
{
	struct header_list *temp = (*head_ref);

	while (temp != NULL)
	{
		//printf("Data: %s\n", temp->val);
		temp = temp->next;
	}
}

/* Free the list of headers */
static void free_header_list(struct header_list **head_ref)
{
	struct header_list *current = *head_ref;
	struct header_list *next;

	while (current != NULL)
	{
		next = current->next;
		free(current);
		current = next;
	}
	*head_ref = NULL;
}

int main(int argc, char *argv[])
{
	int c;
	int ret = 0;
	int n_req = 1; /* number of HTTP requests to make, default 1 */
	int requests = 0;
	struct header_list *headers = NULL;
	char *url;
	struct http_request req;
	struct http_response resp;
	struct http_response total;

	memset(&req, 0, sizeof(struct http_request));
	memset(&resp, 0, sizeof(struct http_response));
	memset(&total, 0, sizeof(struct http_response));

	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &set, 0); /* ignore sigpipe */

	struct sigaction s;
	memset(&s, 0, sizeof(s));

	s.sa_flags = SA_SIGINFO; /* not SA_RESTART */
	s.sa_sigaction = control_c_handler;
	sigemptyset(&s.sa_mask);

	sigaction(SIGTERM, &s, 0);

	s.sa_flags = SA_SIGINFO | SA_RESTART;
	sigaction(SIGINT, &s, 0);

	pipe(end_of_game_pipe);

	while (1)
	{
		int option_index = 0;
		static struct option long_options[] =
		{
			/* These options set a flag. */
			{"verbose", no_argument, &verbose_flag, 'v'},
			{"brief",   no_argument, &verbose_flag, 0},
			/* These options don't set a flag.
			 * We distinguish them by their indices. */
			{"header",  required_argument, NULL, 'H'},
			{"request", required_argument, NULL, 'X'},
			{"url",     required_argument, NULL, 'U'},
			{"n",       required_argument, NULL, 'n'},
			{"help",    optional_argument, NULL, 'h'},
			{NULL,      0,                 NULL,  0 }
		};

		c = getopt_long(argc, argv, "vbH:X:U:n:h:012", long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
			case 0:
				logmsg("long option %s", long_options[option_index].name);
				if (optarg)
					logmsg("with arg %s", optarg);
				logmsg("\n");
				break;

			case 1:
				logmsg("regular argument '%s'\n", optarg); /* non-option arg */
				break;

			case 'X':
				logmsg("Request - %s\n", optarg);
				req.req_type = optarg;
				break;

			case 'H':
				logmsg("Header - \"%s\"\n", optarg);
				ret = add_header_to_list(optarg, &headers);
				if (ret == -1)
				{
					logmsg("Error while adding header to the list\n", optarg);
					exit(EXIT_FAILURE);
				}
				break;

			case 'n':
				n_req = atoi(optarg);
				if (n_req < 0)
					n_req = 1;	/* set default for negative requests */
				logmsg("Number of requests - %d\n", n_req);
				break;

			case 'U':
				logmsg("URL - %s\n", optarg);
				req.url = optarg;
				break;

			case 'h':
				logmsg("Usage: %s -H \"Accept: application/json\" -H \"Content-Type: application/json\" -X GET http://google.com\n", argv[0]);
				break;

			case 'v':
				logmsg("Verbose\n");
				verbose_flag = 1;
				break;

			case '?':
				logmsg("Unknown option: %c\n", optopt);
				break;

			case ':':
				logmsg("Missing option for %c\n", optopt);
				break;

			default:
				logmsg("?? getopt returned character code 0x%x ??\n", c);
				break;
		}
	}

	if (verbose_flag)
		puts ("verbose flag is set");

	/* Print any remaining command line arguments (not options). */
	if (optind < argc)
	{
		logmsg("non-option ARGV-elements: ");
		while (optind < argc)
			logmsg("%s ", argv[optind++]);
		putchar ('\n');
	}

	if (req.url == NULL || req.req_type == NULL)
	{
		logmsg("Missing arguments, aborting request!\n");
		exit(EXIT_FAILURE);
	}

	req.h_list = headers;
	req.n_req = n_req;

	for (requests = 0; requests < req.n_req; requests++)
	{
		logmsg("%03d ", requests + 1);
		send_http_request(&req, &resp);
		add_stats(&total, &resp);
		print_http_response(&resp);
	}
	compute_median(&total, req.n_req);
	logmsg("\n");

	logmsg("Median of values\n");
	print_http_response(&total);

	/* free the list of headers */
	free_header_list(&headers);

	exit(EXIT_SUCCESS);
}
