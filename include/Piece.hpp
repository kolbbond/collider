// class for piece information

#pragma once

#include "collider.hpp"

namespace cldr {
using ShPiecePr = std::shared_ptr<class Piece>;
enum class PieceType { NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
enum class PieceColor { NONE, WHITE, BLACK };

class Piece {
    public:
        PieceType type;
        PieceColor color;
public:
	Piece();
	~Piece();
	static ShPiecePr create();
};
} // namespace cldr
