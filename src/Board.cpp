#include <armadillo>
#include "Extra.hpp"
#include "Board.hpp"
#include "debug.hpp"
#include "PieceMoves.hpp"
#include "typedefs.hh"

namespace cldr {

Board::Board() {}
Board::Board(std::string fen, ShLogPr lg) { init(fen, lg); }
ShBoardPr Board::create() { return std::make_shared<Board>(); }
ShBoardPr Board::create(std::string fen, ShLogPr lg) { return std::make_shared<Board>(fen, lg); }

// default fen
// "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
void Board::init(std::string fen, ShLogPr lg) {
	// parse fen and initialize board

	set_log(lg);

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

void Board::update(ShLogPr lg) { update_movelist(lg); }

// update valid moves
void Board::update_movelist(ShLogPr lg) {

	// allocate movelist
	std::vector<arma::uword> from_vec;
	std::vector<arma::uword> to_vec;

	// walk over squares
	// @hey: if we had just the pieces we won't need a full loop
	//       probably not an issue???
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
			arma::Row<arma::uword> moves = get_moves(sq120);

			// add to vectors
			from_vec.insert(from_vec.end(), moves.n_elem, sq120);
			to_vec.insert(to_vec.end(), moves.begin(), moves.end());
		}
	}

	// convert to arma::Mat
	_movelist = arma::join_vert(arma::Row<arma::uword>(from_vec), arma::Row<arma::uword>(to_vec));
}

// return possible to squares in 120 index
arma::Row<arma::uword> Board::get_moves(arma::uword sq120) {

	// check piece
	ShPiecePr pc = _board120[sq120];
	PieceType type = pc->get_type();
	PieceColor color = pc->get_color();

    // early exit
	if(type == PieceType::NONE) { return arma::Row<arma::uword>(); }

	// get base move directions
	arma::Row<arma::sword> move_directions = PieceMoves::get_moves(type, color);


	// allocate output
	arma::Row<arma::uword> moves;
	std::vector<arma::uword> move_vec;

	// @hey: generate all possible squares or spread out

	// handle each piece type
	// @hey: likely can handle knight with sliding pieces but with step of 1
	if(type == PieceType::KNIGHT) {

		// walk over directions
		for(arma::sword i = 0; i < move_directions.n_elem; i++) {
			arma::sword mydir = move_directions(i);
			arma::uword tosq = sq120 + mydir;
			assert(tosq < 120);
			ShPiecePr to_pc = _board120[tosq];

			//std::cout << "Knight move to sq120: " << tosq << " which is (" << Extra::sq120to64(tosq) << ") and has piece: " << to_pc->get_piece_char() << std::endl;

			// check if offboard
			if(to_pc->get_type() == PieceType::OFFBOARD) continue;

			// check if same color
			if(to_pc->get_color() == color) continue;

			// if opposite color it is a capture
			if(to_pc->get_color() == pc->get_enemy_color(color)) { move_vec.push_back(tosq); }

			// if empty square also good
			if(to_pc->get_color() == PieceColor::NONE) { move_vec.push_back(tosq); }
		}
	}

	// sliding pieces
	else if(type == PieceType::ROOK || type == PieceType::BISHOP || type == PieceType::QUEEN) {

		// walk over directions
		for(arma::sword i = 0; i < move_directions.n_elem; i++) {

			// walk until failure
			arma::uword step = 1;
			for(;;) {
				arma::sword mydir = move_directions(i);
				arma::uword tosq = sq120 + mydir * step;
				assert(tosq < 120);
				ShPiecePr to_pc = _board120[tosq];

				// check if offboard
				if(to_pc->get_type() == PieceType::OFFBOARD) break;

				// check if same color
				if(to_pc->get_color() == color) break;

				// if opposite color it is a capture
				if(to_pc->get_color() == pc->get_enemy_color()) {
					move_vec.push_back(tosq);
					break;
				}

				// if empty square also good
				if(to_pc->get_color() == PieceColor::NONE) { move_vec.push_back(tosq); }

				// step
				step++;
			}
		}
	}

	// Exceptions: pawn first move, en passant, castling, king moving into check

	// handle pawn
	else if(type == PieceType::PAWN) {

		// walk over directions
		for(arma::sword i = 0; i < move_directions.n_elem; i++) {
			arma::sword mydir = move_directions(i);
			arma::uword tosq = sq120 + mydir;
			assert(tosq < 120);
			ShPiecePr to_pc = _board120[tosq];

			// pawn moves
			if(mydir == static_cast<arma::sword>(MoveDirections::UP) || mydir == static_cast<arma::sword>(MoveDirections::DOWN)) {

				if(to_pc->get_type() != PieceType::NONE)
					continue;

				else {
					// add to move vec
					move_vec.push_back(tosq);

					// if moved then
					// check first move for double step
					if(!pc->has_moved()) {
						arma::sword mydir = move_directions(i);
						arma::uword tosq = sq120 + mydir * 2;
						assert(tosq < 120);
						ShPiecePr to_pc = _board120[tosq];
						if(to_pc->get_type() != PieceType::NONE) {
							continue;
						} else {
							move_vec.push_back(tosq);
						}
					}
				}

				// pawn captures
				if(to_pc->get_color() == pc->get_enemy_color(color) || to_pc->get_type() == PieceType::ENPASSANT) {
					// add to move vec
					move_vec.push_back(tosq);
				}
			}
		}
	}

	// handle king
	else if(type == PieceType::KING) {

		// walk over directions
		for(arma::sword i = 0; i < move_directions.n_elem; i++) {
			arma::sword mydir = move_directions(i);
			arma::uword tosq = sq120 + mydir;
			assert(tosq < 120);
			ShPiecePr to_pc = _board120[tosq];

			// check if offboard
			if(to_pc->get_type() == PieceType::OFFBOARD) continue;

			// check if same color
			if(to_pc->get_color() == color) continue;

			// if opposite color it is a capture
			if(to_pc->get_color() == pc->get_enemy_color(color)) { move_vec.push_back(tosq); }

			// if empty square
			// @hey: check if moving into check
			if(to_pc->get_color() == PieceColor::NONE) {
				// @TODO: need to generate movelist for opposing color
				// or make this move then generate movelist for opposing color
				// to check if it is a capture

				// add the move
				move_vec.push_back(tosq);
			}
		}

		// castling?


	}

	// shouldn't be here
	else {
		collider_throw_line("Invalid piece type for move generation.");
	}

	// convert to arma and return
	moves = arma::Row<arma::uword>(move_vec);
	return moves;
}

