
#pragma once

#include <iostream>
#include <sstream>
#include "Log.hxx"
#include <csignal>
#include "error.hxx"


#define GDEBUG

// to store debug utilities
class collider_debug {
private:
	std::string msg;

public:
	// constructor
	collider_debug(const std::string& arg, const char* file, const char* function, int line) {
		std::ostringstream o;
		o << KYEL << KBLD << "debug: " << KNRM << file << ":" << function << ":" << KGRN << KBLD << line << KNRM << ": " << std::endl << " " KYEL << arg << KNRM << std::endl;
		msg = o.str();
		std::cout << msg;
	}

	// destructor
	~collider_debug() throw() {}
	/*
	const char* what() const throw() override {
		return msg.c_str();
	}
    */
};

#define COLLIDER_DEBUG(arg) collider_debug(arg, __FILENAME__, __FUNCTION__, __LINE__);
