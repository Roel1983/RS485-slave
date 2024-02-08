#ifndef COMMUNICATION_COMMANDNOTIFIER_H_
#define COMMUNICATION_COMMANDNOTIFIER_H_

template<typename T>
struct SingleBlockCommandNotifierByRef {
	typedef bool(*OnReceivedFunc)(const T&);
	static bool onReceived(void *func, const uint8_t relative_block_nr, const uint8_t * const block) {
		return ((OnReceivedFunc)func)(*((T*)block));
	}
};

template<typename T>
struct SingleBlockCommandNotifierByVal {
	typedef bool(*OnReceivedFunc)(T);
	static bool onReceived(void* func, const uint8_t relative_block_nr, const uint8_t * const block) {
		return ((OnReceivedFunc)func)(*((T*)block));
	}
};

template<typename T>
struct MultipleBlockCommandNotifierByRef {
	typedef bool(*OnReceivedFunc)(const uint8_t relative_block_nr, const T&);
	static bool onReceived(void* func, const uint8_t relative_block_nr, const uint8_t * const block) {
		return ((OnReceivedFunc)func)(relative_block_nr, *((T*)block));
	}
};

template<typename T>
struct MultipleBlockCommandNotifierByVal {
	typedef bool(*OnReceivedFunc)(const uint8_t relative_block_nr, T);
	static bool onReceived(void* func, const uint8_t relative_block_nr, const uint8_t * const block) {
		return ((OnReceivedFunc)func)(relative_block_nr, *((T*)block));
	}
};

template<typename T, uint8_t N>
struct CommandNotifierTrait {
	using Notifier = MultipleBlockCommandNotifierByRef<T>;
};

template<uint8_t N>
struct CommandNotifierTrait<uint8_t, N> {
	using Notifier = MultipleBlockCommandNotifierByVal<uint8_t>;
};

template<uint8_t N>
struct CommandNotifierTrait<uint16_t, N> {
	using Notifier = MultipleBlockCommandNotifierByVal<uint16_t>;
};

template<typename T>
struct CommandNotifierTrait<T, 1> {
	using Notifier = SingleBlockCommandNotifierByRef<T>;
};

template<>
struct CommandNotifierTrait<uint8_t, 1> {
	using Notifier = SingleBlockCommandNotifierByVal<uint8_t>;
};

template<>
struct CommandNotifierTrait<uint16_t, 1> {
	using Notifier = SingleBlockCommandNotifierByVal<uint16_t>;
};

#endif // COMMUNICATION_COMMANDNOTIFIER_H_
