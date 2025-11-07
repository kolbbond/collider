#pragma once
// UCI

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

//
#include "typedefs.hh"
#include <memory>

namespace cldr {
using ShUCIPr = std::shared_ptr<class UCI>;
class UCI {
public:
	// handle prompt from cpu
	void prompt(std::string input);
};
} // namespace cldr
