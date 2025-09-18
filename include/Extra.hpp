#pragma once

#include "typedefs.hh"

namespace cldr {
class Extra {
public:
	// rank and file to sq macros/functions
	// #define FR2SQ64(f, r) ((f) + ((r) * 8)) //Finds 64 square index
	// #define FR2SQ(f, r) ((21 + (f)) + ((r) * 10)) //Finds 120 square index
	static inline u64 rf2sq64(u64 r, u64 f) {
        return (r * 8) + f; 
    }
	static inline u64 rf2sq120(u64 r, u64 f) { return (r * 10) + f + 21; }
	static inline u64 sq64to120(u64 sq64) {
		u64 r = sq64 / 8;
		u64 f = sq64 % 8;
		return rf2sq120(r, f);
	}
    static inline u64 sq120to64(u64 sq120) {
        u64 r = (sq120 - 21) / 10;
        u64 f = (sq120 - 21) % 10;

        return rf2sq64(r, f);
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
};
} // namespace cldr
