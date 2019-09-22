/* req.c */
#include <assert.h>
#include <libdill.h>
#include <string.h>

#include "req.h"
#include "url.h"


struct reqstats* reqstats_new() {
    return calloc(1, sizeof(struct reqstats));
}

void reqstats_free(struct reqstats *prs) {
    if(prs) free(prs);
}

int
make_http_request(struct parsed_url *purl, unsigned int timeout,
                    struct reqstats *pstats) {
    int rc = 0;
    int err = 0;

    if(purl == NULL || pstats == NULL ) {err = EINVAL; goto exit1;}
    /* Set up the http/s connection */
    int s = happyeyeballs_connect(purl->host, purl->port, now() + 2000);
    if(s < 0) {err = errno; goto exit1;}
    /* Attach TLS if using HTTPS */
    int is_https = strcmp(purl->scheme, "https") == 0;
    if(is_https) {
        s = tls_attach_client(s, now() + 1000);
        /* s could be >= 0 even if there's a TLS error. This can happen if
         * talking SSL to server listing for HTTP only. I think libdill should
         * invalidate the socket in this case but it doesn't.
         */
        if(s < 0) {err = errno; goto exit2;}
        if(errno) {err = errno; goto exit3;}
    }
    /* Attach HTTP protocol */
    s = http_attach(s);
    if(s < 0) {err = errno; goto exit3;}

    /* Send request and start timer */
    int64_t start_req = now();
    rc = http_sendrequest(s, "GET", purl->path ? purl->path : "/",
            now() + 1000);
    if(rc != 0) {err = errno; goto exit4;}

    rc = http_sendfield(s, "Host", purl->host, now() + 1000);
    if(rc != 0) {err = errno; goto exit4;}

    rc = http_sendfield(s, "Connection", "close", now() + 1000);
    if(rc != 0) {err = errno; goto exit4;}

    rc = http_done(s, now() + 1000);
    if(rc != 0) {err = errno; goto exit4;}

    /* Read and store the http server response. */
    char reason[256];
    rc = http_recvstatus(s, reason, sizeof(reason), now() + timeout);
    if(rc < 0) {err = errno; goto exit4;}
    pstats->http_code = rc;

    /* Read the response headers */
    char name[256];
    char value[256];
    while(1) {
        rc = http_recvfield(s, name, sizeof(name), value, sizeof(value),
                now() + 1000);
        if(rc == -1 && ((errno == EPIPE) || (ECONNRESET))) break;
        if(rc != 0) {err = errno; goto exit4;}
    }

    /* Detach http and read response body */
    s = http_detach(s, now() + 1000);
    if(s < 0) {err = errno; goto exit3;}

    char remaining[1024];
    while(1) {
        rc = brecv(s, remaining, sizeof(remaining), now() + 1000);
        if(rc == -1 && ((errno == EPIPE) || (ECONNRESET))) break;
        if(rc != 0) {err = errno; goto exit3;}
    }

    /* Save http request duration. */
    pstats->tm = now() - start_req;

    goto exit3;

exit4:
    s = http_detach(s, now() + 1000);
    assert(s >= 0 && "Error detaching HTTP");
exit3:
    if(is_https) {
        s = tls_detach(s, now() + 1000);
        assert(s >= 0 && "Error detaching TLS");
    }
exit2:
    rc = tcp_close(s, now() + 1000);
    assert(rc == 0 && "Error closing TCP connection.");
exit1:
    errno = err;
    return rc;
}

