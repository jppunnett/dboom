static int getRequests(const char*);
static int getConcurrentReqs(const char*);
static int getTimeout(const char*);
static void usage();

coroutine void boom(const char*, unsigned int, int, int, int);
coroutine void stats(int, int);

struct reqstats {
	/* request time in milliseconds */
	int64_t tm;
};
