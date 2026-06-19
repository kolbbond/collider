#include "Engine.hxx"
#include "debug.hxx"
#include "error.hxx"
#include <armadillo>
#include <ostream>
#include <sstream>

namespace {
// Parse the unsigned integer that follows a keyword token in a UCI command,
// e.g. parse_uint("go depth 4 movetime 100", "depth") -> 4. Returns the
// supplied fallback when the keyword (or its value) is absent.
arma::uword parse_uint(const std::string& input, const std::string& key, arma::uword fallback) {
	std::istringstream ss(input);
	std::string tok;
	while(ss >> tok) {
		if(tok == key) {
			arma::uword value = 0;
			if(ss >> value) { return value; }
			return fallback;
		}
	}
	return fallback;
}
} // namespace


// uci prompt input

// need to handle specific strings from the cpu
// these strings are from stdin

// General
// * must always be able to process input from stdin, even while thinking
// * all strings in/out should end in \n
// * arbitrary whitespace is allowed
// * always in forced mode, never start calc or ponder without "go"
// * before asked to search will receive position command
// * default GUI handles book, this can be overridden
// * if receiving unknown command should still parse rest of string
// * if receiving command it should not it should ignore it

// Moves
// * moves are in long algebraic notation
// i.e. e2e4, e7e8q, 0000 is null move

// Commands
// "uci" - sent as first command, engine responds with "id" and "option" commands
//         then "uciok"
// "debug" - to enable extra info from engine with "info string" command. can be sent at any time
// "isready" - check if engine is ready, must answer with "readyok"
// "setoption name <id> [value <x>]"
// "register" - register the engine, reply with "later", "name <x>", or "code <y>"
// "ucinewgame" - sent when a new game is started, not required
// "position [fen <fenstring> | startpos ]  moves <move1> .... <movei>"
// - start from a position fen or startpos or set of moves
// "go" start calculating, this command can be followed by multiple commands in the same string
//      - "searchmoves <move1> ... <movei>" - search only these moves
//      - "ponder" - start in pondering mode
//      - "wtime <x>" - white has x msec left on the clock
//     - "btime <x>" - black has x msec left on the clock
//     - "winc <x>" - white increment per move in msec if x > 0
//    - "binc <x>" - black increment per move in msec if x > 0
//    - "movestogo <x>" - there are x moves to the next time control
//   - "depth <x>" - search x plies only
//  - "nodes <x>" - search x nodes only
// - "mate <x>" - search for a mate in x moves
// - "movetime <x>" - search exactly x msec
// - "infinite" - search until the "stop" command
//
// "stop" - stop calculating as soon as possible, reply with bestmove
// "ponderhit" - the user has played the expected move while pondering,
// "quit" - quit the program as soon as possible

// Engine
// "id" - must be sent after "uci" command
// - "name <x>" - the name of the engine
// - "author <x>" - the author of the engine
// "uciok" - must be sent after "id" and "option" commands, engine is ready
// "readyok" - must be sent as response to "isready" command
// "bestmove <move> [ponder <move>]" - sent after "go" or "stop" command

// "copyprotection [ok | error]" - sent after "cp" command
// "registration [ok | error]" - sent after "register" command
// "info" - sent during search to inform the GUI about the search status
// - "depth <x>" - current search depth in plies
// - "seldepth <x>" - current selective search depth in plies
// - "time <x>" - total search time in msec
// - "nodes <x>" - total number of nodes searched
// - "pv <move1> ... <movei>" - the principal variation line
// - "multipv <x>" - the x-th line of the multipv list
// - "score cp <x>" - the score from the engine's point of view in centipawns
// - "score mate <x>" - mate in x moves from the engine's point of view
// - "currmove <move>" - currently searched move
// - "currmovenumber <x>" - currently searched move number
// - "hashfull <x>" - hash table is x/1000 full
// - "nps <x>" - nodes per second
// - "tbhits <x>" - number of tablebase hits
// - "cpuload <x>" - current CPU load in percent
// - "string <x>" - string to be displayed in the GUI
// - "refutation <move1> ... <movei>" - the refutation line
// - "currline <x> <move1> ... <movei>" - the current line at depth x

