#pragma once
#include <iostream>
#include <stdexcept>

// #define DEBUGE_MODE

#ifdef DEBUGE_MODE
void dprintf (const char* s) {
	while (*s) {
		if (*s == '%' && *++s != '%') {
			throw std::runtime_error("invalid format string: missing arguments");
		}
		std::cout << *s++;
	}
}

template<typename T, typename... Args>
void dprintf (const char* s, const T& value, const Args&... args) {
	while (*s) {
		if (*s == '%' && *++s != '%') {
			std::cout << value;
			return dprintf(++s, args...);
		}
		std::cout << *s++;
	}
	throw std::runtime_error("extra arguments provided to printf");
}

#else
#define dprintf(args...) (void) 0
#endif

inline long long nanotime( void ) {
	timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return ts.tv_sec * 1000000000ll + ts.tv_nsec;
}