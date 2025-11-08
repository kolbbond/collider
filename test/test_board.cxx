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

	// create board
	cldr::ShBoardPr board = cldr::Board::create(cldr::Board::start_fen(), lg);

	// show board
	board->display_board(lg);

	// update movelist
	board->update_movelist();

	// display movelist
	board->display_movelist(lg);

	// get movelist
	arma::Mat<arma::uword> movelist = board->get_movelist();

	// debug
	std::cout << "Movelist:\n" << movelist << std::endl;

	// movelist should be what size?
	// 2 moves per pawn plus 2 per knight
	// 2*8*2 + 2*2*2 = 40
	assert(movelist.n_cols == 20);

	// move a piece
	board->move("e2e4");

	// update movelist
	board->update_movelist();

	// show board
	board->display_board(lg);

	// display movelist
	board->display_movelist(lg);

	// this should fail
	assert(!board->move("e2e4"));

	board->move("g8f6");

	board->display_board(lg);

	// update and display movelist
	board->update_movelist();
	board->display_movelist(lg);

	// run perft test
	lg->msg("%s --- Running Perft Test --- %s\n", KPNK, KNRM);

	// reset board
	board = cldr::Board::create(cldr::Board::start_fen(), lg);

	// run perft
	// https://www.chessprogramming.org/Perft
	// depth   nodes
	// 1       20
	// 2       400
	// 3       8,902
	// 4       197,281
	// 5       4,865,609
	// 6       119,060,324
    // Notes: 
    // 1       tests first move, double pawn moves
    // 2       
	cldr::ShEnginePr engine = cldr::Engine::create(board);
	arma::Row<arma::uword> perft_results = { 20, 400, 8902, 197281, 4865609, 119060324 };
	arma::uword depth = 4;
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
