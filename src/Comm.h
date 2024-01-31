#ifndef COMM_H
#define COMM_H

#include <stdint.h>

typedef enum {
	COMM_ERROR_NONE,
	COMM_ERROR_SIGNAL,
	COMM_ERROR_DATA,
	COMM_ERROR_BUSY
} comm_error_t;
#ifndef UNITTEST
static_assert(sizeof(comm_error_t) == 1, "comm_error_t must by atomic");
#endif

struct comm_send_message_base_t;
comm_send_message_base_t * const SEND_MESSAGE_NEXT_UNUSED = (comm_send_message_base_t*)1;
struct comm_send_message_base_t {
	comm_send_message_base_t* next = SEND_MESSAGE_NEXT_UNUSED;
	uint8_t                   size;
	uint8_t                   command_id;
	uint8_t                   value[0];
};

template<uint8_t _command_id, typename T>
struct comm_send_message_t : comm_send_message_base_t {
	volatile T value;
};

#ifdef UNITTEST
void CommReset();
#endif

void CommBegin();
void CommLoop();

comm_error_t CommGetError();
void         CommSetDeviceNr(uint8_t device_nr);
void         CommSetStripNr (uint8_t strip_nr);

inline bool CommCanUseSendMessage(comm_send_message_base_t& message);
template<uint8_t command_id, typename T> void CommSend(comm_send_message_t<command_id, T>& send_message);

void CommSetDeviceNr(uint8_t device_nr);
void CommSetStripNr(uint8_t strip_nr);

inline bool CommCanUseSendMessage(comm_send_message_base_t& message) {
	return message.next == SEND_MESSAGE_NEXT_UNUSED;
}

void        CommSend(uint8_t command_id, comm_send_message_base_t& send_message, uint8_t size);

template<uint8_t command_id, typename T>
void CommSend(comm_send_message_t<command_id, T>& send_message) {
	CommSend(command_id, send_message, sizeof(T));
}

#endif
