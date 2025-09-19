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

	// log
	ShLogPr _lg = NullLog::create();

	// timers

public:
	Board();
	Board(std::string fen, ShLogPr log = nullptr);

	static ShBoardPr create();
	static ShBoardPr create(std::string fen, ShLogPr log = NullLog::create());

	void init(std::string fen = start_fen(), ShLogPr lg = NullLog::create());
	bool move(std::string move_str);
	void update(ShLogPr lg = NullLog::create());
	void update_movelist(ShLogPr lg = NullLog::create());

	void set_log(ShLogPr lg);
	arma::Row<arma::uword> get_moves(arma::uword sq120);
	arma::Mat<arma::uword> get_movelist() const;

	std::array<ShPiecePr, 64> get_board64() const;
	std::array<ShPiecePr, 120> get_board120() const;

	bool is_valid(arma::uword fr_sq120, arma::uword to_sq120);
	void display_board(ShLogPr lg = NullLog::create());

	static std::string start_fen();
};

} // namespace cldr
