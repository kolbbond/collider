// board class
#pragma once

#include <armadillo>
#include <memory>
#include <vector>

#include "Log.hpp"
#include "Piece.hpp"
#include "typedefs.hh"

namespace cldr {

using ShBoardPr = std::shared_ptr<class Board>;


class Board {
public:
	// board storage using Piece class
	ShPiecePr _board120[SQNUM]; // 120 board representation to handle off-board moves
	ShPiecePr _board64[64]; //

public:
	Board();
	Board(std::string fen, ShLogPr log = nullptr);

	static ShBoardPr create();
	static ShBoardPr create(std::string fen, ShLogPr log = NullLog::create());

	void init(std::string fen = start_fen(), ShLogPr lg = NullLog::create());

	void display_board(ShLogPr lg = NullLog::create());

	static std::string start_fen();
};

} // namespace cldr
