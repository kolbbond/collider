// board class
#pragma once

#include <memory>

namespace cldr {

using ShBoardPr = std::shared_ptr<class Board>;

class Board {
public:
	Board();
	~Board();
	static ShBoardPr create();
};

} // namespace cldr
