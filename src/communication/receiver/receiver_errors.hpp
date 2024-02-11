#ifndef COMMUNICATION_RECEIVER_RECEIVERERRORS_H_
#define COMMUNICATION_RECEIVER_RECEIVERERRORS_H_

#include "stdint.h"

namespace communication {
namespace receiver {

typedef enum {
	ERROR_NONE,
	ERROR_SIGNAL,
	ERROR_PREAMBLE,
	ERROR_UNKNOW_COMMAND,
	ERROR_INVALID_LENGTH,
	ERROR_BUSY,
	ERROR_CRC
} Error;
constexpr int error_count = 6;

#ifdef UNITTEST
void ReceiverErrorsTearDown();
#endif

void raiseError(const Error error);

}}
#endif // COMMUNICATION_RECEIVER_RECEIVERERRORS_H_