// Options
// "option name <id> type <t> [default <x>] [min <x>] [max <x>] [var <x1> ... <xi>]"
// - "name <id>" - the name of the option, must be unique
// - "type <t>" - the type of the option, one of "check", "spin", "combo", "button", or "string"
// - "default <x>" - the default value of the option
// - "min <x>" - the minimum value for spin options
// - "max <x>" - the maximum value for spin options
// - "var <x1> ... <xi>" - the list of possible values for combo options
void cldr::Engine::prompt(std::string input) {
	// check string
	assert(!input.empty());

	// UCI reserves stdout for protocol replies only. When debug mode is on we
	// route engine logging to stderr; otherwise it is suppressed. Either way
	// stdout carries nothing but protocol lines.
	ShLogPr lg = _debug ? StderrLog::create() : NullLog::create();

	// parse input

	// uci options
	if(input == "uci") {
		// id name
		std::cout << "id name Collider V2.0\n";
		std::cout << "id author Dylan Kolb-Bond\n";
		// uciok
		std::cout << "uciok\n";
		return;
	} else if(input == "debug" || input == "debug on") {
		// "debug [on|off]" toggles extra info; bare "debug" turns it on
		_debug = true;
		return;
	} else if(input == "debug off") {
		_debug = false;
		return;
	} else if(input == "isready") {
		// readyok
		std::cout << "readyok\n";
		return;
	} else if(input == "ucinewgame") {
		// new game, reset board
		if(_board != NULL) { _board->init(Board::start_fen(), lg); }
		return;
	}

	// if input has "position"
	if(input.find("position") != std::string::npos) {
		if(input.find("startpos") != std::string::npos) {
			// set to start position
			_board->init(Board::start_fen(), lg);
		} else {
			std::cout << "Error: Only 'startpos' is currently supported in 'position' command." << std::endl;
			//std::err << "Error:";
			collider_throw_line("Only 'startpos' is currently supported in 'position' command.");
		}
		// "moves"
		if(input.find("moves") != std::string::npos) {

			// find algebraic moves
			size_t pos = input.find("moves");
			pos += 6; // move past "moves "
			while(pos < input.length()) {
				// get next move
				std::string move_str = "";
				while(pos < input.length() && input[pos] != ' ') {
					//while(pos < input.length()) {
					move_str += input[pos];
					pos++;
				}

				assert(_board != NULL);

				// apply move
				if(!_board->move(move_str)) {
					std::cerr << "Failed to apply move: " << move_str << std::endl;
					collider_throw_line("Invalid move attempted in 'moves' command.");
				}

				_board->update_movelist(lg);

				// skip space
				while(pos < input.length() && input[pos] == ' ') { pos++; }
			}
		}
		return;
	}

	// "go" command
	if(input.find("go") != std::string::npos) {
		// honor "go depth <N>"; fall back to a sane default otherwise.
		// "movetime"/clock controls are parsed but not yet used for a timed
		// search (no iterative deepening yet) -- documented gap.
		const arma::uword default_depth = 4;
		const arma::uword depth = parse_uint(input, "depth", default_depth);
		const std::string best_move_str = best_move(depth, lg);
		if(!best_move_str.empty()) {
			std::cout << "bestmove " << best_move_str << std::endl;
		}
		return;
	}

	// "stop" command
	if(input.find("stop") != std::string::npos) {
		//
	}


	// return
	return;
}

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
// search the root position and return the best move in algebraic notation
std::string cldr::Engine::best_move(arma::uword depth, ShLogPr lg) {
	// remember the requested depth before the per-move reset clears _max_depth
	_last_depth = depth;

	// generate the root moves
	_board->update_movelist(lg);
	const arma::Mat<arma::uword> movelist = _board->get_movelist();
	const arma::uword num_moves = movelist.n_cols;

	// no legal moves (checkmate or stalemate)
	if(num_moves == 0) { return ""; }

	arma::uword idx_best_move = 0;
	arma::sword best_score = -100000;
	const int alpha = -100000;
	const int beta = 100000;

	// search each root move
	for(arma::uword i = 0; i < num_moves; i++) {

		// run alphabeta on each move to find the best move
		const arma::Col<arma::uword> mymove = movelist.col(i);
		const std::string movestr = _board->get_algebraic_string(mymove(0), mymove(1), static_cast<PieceType>(mymove(2)));
		lg->msg("%s %5s %5llu %s", KYEL, movestr.c_str(), depth, KNRM);
		if(!_board->move(movestr)) {
			lg->msg("%sError moving move: %s%s\n", KRED, movestr.c_str(), KNRM);
			continue;
		}
		// alphabeta
		const int score = -alpha_beta(alpha, beta, depth, lg);
		lg->msg("%s    %05d%s", KYEL, score, KNRM);
		display_alphabeta(lg);
		reset_alphabeta();
		if(!_board->unmove(movestr)) {
			lg->msg("%sError unmoving move: %s%s\n", KRED, movestr.c_str(), KNRM);
			continue;
		}

		// check best score
		if(score > best_score) {
			best_score = score;
			idx_best_move = i;
		}
	}

	// build the best move string (board is left unchanged)
	const arma::Col<arma::uword> best = movelist.col(idx_best_move);
	return _board->get_algebraic_string(best(0), best(1), static_cast<PieceType>(best(2)));
}

