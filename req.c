/* req.c */
#include <assert.h>
#include <curl/curl.h>
#include <libdill.h>

#include "req.h"
#include "url.h"
#include "dbg.h"


/* libcurl write function. Drops all data. */
static size_t
dropDataCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    return size * nmemb;
}

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
make_http_request(int http_sock, struct parsed_url *purl, int timeout,
                    struct reqstats *pstats) {
    
    int rc = 0;

    check(http_sock >= 0, "Bad HTTP socket. http_sock = %d", http_sock);
    check(purl != NULL, "purl is NULL.");
    check(pstats != NULL, "pstats is NULL.");
    check(timeout >= 0, "Bad timeout value. timeout = %d", timeout);
    check(pstats->tm == 0, "Expect pstats->tm == 0. pstats->tm");
    check(pstats->http_code == 0,
        "Expect pstats->http_code to be zero. pstats->http_code = %d",
        pstats->http_code);
    
    /* Start timing now */
    int64_t start = now();
    rc = http_sendrequest(http_sock, "GET", purl->path ? purl->path : "/", -1);
    check(rc == 0, "Error sending GET request");
    rc = http_sendfield(http_sock, "Host", purl->host, -1);
    check(rc == 0, "Error sending Host field");
    rc = http_sendfield(http_sock, "Connection", "close", -1);
    check(rc == 0, "Error sending Connection field");
    rc = http_done(http_sock, -1);
    check(rc == 0, "Error sending Connection field");

    /* read the http server response. */
    char reason[256];
    rc = http_recvstatus(http_sock, reason, sizeof(reason), now() + timeout);
    check(rc != -1, "Error receiving status");
    pstats->http_code = rc;
    while(1) {
        char name[256];
        char value[256];
        rc = http_recvfield(http_sock, name, sizeof(name),
                value, sizeof(value), -1);
        if(rc == -1 && ((errno == EPIPE) || (ECONNRESET))) break;
        check(rc == 0, "Error receiving field.");
    }
    http_sock





    return rc;

error:

    return -1;
}

int
MakeRequest(const char* url, int timeout, struct reqstats *rsp)
{
    /* TODO: is this the right way to use the curl easy interface?
       I.e once per request? I doubt it... */

    int rc = 0;
    assert(timeout >= 0);
    assert(rsp->tm == 0);
    assert(rsp->http_code == 0);

    /* Must pass something to dropDataCallback */
    int dummy = 0;

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        /* Follow redirection */ 
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        /* Send all data to this function  */ 
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dropDataCallback);
        /* Pass "dummy" pointer to callback function */ 
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&dummy);
        /* Be a friendly Web citizen and declare who is making the request */
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "dboom/1.0");
        /* CURL request timeout in seconds */
        if(timeout > 0)
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout);
        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            fprintf(stderr, "%s\n", curl_easy_strerror(res));
            rc = -1;
        } else {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            rsp->http_code = http_code;

            double reqtime = 0;
            curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &reqtime);
            /* Convert from curl's unit of measure (seconds) to milliseconds */
            rsp->tm = (int64_t)(reqtime * 1000);
        }
        curl_easy_cleanup(curl);
    }
    return rc;
}
