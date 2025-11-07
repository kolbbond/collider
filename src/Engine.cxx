#include "Engine.hxx"

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
arma::uword cldr::Engine::perft(arma::uword depth) {
	// early exit
	if(depth == 0) { return 1; }

	assert(_board != NULL);

	// counter for nodes
	u64 nodes = 0;

	// generate moves
	_board->update_movelist();
	arma::Mat<arma::uword> movelist = _board->get_movelist();
	arma::uword num_moves = movelist.n_cols;

	// walk over moves
	for(arma::uword i = 0; i < num_moves; i++) {
		arma::Col<arma::uword> mymove = movelist.col(i);
		arma::uword frsq = mymove(0);
		arma::uword tosq = mymove(1);

		// in algebraic notation
		std::string movestr = _board->get_algebraic_string(frsq, tosq);

		// make move
		_board->move(movestr);
        nodes += perft(depth - 1);
        _board->unmove(movestr);
	}
}
