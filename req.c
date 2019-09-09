/* req.c */
#include <assert.h>
#include <curl/curl.h>
#include <libdill.h>

#include "req.h"
/* #include "dboom.h" */
#include "url.h"
#include "dbg.h"


struct reqstats {
	/* request time in milliseconds */
	int64_t tm;
	unsigned int http_code;
};

/* libcurl write function. Drops all data. */
static size_t
dropDataCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    return size * nmemb;
}

int
MakeRequest2(int http_sock, struct parsed_url *purl, int timeout,
                struct reqstats *preqs) {
    
    int rc = 0;

    check(http_sock >= 0, "Bad HTTP socket. http_sock = %d", http_sock);
    check(purl != NULL, "purl is NULL.");
    check(preqs != NULL, "preqs is NULL.");
    check(timeout >= 0, "Bad timeout value. timeout = %d", timeout);
    check(preqs->tm == 0, "Expect preqs->tm == 0. preqs->tm");
    check(preqs->http_code == 0,
        "Expect preqs->http_code to be zero. preqs->http_code = %d",
        preqs->http_code);

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
