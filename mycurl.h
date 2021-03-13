#ifndef MYCURL_H
#define MYCURL_H

#include <stdio.h>
#include <stdint.h>
#include <curl/curl.h>

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

/* API to send http request to a URL */
int send_http_request(struct http_request *req);
#endif /* MYCURL_H */
