#ifdef UNITTEST
#include <cstring>
#endif

#include "receiver_errors.hpp"

namespace communication {
namespace receiver {

volatile uint8_t error_counts[error_count] = { 0 };
volatile bool    must_reset = false;

#ifdef UNITTEST
void ReceiverErrorsTearDown() {
	memset((void*)&error_counts, 0, sizeof(error_counts));
	must_reset = false;
}
#endif

void raiseError(const Error error) {
	if(error_counts[error] != 0xff) {
		error_counts[error]++;
	}
}

}}
