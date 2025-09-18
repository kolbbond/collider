#include <armadillo>
#include "Extra.hpp"
#include "Board.hpp"
#include "debug.hh"

namespace cldr {

Board::Board() {}
Board::Board(std::string fen, ShLogPr lg) { init(fen, lg); }
ShBoardPr Board::create() { return std::make_shared<Board>(); }
ShBoardPr Board::create(std::string fen, ShLogPr lg) { return std::make_shared<Board>(fen, lg); }

// default fen
// "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
void Board::init(std::string fen, ShLogPr lg) {
	// parse fen and initialize board

	// log
	assert(lg != nullptr);
	lg->msg("%sInitializing board with FEN: %s%s\n", KPNK, fen.c_str(), KNRM);

	// create a timer
	arma::wall_clock timer;
	timer.tic();

	// confirm there are 7 slashes
	arma::uword slash_count = 0;
	for(char c : fen) {
		if(c == '/') { slash_count++; }
	}
	assert(slash_count == 7);

	std::printf("Slash count: %llu\n", slash_count);
	// "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
	// initialize boards @hey: can we just use one?
	//for(arma::uword i = 0; i < 64; i++) { _board64[i] = Piece::create(PieceColor::NONE, PieceType::NONE, i); }
	for(arma::uword i = 0; i < 120; i++) { _board120[i] = Piece::create(PieceColor::NONE, PieceType::OFFBOARD, Extra::sq120to64(i)); }

	// fill empty squares
	for(arma::uword r = 0; r < 8; r++) {
		for(arma::uword f = 0; f < 8; f++) {
			arma::uword sq120 = Extra::rf2sq120(r, f);
			_board120[sq120] = Piece::create(PieceColor::NONE, PieceType::NONE, r, f);
		}
	}

	// walk over characters in fen
	arma::uword rank = 0;
	arma::uword file = 0;
	arma::uword cnt = 0;
	for(char c : fen) {

		//std::printf("(r,f,counter): (%llu, %llu, %llu)\n", rank, file, cnt);
		// @hey: we can break if the rf2sq64 == 64
		if(cnt >= 64) break;

		// check if slash
		if(c == '/') {
			// next rank
			rank++;
			file = 0;
			continue;
		}

		// check if number
		if(isdigit(c)) {
			// number of empty squares
			arma::uword num_empty = c - '0';
			file += num_empty;
			cnt += num_empty;
			continue;
		}

		// assume empty
		PieceType ptype = PieceType::NONE;
		PieceColor pcolor = PieceColor::NONE;

		// check char is piece
		switch(tolower(c)) {
		case 'p': ptype = PieceType::PAWN; break;
		case 'r': ptype = PieceType::ROOK; break;
		case 'n': ptype = PieceType::KNIGHT; break;
		case 'b': ptype = PieceType::BISHOP; break;
		case 'q': ptype = PieceType::QUEEN; break;
		case 'k': ptype = PieceType::KING; break;
		default: break;
		}

		// check color
		if(ptype != PieceType::NONE) {
			if(isupper(c)) {
				pcolor = PieceColor::BLACK;
			} else {
				pcolor = PieceColor::WHITE;
			}
		}

		// debug output

		/*
		lg->msg("Placing piece: (%c, %c) (%s, %s) at rank %llu, file %llu (sq64: %llu, sq120: %llu)\n",
			c,
			Piece::get_piece_char(pcolor, ptype),
			Piece::get_type_string(ptype).c_str(),
			Piece::get_color_string(pcolor).c_str(),
			rank,
			file,
			Extra::rf2sq64(rank, file),
			Extra::rf2sq120(rank, file));
            */

		// insert into board
		//_board64[Extra::rf2sq64(rank, file)] = Piece::create(pcolor, ptype, rank, file);
		_board120[Extra::rf2sq120(rank, file)] = Piece::create(pcolor, ptype, rank, file);

		// increment
		cnt++;
		file++;
	}


	// time and log out
	lg->msg("%sBoard initialized in %.6f seconds.%s\n", KGRN, timer.toc(), KNRM);
}

// update valid moves
void Board::update_movelist() {

	// allocate movelist
	_movelist.set_size(2, 1024); // two rows: from, to

	// walk over squares
	// @hey: if we had just the pieces we won't need a full loop
	//       probably not an issue???
	arma::uword cnt = 0;
	for(arma::uword r = 0; r < 8; r++) {
		for(arma::uword f = 0; f < 8; f++) {

			// get sq
			arma::uword sq120 = Extra::rf2sq120(r, f);

			// check piece
			ShPiecePr pc = _board120[sq120];
			PieceType type = pc->get_type();
			PieceColor color = pc->get_color();
			if(type == PieceType::NONE) continue;

			// generate possible moves for this piece at this square


		}
	}
}


// move in UCI format, e.g. e2e4
void Board::move(std::string move_str) {

	// parse move
	assert(move_str.length() == 4 || move_str.length() == 5); //

	// get first two chars
	std::string frsq = move_str.substr(0, 2);
	std::string tosq = move_str.substr(2, move_str.length() - 1);

	// get ranks and files
	arma::uword fr_rank = Extra::char2rank(frsq[1]);
	arma::uword fr_file = Extra::char2file(frsq[0]);

	arma::uword to_rank = Extra::char2rank(tosq[1]);
	arma::uword to_file = Extra::char2file(tosq[0]);

	arma::uword fr_sq120 = Extra::rf2sq120(fr_rank, fr_file);
	arma::uword to_sq120 = Extra::rf2sq120(to_rank, to_file);

	// debug
	std::printf("From square: %s, To square: %s\n", frsq.c_str(), tosq.c_str());
	std::printf("fr_rank: %llu, fr_file: %llu\n", fr_rank, fr_file);
	std::printf("to_rank: %llu, to_file: %llu\n", to_rank, to_file);
	std::printf("fr_sq120: %llu, to_sq120: %llu\n", fr_sq120, to_sq120);

	// check piece on from square
	ShPiecePr fr_pc = _board120[fr_sq120];
	ShPiecePr to_pc = _board120[to_sq120];
	assert(fr_pc->get_type() != PieceType::NONE);

	// move piece
	_board120[to_sq120] = fr_pc;
	fr_pc->set_pos(to_rank, to_file);
	_board120[fr_sq120] = Piece::create(PieceColor::NONE, PieceType::NONE, fr_rank, fr_file);
}

std::array<ShPiecePr, 120> Board::get_board120() const { return _board120; }
std::array<ShPiecePr, 64> Board::get_board64() const {

	// convert 120 board to 64 board
	std::array<ShPiecePr, 64> board64;
	for(arma::uword r = 0; r < 8; r++) {
		for(arma::uword f = 0; f < 8; f++) {
			arma::uword sq64 = Extra::rf2sq64(r, f);
			arma::uword sq120 = Extra::rf2sq120(r, f);
			board64[sq64] = _board120[sq120];
		}
	}

	// return board
	return board64;
}


void Board::display_board(ShLogPr lg) {
	lg->msg("%sDisplaying board:%s\n", KCYN, KNRM);

	// get 64 board
	std::array<ShPiecePr, 64> board64 = get_board64();

	// print board backwards
	for(int rank = 7; rank >= 0; rank--) {
		for(int file = 0; file < 8; file++) {

			// get this sq64 index
			u64 sq64 = Extra::rf2sq64(rank, file);
			ShPiecePr pc = board64[sq64];
			char mychar = pc->get_piece_char();

			// get color
			std::string KMY = KNRM;
			if(pc->get_color() == PieceColor::WHITE)
				KMY = KYEL;
			else if(pc->get_color() == PieceColor::BLACK)
				KMY = KBLU;
			else
				KMY = KNRM;

			// place piece character
			lg->msg("%s%c %s", KMY.c_str(), mychar, KNRM);
		}

		// new rank
		lg->msg("\n");
	}
}

std::string Board::start_fen() { return "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; }

} // namespace cldr
