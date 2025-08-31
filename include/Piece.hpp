// class for piece information

#pragma once

#include "collider.hpp"
#include <memory>
#include <vector>
#include "typedefs.hh"

namespace cldr {

using ShPiecePr = std::shared_ptr<class Piece>;
enum class PieceType { NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
enum class PieceColor { NONE, WHITE, BLACK };

class Piece {
public:
	PieceType type;
	PieceColor color;
	int pos;
	int pos64;
	int color;
	int type;
	int value;
	bool life;
	U64 bitboard;
	int moved;
	std::vector<int> mL;
	std::vector<int> caps;

public:
	Piece();
	~Piece();
	static ShPiecePr create();
};
} // namespace cldr
