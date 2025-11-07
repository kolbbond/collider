// testing template with logging and timer
#include <armadillo>
#include <smg/GuiBase.hh>
#include "Board.hxx"
#include "chug/log.hh"

int main(int argc, char** argv) {

	// create timer
	arma::wall_clock timer;
	timer.tic();

	// create log
	chug::ShLogPr lg = chug::Log::create();
	lg->msg("%s --- Test Template: %s%s --- %s\n", KPNK, __DATE__, __TIME__, KNRM);

	// create gui
	smg::GuiBase gb = smg::GuiBase({ argc, argv });

    // create board
    cldr::ShBoardPr board = cldr::Board::create();

	// timer and log out
	lg->msg("%s --- Test Time Elapsed: %0.2f --- %s\n", KCYN, timer.toc(), KNRM);

	// return success
	return 0;
}

