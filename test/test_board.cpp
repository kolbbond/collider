// testing template with logging and timer
#include <armadillo>
#include "Log.hpp"
#include "chug/log.hh"

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

	// move a piece
	board->move("e2e4");

    // show board
	board->display_board(lg);

	// timer and log out
	lg->msg("%s --- Test Time Elapsed: %0.2f --- %s\n", KCYN, timer.toc(), KNRM);

	// return success
	return 0;
}
