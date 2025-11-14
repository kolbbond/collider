#include "Engine.hxx"
#include "debug.hxx"
#include "error.hxx"
#include <armadillo>

//int alphaBeta( int alpha, int beta, int depthleft ) {
//   if( depthleft == 0 ) return quiesce( alpha, beta );
//   bestValue = -infinity;
//   for ( all moves)  {
//      score = -alphaBeta( -beta, -alpha, depthleft - 1 );
//      if( score > bestValue )
//      {
//         bestValue = score;
//         if( score > alpha )
//            alpha = score; // alpha acts like max in MiniMax
//      }
//      if( score >= beta )
//         return bestValue;   //  fail soft beta-cutoff, existing the loop here is also fine
//   }
//   return bestValue;
//}
// alphabeta (negamax)
int cldr::Engine::alpha_beta(int alpha, int beta, int depth, ShLogPr lg) {
	// log
	//lg->msg("%sAlphaBeta: depth(%d), alpha(%d), beta(%d)%s\n", KYEL, depth, alpha, beta, KNRM);
	// check depth
	if(depth > _max_depth) {
		_max_depth = depth;
		_time_alphabeta.resize(_max_depth);
		_num_nodes_per_depth.resize(_max_depth);
	}

	// timer
	arma::wall_clock timer;
	timer.tic();

	if(depth == 0) {
		_num_nodes++;

		// run evaluation
		return _board->evaluate();

		// @hey: add quiescence search
	}

	int best_value = -99999;

	// generate moves
	_board->update_movelist();
	const arma::Mat<arma::uword> movelist = _board->get_movelist();
	const arma::uword num_moves = movelist.n_cols;
	int score = 0;
	for(arma::uword i = 0; i < num_moves; i++) {
		arma::Col<arma::uword> mymove = movelist.col(i);
		std::string movestr = _board->get_algebraic_string(mymove(0), mymove(1), static_cast<PieceType>(mymove(2)));
		if(!_board->move(movestr)) { collider_throw_line("Invalid move attempted in alpha_beta."); }

		// recursive score call
		score = -alpha_beta(-beta, -alpha, depth - 1, lg);

		// check score against best value
		if(score > best_value) {
			best_value = score;
			if(score > alpha) {
				alpha = score; // alpha acts like max in MiniMax
			}
		}

		// early exit
		if(score >= beta) {
			//COLLIDER_DEBUG("early beta exit");
			if(!_board->unmove(movestr)) { collider_throw_line("Invalid unmove attempted in alpha_beta."); }
			return best_value; // fail soft beta-cutoff, exiting the loop here is also fine
		}

		if(!_board->unmove(movestr)) { collider_throw_line("Invalid unmove attempted in alpha_beta."); }
	}

	// timer and log
	const double time_elapsed = timer.toc();
	_time_alphabeta(depth - 1) += time_elapsed;
	//lg->msg("%sdepth(%llu) completed in %0.3f [s]%s\n", KCYN, depth, time_elapsed, KNRM);

	// return best move
	return best_value;
}

void cldr::Engine::display_alphabeta(cldr::ShLogPr lg) {
	//lg->msg("%sAlphaBeta Times per Depth:%s\n", KGRN, KNRM);
	//for(arma::uword d = 0; d < _max_depth; d++) {
	//
	//lg->msg("%s (%llu): %0.3f [s]%s", KCYN, d + 1, _time_alphabeta(d), KNRM);
	//}
	lg->msg("%s %8llu %8.3f %8.0f%s", KYEL, _num_nodes, arma::accu(_time_alphabeta), _num_nodes / arma::accu(_time_alphabeta), KNRM);
	lg->newl();
}

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
		_total_stats += stats; // with overloaded +=

		//COLLIDER_DEBUG("perft depth (1)");
		_board->display_movelist(lg);
		_board->display_board(lg);

		//lg->msg("%sGenerated %llu moves for Perft(%llu)%s\n", KBLU, num_moves, depth, KNRM);
		//_board->display_movelist(lg);
		//_board->display_board(lg);
	}

	// check for empty movelist
	if(num_moves == 0) {
		//COLLIDER_DEBUG("No moves left, this is a checkmate.");
		//		return 0;
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


		// make move
		if(!_board->move(movestr)) {
			_board->display_board(lg);
			collider_throw_line("Invalid move attempted.");
		}

		// increment count and recurse
		//nodes += perft(depth - 1, lg);
		nodes += perft(depth - 1);

		// unmove
		if(!_board->unmove(movestr)) {
			_board->display_board(lg);
			collider_throw_line("Invalid unmove attempted.");
		}
	}

	// display perft stats
	lg->msg("%sTotal Perft(%llu): N(%llu), Captures(%llu), Enpassant(%llu),  Castles(%llu), Promotions(%llu), Checks(%llu), "
			"Checkmates(%llu), ...%s\n",
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
