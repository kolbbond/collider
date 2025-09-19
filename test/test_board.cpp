// testing template with logging and timer
#include <armadillo>
#include "Log.hpp"
#include "debug.hpp"

#include "Board.hpp"

int main(int argc, char** argv) {

	// create timer
	arma::wall_clock timer;
	timer.tic();

	// create log
	cldr::ShLogPr lg = cldr::Log::create();
	lg->msg("%s --- Test Template: %s%s --- %s\n", KPNK, __DATE__, __TIME__, KNRM);

	// create board
	cldr::ShBoardPr board = cldr::Board::create(cldr::Board::start_fen(), lg);

	// show board
	board->display_board(lg);

	// update movelist
	board->update_movelist();

	// get movelist
	arma::Mat<arma::uword> movelist = board->get_movelist();

	// debug
	std::cout << "Movelist:\n" << movelist << std::endl;

	// movelist should be what size?
	// 2 moves per pawn plus 2 per knight
	// 2*8*2 + 2*2*2 = 40
	assert(movelist.n_cols == 40);

	// move a piece
	board->move("e2e4");

	// update movelist
	board->update_movelist();

	// show board
	board->display_board(lg);

	// timer and log out
	lg->msg("%s --- Test Time Elapsed: %0.2f --- %s\n", KCYN, timer.toc(), KNRM);

	// return success
	return 0;
}
