#!/bin/bash
emcc ./ttt.c -s STANDALONE_WASM -o ./ttt.wasm --no-entry
