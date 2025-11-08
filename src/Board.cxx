#include <armadillo>
#include <iostream>
#include "Extra.hxx"
#include "Board.hxx"
#include "debug.hxx"
#include "PieceMoves.hxx"
#include "error.hxx"
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

	//////////////////////////////////////////////////
	// Parse FEN
	//////////////////////////////////////////////////

	// walk over characters in fen
	arma::uword rank = 0;
	arma::uword file = 0;
	arma::uword cnt = 0;
	arma::uword num_chars = 0;
	for(char c : fen) {
		num_chars++;

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

	// parse rest of FEN string
	// this is includes whos turn it is, where can castle
	// w KQkq - 0 1"
	cnt = 0;
	for(char c : fen) {
		cnt++;
		if(cnt < num_chars) continue;

		// check color
		if(c == 'w') {
			set_color(PieceColor::WHITE);
			continue;
		} else if(c == 'b') {
			set_color(PieceColor::BLACK);
			continue;
		}

		// check castling rights
		if(c == 'K') {
			continue;
		} else if(c == 'Q') {
			continue;
		} else if(c == 'k') {
			continue;
		} else if(c == 'q') {
			continue;
		}
	}

	// time and log out
	lg->msg("%sBoard initialized in %.6f seconds.%s\n", KGRN, timer.toc(), KNRM);
}

void Board::update(ShLogPr lg) { update_movelist(lg); }

// overload for depth=1? @hey: or just change the default value of depth in the function signature?
// don't want to have two default arguments???
void Board::update_movelist(ShLogPr lg) {
	const arma::uword depth = 1;
	update_movelist(depth, lg);
}

// update valid moves
void Board::update_movelist(arma::uword depth, ShLogPr lg) {

	// early exit
	if(depth == 0) { return; }

	arma::Mat<arma::uword> movelist = create_movelist(1, lg);
	assert(movelist.n_rows == 2); // should be from and to rows

	// check for checks
	// naively seems like make the move, check if there is a king capture, unmove?
	arma::Row<arma::uword> moveidx(movelist.n_cols, arma::fill::ones); // assume all valid

	// walk moves
	for(arma::uword i = 0; i < movelist.n_cols; i++) {

		// this move
		arma::uword fr_sq120 = movelist(0, i);
		arma::uword to_sq120 = movelist(1, i);
		std::string movestr = get_algebraic_string(fr_sq120, to_sq120);

		// move
		if(!move(movestr)) { lg->msg("%sError moving move: %s%s\n", KRED, movestr.c_str(), KNRM); }

		// generate movelist for opponent

		const arma::uword depth_check = 1;
		arma::Mat<arma::uword> checklist = create_movelist(depth_check, lg);
		assert(checklist.n_rows == 2); // should be from and to rows
		assert(!checklist.is_empty());

		// walk checklist and check for king captures
		for(arma::uword j = 0; j < checklist.n_cols; j++) {

			//debug
			//std::cout << "Checking opponent move: \n" << checklist.col(j).t() << std::endl;

			// this opp move
			arma::uword opp_fr_sq120 = checklist(0, j);
			arma::uword opp_to_sq120 = checklist(1, j);

			// check piece on to square
			ShPiecePr opp_to_pc = get_piece(opp_to_sq120);
			PieceType opp_to_type = opp_to_pc->get_type();
			if(opp_to_type == PieceType::KING) {
				// this move is invalid
				moveidx(i) = 0;
				COLLIDER_DEBUG("found a check");
				//collider_throw_line("Check found in movelist generation.");
			}
		}

		// unmove
		if(!unmove(movestr)) { lg->msg("%sError unmoving move: %s%s\n", KRED, movestr.c_str(), KNRM); }
	}
	// shorten movelist
	movelist = movelist.cols(arma::find(moveidx == 1));

	// debug check movelist
	arma::uword num_moves = movelist.n_cols;
	arma::Row<arma::uword> movehash = arma::Row<arma::uword>(num_moves, arma::fill::zeros);
	movehash = movelist.row(0) * 100 + movelist.row(1);
	arma::Row<arma::uword> unique_movehash = arma::unique(movehash);
	assert(unique_movehash.n_elem == num_moves); // check for duplicates
	if(unique_movehash.n_elem != num_moves) {
		// check for duplicates
		collider_throw_line("Duplicate moves found in movelist generation.");
	}

	// set
	_movelist = movelist;

	// debug loop
	for(arma::uword i = 0; i < movelist.n_cols; i++) {
		arma::uword to_sq120 = movelist(1, i);
		ShPiecePr to_pc = get_piece(to_sq120);
		if(to_pc->get_type() == PieceType::KING) {
			std::cout << "King move found: " << get_algebraic_string(movelist(0, i), movelist(1, i)) << std::endl;
			collider_throw_line("King move found in movelist generation, should not happen.");
		}
		//display_movelist(lg);
		//display_board(lg);
		//collider_throw_line("enpassant");
	}
}

