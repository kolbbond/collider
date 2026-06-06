// lightweight SDL2 board display / interactive play
//
// plain SDL2 only: no SDL2_image, no smg/Magnum. loads the bundled .bmp
// assets directly and renders the live Board, letting the user play against
// the engine by clicking.
#pragma once

#include <SDL2/SDL.h>
#include <array>
#include <memory>
#include <string>
#include <vector>

#include "Board.hxx"
#include "Engine.hxx"
#include "Log.hxx"
#include "typedefs.hh"

namespace cldr {

using ShDisplayPr = std::shared_ptr<class Display>;

class Display {

	// geometry
	static constexpr int _square_px = 90;             // pixels per square
	static constexpr int _board_px = _square_px * 8;  // window is square

	// sdl handles
	SDL_Window* _window = nullptr;
	SDL_Renderer* _renderer = nullptr;

	// piece textures indexed by piece_index(color, type); 6 white then 6 black
	std::array<SDL_Texture*, 12> _piece_tex = {};
	// square textures: [0] light, [1] dark
	std::array<SDL_Texture*, 2> _square_tex = {};

	// view state
	bool _flip = false;  // true => black at bottom
	bool _ok = false;    // construction succeeded

	// helpers
	SDL_Texture* load_texture(const std::string& file, ShLogPr lg);
	void load_media(ShLogPr lg);
	int piece_index(PieceColor color, PieceType type) const;

public:
	explicit Display(ShLogPr lg = NullLog::create());
	~Display();

	// non-copyable (owns SDL resources)
	Display(const Display&) = delete;
	Display& operator=(const Display&) = delete;

	static ShDisplayPr create(ShLogPr lg = NullLog::create());

	// whether the window/renderer/textures came up
	bool ok() const { return _ok; }

	// pixel -> 120 square (respects flip); returns 0 (off-board) if outside
	arma::uword square_at(int mx, int my) const;

	// draw the board, optionally tinting a selected square and legal targets
	void render(const Board& board, arma::uword selected = 0, const std::vector<arma::uword>& highlights = {});

	// interactive loop: user clicks to move, engine replies; F flips, Esc quits
	void run(ShBoardPr board, ShEnginePr engine, ShLogPr lg = NullLog::create());
};

} // namespace cldr
