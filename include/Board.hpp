// board class
#pragma once

#include <armadillo>
#include <memory>
#include <vector>
#include <array>

#include "Log.hpp"
#include "Piece.hpp"
#include "typedefs.hh"

namespace cldr {

using ShBoardPr = std::shared_ptr<class Board>;


class Board {
public:
	// board storage using Piece class
	std::array<ShPiecePr, 120> _board120; // 120 board representation to handle off-board moves
	//ShPiecePr _board64[64]; //
	arma::Mat<arma::uword> _movelist;

public:
	Board();
	Board(std::string fen, ShLogPr log = nullptr);

	static ShBoardPr create();
	static ShBoardPr create(std::string fen, ShLogPr log = NullLog::create());

	void init(std::string fen = start_fen(), ShLogPr lg = NullLog::create());
	void move(std::string move_str);
	void update_movelist();
	arma::Row<arma::uword> get_moves(arma::uword sq120);

	std::array<ShPiecePr, 64> get_board64() const;
	std::array<ShPiecePr, 120> get_board120() const;

	void display_board(ShLogPr lg = NullLog::create());

	static std::string start_fen();
};

} // namespace cldr
