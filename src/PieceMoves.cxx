
#include "PieceMoves.hxx"
#include "error.hxx"
#include "typedefs.hh"
#include <armadillo>


// @hey: these are single step move generation
//       we could add an input for multiple steps... for now leave in movegen
namespace cldr {

// get base move directions for a piece type and color
arma::Row<arma::sword> PieceMoves::get_moves(PieceType type, PieceColor color) {
	switch(type) {
	case PieceType::PAWN: return PieceMoves::get_pawn_moves(color);
	case PieceType::KNIGHT: return PieceMoves::get_knight_moves();
	case PieceType::BISHOP: return PieceMoves::get_bishop_moves();
	case PieceType::ROOK: return PieceMoves::get_rook_moves();
	case PieceType::QUEEN: return PieceMoves::get_queen_moves();
	case PieceType::KING: return PieceMoves::get_king_moves();
	default: collider_throw_line("Invalid piece type for move generation.");
	}
}

arma::Row<arma::sword> PieceMoves::get_pawn_moves(PieceColor color) {
	arma::Row<arma::sword> moves;
	if(color == PieceColor::WHITE) {
		// clang-format off
		moves = arma::Row<arma::sword>{
            // put moves first for pawn
            static_cast<arma::sword>(MoveDirections::UP),
			static_cast<arma::sword>(MoveDirections::UPLEFT),
            static_cast<arma::sword>(MoveDirections::UPRIGHT)
		};
	} else if(color == PieceColor::BLACK) {
		moves = arma::Row<arma::sword>{
            // put moves first for pawn
            static_cast<arma::sword>(MoveDirections::DOWN),
			static_cast<arma::sword>(MoveDirections::DOWNLEFT),
            static_cast<arma::sword>(MoveDirections::DOWNRIGHT)
		};
    } else {
		collider_throw_line("Invalid piece color for pawn moves.");
	}
	// clang-format on

	// return
	return moves;
}

arma::Row<arma::sword> PieceMoves::get_knight_moves() {

	// clang-format off
    return arma::Row<arma::sword>{ 
        static_cast<arma::sword>(MoveDirections::KNIGHTUPRIGHT),
        static_cast<arma::sword>(MoveDirections::KNIGHTRIGHTUP),
        static_cast<arma::sword>(MoveDirections::KNIGHTRIGHTDOWN),
        static_cast<arma::sword>(MoveDirections::KNIGHTDOWNRIGHT),
        static_cast<arma::sword>(MoveDirections::KNIGHTDOWNLEFT),
        static_cast<arma::sword>(MoveDirections::KNIGHTLEFTDOWN),
        static_cast<arma::sword>(MoveDirections::KNIGHTLEFTUP),
        static_cast<arma::sword>(MoveDirections::KNIGHTUPLEFT)
    };
	// clang-format on
}

// single step
arma::Row<arma::sword> PieceMoves::get_bishop_moves() {

	// clang-format off
    return arma::Row<arma::sword>{ 
        static_cast<arma::sword>(MoveDirections::UPRIGHT),
        static_cast<arma::sword>(MoveDirections::DOWNRIGHT),
        static_cast<arma::sword>(MoveDirections::DOWNLEFT),
        static_cast<arma::sword>(MoveDirections::UPLEFT)
    };
	// clang-format on
}


// single step
arma::Row<arma::sword> PieceMoves::get_rook_moves() {

	// clang-format off
    return arma::Row<arma::sword>{ 
        static_cast<arma::sword>(MoveDirections::UP),
        static_cast<arma::sword>(MoveDirections::RIGHT),
        static_cast<arma::sword>(MoveDirections::DOWN),
        static_cast<arma::sword>(MoveDirections::LEFT)
    };
	// clang-format on
}

// use king moves for queen and handle multiple steps in move gen
arma::Row<arma::sword> PieceMoves::get_queen_moves() { return PieceMoves::get_king_moves(); }
arma::Row<arma::sword> PieceMoves::get_king_moves() {
	// king is combination of rook and bishop one step
	return arma::join_horiz(PieceMoves::get_rook_moves(), PieceMoves::get_bishop_moves());
}

arma::Row<arma::sword> PieceMoves::get_castling_moves() {
    // castling is 2 moves right and 3 moves left regardless of color
	return arma::Row<arma::sword>{ static_cast<arma::sword>(MoveDirections::RIGHT) * 2, static_cast<arma::sword>(MoveDirections::LEFT) * 3 };
}
} // namespace cldr
