#pragma once

#include <string>

#include "typedefs.hh"

namespace cldr {
class Extra {
public:
	// rank and file to sq macros/functions
	// #define FR2SQ64(f, r) ((f) + ((r) * 8)) //Finds 64 square index
	// #define FR2SQ(f, r) ((21 + (f)) + ((r) * 10)) //Finds 120 square index
	static inline u64 rf2sq64(u64 r, u64 f) { return (r * 8) + f; }
	static inline u64 rf2sq120(u64 r, u64 f) { return (r * 10) + f + 21; }
	static inline u64 sq64to120(u64 sq64) {
		u64 r = get_rank64(sq64);
		u64 f = get_file64(sq64);
		return rf2sq120(r, f);
	}
	static inline u64 sq120to64(u64 sq120) {
		u64 r = get_rank120(sq120);
		u64 f = get_file120(sq120);

		return rf2sq64(r, f);
	}

	static inline u64 get_rank64(u64 sq64) {
		u64 r = sq64 / 8;
		return r;
	}

	static inline u64 get_file64(u64 sq64) {
		u64 f = sq64 % 8;
		return f;
	}

	static inline u64 get_rank120(u64 sq120) {
		u64 r = (sq120 - 21) / 10;
		return r;
	}

	static inline u64 get_file120(u64 sq120) {
		u64 f = (sq120 - 21) % 10;
		return f;
	}

	static inline u64 char2rank(char c) {
		// '1' -> 0
		// '8' -> 7
		return c - '1';
	}
	static inline u64 char2file(char c) {
		// 'a' -> 0
		// 'h' -> 7
		return c - 'a';
	}
	static inline char rank2char(u64 rank) { return rank + '1'; }
	static inline char file2char(u64 file) { return file + 'a'; }
};
} // namespace cldr
