// class for piece information
#pragma once

#include "Log.hxx"
#include "collider.hxx"
#include <armadillo>
#include <memory>
#include <vector>
#include "typedefs.hh"

namespace cldr {

using ShPiecePr = std::shared_ptr<class Piece>;

class Piece {


protected:
	PieceType _type = PieceType::NONE;
	PieceColor _color = PieceColor::NONE;

	// linear position 64 squares
	u64 _pos;

	bool _alive = true;
	bool _moved = false;

	u64 _bitboard;

public:
	arma::uword _move_count = 0;

public:
	Piece();
	Piece(PieceColor color, PieceType type, u64 pos);
	Piece(PieceColor color, PieceType type, u64 rank, u64 file);

	static ShPiecePr create();
	static ShPiecePr create(PieceColor color, PieceType type, u64 pos);
	static ShPiecePr create(PieceColor color, PieceType type, u64 rank, u64 file);

	void set_color(PieceColor color);
	void set_type(PieceType type);
	void set_pos(u64 pos);
	void set_pos(u64 rank, u64 file);

	PieceColor get_color() const;
	PieceType get_type() const;
	u64 get_pos() const;
	u64 get_rank() const;
	u64 get_file() const;

	void set_alive(bool alive);
	void set_moved(bool moved);
	bool is_alive() const;
	bool has_moved() const;
	PieceColor get_enemy_color() const;

	char get_piece_char() const;
	static char get_piece_char(PieceColor color, PieceType type);
	static std::string get_type_string(PieceType type);
	static std::string get_color_string(PieceColor color);
	static PieceColor get_enemy_color(PieceColor color);

	void display(ShLogPr lg = NullLog::create()) const;
};
} // namespace cldr
