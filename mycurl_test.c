#include "mycurl.h"
#include <stdlib.h>	/* exit */
#include <string.h>	/* memset */
#include <getopt.h>	/* getopt_long */

/* Flag set by ‘--verbose’. */
static int verbose_flag;

static struct header_list *alloc_header(char *optarg)
{
	/* allocate memory for new node */
	struct header_list *node = (struct header_list *)malloc(sizeof(struct header_list));
	if (node == NULL)
	{
		fprintf(stderr, "Out of memory\n");
		return NULL;
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
	int n_req = 0; /* number of HTTP requests to make */
	struct header_list *headers = NULL;
	char *request_type;
	char *url;
	struct http_request req;
	memset(&req, 0, sizeof(struct http_request));

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

		c = getopt_long(argc, argv, "vbH:X:U:n:h", long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
			case 0:
				fprintf(stdout, "long option %s", long_options[option_index].name);
				if (optarg)
					fprintf(stdout, "with arg %s", optarg);
				fprintf(stdout, "\n");
				break;

			case 1:
				fprintf(stdout, "regular argument '%s'\n", optarg); /* non-option arg */
				break;

			case 'X':
				fprintf(stdout, "Request - %s\n", optarg);
				req.req_type = optarg;
				break;

			case 'H':
				fprintf(stdout, "Header - %s\n", optarg);
				ret = add_header_to_list(optarg, &headers);
				if (ret == -1)
				{
					fprintf(stdout, "Error while adding header to the list\n", optarg);
					exit(EXIT_FAILURE);
				}
				break;

			case 'n':
				n_req = atoi(optarg);
				if (n_req < 0)
					n_req = 1;	/* set default for negative requests */
				fprintf(stdout, "Number of requests provided - %s, passed to http req - %d\n", optarg, n_req);
				req.n_req = n_req;
				break;

			case 'U':
				fprintf(stdout, "URL - %s\n", optarg);
				req.url = optarg;
				break;

			case 'h':
				fprintf(stdout, "Usage: %s -H \"Accept: application/json\" -H \"Content-Type: application/json\" -X GET http://google.com\n", argv[0]);
				break;

			case 'v':
				fprintf(stdout, "Verbose\n");
				break;

			case '?':
				fprintf(stderr, "Unknown option: %c\n", optopt);
				break;

			case ':':
				fprintf(stderr, "Missing option for %c\n", optopt);
				break;

			default:
				fprintf(stdout, "?? getopt returned character code 0x%x ??\n", c);
				break;
		}
	}

	if (verbose_flag)
		puts ("verbose flag is set");

	/* Print any remaining command line arguments (not options). */
	if (optind < argc)
	{
		fprintf (stdout, "non-option ARGV-elements: ");
		while (optind < argc)
			fprintf (stdout, "%s ", argv[optind++]);
		putchar ('\n');
	}

	req.h_list = headers;

	ret = send_http_request(&req);
	fprintf (stdout, "Sent HTTP request with ret code: %d\n", ret);

	/* free the list of headers */
	free_header_list(&headers);

	exit(EXIT_SUCCESS);
}
