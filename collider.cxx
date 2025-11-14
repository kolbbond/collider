// main entry point
#include "Log.hxx"
#include "Board.hxx"
#include "Engine.hxx"

int main(int argc, char** argv) {
	// create log
	cldr::ShLogPr lg = cldr::Log::create();
	lg->msg("%s --- Collider V2 --- %s\n", KPNK, KNRM);

	// create board and engine
	cldr::ShBoardPr board = cldr::Board::create(cldr::Board::start_fen(), lg);
	cldr::ShEnginePr engine = cldr::Engine::create(board);

	std::string input_move;
	bool done = false;
	while(!done) {

		// display board

		// take user input
		bool valid_input = false;
		while(!valid_input) {
			board->display_board(lg);
			std::cout << "Enter your move (or 'exit' to quit): ";
			std::cin >> input_move;
			if(input_move == "exit") {
				done = true;
				break;
			} else {
				try {
					if(board->move(input_move)) {
						lg->msg("%sMove %s played successfully.%s\n", KGRN, input_move.c_str(), KNRM);
						board->display_board(lg);
						valid_input = true;
					} else {
						lg->msg("%sInvalid move: %s%s\n", KRED, input_move.c_str(), KNRM);
					}
				} catch(...) { lg->msg("%sError processing move: %s%s\n", KRED, input_move.c_str(), KNRM); }
			}
		}

		// update engine or board state if necessary
		// create movelist
		lg->msg("%sEngine is calculating its move...%s\n", KBLU, KNRM);
		lg->msg("%s #move   #ply   #score   #nodes    #time     #nps %s\n", KYEL, KNRM);
		board->update_movelist(lg);
		const arma::Mat<arma::uword> movelist = board->get_movelist();
		const arma::uword num_moves = movelist.n_cols;
		arma::uword idx_best_move = 0;
		arma::sword best_score = -100000;
		const int alpha = -100000;
		const int beta = 100000;
		const arma::uword depth = 4;
		for(arma::uword i = 0; i < num_moves; i++) {

			// run alphabeta on each move to find best move
			const arma::Col<arma::uword> mymove = movelist.col(i);
			std::string movestr = board->get_algebraic_string(mymove(0), mymove(1), static_cast<PieceType>(mymove(2)));
			lg->msg("%s %5s %5llu %s", KYEL, movestr.c_str(), depth, KNRM);
			if(!board->move(movestr)) {
				lg->msg("%sError moving move: %s%s\n", KRED, movestr.c_str(), KNRM);
				continue;
			}
			// alphabeta
			int score = -engine->alpha_beta(alpha, beta, depth, lg);
			lg->msg("%s    %05d%s", KYEL, score, KNRM);
			engine->display_alphabeta(lg);
			//engine->_time_alphabeta.clear();
			engine->reset_alphabeta();
			if(!board->unmove(movestr)) {
				lg->msg("%sError unmoving move: %s%s\n", KRED, movestr.c_str(), KNRM);
				continue;
			}

			// check best score
			if(score > best_score) {
				best_score = score;
				idx_best_move = i;
			}
		}

		// move for engine
		if(num_moves > 0) {
			arma::Col<arma::uword> best_move = movelist.col(idx_best_move);
			std::string best_move_str = board->get_algebraic_string(best_move(0), best_move(1), static_cast<PieceType>(best_move(2)));
			if(board->move(best_move_str)) {
				lg->msg("%sEngine plays move: %s with score %d %s\n", KGRN, best_move_str.c_str(), best_score, KNRM);
			} else {
				lg->msg("%sEngine failed to play move: %s%s\n", KRED, best_move_str.c_str(), KNRM);
			}
		} else {
			// checkmate or stalemate
			lg->msg("%sNo valid moves left for engine, checkmate or stalemate.%s\n", KRED, KNRM);
			done = true;
		}

		// loop
	}
	// return success
	return 0;
}
