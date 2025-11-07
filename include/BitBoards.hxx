#pragma once

#include "common.hh"
#include "typedefs.hh"
#include <chug/log.hh>

// bitboards class

namespace cldr {

using ShBitBoardsPr = std::shared_ptr<class BitBoards>;

class BitBoards {
public:
	u64 _pawnTable[64];
	u64 _knightTable[64];
	u64 _bishopTable[64];
	u64 _rookTable[64];
	u64 _queenTable[64];
	u64 _kingTable1[64];
	u64 _kingTable2[64];

	// boards for pieces
	// @hey: do we need separate boards for each color or can we use the general color & piece operation?
	u64 _pieces;
	u64 _wpieces;
	u64 _bpieces;
	u64 _pawns;
	u64 _wpawns;
	u64 _bpawns;
	u64 _knights;
	u64 _wknights;
	u64 _bknights;
	u64 _bishops;
	u64 _wbishops;
	u64 _bbishops;
	u64 _rooks;
	u64 _wrooks;
	u64 _brooks;
	u64 _queens;
	u64 _wqueens;
	u64 _bqueens;
	u64 _kings;
	u64 _wkings;
	u64 _bkings;

	// attack boards
	// @hey: can we pre initialize these?
	// pawn attacks are direction dependent
	u64 _wpawnattacks[64];
	u64 _bpawnattacks[64];
	u64 _knightattacks[64];
	u64 _bishopattacks[64];
	u64 _rookattacks[64];
	u64 _queenattacks[64]; // queen is combination of rook and bishop
	u64 _kingattacks[64];

	// options
	bool _memory_efficient = false; // don't precalculate boards
public:
	// methods
	BitBoards();
	ShBitBoardsPr create();
	void set_position(std::string fen);
	void init(chug::ShLogPr);

	// create the attack boards
	void create_attack_bitboards();

	//
	// from bak, set a specific sqs bit
	void setBit(int sq, U64& bbAddress) {
		U64 setBit = 1;
		U64 bb = bbAddress;
		setBit <<= sq;
		bb = bb | setBit;
		bbAddress = bb;
	}
};
} // namespace cldr
