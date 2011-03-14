#ifndef HJ_PROTO_H_
#define HJ_PROTO_H_

#include <stdint.h>
#ifndef __packed
# define __packed __attribute__((packed))
#endif

/* naming notes:
 * hj : prefix from project name
 * hja: from the avr
 * hjb: from the controlling thing, to the avr
 */

/** packet components **/

/* sent at the start of each packet */
struct hj_pktc_header {
	uint8_t type;
} __packed;

struct hj_pktc_enc {
	uint32_t p;
	uint32_t n;
} __packed;

struct hj_pktc_motor_info {
	uint16_t current;
	struct hj_pktc_enc e;
	int16_t pwr;
	int16_t vel;
} __packed;

/** packets dispatched TO the hj. **/
struct hjb_pkt_set_speed {
	struct hj_pktc_header head;
	/* "vel" of both attached motors, TODO: determine units. */
#define HJ_MOTOR_L 0
#define HJ_MOTOR_R 1
	int16_t vel[2];
} __packed;

struct hjb_pkt_req_info {
	struct hj_pktc_header head;
} __packed;

/* allow decrimenting the encoders to prevent overflow
 *
 * XXX: this is really a bad idea, as we have no confirmation that the packet
 * was properly recieved other than by detecting an approximate drop in encoder
 * tics in the next info packet. */
struct hjb_pkt_enc_dec {
	struct hj_pktc_header head;
	struct hj_pktc_enc e[2];
} __packed;

/** packets returned FROM the hj. **/
struct hja_pkt_info {
	struct hj_pktc_header head;
	struct hj_pktc_motor_info m[2];
} __packed;

struct hja_pkt_timeout {
	struct hj_pktc_header head;
} __packed;

struct hja_pkt_error {
	struct hj_pktc_header head;
	uint8_t errnum;
	uint16_t line;
	char file[6];
} __packed;

/** **/

enum hj_pkt_len {
	HJA_PL_TIMEOUT = sizeof(struct hja_pkt_timeout),
	HJA_PL_INFO = sizeof(struct hja_pkt_info),
	HJA_PL_ERROR = sizeof(struct hja_pkt_error),
	HJB_PL_SET_SPEED = sizeof(struct hjb_pkt_set_speed),
	HJB_PL_REQ_INFO = sizeof(struct hjb_pkt_req_info)
};

union hj_pkt_union {
	struct hja_pkt_timeout a;
	struct hja_pkt_info b;
	struct hja_pkt_error c;
	struct hjb_pkt_set_speed d;
	struct hjb_pkt_req_info e;
	struct hjb_pkt_enc_dec f;
};


#define HJ_PL_MIN sizeof(struct hj_pktc_header)
#define HJ_PL_MAX sizeof(union hj_pkt_union)

enum hj_pkt_type {
	HJA_PT_TIMEOUT = 'a',
	HJA_PT_INFO,
	HJA_PT_ERROR,
	HJB_PT_SET_SPEED,
	HJB_PT_REQ_INFO,
	HJB_PT_ENC_DEC
};

#define HJA_PKT_TIMEOUT_INITIALIZER { .head = { .type = HJA_PT_TIMEOUT } }
#define HJA_PKT_INFO_INITIALIZER { .head = { .type = HJA_PT_INFO } }
#define HJA_PKT_ERROR_INITIALIZER(err) { .head = { .type = HJA_PT_ERROR }, \
	.line = htons(__LINE__), .file = __FILE__, .errnum = htons(err) }
#define HJB_PKT_REQ_INFO_INITIALIZER { .head = { .type = HJB_PT_REQ_INFO } }
#define HJB_PKT_SET_SPEED_INITIALIZER(a,b)		\
	{ .head = { .type = HJB_PT_SET_SPEED},		\
		.vel = { htons(a), htons(b) } }

#endif
