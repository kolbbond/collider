// picked up from Van Nugterens project-rat
#pragma once

// general headers
#include <cassert>
#include <memory>
#include <stdio.h>
#include <stdarg.h> /* va_list, va_start, va_arg, va_end */
#include <mutex>
#include <cstdio>
#include <string>

// specific headers
//#include "error.hh"

// Terminal color definitions
// color definitions
#define KNRM "\x1B[0;0m"
#define KBLD "\033[1m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"
#define KPNK "\033[38;2;255;20;200m" // brighter?
#define KBNK "\033[34;5;210m" // orange
#define KORG "\033[38;5;208m"
#define KOGB "\033[5;38;5;208m"

namespace cldr {

// shared pointer definition for Log
typedef std::shared_ptr<class Log> ShLogPr;

// shared pointer definition for no-output log
//typedef std::shared_ptr<class NullLog> ShNullLogPr;

// output types
//enum VerboseType {general,fmm};

// logging to the terminal
class Log {
	// properties
protected:
	// number of indentations
	int num_indent_ = 0;

	// create a mutex
	std::mutex mtx_;

	// methods
public:
	// constructor
	explicit Log();

	// factory
	static ShLogPr create();

	// virtual destructor (obligatory)
	virtual ~Log() {};

	// only change indent
	virtual void msg(const int incr);

	// new line
	virtual void newl();

	// horizontal line
	virtual void hline(const int width, const char ch = '=', const std::string& str1 = {}, const std::string& str2 = {});

	// access to indentation
	virtual int get_num_indent();

	// set indentation
	virtual void set_num_indent(const int num_indent);

	// cancelled flag
	virtual bool is_cancelled() const { return false; }


	// send text to logbook
	virtual void msg(const char* fmt, ...) {
		// lock for thread safety
		mtx_.lock();

		// create indentation
		for(int i = 0; i < num_indent_; i++) std::printf(" ");

		// process arguments and output
		va_list arg;
		va_start(arg, fmt);
		std::vprintf(fmt, arg);
		va_end(arg);

		// unlock
		mtx_.unlock();
	}

	// send text to logbook and change indentation afterwards
	virtual void msg(const int incr, const char* fmt, ...) {
		// lock for thread safety
		mtx_.lock();

		// create indentation
		if(incr != 0)
			for(int i = 0; i < num_indent_; i++) std::printf(" ");

		// process arguments and output
		va_list arg;
		va_start(arg, fmt);
		std::vprintf(fmt, arg);
		va_end(arg);

		// increment indentation
		assert(static_cast<int>(num_indent_) >= -incr);
		if(static_cast<int>(num_indent_) >= -incr) num_indent_ += incr;

		// unlock
		mtx_.unlock();
	}
};

// null logger (no output)
// used as a placeholder when no log is present
class NullLog: public Log {
	// methods
public:
	// constructor
	NullLog() {};

	// factory
	static ShLogPr create() { return std::make_shared<NullLog>(); }

	// send text to logbook
	void msg(const char*, ...) override final {}

	// send text to logbook and change indentation afterwards
	void msg(const int, const char*, ...) override final {}

	// only change indent
	void msg(const int) override final {}

	// new line
	void newl() override final {}

	// access to indentation
	int get_num_indent() override final { return 0; }
};

// like Log but writes to stderr (keeps stdout clean for UCI)
class StderrLog: public Log {
public:
	StderrLog() {};

	// factory
	static ShLogPr create() { return std::make_shared<StderrLog>(); }

	// send text to logbook
	void msg(const char* fmt, ...) override {
		mtx_.lock();
		for(int i = 0; i < num_indent_; i++) std::fprintf(stderr, " ");
		va_list arg;
		va_start(arg, fmt);
		std::vfprintf(stderr, fmt, arg);
		va_end(arg);
		mtx_.unlock();
	}

	// send text to logbook and change indentation afterwards
	void msg(const int incr, const char* fmt, ...) override {
		mtx_.lock();
		if(incr != 0)
			for(int i = 0; i < num_indent_; i++) std::fprintf(stderr, " ");
		va_list arg;
		va_start(arg, fmt);
		std::vfprintf(stderr, fmt, arg);
		va_end(arg);
		assert(static_cast<int>(num_indent_) >= -incr);
		if(static_cast<int>(num_indent_) >= -incr) num_indent_ += incr;
		mtx_.unlock();
	}

	// new line
	void newl() override { std::fprintf(stderr, " \n"); }
};

} // namespace cldr
