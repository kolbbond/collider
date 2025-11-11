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

	const bool run_position1 = false;
	const bool run_position2 = false;
	const bool run_position3 = false;
	const bool run_position4 = true;

	// run perft on initial position
	// https://www.chessprogramming.org/Perft
	// depth   nodes
	// 1       20
	// 2       400
	// 3       8902
	// 4       197281
	// 5       4865609
	// 6       119060324

	// Notes: learnings
	// 1      double pawn move, basic movegen
	// 2      above but for black
	// 3      captures, pawn captures
	// 4      checks
	// 5      enpassant
	// 6      castling
	// 7      promotions???
	// @hey: do we need to add an additional perft position?
	if(run_position1) {
		lg->msg("%s --- Starting Position --- %s\n", KBLU, KNRM);
		arma::Row<arma::uword> perft_results = { 20, 400, 8902, 197281, 4865609, 119060324 };
		arma::uword depth = 4;
		for(arma::uword d = 1; d <= depth; d++) {
			// reset board
			cldr::ShBoardPr board = cldr::Board::create(cldr::Board::start_fen(), lg);
			cldr::ShEnginePr engine = cldr::Engine::create(board);
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
	}
	lg->newl();

	// run perft on position 2 (Kiwipete)
	// https://www.chessprogramming.org/Perft_Results
	// depth   nodes
	// 1       48
	// 2       2039
	// 3       97862
	// 4       4085603

	// Notes: learnings
	// 1      double pawn move, basic movegen
	// 2      above but for black
	// 3      captures, pawn captures
	// 4      checks
	// 5      enpassant
	// 6      castling
	// 7      promotions???
	// @hey: do we need to add an additional perft position?
	// new scope
	if(run_position2) {
		lg->msg("%s --- Position 2 (Kiwipete) --- %s\n", KBLU, KNRM);
		std::string kiwipete_fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
		arma::Row<arma::uword> perft_results = { 48, 2039, 97862, 4085603 };
		const arma::uword depth = 3;
		for(arma::uword d = 1; d <= depth; d++) {
			cldr::ShBoardPr board = cldr::Board::create(kiwipete_fen, lg);
			cldr::ShEnginePr engine = cldr::Engine::create(board);
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
	}
	lg->newl();

	// position 3
	if(run_position3) {
		lg->msg("%s --- Position 3 --- %s\n", KBLU, KNRM);
		std::string kiwipete_fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1 ";
		arma::Row<arma::uword> perft_results = { 14, 191, 2812, 43238 };
		const arma::uword depth = 4;
		for(arma::uword d = 1; d <= depth; d++) {
			cldr::ShBoardPr board = cldr::Board::create(kiwipete_fen, lg);
			cldr::ShEnginePr engine = cldr::Engine::create(board);
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
	}
	lg->newl();

	//////////////////////////////////////////////////
	// Position 4 (as there are promotions on depth 2)
	if(run_position4) {
		lg->msg("%s --- Position 4 --- %s\n", KBLU, KNRM);
		std::string position4_fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
		arma::Row<arma::uword> perft_results = { 6, 264, 9467, 422333 };
		const arma::uword depth = 4;
		for(arma::uword d = 1; d <= depth; d++) {
			cldr::ShBoardPr board = cldr::Board::create(position4_fen, lg);
			cldr::ShEnginePr engine = cldr::Engine::create(board);
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
	}
	lg->newl();

	// timer and log out
	lg->msg("%s --- Test Time Elapsed: %0.2f --- %s\n", KCYN, timer.toc(), KNRM);

	// return success
	return 0;
}
