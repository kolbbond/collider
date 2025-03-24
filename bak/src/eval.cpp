
#include "eval.h"
#include <primer.h>
#include "board.h"
#include "move.h"
#include "debug.h"

	int flip[64] = {         56, 57, 58, 59, 60, 61, 62, 63,
				 48, 49, 50, 51, 52, 53, 54, 55,
				 40, 41, 42, 43, 44, 45, 46, 47,
				 32, 33, 34, 35, 36, 37, 38, 39,
				 24, 25, 26, 27, 28, 29, 30, 31,
				 16, 17, 18, 19, 20, 21, 22, 23,
				  8,  9, 10, 11, 12, 13, 14, 15,
				  0,  1,  2,  3,  4,  5,  6,  7 };


	int pawnTable[64] = {          0,   0,   0,   0,   0,   0,   0,   0,
				       0,   0, -15, -20, -20, -10,   0,   0,
				       5,   5,   5,   5,   5,   5,  10,  15,
				       5,   5,  20,  30,  35,  10,   5,   5,
				      17,  17,  10,  35,  35,  25,  25,  10,
				      15,  35,  40,  40,  40,  40,  30,  15,
				      80,  80,  80,  80,  80,  80,  80,  80,
				       0,   0,   0,   0,   0,   0,   0,   0 };

	int pawnTable2[64] = {          0,   0,   0,   0,   0,   0,   0,   0,
				      -20, -20, -20, -20, -20, -20, -20, -20,
				       5,   5,   5,   5,   5,   5,  10,  15,
				      15,  15,  10,  25,  30,  10,  15,  25,
				      17,  17,  50,  35,  35,  25,  55,  10,
				      45,  60,  60,  60,  60,  60,  60,  45,
				      60,  80,  80,  80,  80,  80,  80,  60,
				       0,   0,   0,   0,   0,   0,   0,   0 };
	
	int knightTable[64] = {        -30, -20,   0,   0,   0, -10, -30, -30,
				         0,   0,   0,  10,  10,   0,   0,   0,
				         0,   5,  15,   5,   5,  15,  10,   0,
				         0,   5,  10,  15,  20,  20,  20,   0,
				         0,  15,  10,  25,  25,  20,  15,   0,
					 0,  10,  15,  20,  25,  20,  15,   0,
				         0,   5,  20,  10,  10,  25,   5,   5,
			               -30,   0,   0,   0,   0,   0,   0, -30 };

	int bishopTable[64] = {          0,   0, -20,   0,   0, -20,   0,   0,
				         5,  10,   0,   5,  10,   0,  15,   5,
				         0,   0,   5,  15,  10,   0,   0,   0,
				         0,   5,  20,   5,   5,  15,   5,   0,
				         5,  20,   5,  15,  15,  10,  20,   0,
				         5,   0,  10,   5,  15,   5,   5,   0,
			                 0,   0,   0,   0,   0,   0,   0,   0,
				         0,   0,   0,   0,   0,   0,   0,   0 };   
			 
	int rookTable[64] = {           0,  10,  20,  40,  40,  20,  10,   0,
				        0,   0,   0,   0,   0,   0,   0,   0,
				        0,   0,   0,   0,  10,  15,  10,  10,
				        0,   0,   0,   0,   0,   0,   0,   0,
				        0,   0,   0,   0,   0,   0,   0,   0,
				        0,   0,   0,   0,   0,   5,   0,   5,
				       50,  50,  50,  50,  50,  50,  50,  50,
				       20,  20,  20,  20,  20,  20,  20,  20 };

	int queenTable[64] = {          0,   0,   5, -10,   0,   0,   0,   0,
					0,   0,  10,  10,  10,   0,   0,   0,
				        0,   0,   0,   5,   0,   0,   0,   0,
					0,   0,   0,   0,   0,   5,   0,   5,
					0,  10,   0,   0,   0,   5,   5,   0,
					0,   0,   0,   0,   0,   0,   0,   0,
				        5,   5,   5,   5,   5,  10,  10,  10,
				       10,  10,  10,  10,  10,  10,  15,  20 };

	int kingTable1[64] = {        20,  40,  20, -20, -40, -50,  50,  40,
				       0,   0,   0, -30, -80, -20,  10,  15,
				     -20, -30, -40, -50, -50, -40, -30, -20,
				     -50, -50, -50, -50, -50, -50, -50, -50,
				     -50, -50, -50, -50, -50, -50, -50, -50,
			   	     -50, -50, -50, -50, -50, -50, -50, -50,
				     -50, -50, -50, -50, -50, -50, -50, -50,
			  	     -50, -50, -50, -50, -50, -50, -50, -50 }; 

	int kingTable2[64] = {         -50, -40, -10,   0,   0,  5, -40, -20,
				        10,  10,  15,  20,  15, 15,  10,   5,
				        15,  15,  15,  20,  25, 10,  15,  10,
				        10,  10,  15,  25,  25, 15,  10,  10,
				        10,  10,  15,  25,  25, 15,  30,  10,
					10,  10,  10,  25,  20, 15,  30,  10,
				        15,  15,  35,  35,  35, 35,  30,  10,
					 0,   0,   0,   0,   0,  0,   0,   0 };

