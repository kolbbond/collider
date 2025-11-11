#include "Engine.hxx"
#include "debug.hxx"
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

	// early exit and counter
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
		// get stats
		PerftStats stats = _board->get_perft_stats(lg);
		//_total_stats = _total_stats + stats;
		_total_stats += stats; // with overloaded +=

		//lg->msg("%sGenerated %llu moves for Perft(%llu)%s\n", KBLU, num_moves, depth, KNRM);
		//_board->display_movelist(lg);
		//_board->display_board(lg);
	}

	// walk over moves
	for(arma::uword i = 0; i < num_moves; i++) {
		const arma::Col<arma::uword> mymove = movelist.col(i);
		const arma::uword frsq = mymove(0);
		const arma::uword tosq = mymove(1);
		const PieceType promo = static_cast<PieceType>(mymove(2));

		// in algebraic notation
		const std::string movestr = _board->get_algebraic_string(frsq, tosq, promo);
		assert(movestr.length() == 4 || movestr.length() == 5); // should be in format e2e4 or e7e8q

		// debug
		//std::printf("Move %llu: %s\n", i, movestr.c_str());

		// get stats
		//		PerftStats stats = _board->get_perft_stats(lg);
		//        total_stats.nodes += stats.nodes;
		//        total_stats.captures += stats.captures;
		//        total_stats.castles += stats.castles;

		// display perft stats
		//		lg->msg("%sPerft(%llu): %llu nodes, %llu captures, %llu enpassant, %llu castles, %llu promotions%s\n",
		//			KBLU,
		//			depth,
		//			stats.nodes,
		//			stats.captures,
		//			stats.enpassant,
		//			stats.castles,
		//			stats.promotions,
		//			KNRM);

		// make move
		if(!_board->move(movestr)) {
			_board->display_board(lg);
			collider_throw_line("Invalid move attempted.");
		}

		// increment count and recurse
		//	nodes += perft(depth - 1, lg);
		nodes += perft(depth - 1);

		// unmove
		if(!_board->unmove(movestr)) {
			_board->display_board(lg);
			collider_throw_line("Invalid unmove attempted.");
		}
	}

	// display perft stats
	lg->msg("%sTotal Perft(%llu): N(%llu), Captures(%llu), Enpassant(%llu),  Castles(%llu), Promotions(%llu), Checks(%llu), Checkmates(%llu), ...%s\n",
		KGRN,
		depth,
		_total_stats.nodes,
		_total_stats.captures,
		_total_stats.enpassants,
		_total_stats.castles,
		_total_stats.promotions,
		_total_stats.checks,
		_total_stats.checkmates,
		KNRM);

	// timer and log
	const double time_elapsed = timer.toc();
	lg->msg("%sPerft(%llu): %llu [N], %0.3f [s], %0.3f [N/s]%s\n", KCYN, depth, time_elapsed, nodes, ((double)nodes) / time_elapsed, KNRM);

	// return number of moves
	return nodes;
}
