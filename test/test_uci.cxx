// test uci interface
//
// Drives cldr::Engine::prompt() the way a chess GUI would: feed protocol
// lines, capture what the engine writes to stdout, and assert on it.
#include <armadillo>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

#include "Board.hxx"
#include "Engine.hxx"
#include "Log.hxx"

namespace {

int g_failures = 0;

void check(bool cond, const std::string& what) {
	if(cond) {
		std::cerr << KGRN << "  ok   " << KNRM << what << "\n";
	} else {
		std::cerr << KRED << "  FAIL " << KNRM << what << "\n";
		++g_failures;
	}
}

// Run one prompt line and return everything the engine wrote to the real
// stdout file descriptor. We capture at the fd level (not just std::cout's
// streambuf) because the engine logs via printf too, and a chess GUI sees
// every byte on fd 1 regardless of which API produced it.
std::string capture_prompt(cldr::ShEnginePr engine, const std::string& line) {
	const char* tmp = "test_uci_capture.tmp";

	std::cout.flush();
	std::fflush(stdout);
	int saved = dup(STDOUT_FILENO);
	std::FILE* redir = std::freopen(tmp, "w", stdout);
	(void)redir;

	engine->prompt(line);

	std::cout.flush();
	std::fflush(stdout);
	dup2(saved, STDOUT_FILENO);
	close(saved);
	clearerr(stdout);

	std::ifstream in(tmp);
	std::stringstream buf;
	buf << in.rdbuf();
	in.close();
	std::remove(tmp);
	return buf.str();
}

cldr::ShEnginePr fresh_engine() {
	cldr::ShBoardPr board = cldr::Board::create(cldr::Board::start_fen(), cldr::NullLog::create());
	return cldr::Engine::create(board);
}

// "position ... moves ..." must not write anything to stdout. The UCI protocol
// reserves stdout for protocol replies only; debug chatter belongs on stderr.
void test_position_moves_no_stdout_pollution() {
	cldr::ShEnginePr engine = fresh_engine();
	const std::string out = capture_prompt(engine, "position startpos moves e2e4");
	check(out.empty(), "position startpos moves e2e4 emits nothing on stdout");
}

// "ucinewgame" must restore the starting position after moves were played.
// We probe side-to-move: White at the start, Black after one White move,
// White again once the board is reset.
void test_ucinewgame_resets_board() {
	cldr::ShEnginePr engine = fresh_engine();
	capture_prompt(engine, "position startpos moves e2e4");
	check(engine->_board->get_turn() == PieceColor::BLACK, "Black to move after e2e4 (precondition)");

	capture_prompt(engine, "ucinewgame");
	check(engine->_board->get_turn() == PieceColor::WHITE, "ucinewgame restores White to move");
}

bool starts_with(const std::string& s, const std::string& prefix) {
	return s.rfind(prefix, 0) == 0;
}

// "go" must answer with a "bestmove <move>" line on stdout.
void test_go_outputs_bestmove() {
	cldr::ShEnginePr engine = fresh_engine();
	capture_prompt(engine, "position startpos");
	const std::string out = capture_prompt(engine, "go depth 2");
	check(starts_with(out, "bestmove "), "go replies with a bestmove line");
}

// "go depth N" must search to the requested depth, not a hardcoded one.
void test_go_depth_is_honored() {
	cldr::ShEnginePr engine = fresh_engine();
	capture_prompt(engine, "position startpos");
	capture_prompt(engine, "go depth 3");
	check(engine->_last_depth == 3, "go depth 3 searches to depth 3");
}

// The "debug" command toggles the engine's debug logging flag.
void test_debug_command_toggles_flag() {
	cldr::ShEnginePr engine = fresh_engine();
	capture_prompt(engine, "debug on");
	check(engine->_debug, "debug on enables debug logging");
	capture_prompt(engine, "debug off");
	check(!engine->_debug, "debug off disables debug logging");
}

// Even with debug enabled, engine logging must not reach stdout -- it belongs
// on stderr so the GUI's protocol stream stays clean.
void test_debug_on_keeps_stdout_clean() {
	cldr::ShEnginePr engine = fresh_engine();
	capture_prompt(engine, "debug on");
	const std::string out = capture_prompt(engine, "position startpos moves e2e4");
	check(out.empty(), "debug logging stays off stdout");
}

} // namespace

int main(int argc, char** argv) {
	arma::wall_clock timer;
	timer.tic();

	cldr::ShLogPr lg = cldr::Log::create();
	lg->msg("%s --- Test UCI: %s %s --- %s\n", KPNK, __DATE__, __TIME__, KNRM);

	test_position_moves_no_stdout_pollution();
	test_ucinewgame_resets_board();
	test_go_outputs_bestmove();
	test_go_depth_is_honored();
	test_debug_command_toggles_flag();
	test_debug_on_keeps_stdout_clean();

	lg->msg("%s --- Test Time Elapsed: %0.2f --- %s\n", KCYN, timer.toc(), KNRM);
	return g_failures == 0 ? 0 : 1;
}
