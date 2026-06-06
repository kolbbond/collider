// main entry point
#include "debug.hxx"
#include "Log.hxx"
#include "Board.hxx"
#include "Engine.hxx"
#ifdef ENABLE_GUI
#include "Display.hxx"
#endif

// display help options
void display_help() {
	std::cout << "Collider V2.0\n";
	std::cout << "Usage: collider [options]\n";
	std::cout << "Options:\n";
	std::cout << "  --cli        Start in command-line gameplay mode.\n";
#ifdef ENABLE_GUI
	std::cout << "  --gui        Start in graphical (SDL) gameplay mode.\n";
#endif
	std::cout << "  --help       Display this help message.\n";
	std::cout << "If no options are provided, the engine starts in UCI mode.\n";
}
int main(int argc, char** argv) {

	// logger
	cldr::ShLogPr lg = cldr::Log::create();
	lg->msg("%s --- Collider V2.0 --- %s\n", KPNK, KNRM);

	// check for command line arguments
	bool uci_mode = true;
	bool gui_mode = false;
	if(argc > 1) {
		std::cout << "Usage: " << argv[0] << "\n";
		if(std::string(argv[1]) == "--cli") {
			uci_mode = false;
		} else if(std::string(argv[1]) == "--gui") {
			uci_mode = false;
			gui_mode = true;
		} else if(std::string(argv[1]) == "--help") {
			// display options
			display_help();
			return 0;
		} else {
			// no options starts in UCI mode
		}
	}

#ifdef ENABLE_GUI
	// graphical gameplay mode
	if(gui_mode) {
		cldr::ShLogPr glg = cldr::Log::create();
		cldr::ShBoardPr board = cldr::Board::create(cldr::Board::start_fen(), glg);
		cldr::ShEnginePr engine = cldr::Engine::create(board);
		cldr::ShDisplayPr display = cldr::Display::create(glg);
		if(!display->ok()) {
			glg->msg("%sFailed to start SDL display.%s\n", KRED, KNRM);
			return 1;
		}
		display->run(board, engine, glg);
		return 0;
	}
#else
	if(gui_mode) {
		std::cout << "GUI support not built. Reconfigure with -DENABLE_GUI=ON.\n";
		return 1;
	}
#endif

	// uci mode for engine communication
	if(uci_mode) {
		std::cout << "Entering UCI mode...\n";
		cldr::ShLogPr lg = cldr::Log::create();
		lg->msg("%s --- Collider V2 --- %s\n", KPNK, KNRM);

		// create board and engine
		cldr::ShBoardPr board = cldr::Board::create(cldr::Board::start_fen(), lg);
		cldr::ShEnginePr engine = cldr::Engine::create(board);

		bool done = false;
		while(!done) {
			std::string input_line;
			std::getline(std::cin, input_line);
			if(input_line == "quit") {
				done = true;
			} else {
				// process UCI commands
				engine->prompt(input_line);
			}
		}
		// get

	}
	// user gameplay mode
	else if(!uci_mode) {
		// user gameplay
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

			// engine reply
			lg->msg("%sEngine is calculating its move...%s\n", KBLU, KNRM);
			lg->msg("%s #move   #ply   #score   #nodes    #time     #nps %s\n", KYEL, KNRM);
			const arma::uword depth = 3;
			const std::string best_move_str = engine->best_move(depth, lg);
			if(!best_move_str.empty()) {
				if(board->move(best_move_str)) {
					lg->msg("%sEngine plays move: %s%s\n", KGRN, best_move_str.c_str(), KNRM);
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
	} else {
		collider_throw_line("Unknown mode selected.");
	}
}
