# dboom
Boom using libdill and dsock C libraries.

A C version of "hey" (https://github.com/rakyll/hey) using the excellent:

- libdill (https://github.com/sustrik/libdill) and
- dsock (https://github.com/sustrik/dsock)

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
