#ifndef DEBUG_H
#define DEBUG_H
//debug.h

//Functions
void bitboardDebug();
void bbDisp( U64 bb );
void boardDebug();
void debugAll();
void debugVal();
void debugColor();
void debugLife();
void debugMove();
void debugType();
void debugML();
void debugEval();
U64 perft( int depth );
void runPerf();
#endif
