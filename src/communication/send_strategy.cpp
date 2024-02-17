#include <avr/interrupt.h>

#include "../timestamp.hpp"
#include "../macros.hpp"

#include "send_strategy.hpp"

namespace communication {
namespace send_strategy {
	
PRIVATE volatile  SendStrategy send_strategy = STRATEGY_SEND_ON_DEMAND;
PRIVATE volatile  uint16_t last_request_to_send_timestamp_ms;
PRIVATE constexpr uint16_t send_on_demand_strategy_timeout_ms = 100;

void setup() {
	send_strategy                     = STRATEGY_SEND_ON_DEMAND;
	last_request_to_send_timestamp_ms = timestamp::getMsTimestamp();
}

void loop() {
	if (send_strategy != STRATEGY_SEND_AT_WILL) {
		cli();
		uint16_t current_timestamp_ms = timestamp::getMsTimestamp();
		uint16_t last_timestamp_ms = last_request_to_send_timestamp_ms;
		sei();
		if ((current_timestamp_ms - last_timestamp_ms) > send_on_demand_strategy_timeout_ms) {
			send_strategy = STRATEGY_SEND_AT_WILL;
		}
	}
}

void setOnDemand() {
	last_request_to_send_timestamp_ms = timestamp::getMsTimestamp();
	send_strategy = STRATEGY_SEND_ON_DEMAND;
}

SendStrategy get() {
	return send_strategy;
}

}} // End of: namespace communiction::send_strategy