// creates movelist without check checking
arma::Mat<arma::uword> Board::create_movelist(arma::uword depth, ShLogPr lg) {

	// early exit
	if(depth == 0) { return arma::Mat<arma::uword>(); }

	// allocate movelist
	std::vector<arma::uword> from_vec;
	std::vector<arma::uword> to_vec;

	// walk over squares
	// @hey: if we had just the pieces we won't need a full loop
	//       probably not an issue? it's 8x8?
	for(arma::uword r = 0; r < 8; r++) {
		for(arma::uword f = 0; f < 8; f++) {

			// get sq
			arma::uword sq120 = Extra::rf2sq120(r, f);

			// check piece
			ShPiecePr pc = _board120[sq120];
			PieceType type = pc->get_type();
			PieceColor color = pc->get_color();
			if(type == PieceType::NONE) continue;
			if(type == PieceType::OFFBOARD) continue;
			if(color != get_turn()) continue;

			// generate possible moves for this piece at this square
			arma::Row<arma::uword> moves = get_moves(sq120);

			// add to vectors
			from_vec.insert(from_vec.end(), moves.n_elem, sq120);
			to_vec.insert(to_vec.end(), moves.begin(), moves.end());
		}
	}

	// return
	return arma::join_vert(arma::Row<arma::uword>(from_vec), arma::Row<arma::uword>(to_vec));
}

void Board::set_color(PieceColor color) { _color = color; }
PieceColor Board::get_color() const { return _color; }
PieceColor Board::get_turn() const { return get_color(); }
bool Board::is_turn(PieceColor color) const { return color == get_turn(); }
bool Board::can_castle(PieceColor color) const {
	// check early
	if(!(color != PieceColor::WHITE || color != PieceColor::BLACK)) collider_throw_line("board color should only be white/black");

	// check color
	if(color == PieceColor::WHITE)
		return _can_castle[0];
	else
		return _can_castle[1];
}

