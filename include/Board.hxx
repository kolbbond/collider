// board class
#pragma once

#include <armadillo>
#include <memory>
#include <vector>
#include <array>

#include "Log.hxx"
#include "Piece.hxx"
#include "typedefs.hh"

namespace cldr {

using ShBoardPr = std::shared_ptr<class Board>;


class Board {
public:
	// board storage using Piece class
	std::array<ShPiecePr, 120> _board120; // 120 board representation to handle off-board moves
	//ShPiecePr _board64[64]; //

	// the moves for this position
	arma::Mat<arma::uword> _movelist;

	// who's turn it is
	PieceColor _color = PieceColor::WHITE;

	// castling rights
	std::array<bool, 2> _can_castle;

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
	bool unmove(std::string move_str);
	//bool move(arma::uword frsq, arma::uword tosq);
	//bool unmove(arma::uword frsq, arma::uword tosq);
	void update(ShLogPr lg = NullLog::create());
	void update_movelist(ShLogPr lg = NullLog::create());

	void set_log(ShLogPr lg);
	arma::Row<arma::uword> get_moves(arma::uword sq120);
	arma::Mat<arma::uword> get_movelist() const;

	std::array<ShPiecePr, 64> get_board64() const;
	std::array<ShPiecePr, 120> get_board120() const;
	ShPiecePr get_piece(arma::uword sq120) const;

	PieceColor get_turn() const;
	PieceColor get_color() const;

	void set_color(PieceColor color);

	bool is_turn(PieceColor color) const;
	bool can_castle(PieceColor color) const;

	bool is_valid(arma::uword fr_sq120, arma::uword to_sq120);
	void display_board(ShLogPr lg = NullLog::create());
	void display_movelist(ShLogPr lg = NullLog::create());
	std::string get_color_string(PieceColor color) const;
	std::string get_color_color(PieceColor color) const;
	std::string get_algebraic_string(arma::uword frsq, arma::uword tosq) const;


	static std::string start_fen();
};

} // namespace cldr
