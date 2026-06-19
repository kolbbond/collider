#pragma once

#include "Board.hxx"
#include <armadillo>
namespace cldr {

using ShEnginePr = std::shared_ptr<class Engine>;

class Engine {
public:
	ShBoardPr _board;

	// UCI "debug" mode: when true, prompt() routes engine logging to stderr
	// (via StderrLog) instead of suppressing it; stdout always stays clean.
	bool _debug = false;

	// perft stats to sum
	PerftStats _total_stats;

	arma::uword _max_depth = 0;
	// depth requested for the most recent best_move() search; survives the
	// per-root-move reset_alphabeta() and feeds "info depth" output.
	arma::uword _last_depth = 0;
	arma::Row<arma::uword> _num_nodes_per_depth;
	arma::uword _num_nodes = 0;
	arma::Row<double> _time_alphabeta;

	arma::uword _max_depth_quiescence = 2;
	arma::uword _num_nodes_quiescence = 0;
	double _time_quiescence = 0;

public:
	Engine();
	Engine(ShBoardPr board) { set_board(board); }
	static ShEnginePr create() { return std::make_shared<Engine>(); }
	static ShEnginePr create(ShBoardPr board) { return std::make_shared<Engine>(board); }
	void set_board(ShBoardPr board) { _board = board; }

	// uci prompt
	void prompt(std::string input);

	// search the root position and return the best move in algebraic notation
	// (e.g. e2e4); returns "" when there are no legal moves. leaves the board
	// unchanged.
	std::string best_move(arma::uword depth, ShLogPr lg = NullLog::create());

	// https://www.chessprogramming.org/Alpha-Beta
	int alpha_beta(int alpha, int beta, arma::uword depth, ShLogPr lg = NullLog::create());
	void display_alphabeta(cldr::ShLogPr lg);
	void reset_alphabeta() {
		_max_depth = 0;
		_time_alphabeta.reset();
		_num_nodes_per_depth.reset();
		_num_nodes = 0;
	}
	int quiescence(int alpha, int beta, arma::uword depth, ShLogPr lg);


	// https://www.chessprogramming.org/Perft
	arma::uword perft(arma::uword depth, cldr::ShLogPr lg = cldr::NullLog::create());
};
} // namespace cldr
