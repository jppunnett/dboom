# dboom
Boom using libdill and dsock C libraries.

A C version of "hey" (https://github.com/rakyll/hey) using the excellent:

- libdill (https://github.com/sustrik/libdill) and
- libcurl (https://curl.haxx.se/libcurl)

I would have loved to have used dsock, but it's not quite ready for prime time.

## Usage

	dboom [options...] <url>

	Options:

		-n   Number of requests to send to <url>. Default is 200.
		-c   Number of concurrent requests. Default is 50.
		-TBD More to come...

## Design Notes

- Each concurrent request is a coroutine
- Stats coroutine to collect statistics
- Should take advantage of multiple cores
