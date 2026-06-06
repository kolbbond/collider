// lightweight SDL2 board display / interactive play
#include "Display.hxx"
#include "Piece.hxx"

#include <cstring>

// resource directory: set by the build (absolute path to ./res); fall back to
// a relative path so the binary also works when run from the source tree.
#ifndef COLLIDER_RES_DIR
#define COLLIDER_RES_DIR "res"
#endif

namespace cldr {

// piece textures are laid out as 6 white (PAWN..KING) then 6 black
int Display::piece_index(PieceColor color, PieceType type) const {
	const int base = (color == PieceColor::BLACK) ? 6 : 0;
	return base + static_cast<int>(type);
}

// load a .bmp into a texture, trying a few candidate resource directories.
// when colorkey is set and the bitmap has no alpha channel, the top-left pixel
// is treated as transparent.
SDL_Texture* Display::load_texture(const std::string& file, ShLogPr lg) {
	const std::array<std::string, 3> dirs = { COLLIDER_RES_DIR, "res", "../res" };

	SDL_Surface* surf = nullptr;
	for(const std::string& dir : dirs) {
		const std::string path = dir + "/" + file;
		surf = SDL_LoadBMP(path.c_str());
		if(surf != nullptr) { break; }
	}
	if(surf == nullptr) {
		lg->msg("%sUnable to load image %s: %s%s\n", KRED, file.c_str(), SDL_GetError(), KNRM);
		return nullptr;
	}

	// fallback transparency for bitmaps without an alpha channel: key out the
	// top-left pixel (the assets use a transparent border around the pieces).
	if(surf->format->Amask == 0) {
		Uint32 key = 0;
		std::memcpy(&key, surf->pixels, surf->format->BytesPerPixel);
		SDL_SetColorKey(surf, SDL_TRUE, key);
	}

	SDL_Texture* tex = SDL_CreateTextureFromSurface(_renderer, surf);
	SDL_FreeSurface(surf);
	if(tex == nullptr) {
		lg->msg("%sUnable to create texture for %s: %s%s\n", KRED, file.c_str(), SDL_GetError(), KNRM);
		return nullptr;
	}
	SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
	return tex;
}

void Display::load_media(ShLogPr lg) {
	// pieces
	const std::array<std::string, 6> type_name = { "Pawn", "Knight", "Bishop", "Rook", "Queen", "King" };
	const std::array<std::pair<PieceColor, std::string>, 2> colors = {
		std::make_pair(PieceColor::WHITE, std::string("white")),
		std::make_pair(PieceColor::BLACK, std::string("black"))
	};
	for(const auto& c : colors) {
		for(int t = 0; t < 6; t++) {
			const std::string file = c.second + type_name[t] + ".bmp";
			_piece_tex[piece_index(c.first, static_cast<PieceType>(t))] = load_texture(file, lg);
		}
	}

	// squares: [0] light, [1] dark
	_square_tex[0] = load_texture("whiteSquare.bmp", lg);
	_square_tex[1] = load_texture("blackSquare.bmp", lg);
}

Display::Display(ShLogPr lg) {
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		lg->msg("%sSDL could not initialize: %s%s\n", KRED, SDL_GetError(), KNRM);
		return;
	}

	_window = SDL_CreateWindow("Collider", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _board_px, _board_px, SDL_WINDOW_SHOWN);
	if(_window == nullptr) {
		lg->msg("%sWindow could not be created: %s%s\n", KRED, SDL_GetError(), KNRM);
		return;
	}

	// prefer an accelerated, vsynced renderer; fall back to software
	_renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if(_renderer == nullptr) { _renderer = SDL_CreateRenderer(_window, -1, 0); }
	if(_renderer == nullptr) {
		lg->msg("%sRenderer could not be created: %s%s\n", KRED, SDL_GetError(), KNRM);
		return;
	}
	SDL_SetRenderDrawBlendMode(_renderer, SDL_BLENDMODE_BLEND);

	load_media(lg);
	_ok = true;
}

Display::~Display() {
	for(SDL_Texture* tex : _piece_tex) {
		if(tex != nullptr) { SDL_DestroyTexture(tex); }
	}
	for(SDL_Texture* tex : _square_tex) {
		if(tex != nullptr) { SDL_DestroyTexture(tex); }
	}
	if(_renderer != nullptr) { SDL_DestroyRenderer(_renderer); }
	if(_window != nullptr) { SDL_DestroyWindow(_window); }
	SDL_Quit();
}

ShDisplayPr Display::create(ShLogPr lg) { return std::make_shared<Display>(lg); }

arma::uword Display::square_at(int mx, int my) const {
	if(mx < 0 || my < 0 || mx >= _board_px || my >= _board_px) { return 0; }
	const int col = mx / _square_px;
	const int row = my / _square_px;
	int file, rank;
	if(!_flip) {
		file = col;
		rank = 7 - row;
	} else {
		file = 7 - col;
		rank = row;
	}
	return FR2SQ(file, rank);
}

