#include "mycurl.h"
#include <string.h>
#include <stdlib.h>

struct memory {
	char *buffer;
	size_t len;
};

/* Print the HTTP response */
void print_http_response(struct http_response *resp)
{
	fprintf(stdout, "SKTEST; ");
	fprintf(stdout, "IP addr: %s; ", resp->ip);
	fprintf(stdout, "HTTP resp code: %ld; ", resp->response_code);
	fprintf(stdout, "name_lookup: %f; ", resp->namelookup_time);
	fprintf(stdout, "connect_time: %f; ", resp->connect_time);
	fprintf(stdout, "app_connect: %f; ", resp->app_connect_time);
	fprintf(stdout, "pre_transfer: %f; ", resp->pre_transfer_time);
	fprintf(stdout, "start_transfer: %f; ", resp->start_transfer_time);
	fprintf(stdout, "total_time: %f;", resp->total_time);
	fprintf(stdout, "\n");
}

/* Add the values from the response */
void add_stats(struct http_response *total, struct http_response *resp)
{
	if (resp == NULL)
	{
		return;
	}

	total->ip = resp->ip;
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
struct http_response *extract_stats(CURL *curl)
{
	CURLcode res = CURLE_OK;
	struct http_response *resp;
	//curl_off_t speed_upload, total_time;

	resp = (struct http_response *)calloc(1, sizeof(struct http_response));
	if (resp == NULL)
	{
		fprintf(stderr, "Out of memory\n");
		return NULL;
	}

	/* get IP address of last connection */
	res = curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &resp->ip);

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
	fprintf(stderr, "Speed: %" CURL_FORMAT_CURL_OFF_T " bytes/sec during %"
		CURL_FORMAT_CURL_OFF_T ".%06ld seconds\n",
		speed_upload,
		(total_time / 1000000), (long)(total_time % 1000000));
#endif

	if (res != CURLE_OK)
	{
		fprintf(stderr, "Returned error code: %d\n", res);
		free(resp);
		return NULL;
	}
	return resp;
}

/* API to send HTTP request to an URL */
struct http_response *send_http_request(struct http_request *req)
{
	CURL *curl;
	CURLcode res = CURLE_OK;
	struct curl_slist *list = NULL;
	struct http_response *resp = NULL;

	if (req == NULL)
	{
		fprintf(stderr, "HTTP request is NULL! Aborting...\n");
		return NULL;
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
			fprintf(stderr, "%d:%s\n", res, curl_easy_strerror(res));
		}
		else
		{
			/* now extract transfer info */
			resp = extract_stats(curl);

			//printf("Total received bytes: %zu\n", chunk.len);
			//printf("Received data:/n%s\n", chunk.buffer);
			free(chunk.buffer);
		}

		curl_slist_free_all(list); /* free the list again */

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	return resp;
}
