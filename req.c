/* req.c */
#include <assert.h>
#include <libdill.h>

#include "req.h"
#include "url.h"
#include "dbg.h"


struct reqstats* reqstats_new() {
    struct reqstats *prs = NULL;
    prs = calloc(1, sizeof(struct reqstats));
    check_mem(prs != NULL);
    return prs;

error:
 return NULL;
}

void reqstats_free(struct reqstats *prs) {
    if(prs) free(prs);
}

int
make_http_request(struct parsed_url *purl, int timeout,
                    struct reqstats *pstats) {
    int rc = 0;

    check(purl != NULL, "purl is NULL.");
    check(pstats != NULL, "pstats is NULL.");
    check(timeout >= 0, "Bad timeout value. timeout = %d", timeout);
    
    /* Set up the http/s connection */
    int s = happyeyeballs_connect(purl->host, purl->port, now() + 2000);
    check(s >= 0, "Could not connect to host, %s:%d",
            purl->host, purl->port);

    /* Attach TLS if using HTTPS */
    if(strcmp(purl->scheme, "https") == 0) {
        s = tls_attach_client(s, now() + 1000);
        check(s >= 0, "Could not attach tls protocol.");
    }

    /* Attach HTTP protocol */
    s = http_attach(s);
    check(s >= 0, "Could not attach HTTP protocol.");

    /* Send request and start timer */
    int64_t start_req = now();
    rc = http_sendrequest(s, "GET", purl->path ? purl->path : "/",
            now() + 1000);
    check(rc == 0, "Error sending GET request");
    rc = http_sendfield(s, "Host", purl->host, now() + 1000);
    check(rc == 0, "Error sending Host header");
    rc = http_sendfield(s, "Connection", "close", now() + 1000);
    check(rc == 0, "Error sending Connection header");
    rc = http_done(s, now() + 1000);
    check(rc == 0, "Error in http_done");

    /* Read the http server response. */
    char reason[256];
    rc = http_recvstatus(s, reason, sizeof(reason), now() + timeout);
    check(rc != -1, "Error receiving status");
    pstats->http_code = rc;

    /* Read the response headers */
    char name[256];
    char value[256];
    while(1) {
        rc = http_recvfield(s, name, sizeof(name), value, sizeof(value),
                now() + 1000);
        if(rc == -1 && ((errno == EPIPE) || (ECONNRESET))) break;
        check(rc == 0, "Error reading response headers.");
    }

    /* Detach http and read response body */
    s = http_detach(s, now() + 1000);
    check(s, "Error detaching http protocol");

    char remaining[1024];
    while(1) {
        rc = brecv(s, remaining, sizeof(remaining), now() + 1000);
        if(rc == -1 && ((errno == EPIPE) || (ECONNRESET))) break;
        check(rc == 0, "Error reading response body");
    }

    /* Save http request duration. */
    pstats->tm = now() - start_req;

    if(strcmp(purl->scheme, "https") == 0) {
        debug("detacthing TLS");
        s = tls_detach(s, now() + 1000);
        check(s >= 0, "Could not detach tls protocol.");
    }
    rc = tcp_close(s, now() + 1000);
    check(rc == 0, "Error closing TCP connection.");

    return 0;

error:
    return -1;
}

