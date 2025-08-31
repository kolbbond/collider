#pragma once

#include <memory>
#include <vector>

#include "typedefs.hh"
#include "Piece.hpp"


namespace cldr {

using ShPiecesPr = std::shared_ptr<class Pieces>;

class Pieces {
public:

    // array of pieces
	ShPiecePr _pieces[33];

public:
	Pieces();
	~Pieces();
	static ShPiecesPr create();

};

} // namespace cldr
