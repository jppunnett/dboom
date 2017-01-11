/* req.h contains functions for making http requests */
#ifndef DBOOM_REQ_INCLUDED
#define DBOOM_REQ_INCLUDED

/* Forward decl */
struct reqstats;

/* MakeRequest: issues a GET for the resource pointed to by url */
void MakeRequest(const char*, int, struct reqstats*);

#endif

