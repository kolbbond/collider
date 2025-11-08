#include "Engine.hxx"
#include "error.hxx"
#include <armadillo>

// https://www.chessprogramming.org/Perft
//typedef unsigned long long u64;
//
//u64 Perft(int depth)
//{
//  MOVE move_list[256];
//  int n_moves, i;
//  u64 nodes = 0;
//
//  if (depth == 0)
//    return 1ULL;
//
//  n_moves = GenerateLegalMoves(move_list);
//  for (i = 0; i < n_moves; i++) {
//    MakeMove(move_list[i]);
//    nodes += Perft(depth - 1);
//    UndoMove(move_list[i]);
//  }
//  return nodes;
//}

arma::uword cldr::Engine::perft(arma::uword depth, cldr::ShLogPr lg) {
	// early exit
	if(depth == 0) { return 1; }

	assert(_board != NULL);

	// log
	lg->msg("%sStarting Perft(%llu)%s\n", KMAG, depth, KNRM);

	// timer
	arma::wall_clock timer;
	timer.tic();

	// counter for nodes
	u64 nodes = 0;

	// generate moves
	_board->update_movelist();
	const arma::Mat<arma::uword> movelist = _board->get_movelist();
	const arma::uword num_moves = movelist.n_cols;

	// debug
	if(depth == 1) {
		lg->msg("%sGenerated %llu moves for Perft(%llu)%s\n", KBLU, num_moves, depth, KNRM);
		_board->display_movelist(lg);
		_board->display_board(lg);
	}

	// walk over moves
	for(arma::uword i = 0; i < num_moves; i++) {
		const arma::Col<arma::uword> mymove = movelist.col(i);
		const arma::uword frsq = mymove(0);
		const arma::uword tosq = mymove(1);

		// in algebraic notation
		const std::string movestr = _board->get_algebraic_string(frsq, tosq);

		// debug
		//std::printf("Move %llu: %s\n", i, movestr.c_str());

		// make move
		if(!_board->move(movestr)) {
			_board->display_board(lg);
			collider_throw_line("Invalid move attempted.");
		}

		// increment count and recurse
		nodes += perft(depth - 1, lg);
		//nodes += perft(depth - 1);

		// unmove
		if(!_board->unmove(movestr)) {
			_board->display_board(lg);
			collider_throw_line("Invalid unmove attempted.");
		}
	}

	// timer and log
	const double time_elapsed = timer.toc();
	lg->msg("%sPerft(%llu): %llu [N], %0.3f [s], %0.3f [N/s]%s\n", KCYN, depth, time_elapsed, nodes, ((double)nodes) / time_elapsed, KNRM);

	// return number of moves
	return nodes;
}