//Evaluate
int evaluate(){

	
	//Add total material count to score and piece table values
	int score = 0, type = 0, sq = 0, tSq = 0;
	int piece = (board.side == WHITE) ? wqR: bqR;
	int bPair = 0;
	bool endgame = false;
	int king = (board.side == WHITE) ? wK: bK;
	int mod = (board.side == WHITE) ? 1: -1;
	int opp = getOpp();
	int testSq = 0;

	//Material Count
	score += board.material[board.side];

	//Determine endgame
	if( score < 102000 ){
		endgame = true;
		std::cout << "Endgame!\n";
	}

	piece = (board.side == WHITE) ? wqR: bqR;
	//Extra Count
	for( int i = piece; i < piece + 16; ++i ){
		if( pce[i].life == false ){
			continue;
		}
		tSq = pce[i].pos;	
		sq = indexA.sq[tSq];
		type = getType(tSq);
		if( board.side == BLACK )
			sq = flip[sq];
		if( type == PAWN ){
			if( endgame )
				score += pawnTable2[sq];
			else
				score += pawnTable[sq];
			if( (bb.passPwn[board.side][tSq-31] & bb.pawns[opp]) == 0 ){
				bbDisp( bb.passPwn[board.side][tSq - 31] );
				bbDisp( bb.pawns[opp] );
				bbDisp( pce[i].bitboard );
				std::cout << "Side is " << board.side << "\n";
				std::cout << "Piece is: " << i << "\n";
				std::cout << "Passed pawn!\n";
				score += 50;
			}
		}
		else if( type == KNIGHT ){
			score += knightTable[sq];
		}
		else if( type == BISHOP ){
			score += bishopTable[sq];
			bPair++;
			if( bPair == 2 )
				score += 50;
		}
		else if( type == ROOK ){
			score += rookTable[sq];
			testSq = tSq % 10 - 1;
			if( (bb.file[testSq] & bb.pawns[board.side]) == 0 ){
				score += 20;
				std::cout << "Rook on open file yes.\n";
			}
			if( ((bb.rooks[board.side] ^ pce[i].bitboard) & bb.file[testSq]) != 0  ){
				score += 10;
				std::cout << "Rook battery\n";
			}
		}
		else if( type == QUEEN ){
			score += queenTable[sq];
			if( ply < 10 && pce[i].moved > 1 ){
				score -= 50;
			}
		}
		else if( type == KING ) {
			if(board.castled[board.side]){
				std::cout << "Castled\n";
				score += 50;
			//King Safety
			sq = pce[king].pos + mod * 9; 
			for( int i = 0; i < 3; ++i ){
				if( getType(sq) == PAWN && getColor(sq) == board.side ){
					score += 10;
				}
			sq += mod;
			}
			}
			if( endgame ){
				std::cout << "King endgame.\n";
				score += kingTable2[sq];
			}
			else 
				score += kingTable1[sq];
		}
		else{
			std::cout << "What\n";
		}
		
	}

	return score;

}

int eval(){
	int one = 0, two = 0;
	one = evaluate();
	//one -= 25;
	changeSide();
	two = evaluate();
	changeSide();
	return (one - two);
}

