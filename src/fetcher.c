#include "fetcher.h"

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_RESPONSE_SIZE (10 * 1024 * 1024)

typedef struct
{
    char *data;
    size_t size;
} ResponseBuffer;


/* Called by libcurl whenever response data arrives. */
static size_t write_callback(
    void *contents,
    size_t size,
    size_t nmemb,
    void *user_data
)
{
    size_t bytes = size * nmemb;
    ResponseBuffer *buffer = (ResponseBuffer *)user_data;

    if (bytes > MAX_RESPONSE_SIZE - buffer->size)
        return 0;

    char *new_data = realloc(buffer->data, buffer->size + bytes + 1);

    if (new_data == NULL)
        return 0;

    buffer->data = new_data;

    memcpy(buffer->data + buffer->size, contents, bytes);

    buffer->size += bytes;
    buffer->data[buffer->size] = '\0';

    return bytes;
}


char *fetch_json_from_url(const char *url)
{
    CURL *curl;
    CURLcode result;
    long response_code = 0;

    ResponseBuffer buffer = {0};

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK){
        fprintf(stderr,"Failed to initialize libcurl.\n");
        return NULL;
    }

    if (url == NULL ||
        (strncmp(url, "https://", 8) != 0 && strncmp(url, "http://", 7) != 0))
    {
        fprintf(stderr, "Only HTTP and HTTPS URLs are supported.\n");
        return NULL;
    }

    curl = curl_easy_init();

    if (curl == NULL){
        fprintf(stderr,"Failed to create curl handle.\n");
        curl_global_cleanup();
        return NULL ;
    }

        

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    result = curl_easy_perform(curl);

    if (result == CURLE_OK)
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    if (result != CURLE_OK || response_code < 200 || response_code >= 300)
    {
        fprintf(stderr, "Download failed: %s\n",
                result != CURLE_OK
                    ? curl_easy_strerror(result)
                    : "HTTP error");

        free(buffer.data);
        buffer.data = NULL;
    }

    curl_easy_cleanup(curl);

    curl_global_cleanup();
    return buffer.data;
}