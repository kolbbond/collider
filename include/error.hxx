
#pragma once

//#include "common.hh"
//#include "rat/common/log.hh"
#include <csignal>
#include <ostream>
#include <iostream>
#include <sstream>

#include "typedefs.hh"

#include "Log.hxx"

// macro for error
// parasitic geese over here
// #define collider_throw_line(arg) throw rat_error(arg, __FILENAME__,
/// __FUNCTION__, __LINE__);
// filename macro for shortening the path
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

class collider_error: public std::runtime_error {
private:
	std::string msg;

public:
	// constructor
	collider_error(const std::string& arg, const char* file, const char* function, int line) : std::runtime_error(arg) {
		std::ostringstream o;
		o << KRED << KBLD << "error: " << KNRM << file << ":" << function << ":" << KGRN << KBLD << line << KNRM << ": " << std::endl << " " KYEL << arg << KNRM;
		msg = o.str();
	}

	// destructor
	~collider_error() throw() {}
	const char* what() const throw() override { return msg.c_str(); }
};

class collider_assert {
private:
public:
	// constructor
	collider_assert(bool flag, const std::string& arg, const char* file, const char* function, int line) {
		if(!flag) { throw collider_error(arg, file, function, line); }

		// destructor
		//~collider_assert()  {}
	};
};

// macro for error
#define collider_throw_line(arg) throw collider_error(arg, __FILENAME__, __FUNCTION__, __LINE__);
#define collider_assert(bool, arg) collider_assert(bool, arg, __FILENAME__, __FUNCTION__, __LINE__);

// macro for debug statement
#define collider_cout(a)                                                                                                                                                           \
	do { std::cout << #a " is value " << (a) << std::endl; } while(false)
