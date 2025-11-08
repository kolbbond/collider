#pragma once

#include <armadillo>
#include "typedefs.hh"
namespace cldr {
class PieceMoves {
public:
	static arma::Row<arma::sword> get_moves(PieceType type, PieceColor color);
	static arma::Row<arma::sword> get_pawn_moves(PieceColor color);
	static arma::Row<arma::sword> get_knight_moves();
	static arma::Row<arma::sword> get_bishop_moves();
	static arma::Row<arma::sword> get_rook_moves();
	static arma::Row<arma::sword> get_queen_moves();
	static arma::Row<arma::sword> get_king_moves();
	static arma::Row<arma::sword> get_castling_moves();
};
} // namespace cldr
