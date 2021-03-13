#include "mycurl.h"
#include <string.h>
#include <stdlib.h>

struct memory {
	char *buffer;
	size_t len;
};

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

/* API to send http request to a URL */
int send_http_request(struct http_request *req)
{
	CURL *curl;
	CURLcode res = CURLE_OK;
	long response_code;
	double namelookup;
	double connect;
	double start;
	double total;
	curl_off_t speed_upload, total_time;
	struct curl_slist *list = NULL;

	if (req == NULL)
	{
		fprintf(stderr, "HTTP request is NULL! Aborting...\n");
		return CURLE_FAILED_INIT;
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

			//printf("Total received bytes: %zu\n", chunk.len);
			//printf("Received data:/n%s\n", chunk.buffer);
			free(chunk.buffer);

			/* get the last response code */
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
			printf("Response code: %ld", response_code);

			/* get the name lookup time */
			res = curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &namelookup);
			if (CURLE_OK == res)
			{
				printf("Time: %.1f", namelookup);
			}

			/* get the time until connect */
			res = curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &connect);
			if(CURLE_OK == res)
			{
				printf("Time: %.1f", connect);
			}

			/* get the time until the first byte is received */
			res = curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME, &start);
			if(CURLE_OK == res)
			{
				printf("Time: %.1f", start);
			}

			/* get total time of previous transfer */
			res = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &total);
			if(CURLE_OK == res)
			{
				printf("Time: %.1f\n", total);
			}

			curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &speed_upload);
			curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME_T, &total_time);
			fprintf(stderr, "Speed: %" CURL_FORMAT_CURL_OFF_T " bytes/sec during %"
				CURL_FORMAT_CURL_OFF_T ".%06ld seconds\n",
				speed_upload,
				(total_time / 1000000), (long)(total_time % 1000000));
		}

		curl_slist_free_all(list); /* free the list again */

		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	return (int)res;
}
