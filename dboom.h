/* dboom.h */
#ifndef DBOOM_DBOOM_INCLUDED
#define DBOOM_DBOOM_INCLUDED

struct reqstats {
	/* request time in milliseconds */
	int64_t tm;
	unsigned int http_code;
};

#endif
