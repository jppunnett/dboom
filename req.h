/* req.h contains functions for making http requests */
#ifndef DBOOM_REQ_INCLUDED
#define DBOOM_REQ_INCLUDED

/* Forward decl */
struct reqstats;
struct parsed_url;

/* MakeRequest: issues a GET for the resource pointed to by url
   Returns 0 if got a response from server, otherwise returns -1.
   reqstats contains server response time and HTTP code.
*/
int MakeRequest(const char*, int, struct reqstats*);

int MakeRequest2(int sock, struct parsed_url *purl, int timeout, struct reqstats *preqs);
#endif

