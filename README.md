# dboom
Boom using libdill and libcurl C libraries.

A C version of "hey" (https://github.com/rakyll/hey) using the excellent:

- libdill (https://github.com/sustrik/libdill) and
- libcurl (https://curl.haxx.se/libcurl)

I would have loved to have used dsock, but it's not quite ready for prime time.

## Usage

	dboom [options...] <url>

	Options:

		-n nreqs    Number of requests to send to <url>. Default is 200.
		-c nconcurr Number of concurrent requests. Default is 50.
		-t timeout  Number of milliseconds to wait until timing out.
		            Default is 0, never time out.

## Design Notes

- Each concurrent request is a coroutine
- Stats coroutine to collect statistics
- Should take advantage of multiple cores

## TODO
- review how invoking curl. Can be more efficient?
- update Makefile to use += on CFLAGS and others
- Use Travis
- Update to use latest version of libdill.
