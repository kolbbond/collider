#include "Piece.hxx"

#include "Extra.hxx"
#include "typedefs.hh"
#include <cassert>
#include <cctype>
#include <string>

namespace cldr {
Piece::Piece() {}

Piece::Piece(PieceColor color, PieceType type, u64 pos) {
	set_color(color);
	set_type(type);
	set_pos(pos);
}

Piece::Piece(PieceColor color, PieceType type, u64 rank, u64 file) {
	set_color(color);
	set_type(type);
	set_pos(rank, file);
}

ShPiecePr Piece::create() { return std::make_shared<Piece>(); }
ShPiecePr Piece::create(PieceColor color, PieceType type, u64 pos) { return std::make_shared<Piece>(color, type, pos); }
ShPiecePr Piece::create(PieceColor color, PieceType type, u64 rank, u64 file) { return std::make_shared<Piece>(color, type, rank, file); }

void Piece::set_color(PieceColor color) { _color = color; }

void Piece::set_type(PieceType type) { _type = type; }

void Piece::set_pos(u64 pos) {
	// check pos is valid
	//	assert(pos < 64);
	if(pos >= 64) pos = -1;

	_pos = pos;
}

void Piece::set_alive(bool alive) { _alive = alive; }
void Piece::set_moved(bool moved) { _moved = moved; }

void Piece::set_pos(u64 rank, u64 file) { set_pos(Extra::rf2sq64(rank, file)); }

PieceColor Piece::get_color() const { return _color; }
PieceType Piece::get_type() const { return _type; }


bool Piece::is_alive() const { return _alive; }
bool Piece::has_moved() const { return _move_count; }

char Piece::get_piece_char() const { return get_piece_char(_color, _type); }
char Piece::get_piece_char(PieceColor color, PieceType type) {

	// my char
	char c = ' ';

	// get type
	switch(type) {
	case PieceType::OFFBOARD: c = '!'; break;
	case PieceType::NONE: c = '.'; break;
	case PieceType::ENPASSANT: c = '*'; break;
	case PieceType::PAWN: c = 'P'; break;
	case PieceType::KNIGHT: c = 'N'; break;
	case PieceType::BISHOP: c = 'B'; break;
	case PieceType::ROOK: c = 'R'; break;
	case PieceType::QUEEN: c = 'Q'; break;
	case PieceType::KING: c = 'K'; break;
	default: break;
	}

	// check color
	if(color == PieceColor::BLACK) { c = tolower(c); }

	return c;
}

std::string Piece::get_type_string(PieceType type) {
	std::string s = "NONE";
	switch(type) {
	case PieceType::NONE: s = "none"; break;
	case PieceType::PAWN: s = "pawn"; break;
	case PieceType::KNIGHT: s = "knight"; break;
	case PieceType::BISHOP: s = "bishop"; break;
	case PieceType::ROOK: s = "rook"; break;
	case PieceType::QUEEN: s = "queen"; break;
	case PieceType::KING: s = "king"; break;
	default: break;
	}
	return s;
}

std::string Piece::get_color_string(PieceColor color) {
	switch(color) {
	case PieceColor::NONE: return "none"; break;
	case PieceColor::WHITE: return "white"; break;
	case PieceColor::BLACK: return "black"; break;
	default: break;
	}
	return "unknown";
}

PieceColor Piece::get_enemy_color() const { return get_enemy_color(_color); }
PieceColor Piece::get_enemy_color(PieceColor color) {
	if(color == PieceColor::WHITE) return PieceColor::BLACK;
	if(color == PieceColor::BLACK) return PieceColor::WHITE;
	return PieceColor::NONE;
}

void Piece::display(ShLogPr lg) const {
	lg->msg("%sPiece: %c (%s, %s) at sq64: %llu%s\n", KMAG, get_piece_char(), get_color_string(_color).c_str(), get_type_string(_type).c_str(), _pos, KNRM);
}


} // namespace cldr
