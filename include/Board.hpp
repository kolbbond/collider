// board class
#pragma once

#include <memory>
#include <vector>

#include "typedefs.hh"

namespace cldr {

using ShBoardPr = std::shared_ptr<class Board>;

class Board {
public:
	int side;

	int newfrSq;
	int newtoSq;

	int frSq;
	int toSq;

	int sq[SQNUM];
	int sq64[64];

	bool castling;
	bool castled[3];
	int material[3];

	int enPas;
	std::vector<int> mL;
	std::vector<int> score;

public:
	Board();
	~Board();
	static ShBoardPr create();
};

} // namespace cldr
