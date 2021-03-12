#include "mycurl.h"
#include <stdlib.h>	/* for exit */
#include <getopt.h>	/* for getopt_long */

/* Flag set by ‘--verbose’. */
static int verbose_flag;

int main(int argc, char *argv[])
{
	int c;
	int n_requests = 0; /* number of HTTP requests to make */

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
				break;

			case 'H':
				fprintf(stdout, "Header - %s\n", optarg);
				break;

			case 'n':
				fprintf(stdout, "Number of requests - %s\n", optarg);
				n_requests = atoi(optarg);
				break;

			case 'U':
				fprintf(stdout, "URL - %s\n", optarg);
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

	exit(EXIT_SUCCESS);
}