arma::Mat<arma::uword> Board::get_movelist() const { return _movelist; }

// move in UCI format, e.g. e2e4
bool Board::move(std::string move_str) {

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

	// check if move is valid
	if(!is_valid(fr_sq120, to_sq120)) {
		_lg->msg("%sInvalid move attempted.%s\n", KRED, KNRM);
		return false;
	}

	// check piece on from square
	ShPiecePr fr_pc = _board120[fr_sq120];
	ShPiecePr to_pc = _board120[to_sq120];

	assert(fr_pc->get_type() != PieceType::NONE);
	if(to_pc->get_color() == fr_pc->get_enemy_color() && to_pc->get_type() != PieceType::NONE) {
		_lg->msg("%sCapturing piece: %c at square %s%s\n", KRED, to_pc->get_piece_char(), tosq.c_str(), KNRM);
		to_pc->set_alive(false);
	}

	// move piece finally
	_board120[to_sq120] = fr_pc;
	fr_pc->set_pos(to_rank, to_file);
	fr_pc->set_moved(true);
	_board120[fr_sq120] = Piece::create(PieceColor::NONE, PieceType::NONE, fr_rank, fr_file);

    // success
    return true;
}
// check if move is valid
bool Board::is_valid(arma::uword fr_sq120, arma::uword to_sq120) {

	// get valid moves in this position
	arma::Row<arma::uword> valid_moves = get_moves(fr_sq120);

	// check valid moves
	for(arma::uword i = 0; i < valid_moves.n_elem; i++) {
		if(valid_moves(i) == to_sq120) { return true; }
	}

	// default
	return false;
}


void Board::set_log(ShLogPr lg) {
	assert(lg != nullptr);
	_lg = lg;
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
	//lg->newl();
	//lg->msg("\t%s --- Displaying Board --- %s\n", KCYN, KNRM);
	//lg->newl();

	// get 64 board
	std::array<ShPiecePr, 64> board64 = get_board64();

	// print top file
	lg->newl();
	lg->msg("\t%s --- ---------------- --- %s\n", KCYN, KNRM);
	lg->msg("\t     ");
	for(char f = 'a'; f <= 'h'; f++) lg->msg("%s%c ", KCYN, f);
	lg->newl();
	lg->msg("\t%s --- ---------------- --- %s\n", KCYN, KNRM);

	// print board backwards
	for(int rank = 7; rank >= 0; rank--) {
		lg->msg("\t%s %d | ", KCYN, rank);
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

		// end rank
		lg->msg("%s| %d", KCYN, rank + 1);

		// new rank
		lg->msg("\n");
	}

	// print files at end
	lg->msg("\t%s --- ---------------- --- %s\n", KCYN, KNRM);
	lg->msg("\t     ");
	for(int f = 0; f <= 7; f++) lg->msg("%s%d ", KCYN, f);
	lg->newl();
	lg->msg("\t%s --- ---------------- --- %s\n", KCYN, KNRM);
	lg->newl();
}

std::string Board::start_fen() { return "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; }

} // namespace cldr
