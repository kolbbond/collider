// board class
#pragma once

#include <armadillo>
#include <memory>
#include <vector>

#include "Piece.hpp"
#include "typedefs.hh"

namespace cldr {

using ShBoardPr = std::shared_ptr<class Board>;
using u64 = U64;

enum class PieceEnum : U64 { Pawn, Knight, Bishop, Rook, Queen, King };

class Board {
public:
	// board to
	//arma::Mat<u64>::fixed<8, 8> _board;
	arma::Mat<u64>::fixed<8, 8> _wboard;
	arma::Mat<u64>::fixed<8, 8> _bboard;

	ShPiecePr _pboard[8][8];

	int side;

	int newfrSq;
	int newtoSq;

	int frSq;
	int toSq;

	int sq[SQNUM];
	int sq64[64];

	bool castling;
	bool castled[3];
	int material[3];

	int enPas;
	std::vector<int> mL;
	std::vector<int> score;

public:
	Board();
	~Board();
	static ShBoardPr create();
};

} // namespace cldr
