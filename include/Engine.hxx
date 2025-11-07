#pragma once

#include "Board.hxx"
#include <armadillo>
namespace cldr {

using ShEnginePr = std::shared_ptr<class Engine>;

class Engine {
public:
	ShBoardPr _board;

public:
	Engine();
	Engine(ShBoardPr board) { set_board(board); }
	static ShEnginePr create() { return std::make_shared<Engine>(); }
	static ShEnginePr create(ShBoardPr board) { return std::make_shared<Engine>(board); }
	void set_board(ShBoardPr board) { _board = board; }

	// https://www.chessprogramming.org/Perft
	arma::uword perft(arma::uword depth);
};
} // namespace cldr
