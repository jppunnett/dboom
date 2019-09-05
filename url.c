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
     * scheme://host:port/resource/
     */


struct parsed_url*
parse_url(const char* url) {
    assert(url);
    
    char *pch = NULL;
    int len = 0;
    int i = 0;
    int valid_scheme = 0;
    struct parsed_url* purl = NULL;

    purl = calloc(1, sizeof(struct parsed_url));
    check_mem(purl);

    /* Extract scheme */
    pch = strchr(url, ':');
    /* Scheme marker not found */
    check(pch != NULL, "No scheme marker. Looks like invalid URL.");
    len = pch - url;
    check(len != 0, "No scheme specified.");
    /* Store the scheme */
    assert(purl->scheme == NULL && "Expect purl->scheme to be NULL.");
    purl->scheme = calloc(len + 1, sizeof(char));
    check_mem(purl->scheme != NULL);
    strncpy(purl->scheme, url, len);
    purl->scheme[len] = '\0';
    assert(strlen(purl->scheme) == len);
    /* Convert scheme to lowercase to ease comparison */
    for(i = 0; i < len; ++i)
        purl->scheme[i] = tolower(purl->scheme[i]); 
    /* We want http or https. Anything else (e.g. ftp) we consider it an
     * error.
     */
    valid_scheme =
        (strcmp("https", purl->scheme) == 0)
        || (strcmp("http", purl->scheme) == 0);
    check(valid_scheme, "Only interested in HTTP/S. Scheme is '%s'", purl->scheme);

    /* Extract port */
    /* Extract host */
    /* Extract resource */

    return purl;

error:
    if(purl) parse_url_free(purl);
    return NULL;
}


void
parse_url_free(struct parsed_url* purl) {
    
    assert(purl != NULL);
    if(purl->scheme) free(purl->scheme);
}
