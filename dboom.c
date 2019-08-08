/* dboom is an HTTP load generator written in C using libdill coroutines */
#include <libdill.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>

#include "dboom.h"
#include "req.h"

#define DEFAULT_REQUESTS    10
#define DEFAULT_CONCURR     5
#define DEFAULT_TIMEOUT     5000    // ms

static unsigned int getRequests(const char*);
static unsigned int getConcurrentReqs(const char*);
static int getTimeout(const char*);
static void usage();
static struct reqstats reqstats_new();

coroutine void boom(const char*, unsigned int, int, int[2]);
coroutine void stats(int[2], int);

int main(int argc, char **argv) {

    char *requests = NULL;
    char *concurr = NULL;
    char *timeout = NULL;
    int verbose = 0;
    
    int c;
    while((c = getopt(argc, argv, "n:c:t:v")) != -1) {
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
        case 'v':
            verbose = 1;
            break;
        default:
            usage();
            break;  // Unreachable
        }
    }

    /* Exit if no url provided */
    if(optind == argc) usage();

    /* Grab URL. TODO: Accept > 1 URL */
    const char* url = argv[optind];

    /* Validate program args */
    unsigned int nreqs = getRequests(requests);
    unsigned int nconcurr = getConcurrentReqs(concurr);
    int ntimeout = getTimeout(timeout);
    if(nreqs == 0 || nconcurr == 0) {
        fprintf(stderr,
            "The number of requests (%d) and the number of"
             "concurrent requests (%d) must be greater than 0.\n",
             nreqs, nconcurr);
        exit(EXIT_FAILURE);
    }

    /* The number of requests cannot be less than the number of concurrent
       requests. */
    if(nreqs < nconcurr) {
        fprintf(stderr,
            "The number of requests (%d) cannot be less than the number of "
             "concurrent requests (%d)\n", nreqs, nconcurr);
        exit(EXIT_FAILURE);
    }

    printf("Running dboom\n\
        Url: %s\n\
        Total Requests: %d\n\
        Concurrent Requests: %d\n\
        Timeout: %d ms\n", url, nreqs, nconcurr, ntimeout);
    
    /* Record start time */
    time_t start_t, end_t;
    time(&start_t);

    int rc = 0;

    /* Each boom() coroutine uses this channel to record statistics. */
    int stats_ch[2];
    rc = chmake(stats_ch);
    if(rc != 0) {
        perror("Failed to make stats channel");
        exit(EXIT_FAILURE);
    }

    /* Launch coroutine for recording statistics */
    int stats_bun = go(stats(stats_ch, verbose));
    if(stats_bun < 0) {
        perror("Could not start stats coroutine");
        exit(EXIT_FAILURE);
    }

    /* Launch nconcurr coroutines, each one sending nreqs/nconcurr requests. */
    int boom_bun = bundle();
    if(boom_bun < 0) {
        perror("Could not create bundle for boom coroutins.");
        exit(EXIT_FAILURE);
    }

    int i;
    for(i = 0; i < nconcurr; ++i) {
        rc = bundle_go(boom_bun, boom(url, nreqs/nconcurr, ntimeout, stats_ch));
        if(rc != 0) {
            perror("Could not launch boom coroutine");
            exit(EXIT_FAILURE);
        }
    }

    /* Wait for boom() coroutines to end */
    rc = bundle_wait(boom_bun, -1);
    if(rc != 0) {
        perror("Failed when waiting for boom coroutins to complete");
        exit(EXIT_FAILURE);
    }

    /* Close stats channel, causing the stats coroutine to end */
    rc = chdone(stats_ch[1]);
    if(rc != 0) {
        perror("Failed to close stats channel");
        exit(EXIT_FAILURE);
    }

    /* Wait for stats() coroutine to end */
    rc = bundle_wait(stats_bun, -1);
    if(rc != 0) perror("Failed while waiting for stats coroutine to end");
    
    /* Print run time */
    time(&end_t);
    printf("Run time: %fs\n", difftime(end_t, start_t));
    
    exit(EXIT_SUCCESS);
}

coroutine void boom(const char* url, unsigned int nreqs, int timeout,
                    int stats_ch[2]) {
    int rc = 0;
    /* Send requests until no more requests */
    int i;
    for(i = nreqs; i > 0; --i) {
        struct reqstats rs = reqstats_new();
        if(MakeRequest(url, timeout, &rs) == 0) {
            rc = chsend(stats_ch[1], &rs, sizeof(rs), -1);
            if(rc != 0) {
                perror("Failed to send request stats");
                return;
            }
        }
    }
}

coroutine void stats(int stats_ch[2], int verbose)
{
    int rc = 0;
    int nrequests = 0;
    struct reqstats rs;
    unsigned int total = 0;

    while(1) {
        rc = chrecv(stats_ch[0], &rs, sizeof(rs), -1);
        if(rc != 0) {
            if(errno != EPIPE) {
                /* We expect to get EPIPE when main closes stats channel */
                perror("Unexpected error reading stats channel");
            }

            /* Break out for any error */
            break;
        }

        /* Request stats available */
        nrequests++;
        total += rs.tm;
        if(verbose)
            printf("%d,%ld\n", rs.http_code, rs.tm);
    }

    /* Display stats if we have something to display */
    if(nrequests > 0)
        printf("Avg response time for %d requests: %d ms\n", nrequests, total/nrequests);
}

/* Create and initialize a new reqstat struct */
static struct reqstats
reqstats_new()
{
    struct reqstats rs;
    rs.tm = 0;
    rs.http_code = 0;
    return rs;
}

static
void usage()
{
    fprintf(stderr, "Usage: dboom [-n nreqs] [-c nconcurr] [-t timeoutms] URL.\n");
    exit(EXIT_FAILURE);
}

static
unsigned int getRequests(const char *requests)
{
    return requests ? atoi(requests) : DEFAULT_REQUESTS;
}

static
unsigned int getConcurrentReqs(const char *concurr)
{
    return concurr ? atoi(concurr) : DEFAULT_CONCURR;
}

static
int getTimeout(const char *timeout)
{
    return timeout ? atoi(timeout) : DEFAULT_TIMEOUT;
}
