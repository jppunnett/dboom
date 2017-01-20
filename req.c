/* req.c */
#include <assert.h>
#include <curl/curl.h>
/* For now() */
#include <libdill.h>
#include "req.h"
#include "dboom.h"

/* libcurl write function. Drops all data. */
static size_t
dropDataCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    return size * nmemb;
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
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
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
