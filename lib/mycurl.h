#ifndef MYCURL_H
#define MYCURL_H

#include <stdio.h>
#include <stdarg.h>	/* va_start, va_end */
#include <unistd.h>	/* write */
#include <syslog.h>	/* syslog */
#include <netinet/in.h>	/* INET6_ADDRSTRLEN */
#include <string.h>
#include <curl/curl.h>

#define fatal(why, args...) do { logmsg("fatal error: " why "\n", ## args); abort(); } while (0)

struct header_list	/* list of custom headers passed to the request */
{
	char *val;
	struct header_list *next;
};

struct http_request
{
	int n_req;			/* Number of HTTP requests to execute */
	struct header_list *h_list;	/* list of custom headers passed to the request */
	char *req_type;			/* Type of request - GET, PUT, POST, DELETE */
	char *url;			/* URL to issue the request to */
};

struct http_response
{
	char ip[INET6_ADDRSTRLEN];	/* CURLINFO_PRIMARY_IP */
	long response_code;		/* CURLINFO_RESPONSE_CODE HTTP resp code */
	double namelookup_time;		/* CURLINFO_NAMELOOKUP_TIME */
	double connect_time;		/* CURLINFO_CONNECT_TIME */
	double app_connect_time;	/* CURLINFO_APPCONNECT_TIME */
	double pre_transfer_time;	/* CURLINFO_PRETRANSFER_TIME */
	double start_transfer_time;	/* CURLINFO_STARTTRANSFER_TIME */
	double total_time;		/* CURLINFO_TOTAL_TIME */
};

/* API to send HTTP request to an URL and send
 * back an HTTP response having the response code
 * and various other statistics.
 *
 */
void send_http_request(struct http_request *req,
			struct http_response *resp);

/* Add the values from the response */
void add_stats(struct http_response *total,
		struct http_response *resp);

/* Compute mean */
void compute_mean(struct http_response *total,
			int n_requests);

/* Compute median */
double compute_median(double time[], int n);

/* sort the time array using bubble sort */
void bubble_sort(double time[], int n);

/* Print the HTTP response */
void print_http_response(struct http_response *resp);

/*
 * log a message to the syslog and to stdout
 */
int logmsg(const char *format, ...);

#endif /* MYCURL_H */
