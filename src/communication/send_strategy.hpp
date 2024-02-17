#ifndef SEND_STRATEGY_H_
#define SEND_STRATEGY_H_

namespace communication {
namespace send_strategy {
	
enum SendStrategy {
	STRATEGY_SEND_AT_WILL,
	STRATEGY_SEND_ON_DEMAND
};

void setup();
void loop();
void setOnDemand();
SendStrategy get();

}} // End of: namespace communication::send_strategy

#endif
