#ifndef FETCHER_H
#define FETCHER_H

/*
 * Downloads content from a URL.
 *
 * Returns:
 *   Dynamically allocated JSON text on success.
 *   NULL on failure.
 *
 * Caller must free the returned string.
 */
char *fetch_json_from_url(const char *url);

#endif