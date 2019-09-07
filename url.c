/* url.c
 * Contains functions for working with URLs
 * Did not write these functions for reuse outside of dboom. E.g. parse_url
 * does not care about anything but HTTP/S protocol. Also does not honor
 * user and password in URL.
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

/* Inspired by http://draft.scyphus.co.jp/lang/c/url_parser.html */
struct parsed_url*
parse_url(const char* url) {
    assert(url);
    
    char *pch = NULL;
    char *temp_pch = NULL;
    char *buf = NULL;
    int len = 0;
    int i = 0;
    int valid_scheme = 1;
    struct parsed_url* purl = NULL;

    purl = calloc(1, sizeof(struct parsed_url));
    check_mem(purl);

    /* Extract scheme */
    pch = strstr(url, "://");
    /* Scheme marker not found */
    check(pch != NULL, "No scheme marker. Looks like invalid URL.");
    len = pch - url;
    check(len != 0, "No scheme specified.");
    /* Store the scheme */
    assert(purl->scheme == NULL && "Expect purl->scheme to be NULL.");
    purl->scheme = calloc(len + 1, sizeof(char));
    check_mem(purl->scheme != NULL);
    strncpy(purl->scheme, url, len);
    assert(purl->scheme[len] == '\0');
    assert(strlen(purl->scheme) == len);
    /* Convert scheme to lowercase to ease comparison */
    for(i = 0; i < len; ++i)
        purl->scheme[i] = tolower(purl->scheme[i]); 
    /* We want http or https. Anything else (e.g. ftp) we consider it an
     * error. Set port to standard HTTP or HTTPS at same time.
     */
    valid_scheme = 1;
    if(strcmp("https", purl->scheme) == 0) {
        purl->port = 443;
    }
    else if(strcmp("http", purl->scheme) == 0) {
        purl->port = 80;
    }
    else {
        valid_scheme = 0;
    }
    check(valid_scheme, "Only interested in HTTP/S. Scheme is '%s'", purl->scheme);
    debug("purl->scheme = %s", purl->scheme);

    /* Extract host name */
    /* pch pointing to scheme marker so advance to start of host. */
    pch += 3;
    temp_pch = pch;
    while(*temp_pch != '\0') {
        if(*temp_pch == '/') break;
        if(*temp_pch == ':') break;
        temp_pch++;
    }
    len = temp_pch - pch;
    check(len != 0, "No host specified.");
    /* Store the host name */
    assert(purl->host == NULL && "Expect purl->host to be NULL.");
    purl->host = calloc(len + 1, sizeof(char));
    check_mem(purl->host != NULL);
    strncpy(purl->host, pch, len);
    assert(purl->host[len] == '\0');
    assert(strlen(purl->host) == len);
    debug("purl->host = %s", purl->host);

    /* Extract port */
    if(*temp_pch == ':') {
        temp_pch++;
        pch = temp_pch;
        /* Advance to start of resource to figure out where port ends */
        while(*temp_pch != '\0') {
            if(*temp_pch == '/') break;
            check(isdigit(*temp_pch), "Invalid port character.");
            temp_pch++;
        }
        len = temp_pch - pch;
        check(len != 0, "No port specified.");
        check(len < 6, "Port too big: %d", len);
        /* Save the port and convert to integer */
        buf = calloc(len + 1, sizeof(char));
        check_mem(buf != NULL);
        strncpy(buf, pch, len);
        assert(buf[len] == '\0');
        assert(strlen(buf) == len);
        check(sscanf(buf, "%d", &purl->port) == 1, "Failed to convert port to int");
        free(buf);
    }
    debug("purl->port: %d", purl->port);

    /* Extract path */
    if(*temp_pch == '/') {
        /* Tired of writing this parsing code. Need to test libdill now.
         * We have enough to test libdill sockets at this point--for certain
         * URLs, anyway :)
         *
         * Grab everthing to end of url and treat as path--for now, anyway.
         */
        len = strlen(temp_pch);
        assert(len > 0);
        purl->path = calloc(len + 1, sizeof(char));
        check_mem(purl->path != NULL);
        strncpy(purl->path, temp_pch, len);
        assert(purl->path[len] == '\0');
        assert(strlen(purl->path) == len);
    }
    debug("purl->path: %s", purl->path);

    return purl;

error:
    if(purl) parse_url_free(purl);
    if(buf) free(buf);
    return NULL;
}


void
parse_url_free(struct parsed_url* purl) {
    
    assert(purl);
    if(purl->scheme)    free(purl->scheme);
    if(purl->host)      free(purl->host);
    if(purl->path)      free(purl->path);
    if(purl->query)     free(purl->query);
    if(purl->fragment)  free(purl->fragment);
}

