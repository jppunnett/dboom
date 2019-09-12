/* dboom is an HTTP load generator written in C using libdill coroutines */
#include <libdill.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <assert.h>

#include "dboom.h"
#include "req.h"
#include "url.h"
#include "dbg.h"

#define DEFAULT_REQUESTS    10
#define DEFAULT_CONCURR     5
#define DEFAULT_TIMEOUT     5000    // ms

coroutine void boom(struct parsed_url *purl, const char *url, unsigned int nreqs, int timeout,
                    int stats_ch[2]) {
    assert(purl);
    assert(purl->host);
    assert(purl->port != 0);

    int rc = 0;
    int sock = -1;
    int i;
    struct reqstats *prs = NULL;

    /* Set up the http/s connection */
    sock = happyeyeballs_connect(purl->host, purl->port, now() + 2000);
    check(sock >= 0, "Could not connect to host, %s:%d",
            purl->host, purl->port);
    /* Layer TLS on TCP if using HTTPS */
    if(strcmp(purl->scheme, "https") == 0) {
        sock = tls_attach_client(sock, now() + 1000);
        check(sock >= 0, "Could not attach tls protocol.");
    }

    /* Send requests */
    for(i = nreqs; i > 0; --i) {
        prs = reqstats_new();
        /* stats coroutine responsible for freeing stats */
        check_mem(prs != NULL);
        rc = make_http_request(sock, purl, timeout, prs);
        check(rc == 0, "Failed sending HTTP request");
        rc = chsend(stats_ch[1], &prs, sizeof(prs), -1);
        check(rc == 0, "Failed to send request stats");
    }

    /* Fall through */

error:
    if(prs) reqstats_free(prs);

    if(sock >= 0) {
        if(strcmp(purl->scheme, "https") == 0) {
            sock = tls_detach(sock, now() + 5000);
            if(sock < 0) {
                perror("Could not detach tls protocol.");
                return;
            }
        }
        rc = tcp_close(sock, now() + 5000);
        if(rc != 0)
            perror("Error closing TCP connection.");
    }
}

coroutine void stats(int stats_ch[2], int verbose)
{
    int rc = 0;
    int nrequests = 0;
    struct reqstats *prs = NULL;
    unsigned int total = 0;

    while(1) {
        rc = chrecv(stats_ch[0], &prs, sizeof(prs), -1);
        if(rc != 0) {
            if(errno != EPIPE) {
                /* We expect to get EPIPE when main closes stats channel */
                perror("Unexpected error reading stats channel");
            }

            /* Break out for any error */
            break;
        }

        /* Request stats available */
        if(prs != NULL) {
            nrequests++;
            total += prs->tm;
            if(verbose)
                printf("%d,%ld\n", prs->http_code, prs->tm);
            reqstats_free(prs);
        }
    }

    /* Display stats if we have something to display */
    if(nrequests > 0)
        printf("Avg response time for %d requests: %d ms\n", 
            nrequests, total/nrequests);
}

static
void usage()
{
    fprintf(stderr, "Usage:\n\tdboom [-n nreqs] [-c nconcurr] [-t timeoutms] [-v] URL.\n");
    fprintf(stderr, "\t  -v,\tOutput each request's stats.\n");

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

int main(int argc, char **argv) {

    char *requests = NULL;
    char *concurr = NULL;
    char *timeout = NULL;
    int verbose = 0;
    int rc = 0;
    struct parsed_url *purl;
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

    /* Parse URL */
    purl = parse_url(url);
    if(purl == NULL) {
        fprintf(stderr, "Problem parsing URL: %s\n", url);
        exit(EXIT_FAILURE);
    }

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
        rc = bundle_go(boom_bun, boom(purl, url, nreqs/nconcurr, ntimeout, stats_ch));
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

    parse_url_free(purl);
    
    exit(EXIT_SUCCESS);
}

