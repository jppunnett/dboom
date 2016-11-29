/* dboom is an HTTP load generator written in C using libdill coroutines */
#include <libdill.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "dboom.h"

int main(int argc, char **argv) {

    srand(time(NULL));

    char *requests = NULL;
    char *concurr = NULL;
    char *timeout = NULL;

    int c;
    while((c = getopt(argc, argv, "n:c:t:")) != -1) {
        switch(c) {
        case 'n':
            requests = optarg;
            break;
        case 'c':
            concurr = optarg;
            break;
        case 't':
            timeout = optarg;
            break;
        default:
            usage();
            break;  // Unreachable
        }
    }
    /* Exit if not url provided */
    if(optind == argc) usage();
    /* TODO: Accept > 1 URL */
    const char* url = argv[optind];
    /* Validate option args */
    int nreqs = getRequests(requests, DEFAULT_REQUESTS);
    int nconcurr = getConcurrentReqs(concurr, DEFAULT_CONCURR);
    int ntimeout = getTimeout(timeout, DEFAULT_TIMEOUT);

    printf("Running dboom\n\
        Url: %s\n\
        Total Requests: %d\n\
        Concurrent Requests: %d\n\
        Timeout: %d ms\n", url, nreqs, nconcurr, ntimeout);
    
    /* Each boom() coroutine writes to this channel when done */
    int done_ch = channel(sizeof(int), 0);
    /* The boom() and stat() coroutines check this channel to see if they need
       to stop (e.g. user interruption or runtime error.) */
    int quit_ch = channel(sizeof(int), 0);
    /* Each boom() coroutine uses this channel to send stats. */
    /* TODO: stats channel should include more than response time. */
    int stats_ch = channel(sizeof(int), 0);

    if(done_ch < 0 || quit_ch < 0 || stats_ch < 0) {
        perror("channel() failed");
        exit(EXIT_FAILURE);
    }
    /* Launch nconcurr coroutines, each one sending nreqs/nconcurr requests. */
    for(int i, cr = 0; i < nconcurr; ++i) {
        cr = go(boom(url, nreqs/nconcurr, ntimeout,
                     done_ch, stats_ch, quit_ch));
        if(cr < 0) {
            perror("go() failed");
            exit(EXIT_FAILURE);
        }
    }

    /* Wait for boom() coroutines to end */
    int rc = 0;
    int done = 0;
    for(int i = nconcurr; i > 0; --i) {
        rc = chrecv(done_ch, &done, sizeof(done), -1);
        if(rc != 0) {
            perror("main() - chrecv() failed");
            exit(EXIT_FAILURE);
        }
    }
    exit(EXIT_SUCCESS);
}

coroutine void boom(const char* url, int nreqs, int timeout,
                    int done_ch, int stats_ch, int quit_ch) {
    printf("Entering boom()\n");
    printf("boom: url=%s, nreq=%d, timeout=%d\n\
           done_ch=%d, stats_ch=%d, quit_ch=%d\n", url, nreqs, timeout,
           done_ch, stats_ch, quit_ch);

    /* do work... */
    msleep(now() + (1000 + (rand() % 5000)));
    /* clean up ... */
    printf("Cleaning up boom()\n");
    /* Signal done */
    int done = 1;
    int rc = chsend(done_ch, &done, sizeof(done), -1);
    if(rc != 0) perror("boom() - chsend() failed");
}

void usage()
{
    fprintf(stderr, "Usage: dboom [-n nreqs] [-c nconcurr] [-t timeoutms] URL.\n");
    exit(EXIT_FAILURE);
}

int getRequests(const char *requests, int defaultval)
{
    return defaultval;
}

int getConcurrentReqs(const char *concurr, int defaultval)
{
    return defaultval;
}

int getTimeout(const char *timeout, int defaultval)
{
    return defaultval;
}