// return possible to squares in 120 index
arma::Row<arma::uword> Board::get_moves(arma::uword sq120) {

	// check piece
	ShPiecePr pc = _board120[sq120];
	PieceType type = pc->get_type();
	PieceColor color = pc->get_color();

	// early exit
	if(type == PieceType::NONE) { return arma::Row<arma::uword>(); }
	if(type == PieceType::OFFBOARD) { return arma::Row<arma::uword>(); }

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

			// if opposite color it is a capture
			if(to_pc->get_color() == pc->get_enemy_color(color)) {
				move_vec.push_back(tosq);
				continue;
			}

			// if empty square also good
			if(to_pc->get_color() == PieceColor::NONE) {
				move_vec.push_back(tosq);
				continue;
			}

			// check if same color
			if(to_pc->get_color() == color) continue;
		}
	}

	// sliding pieces
	else if(type == PieceType::ROOK || type == PieceType::BISHOP || type == PieceType::QUEEN) {

		// walk over directions
		for(arma::sword i = 0; i < move_directions.n_elem; i++) {

			// walk until failure
			arma::uword step = 1;
			for(;;) {
				//COLLIDER_DEBUG("loop");
				arma::sword mydir = move_directions(i);
				arma::uword tosq = sq120 + mydir * step;
				assert(tosq < 120);
				ShPiecePr to_pc = _board120[tosq];

				// check if offboard
				if(to_pc->get_type() == PieceType::OFFBOARD) break;

				// if empty square also good
				if(to_pc->get_color() == PieceColor::NONE) {
					move_vec.push_back(tosq);
					// step
					step++;
					continue;
				}

				// check if same color
				if(to_pc->get_color() == color) break;

				// if opposite color it is a capture
				if(to_pc->get_color() == pc->get_enemy_color()) {
					move_vec.push_back(tosq);
					break;
				}
			}
		}
	} // sliding pieces
	else if(type == PieceType::KING) {
		// handle king

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
			if(to_pc->get_color() == pc->get_enemy_color(color)) {
				move_vec.push_back(tosq);
				continue;
			}

			// if empty square
			// @NOTE: we check for checks in the update_movelist() function, this assumes the move is valid
			if(to_pc->get_color() == PieceColor::NONE) {

				// add the move
				move_vec.push_back(tosq);

				// Exceptions:  castling
				/*
				if(can_castle(pc->get_color())) {
					//	arma::Row<arma::sword> move_castling = PieceMoves::get_castling_moves();

					// set castling move
					arma::sword mydir2 = mydir == static_cast<arma::sword>(MoveDirections::RIGHT) ? mydir * 2 : mydir * 3;
					arma::uword tosq = sq120 + mydir;
					ShPiecePr to_pc = _board120[tosq];

					//assert(tosq < 120);
					if(tosq >= 120) collider_throw_line("Invalid castling move: tosq out of bounds.");

					// check if offboard
					if(to_pc->get_type() == PieceType::OFFBOARD) continue;

					// check if same color
					if(to_pc->get_color() == color) continue;

					// if opposite color can't castle
					if(to_pc->get_color() == pc->get_enemy_color(color)) continue;

					// if empty square
					// @NOTE: we check for checks in the update_movelist() function, this assumes the move is valid
					if(to_pc->get_color() == PieceColor::NONE) {
						COLLIDER_DEBUG("think we can castle");

						// add the move
						move_vec.push_back(tosq);

						// @hey: how do we denote castling so we can move the rook too?
					}
				}
                */
			}
		}
	} // king loop
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

				// cannot capture with up/down moves
				if(to_pc->get_type() != PieceType::NONE)
					continue;
				else {
					// add to move vec
					move_vec.push_back(tosq);

					// if moved then
					// check first move for double step
					// Exception: double pawn move on first turn
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
			} // up/down moves

			// pawn captures
			if(to_pc->get_color() == pc->get_enemy_color(color)) {
				// add to move vec
				move_vec.push_back(tosq);
			}
			// Exception: en passant capture (enpassant)
			EnPassantInfo enpassant_info = _enpassant_list.back();
			if(enpassant_info.color == pc->get_enemy_color() && tosq == enpassant_info.square) {
				// debug
				COLLIDER_DEBUG("enpassant capture found ->");
				std::cout << "En passant capture found: from " << get_algebraic_string(sq120, tosq) << " enpassant square: " << Extra::sq120to64(enpassant_info.square)
						  << std::endl;
				std::cout << _enpassant_list.size() << " enpassant squares in list." << std::endl;
				for(auto info : _enpassant_list) { std::cout << "Enpassant square: " << info.square << std::endl; }
				//collider_throw_line("enpassant capture found <-");
			}
		}
	} // pawn
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
	//	std::printf("Checking new move: \n");
	//	std::printf("From square: %s, To square: %s\n", frsq.c_str(), tosq.c_str());
	//	std::printf("fr_rank: %llu, fr_file: %llu\n", fr_rank, fr_file);
	//	std::printf("to_rank: %llu, to_file: %llu\n", to_rank, to_file);
	//	std::printf("fr_sq120: %llu, to_sq120: %llu\n", fr_sq120, to_sq120);

	// check if move is valid
	if(!is_valid(fr_sq120, to_sq120)) {
		_lg->msg("%sInvalid move attempted.%s\n", KRED, KNRM);
		//collider_throw_line("Invalid move attempted.");
		return false;
	}

	// check piece on from square
	ShPiecePr fr_pc = _board120[fr_sq120];
	ShPiecePr to_pc = _board120[to_sq120];

	assert(fr_pc->get_type() != PieceType::NONE);
	assert(fr_pc->get_type() != PieceType::OFFBOARD);

	// captures
	if(to_pc->get_color() == fr_pc->get_enemy_color() && to_pc->get_type() != PieceType::NONE) {
		// @hey: debug statements should probably not be colored...
		//_lg->msg("%sCapturing piece: %c at square %s%s\n", KRED, to_pc->get_piece_char(), tosq.c_str(), KNRM);
		to_pc->set_alive(false);
	}

	// check for enpassant and reset last enpassant square
	assert(!_enpassant_list.empty());
	EnPassantInfo last_enpassant_info = _enpassant_list.back();
	assert(last_enpassant_info.square < 120);
	_board120[last_enpassant_info.square] = Piece::create(PieceColor::NONE, PieceType::NONE, Extra::sq120to64(last_enpassant_info.square));
	EnPassantInfo new_enpassant_info = { 0, PieceColor::NONE };
	if(fr_pc->get_type() == PieceType::PAWN) {
		// handle creation of new enpassant squares
		arma::sword diff = to_sq120 - fr_sq120;
		if(diff == static_cast<arma::sword>(MoveDirections::UP) * 2 || diff == static_cast<arma::sword>(MoveDirections::DOWN) * 2) {

			// debug
			COLLIDER_DEBUG("enpassant square setting");
			//std::cout << "En passant move detected: from " << frsq << " to " << tosq << std::endl;
			//std::cout << "fr_sq120: " << fr_sq120 << ", to_sq120: " << to_sq120 << ", diff: " << diff << std::endl;
			// set enpassant square
			new_enpassant_info.square = fr_sq120 + (diff / 2);
			new_enpassant_info.color = fr_pc->get_color();

			assert(new_enpassant_info.square < 120);

			// debug
			std::cout << "Setting enpassant square: " << Extra::sq120to64(new_enpassant_info.square) << std::endl;
			std::cout << "En passant info: square: " << Extra::sq120to64(new_enpassant_info.square) << ", color: " << Piece::get_color_string(new_enpassant_info.color)
					  << std::endl;
			//_board120[new_enpassant_sq] = Piece::create(fr_pc->get_color(), PieceType::NONE, Extra::sq120to64(new_enpassant_sq));
		}
	}

	// move piece finally
	_board120[to_sq120] = fr_pc;
	fr_pc->set_pos(to_rank, to_file);
	fr_pc->set_moved(true);
	fr_pc->_move_count += 1;

	_board120[fr_sq120] = Piece::create(PieceColor::NONE, PieceType::NONE, fr_rank, fr_file);

	// switch color
	if(get_color() == PieceColor::WHITE)
		set_color(PieceColor::BLACK);
	else
		set_color(PieceColor::WHITE);

	// add to move history
	_move_history.push_back(move_str);
	_captured_pieces.push_back(to_pc);
	_enpassant_list.push_back(new_enpassant_info);
	//_castling_list.push_back(_can_castle);

	// success
	return true;
}

