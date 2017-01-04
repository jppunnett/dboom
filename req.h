/* req.h contains functions for making http requests */

/* MakeRequest issues a GET for the resource pointed to by url */
int MakeRequest(const char* url, int timeout);
