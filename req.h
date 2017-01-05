/* req.h contains functions for making http requests */

struct reqstats;

/* MakeRequest issues a GET for the resource pointed to by url */
void MakeRequest(const char*, int, struct reqstats*);