// alphabeta (negamax)
int cldr::Engine::alpha_beta(int alpha, int beta, arma::uword depth, ShLogPr lg) {
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

	const bool use_quiescence = true;
	if(depth == 0) {
		_num_nodes++;
		// run evaluation
		if(use_quiescence) {
			return quiescence(alpha, beta, _max_depth_quiescence, lg);
		} else {
			return _board->evaluate();
		}
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

		// move
		if(!_board->move(movestr)) { collider_throw_line("Invalid move attempted in alpha_beta."); }

		// recursive score call
		score = -alpha_beta(-beta, -alpha, depth - 1, lg);

		// unmove
		if(!_board->unmove(movestr)) { collider_throw_line("Invalid unmove attempted in alpha_beta."); }

		// check score against best value
		if(score > best_value) {
			best_value = score;
			if(score > alpha) {
				alpha = score; // alpha acts like max in MiniMax
			}
		}

		// early exit
		if(score >= beta) {
			return best_value; // fail soft beta-cutoff, exiting the loop here is also fine
		}
	}

	// timer and log
	const double time_elapsed = timer.toc();
	_time_alphabeta(depth - 1) += time_elapsed;
	//lg->msg("%sdepth(%llu) completed in %0.3f [s]%s\n", KCYN, depth, time_elapsed, KNRM);

	// return best move
	return best_value;
}

// quiescence search
// https://www.chessprogramming.org/Quiescence_Search
int cldr::Engine::quiescence(int alpha, int beta, arma::uword depth, ShLogPr lg) {
	// base eval
	int static_eval = _board->evaluate();

	// stand pat
	int best_value = static_eval;
	if(best_value >= beta) { return best_value; }
	if(best_value > alpha) { alpha = best_value; }

	// early exit
	if(depth == 0) { return best_value; }

	// while still captures
	_board->update_movelist();
	arma::Mat<arma::uword> movelist = _board->get_movelist();
	const arma::uword num_moves = movelist.n_cols;

	// prune non-captures
	_board->prune_non_captures(movelist);
	const arma::uword num_captures = movelist.n_cols;

	// walk over captures
	int score = 0;
	for(arma::uword i = 0; i < num_captures; i++) {
		arma::Col<arma::uword> mymove = movelist.col(i);
		std::string movestr = _board->get_algebraic_string(mymove(0), mymove(1), static_cast<PieceType>(mymove(2)));

		// move
		if(!_board->move(movestr)) { collider_throw_line("Invalid move attempted in quiescence."); }

		// recursive score call
		score = -quiescence(-beta, -alpha, depth - 1, lg);

		// unmove
		if(!_board->unmove(movestr)) { collider_throw_line("Invalid unmove attempted in alpha_beta."); }

		// check score
		if(score >= beta) return score;
		if(score > best_value) best_value = score;
		if(score > alpha) alpha = score;
	}


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
