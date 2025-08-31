#pragma once

#include "typedefs.hh"

// bitboards class

namespace cldr {

struct boardStruct
{
	int side;
	
	int newfrSq;
	int newtoSq;

	int frSq;
	int toSq;
	
	int sq[SQNUM];
	int sq64[64];

	bool castling;
	bool castled[3];
	int material[3];

	int enPas;
	std::vector<int> mL;
	std::vector<int> score;
};	

struct pieceStruct
{
	int pos;
	int pos64;
	int color;
	int type;
	int value;
	bool life;
	U64 bitboard;
	int moved;
	std::vector<int> mL;
	std::vector<int> caps;
};

struct bitboardStruct 
{

	U64 rooks[3];
	U64 pawns[3];
	U64 pieces[3];
	U64 occupiedSquares;
	U64 emptySquares; 

	U64 rank[8];
	U64 file[8];
	U64 diagW[8];
	U64 diagB[8];
	U64 passPwn[3][58];
	U64 iso[8];
	U64 opfw[3][48];
	U64 rook_a[3][64];
	U64 bish_a[3][64];
	U64 queen_a[3][64];
	U64 knight_a[3][64];
	U64 king_a[3][64];
	U64 pawn_a[3][64];
};


struct indexStruct
{
	
	int sq[SQNUM];
	int sq64[64];
};

struct undoStruct
{
	int move;
	int enPas;
	int piece;
};
class BitBoards {
public:
	 int pawnTable[64];
	 int knightTable[64];
	 int bishopTable[64];
	 int rookTable[64];
	 int queenTable[64];
	 int kingTable1[64];
	 int kingTable2[64];
};
} // namespace cldr
