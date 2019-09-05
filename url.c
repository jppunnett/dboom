/* url.c
 * Contains functions for working with URLs
 */
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* Experimenting with debugging macros from Zed Shaw */
#include "dbg.h"
#include "url.h"

/*
struct url_parts {
    char *scheme;
    char *host;
    int  port;
    char *resource;
};
*/

    /* 
     * http://www.cplusplus.com/reference/cstring/strstr/
     */

int
parse_url(const char* url, struct url_parts *parts) {
    assert(url);
    assert(parts);
    
    char *pch = NULL;
    int len = 0;
    int i = 0;
    int valid_scheme = 0;

    /* Extract scheme */
    pch = strchr(url, ':');
    /* Scheme marker not found */
    check(pch != NULL, "No scheme marker. Looks like invalid URL.");
    len = pch - url;
    check(len != 0, "No scheme specified.");
    /* Store the scheme */
    assert(parts->scheme == NULL && "Expect parts->scheme to be NULL.");
    parts->scheme = calloc(len + 1, sizeof(char));
    check_mem(parts->scheme != NULL);
    strncpy(parts->scheme, url, len);
    parts->scheme[len] = '\0';
    assert(strlen(parts->scheme) == len);
    /* Convert scheme to lowercase to ease comparison */
    for(i = 0; i < len; ++i)
        parts->scheme[i] = tolower(parts->scheme[i]); 
    /* We want http or https. Anything else (e.g. ftp) we consider it an
     * error.
     */
    valid_scheme =
        (strcmp("https", parts->scheme) == 0)
        || (strcmp("http", parts->scheme) == 0);
    check(valid_scheme, "Only interested in HTTP/S. Scheme is '%s'", parts->scheme);

    /* Extract port */
    /* Extract host */
    /* Extract resource */

    return 0;

error:
    return -1;
}

