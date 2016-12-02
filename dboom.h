int getRequests(const char*);
int getConcurrentReqs(const char*);
int getTimeout(const char*);
void usage();

coroutine void boom(const char*, unsigned int, int, int, int);
coroutine void stats(int, int);