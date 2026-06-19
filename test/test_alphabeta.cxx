// testing alphabeta with logging and timer
#include <armadillo>
#include "Log.hxx"
#include "debug.hxx"

#include "Board.hxx"
#include "Engine.hxx"

int main(int argc, char** argv) {

	// create timer
	arma::wall_clock timer;
	timer.tic();

	// create log
	cldr::ShLogPr lg = cldr::Log::create();
	lg->msg("%s --- Test Template: %s%s --- %s\n", KPNK, __DATE__, __TIME__, KNRM);

	// run perft test
	lg->msg("%s --- Running Perft Test --- %s\n", KPNK, KNRM);

	const bool run_position1 = true;
	lg->msg("%s --- Starting Position --- %s\n", KBLU, KNRM);

	// reset board
	cldr::ShBoardPr board = cldr::Board::create(cldr::Board::start_fen(), lg);
	cldr::ShEnginePr engine = cldr::Engine::create(board);
	int alpha = -100000;
	int beta = 100000;
	int depth = 5;

	// run different depths
	for(int d = 1; d <= depth; d++) {
		lg->msg("%s--- Running Alpha-Beta at depth %d --- %s\n", KPNK, d, KNRM);
		int score = engine->alpha_beta(alpha, beta, d);
		lg->msg("%sAlpha-Beta Result at depth %d: %d%s\n", KGRN, d, score, KNRM);
		lg->newl();
	}

	lg->newl();


	// timer and log out
	lg->msg("%s --- Test Time Elapsed: %0.2f --- %s\n", KCYN, timer.toc(), KNRM);

	// return success
	return 0;
}
