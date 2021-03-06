#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#include <arpa/inet.h>
#include <frame/frame_proto.h>

#include "crc.h"

#define SEND_BYTE(out, c) do {						\
	if ((c) == FRAME_START || (c) == FRAME_ESC || (c) == FRAME_RESET) {\
		fputc(FRAME_ESC, (out));				\
		fputc(FRAME_ESC_MASK ^ (c), (out));			\
	} else {							\
		fputc((c), (out));					\
	}								\
} while(0)

ssize_t frame_send(FILE *out, void *data, size_t nbytes)
{
	char *d;
	char *end;
	uint16_t crc = FRAME_CRC_INIT;

	fputc(FRAME_START, out);
	for(d = data, end = d + nbytes; d < end; d++) {
		char c = *d;
		crc = crc_ccitt_update(crc, c);
		SEND_BYTE(out, c);
	}

	crc = htons(crc);

	SEND_BYTE(out, crc >> 8);
	SEND_BYTE(out, crc & 0xff);

	fputc(FRAME_START, out);

	fflush(out);
	return nbytes;
}

ssize_t frame_recv(FILE *in, void *vbuf, size_t nbytes)
{
	uint16_t crc = FRAME_CRC_INIT;
	size_t i;
	char *buf = vbuf;
	bool recv_started = false;
	bool is_escaped = false;

	for(i = 0;;) {
		int data = fgetc(in);

		if (data == EOF) {
			return EOF;
		}

		if (data < 0) {
			return data;
		}

		if (ferror(in)) {
			return -255;
		}

		if (data == FRAME_START) {
			if (recv_started) {
				if (i != 0) {
					/* success */
					if (crc == 0) {
						ungetc(data, in);
						return i - 2;
					} else {
						fprintf(stderr, "crc = %d\n", crc);
						i = 0;
						crc = FRAME_CRC_INIT;
						continue;
					}
				}
			} else {
				recv_started = true;
			}
			continue;
		}

		if (!recv_started)
			continue;

		if (data == FRAME_RESET) {
			/* restart recv */
			i = 0;
			continue;
		}

		if (data == FRAME_ESC) {
			is_escaped = true;
			continue;
		}

		if (is_escaped) {
			is_escaped = false;
			data ^= FRAME_ESC_MASK;
		}

		crc = crc_ccitt_update(crc, (uint8_t)(data & 0xff));

		if (i < nbytes) {
			buf[i] = data;
			i++;
		} else {
			return -ENOSPC;
		}
	}
	return i;
}
