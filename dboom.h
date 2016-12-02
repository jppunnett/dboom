#define DEFAULT_REQUESTS    10
#define DEFAULT_CONCURR     5
#define DEFAULT_TIMEOUT     5000    // ms

int getRequests(const char*, int);
int getConcurrentReqs(const char*, int);
int getTimeout(const char*, int);
void usage();

coroutine void boom(const char*, int, int, int, int);
coroutine void stats(int, int);