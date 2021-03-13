#include "mycurl.h"
#include <string.h>
#include <stdlib.h>

struct memory {
	char *buffer;
	size_t len;
};

/*
 * log a message to the syslog and to stdout
 */
int logmsg(const char *format, ...)
{
	char buffer[1024];

	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	buffer[sizeof(buffer) - 1] = '\0';
	va_end(args);

	write(2, buffer, strlen(buffer));
	syslog(LOG_DAEMON|LOG_INFO, "%s", buffer);

	return 0;
}

/* Print the HTTP response */
void print_http_response(struct http_response *resp)
{
	logmsg("SKTEST; IP addr: %s; HTTP resp code: %ld; "
		"name_lookup: %f; connect_time: %f; "
		"app_connect: %f; pre_transfer: %f; "
		"start_transfer: %f; total_time: %f;\n",
		resp->ip, resp->response_code,
		resp->namelookup_time, resp->connect_time,
		resp->app_connect_time,	resp->pre_transfer_time,
		resp->start_transfer_time, resp->total_time);
}

/* Utility function to copy IP address */
void copy_ip_addr(char *dst, char *src)
{
	int i = 0;
	while (src[i] != '\0')
	{
		dst[i] = src[i];
		i++;
	}
	dst[i] = '\0';
}

/* Add the values from the response */
void add_stats(struct http_response *total, struct http_response *resp)
{
	if (resp == NULL)
	{
		return;
	}

	copy_ip_addr(total->ip, resp->ip);
	total->response_code = resp->response_code;
	total->namelookup_time += resp->namelookup_time;
	total->connect_time += resp->connect_time;
	total->app_connect_time += resp->app_connect_time;
	total->pre_transfer_time += resp->pre_transfer_time;
	total->start_transfer_time += resp->start_transfer_time;
	total->total_time += resp->total_time;
}

/* Compute median */
void compute_median(struct http_response *total, int n_requests)
{
	total->namelookup_time /= n_requests;
	total->connect_time /= n_requests;
	total->app_connect_time /= n_requests;
	total->pre_transfer_time /= n_requests;
	total->start_transfer_time /= n_requests;
	total->total_time /= n_requests;
}

size_t write_cb(void *data, size_t size, size_t nmemb, void *userdata)
{
	size_t realsize = size * nmemb;
	struct memory *mem = (struct memory*) userdata;

	//printf("receive chunk of %zu bytes\n", realsize);

	mem->buffer = (char *)realloc(mem->buffer, mem->len + realsize + 1);
	if (mem->buffer == NULL)
		return 0;	/* out of memory */

	memcpy(&mem->buffer[mem->len], data, realsize);
	mem->len += realsize;
	mem->buffer[mem->len] = 0;

	return realsize;
}

/* Extract various statistics from the responses */
void extract_stats(CURL *curl, struct http_response *resp)
{
	char *ip;
	CURLcode res = CURLE_OK;
	//curl_off_t speed_upload, total_time;

	/* get IP address of last connection */
	res = curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &ip);
	if (CURLE_OK == res)
	{
		copy_ip_addr(resp->ip, ip);
	}

	/* get the last response code */
	res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp->response_code);

	/* get the name lookup time */
	res = curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &resp->namelookup_time);

	/* get the time until connect */
	res = curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &resp->connect_time);

	/* get the time until the SSL/SSH handshake is completed */
	res = curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &resp->app_connect_time);

	/* get the time until the file transfer */
	res = curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &resp->pre_transfer_time);

	/* get the time until the first byte is received */
	res = curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME, &resp->start_transfer_time);

	/* get total time of previous transfer */
	res = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &resp->total_time);

#if 0
	curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &speed_upload);
	curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total_time);
	logmsg("Speed: %" CURL_FORMAT_CURL_OFF_T " bytes/sec during %"
		CURL_FORMAT_CURL_OFF_T ".%06ld seconds\n",
		speed_upload,
		(total_time / 1000000), (long)(total_time % 1000000));
#endif

	if (res != CURLE_OK)
	{
		logmsg("Returned error code: %d\n", res);
	}
}

/* API to send HTTP request to an URL */
void send_http_request(struct http_request *req, struct http_response *resp)
{
	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *list = NULL;

	if (req == NULL)
	{
		logmsg("HTTP request is NULL! Aborting...\n");
		return;
	}

	struct header_list *temp = req->h_list;

	/* Start a libcurl easy session */
	curl = curl_easy_init();

	struct memory chunk = {.buffer = NULL, .len = 0};

	if (curl)
	{
		/* set options for a curl easy handle */

		/* provide the URL to use in the request */
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, req->req_type);
		curl_easy_setopt(curl, CURLOPT_URL, req->url);
		curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");

		/* ask for an HTTP GET request */
		//curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

		/* add the headers to an slist */
		while (temp != NULL)
		{
			list = curl_slist_append(list, temp->val);
			temp = temp->next;
		}
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

		/* request failure on HTTP response >= 400 */
		curl_easy_setopt (curl, CURLOPT_FAILONERROR, 1L);

		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		/* send all data to this function  */
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);

		/* we pass our 'chunk' struct to the callback function */
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

		/* perform a blocking file transfer */
		res = curl_easy_perform(curl);
		if(res != CURLE_OK)
		{
			logmsg("%d:%s\n", res, curl_easy_strerror(res));
		}
		else
		{
			/* now extract transfer info */
			extract_stats(curl, resp);

			//printf("Total received bytes: %zu\n", chunk.len);
			//printf("Received data:/n%s\n", chunk.buffer);
			free(chunk.buffer);
		}

		curl_slist_free_all(list); /* free the list again */

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
}
