/* url.h
 * Helpers for working with URLs.
 */
#ifndef DBOOM_URL_INCLUDED
#define DBOOM_URL_INCLUDED

struct url_parts {
    char *scheme;
    char *host;
    int  port;
    char *resource;
};



/* parse_url: parses a URL string into its parts.
 * Returns 0 if successful, otherwise returns -1.
 */

int parse_url(const char *url, struct url_parts *parts);

#endif
