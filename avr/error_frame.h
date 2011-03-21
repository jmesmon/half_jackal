#ifndef ERROR_FRAME_H_
#define ERROR_FRAME_H_ 1

#include <string.h>

#define hj_send_error(errnum) \
	__hj_send_error(__LINE__, __FILE__, strlen(__FILE__), errnum)

void __hj_send_error(uint16_t line, char *file, size_t flen, uint8_t errnum);
#endif
