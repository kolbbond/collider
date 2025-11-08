// testing template with logging and timer
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

	// reset board
    cldr::ShBoardPr board = cldr::Board::create(cldr::Board::start_fen(), lg);

	// run perft
	// https://www.chessprogramming.org/Perft
	// depth   nodes
	// 1       20
	// 2       400
	// 3       8,902
	// 4       197,281
	// 5       4,865,609
	// 6       119,060,324

    // Notes: learnings
    // 1      double pawn move, basic movegen
    // 2      above but for black
    // 3      captures, pawn captures
    // 4      checks
    // 5      enpassant
    // 6      castling
    // 7      promotions???
    // @hey: do we need to add an additional perft position?
	cldr::ShEnginePr engine = cldr::Engine::create(board);
	arma::Row<arma::uword> perft_results = { 20, 400, 8902, 197281, 4865609, 119060324 };
	arma::uword depth = 5;
	for(arma::uword d = 1; d <= depth; d++) {
		arma::uword num_nodes = engine->perft(d, lg);

		// check success and log
		bool success = (num_nodes == perft_results(d - 1));
		std::string str_color = success ? KGRN : KRED;
		arma::sword diff = perft_results(d - 1) - num_nodes;
		lg->msg("%sPerft to depth %llu: N %llu (expected %llu) (diff %d) %s\n", str_color.c_str(), d, num_nodes, perft_results(d - 1), diff, KNRM);
		if(!success) {
			board->display_board(lg);
			collider_throw_line("Perft test failed.");
		}
	}

	// timer and log out
	lg->msg("%s --- Test Time Elapsed: %0.2f --- %s\n", KCYN, timer.toc(), KNRM);

	// return success
	return 0;
}