void Display::render(const Board& board, arma::uword selected, const std::vector<arma::uword>& highlights) {
	// screen cell (col, row from top-left) for a given file/rank, honoring flip
	auto cell_rect = [this](int file, int rank) {
		const int col = _flip ? (7 - file) : file;
		const int row = _flip ? rank : (7 - rank);
		return SDL_Rect{ col * _square_px, row * _square_px, _square_px, _square_px };
	};

	SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 255);
	SDL_RenderClear(_renderer);

	// squares + pieces
	for(int rank = 0; rank < 8; rank++) {
		for(int file = 0; file < 8; file++) {
			const SDL_Rect cell = cell_rect(file, rank);

			// a1 (file 0, rank 0) is a dark square
			const bool dark = ((file + rank) % 2 == 0);
			SDL_Texture* sq = _square_tex[dark ? 1 : 0];
			if(sq != nullptr) { SDL_RenderCopy(_renderer, sq, nullptr, &cell); }

			const ShPiecePr pc = board.get_piece(FR2SQ(file, rank));
			if(pc != nullptr) {
				const PieceType t = pc->get_type();
				if(t != PieceType::NONE && t != PieceType::OFFBOARD) {
					SDL_Texture* tex = _piece_tex[piece_index(pc->get_color(), t)];
					if(tex != nullptr) { SDL_RenderCopy(_renderer, tex, nullptr, &cell); }
				}
			}
		}
	}

	// selected square tint
	if(selected != 0) {
		const int file = static_cast<int>(selected % 10) - 1;
		const int rank = static_cast<int>(selected / 10) - 2;
		if(file >= 0 && file < 8 && rank >= 0 && rank < 8) {
			const SDL_Rect cell = cell_rect(file, rank);
			SDL_SetRenderDrawColor(_renderer, 246, 246, 105, 110);
			SDL_RenderFillRect(_renderer, &cell);
		}
	}

	// legal-target dots
	for(const arma::uword sq : highlights) {
		const int file = static_cast<int>(sq % 10) - 1;
		const int rank = static_cast<int>(sq / 10) - 2;
		if(file < 0 || file >= 8 || rank < 0 || rank >= 8) { continue; }
		const SDL_Rect cell = cell_rect(file, rank);
		const int inset = _square_px / 3;
		const SDL_Rect dot = { cell.x + inset, cell.y + inset, _square_px - 2 * inset, _square_px - 2 * inset };
		SDL_SetRenderDrawColor(_renderer, 40, 200, 70, 130);
		SDL_RenderFillRect(_renderer, &dot);
	}

	SDL_RenderPresent(_renderer);
}

void Display::run(ShBoardPr board, ShEnginePr engine, ShLogPr lg) {
	if(!_ok) { return; }

	// engine search depth for replies
	const arma::uword depth = 3;

	arma::uword selected = 0;
	std::vector<arma::uword> highlights;
	arma::Mat<arma::uword> moves;

	// (re)generate the legal movelist for the side to move
	auto refresh_moves = [&]() {
		board->update_movelist(lg);
		moves = board->get_movelist();
	};

	// collect legal target squares from `selected`
	auto select_piece = [&](arma::uword sq) {
		const ShPiecePr pc = board->get_piece(sq);
		if(pc == nullptr || pc->get_type() == PieceType::NONE || pc->get_type() == PieceType::OFFBOARD) { return; }
		if(pc->get_color() != board->get_turn()) { return; }
		selected = sq;
		highlights.clear();
		for(arma::uword i = 0; i < moves.n_cols; i++) {
			if(moves(0, i) == selected) { highlights.push_back(moves(1, i)); }
		}
	};

	refresh_moves();
	render(*board, selected, highlights);

	bool running = true;
	SDL_Event e;
	while(running && SDL_WaitEvent(&e)) {
		if(e.type == SDL_QUIT) {
			running = false;
		} else if(e.type == SDL_KEYDOWN) {
			const SDL_Keycode k = e.key.keysym.sym;
			if(k == SDLK_ESCAPE || k == SDLK_q) {
				running = false;
			} else if(k == SDLK_f) {
				_flip = !_flip;
			}
		} else if(e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
			const arma::uword sq = square_at(e.button.x, e.button.y);
			if(sq == 0) {
				// outside the board: ignore
			} else if(selected == 0) {
				select_piece(sq);
			} else {
				// find a legal move selected -> sq (prefer queen promotion)
				arma::uword chosen = moves.n_cols;
				for(arma::uword i = 0; i < moves.n_cols; i++) {
					if(moves(0, i) == selected && moves(1, i) == sq) {
						chosen = i;
						if(static_cast<PieceType>(moves(2, i)) == PieceType::QUEEN) { break; }
					}
				}

				if(chosen < moves.n_cols) {
					const std::string movestr =
						board->get_algebraic_string(moves(0, chosen), moves(1, chosen), static_cast<PieceType>(moves(2, chosen)));
					selected = 0;
					highlights.clear();
					if(board->move(movestr)) {
						// show the user's move, then let the engine reply
						render(*board, selected, highlights);
						const std::string reply = engine->best_move(depth, lg);
						if(!reply.empty()) { board->move(reply); }
					}
					refresh_moves();
				} else {
					// not a legal target: deselect, or reselect another own piece
					selected = 0;
					highlights.clear();
					select_piece(sq);
				}
			}
		}

		render(*board, selected, highlights);
	}
}

} // namespace cldr
