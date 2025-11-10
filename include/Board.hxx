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

// perft stats
struct PerftStats {
	arma::uword nodes = 0;
	arma::uword captures = 0;
	arma::uword enpassants = 0;
	arma::uword castles = 0;
	arma::uword promotions = 0;
	arma::uword checks = 0;
	arma::uword discovery_checks = 0;
	arma::uword double_checks = 0;
	arma::uword checkmates = 0;

	// override operator+= to sum stats
	constexpr PerftStats& operator+=(const PerftStats& other) noexcept {
		nodes += other.nodes;
		captures += other.captures;
		enpassants += other.enpassants;
		castles += other.castles;
		promotions += other.promotions;
		checks += other.checks;
		discovery_checks += other.discovery_checks;
		double_checks += other.double_checks;
		checkmates += other.checkmates;
		return *this;
	}
};

// + operator
inline constexpr PerftStats operator+(PerftStats lhs, const PerftStats& rhs) noexcept {
	lhs += rhs;
	return lhs;
}

using ShBoardPr = std::shared_ptr<class Board>;

class Board {

	struct EnPassantInfo {
		arma::uword square = 0;
		PieceColor color = PieceColor::NONE;
	};

	//struct CastlingInfo {
	//std::map<PieceColor, bool> kingside = { true, true };
	//std::map<PieceColor, bool> queenside = { true, true };
	//};

	enum class CastlingSide { KINGSIDE, QUEENSIDE };
	using CastlingInfo = std::map<PieceColor, std::map<CastlingSide, bool>>;
	//std::map<PieceColor, std::map<CastlingSide, bool>> _castling_rights = { { PieceColor::WHITE, { { CastlingSide::KINGSIDE, true }, { CastlingSide::QUEENSIDE, true } } },
	//	{ PieceColor::BLACK, { { CastlingSide::KINGSIDE, true }, { CastlingSide::QUEENSIDE, true } } } };


public:
	// board storage using Piece class
	std::array<ShPiecePr, 120> _board120; // 120 board representation to handle off-board moves
	//ShPiecePr _board64[64]; //

	// the moves for this position
	arma::Mat<arma::uword> _movelist;

	// who's turn it is
	PieceColor _color = PieceColor::WHITE;

	// castling rights
	//	std::array<bool, 2> _can_castle;
	//std::vector<CastlingInfo> _castling_list = { { { true, true }, { true, true } } };
	// @hey: this type is a little convuluted
	std::vector<CastlingInfo> _castling_list = { { { PieceColor::WHITE, { { CastlingSide::KINGSIDE, true }, { CastlingSide::QUEENSIDE, true } } },
		{ PieceColor::BLACK, { { CastlingSide::KINGSIDE, true }, { CastlingSide::QUEENSIDE, true } } } } };

	// enpassant squares
	std::vector<EnPassantInfo> _enpassant_list = { { 0, PieceColor::NONE } };
	//arma::uword _enpassant_square;

	// store made moves for unmove
	std::vector<std::string> _move_history;
	std::vector<ShPiecePr> _captured_pieces;

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
	bool check_for_check(arma::Col<arma::uword> movecol, ShLogPr lg = NullLog::create());
	void check_for_checks(arma::Mat<arma::uword>& movelist, ShLogPr lg = NullLog::create());
	void update_movelist(arma::uword depth, ShLogPr lg = NullLog::create());
	arma::Mat<arma::uword> create_movelist(arma::uword depth, ShLogPr lg = NullLog::create());

	std::string create_fen(ShLogPr lg = NullLog::create()) const;
	void set_log(ShLogPr lg);
	arma::Row<arma::uword> get_moves(arma::uword sq120);
	arma::Mat<arma::uword> get_movelist() const;

	std::array<ShPiecePr, 64> get_board64() const;
	std::array<ShPiecePr, 120> get_board120() const;
	ShPiecePr get_piece(arma::uword sq120) const;

	PieceColor get_turn() const;
	PieceColor get_color() const;

	void set_color(PieceColor color);
	void switch_color();

	bool is_turn(PieceColor color) const;
	//bool can_castle(PieceColor color) const;
	bool can_castle(PieceColor color, CastlingSide side) const;

	bool is_valid(arma::uword fr_sq120, arma::uword to_sq120);
	void display_board(ShLogPr lg = NullLog::create());
	void display_movelist(ShLogPr lg = NullLog::create());
	std::string get_color_string(PieceColor color) const;
	std::string get_color_color(PieceColor color) const;
	std::string get_algebraic_string(arma::uword frsq, arma::uword tosq) const;

	PerftStats get_perft_stats(ShLogPr lg = NullLog::create());


	static std::string start_fen();
};

} // namespace cldr
