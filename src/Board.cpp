#include "Board.hpp"

namespace cldr {

Board::Board() {}
Board::~Board() {}
ShBoardPr Board::create() {
	return std::make_shared<Board>();
}

} // namespace cldr
