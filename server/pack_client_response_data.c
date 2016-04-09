#include <msgpack.h>
#include "common.h"

/**
 * @brief Creates a response protocol message 
 * 
 * @param sbuf msgpack buffer
 * @param op operation name to make 
 * @param message_id message id to use
 * @param fmt format of the response message
 * @param  the respone message parameters
 * @return Packet the resulting protocol message
 */
Packet pack_client_response_data(msgpack_sbuffer* sbuf, char* op,
		int message_id, char* fmt, ...) {
	msgpack_sbuffer_init(sbuf);
	msgpack_packer pk;
	msgpack_packer_init(&pk, sbuf, msgpack_sbuffer_write);

	pack_map_int(REQUEST_TYPE_HDR, SERVICE_REQUEST_RESPONSE, &pk);
	pack_map_int(MESSAGE_ID_HDR, message_id, &pk);

	pack_map_str(OPERATION_HDR, op, &pk);

	msgpack_pack_map(&pk, 1);
	msgpack_pack_str(&pk, 5);
	msgpack_pack_str_body(&pk, REPLY_HDR, 5);

	va_list ap;
	va_start(ap, (const char* )fmt);
	char *p, *sval;
	int ival;

	for (p = fmt; *p; p++) {
		if (*p != '%') {
			putchar(*p);
			continue;
		}
		switch (*++p) {
		case 'd':
			ival = va_arg(ap, int);
			msgpack_pack_int(&pk, ival);
			break;
		case 's':
			sval = va_arg(ap, char *);
			int sval_len = strlen(sval);
			msgpack_pack_str(&pk, sval_len);
			msgpack_pack_str_body(&pk, sval, sval_len);
			break;
		}
	}
	Packet ret;
	ret.buffer = sbuf->data;
	ret.len = sbuf->size;
	return ret;
}
