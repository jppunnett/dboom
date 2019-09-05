/* url.h
 * Helpers for working with URLs.
 */
#ifndef DBOOM_URL_INCLUDED
#define DBOOM_URL_INCLUDED

struct parsed_url {
    char *scheme;
    char *host;
    int  port;
    char *resource;
};



/* parse_url: parses a URL string into its parts.
 * Returns a pointer to a parsed_url if successful, otherwise returns NULL.
 * The caller must call parse_url_free() once done with the parsed_url.
 */
struct parsed_url* parse_url(const char *url);

/* parse_url_free: cleans up all resouces for a parsed_url.
 */
void parse_url_free(struct parsed_url* purl);

#endif