// move in UCI format, e.g. e2e4
// @hey: should we only allow unmoving the last move in the history?
//       remove the input argument?
bool Board::unmove(std::string move_str) {

	// parse move
	assert(move_str.length() == 4 || move_str.length() == 5); //

	// check that move is the last move in the history
	assert(!_move_history.empty());

	//assert(move_str == _move_history.back());
	if(move_str != _move_history.back()) {
		std::cout << "Trying to unmove move: " << move_str << " but last move in history is: " << _move_history.back() << std::endl;
		collider_throw_line("Trying to unmove a move that is not the last move in the history.");
	}

	// get first two chars
	std::string frsq = move_str.substr(2, move_str.length() - 1);
	std::string tosq = move_str.substr(0, 2);

	// get ranks and files
	arma::uword fr_rank = Extra::char2rank(frsq[1]);
	arma::uword fr_file = Extra::char2file(frsq[0]);

	arma::uword to_rank = Extra::char2rank(tosq[1]);
	arma::uword to_file = Extra::char2file(tosq[0]);

	arma::uword fr_sq120 = Extra::rf2sq120(fr_rank, fr_file);
	arma::uword to_sq120 = Extra::rf2sq120(to_rank, to_file);

	// debug
	//	std::printf("Checking unmove: \n");
	//	std::printf("From square: %s, To square: %s\n", frsq.c_str(), tosq.c_str());
	//	std::printf("fr_rank: %llu, fr_file: %llu\n", fr_rank, fr_file);
	//	std::printf("to_rank: %llu, to_file: %llu\n", to_rank, to_file);
	//	std::printf("fr_sq120: %llu, to_sq120: %llu\n", fr_sq120, to_sq120);

	// unmove can just check the move_history list?
	// check if move is valid
	//if(!is_valid(fr_sq120, to_sq120)) {

	//	_lg->msg("%sInvalid move attempted.%s\n", KRED, KNRM);
	//	return false;
	//}

	// check piece on from square
	ShPiecePr fr_pc = _board120[fr_sq120];
	ShPiecePr to_pc = _board120[to_sq120];

	// check not trying to move an empty square
	if(fr_pc->get_type() == PieceType::NONE || fr_pc->get_type() == PieceType::OFFBOARD) {
		display_board(_lg);
		collider_throw_line("Trying to unmove an empty square.");
	}

	// unmove should never be capturing...
	//	if(to_pc->get_color() == fr_pc->get_enemy_color() && to_pc->get_type() != PieceType::NONE) {
	//		_lg->msg("%sCapturing piece: %c at square %s%s\n", KRED, to_pc->get_piece_char(), tosq.c_str(), KNRM);
	//		to_pc->set_alive(false);
	//	}

	// move piece finally
	_board120[to_sq120] = fr_pc;
	fr_pc->set_pos(to_rank, to_file);
	fr_pc->set_moved(true);
	fr_pc->_move_count -= 1;

	// unset enpassant square
	EnPassantInfo last_enpassant_sq = _enpassant_list.back();
	_enpassant_list.pop_back();

	//debug
	//	if(enpassant_sq != 0) {
	//		//	std::printf("Unsetting enpassant square: %llu\n", enpassant_sq);
	//	//	display_movelist(_lg);
	//	//	display_board(_lg);
	//	//	collider_throw_line("unset enpassant");
	//		_board120[enpassant_sq] = Piece::create(PieceColor::NONE, PieceType::NONE, Extra::sq120to64(enpassant_sq));
	//
	//        if(to_pc->get_color() == fr_pc->get_enemy_color()) {
	//            // restore captured piece
	//            to_pc->set_alive(true);
	//            _board120[to_sq120] = to_pc;
	//        }
	//	}
	//	_enpassant_list.pop_back();

	// get the captured piece from the last move
	to_pc = _captured_pieces.back();

	_captured_pieces.pop_back();
	_board120[fr_sq120] = to_pc;
	to_pc->set_alive(true);
	to_pc->set_pos(fr_rank, fr_file);

	// switch color
	if(get_color() == PieceColor::WHITE)
		set_color(PieceColor::BLACK);
	else
		set_color(PieceColor::WHITE);

	// pop off the last move from the history
	_move_history.pop_back();


	// success
	return true;
}

