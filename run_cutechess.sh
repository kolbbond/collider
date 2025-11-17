#!/bin/bash


# run cutechess
cutechess-cli \
    -engine cmd="./build/bin/collider" \
    -engine cmd="/usr/games/./stockfish" \
    -each tc=40/10+10 proto=uci timemargin=2000 \
    -pgnout results4.pgn \
    -games 10 \
    -debug

