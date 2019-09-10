/* req.h contains functions for making http requests */
#ifndef DBOOM_REQ_INCLUDED
#define DBOOM_REQ_INCLUDED

/* Forward decl */
struct parsed_url;

struct reqstats {
	/* request time in milliseconds */
	int64_t tm;
	unsigned int http_code;
};

/* MakeRequest: issues a GET for the resource pointed to by url
   Returns 0 if got a response from server, otherwise returns -1.
   reqstats contains server response time and HTTP code.
*/
int MakeRequest(const char*, int, struct reqstats*);

int make_http_request(int sock, struct parsed_url *purl, int timeout, struct reqstats *pstats);

struct reqstats* reqstats_new();

void reqstats_free(struct reqstats *prs);

#endif