// check if move is valid
bool Board::is_valid(arma::uword fr_sq120, arma::uword to_sq120) {

	// get possible moves in this position
	arma::Row<arma::uword> valid_moves = get_moves(fr_sq120);

	// flag
	bool valid = false;

	// check valid moves
	for(arma::uword i = 0; i < valid_moves.n_elem; i++) {
		if(valid_moves(i) == to_sq120) { return true; }
	}

	// default
	return valid;
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

ShPiecePr Board::get_piece(arma::uword sq120) const {
	// check sq?

	return _board120[sq120];
}


void Board::display_board(ShLogPr lg) {

	// header
	lg->newl();
	lg->msg("\t%s --- %sCollider V2%s --- %s\n", KCYN, KORG, KCYN, KNRM);
	lg->msg("\t%s --- Current Move: %s%s%s --- %s\n", KCYN, get_color_color(get_color()).c_str(), get_color_string(get_color()).c_str(), KCYN, KNRM);

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

// quick display of movelist for current position
void Board::display_movelist(ShLogPr lg) {

	// check movelist
	assert(!_movelist.is_empty());
	assert(_movelist.n_rows == 2);

	arma::uword num_moves = _movelist.n_cols;

	// header
	lg->newl();
	lg->msg("\t%s --- %sdisplay movelist: %s%llu%s--- %s\n", KCYN, KORG, KPNK, num_moves, KCYN, KNRM);
	lg->msg("\t%s frsq - tosq - algr - frtype - totype %s\n", KBLU, KNRM);


	// walk over moves
	for(int i = 0; i < num_moves; i++) {
		arma::Col<arma::uword> mymove = _movelist.col(i);
		arma::uword frsq = mymove(0);
		arma::uword tosq = mymove(1);

		ShPiecePr frpc = _board120[frsq];
		PieceType frtype = frpc->get_type();
		PieceColor frcol = frpc->get_color();
		ShPiecePr topc = _board120[tosq];
		PieceType totype = topc->get_type();
		PieceColor tocol = topc->get_color();

		// log
		lg->msg("\t%s  %llu     %llu    %s     %c        %c   %s\n",
			KBLU,
			frsq,
			tosq,
			get_algebraic_string(frsq, tosq).c_str(),
			Piece::get_piece_char(frcol, frtype),
			Piece::get_piece_char(tocol, totype),
			KNRM);
	}
}


// get the algebraic string for this move i.e. 'e2e4'
// assumes sq120
std::string Board::get_algebraic_string(arma::uword frsq, arma::uword tosq) const {
	// check squares

	// get rank and file for both fr/to squares
	arma::uword fr_rank = Extra::get_rank120(frsq);
	arma::uword fr_file = Extra::get_file120(frsq);
	arma::uword to_rank = Extra::get_rank120(tosq);
	arma::uword to_file = Extra::get_file120(tosq);

	// construct string
	char str[4] = { Extra::file2char(fr_file), Extra::rank2char(fr_rank), Extra::file2char(to_file), Extra::rank2char(to_rank) };

	// cast and return
	return std::string(str);
}

std::string Board::get_color_string(PieceColor color) const {
	if(color == PieceColor::WHITE)
		return "White";
	else if(color == PieceColor::BLACK)
		return "Black";
	else
		return "None";
}

std::string Board::get_color_color(PieceColor color) const {
	if(color == PieceColor::WHITE)
		return KYEL;
	else if(color == PieceColor::BLACK)
		return KBLU;
	else
		return KNRM;
}

std::string Board::start_fen() { return "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; }

} // namespace cldr
