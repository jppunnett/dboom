# dboom
Boom using libdill and libcurl C libraries.

A C version of "hey" (https://github.com/rakyll/hey) using the excellent:

- libdill v2.14 (https://github.com/sustrik/libdill)

## Usage

	dboom [options...] <url>

	Options:

		-n nreqs    Total number of requests to send to <url>. Default is 200.
		-c nconcurr Number of concurrent requests. Default is 50.
		-t timeout  Number of milliseconds to wait until timing out.
		            Default is 0, never time out.
		-v verbose  Output each request's statistics.

## TODO

- Use Travis
- Handle more than one URL
- Take advantage of multiple cores
