/*

 TicTacToe AI. K!2020
 Author: Krzysztof Cieslak
 Repository: https://github.com/krcs/tic-tac-toe
 LICENSE: MIT

 You can compile this file in C:

  gcc ttt.c -o ttt

-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 
 TicTacToe AI
 
 BOARD:
   +---+---+---+
   |001|002|004|
   +---+---+---+
   |008|016|032|
   +---+---+---+
   |064|128|256|
   +---+---+---+

  SCORE            MOVE
  2048 1024  512 | 256 128 64  32  16   8   4   2   1
    0    0    0  |  0   0   0   0   0   0   0   0   0
    |    |    |
    X    0    O
   WIN  DRAW WIN
*/

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

#else // DEBUG OR STANDALONE COMPILATION

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#endif // -=-

#define SCORE_MASK 0xfe00 // 0b1111111000000000
#define PLAYER1_WIN_BIT 2048
#define PLAYER2_WIN_BIT 512
#define DRAW_BIT 1024

/*
    check_player(player)

              3 33 222 111 -> row number
    0b0000000 0|00 000 000

   Example:
       001              3 33 222 111
       010 -> 0b0000000 1|00 010 001
       100
*/
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
short check_player(short player) {
  return
    (player & 0x007) == 0x007 ||  // 0b00000000|00000111 ROW1
    (player & 0x038) == 0x038 ||  // 0b00000000|00111000 ROW2
    (player & 0x1C0) == 0x1C0 ||  // 0b00000001|11000000 ROW3
    (player & 0x049) == 0x049 ||  // 0b00000000|01001001 COL1
    (player & 0x092) == 0x092 ||  // 0b00000000|10010010 COL2
    (player & 0x124) == 0x124 ||  // 0b00000001|00100100 COL3
    (player & 0x111) == 0x111 ||  // 0b00000001|00010001 DIAG1
    (player & 0x054) == 0x054;    // 0b00000000|01010100 DIAG2
}

/* 
   transfer_sign(v,s)
   Transfers sign from value s to value v.

   Returns: value v with the sign of value s.

   Example:
    x = transfer_sign(2,-1) -> x = -2
    x = transfer_sign(-3,2) -> x = 3
*/
short transfer_sign(short v, short s) {
  short t = (v ^ s) >> 31;
  return (v ^ t) - t;
}

/*
   player 1 - (1)
   player 2 - (-1)
*/
short NegaMax(short side, short *p1, short *p2)  {
  /*
    Check if it's game over.

     2048 - PLAYER 1 WON
     1024 - DRAW
      512 - PLAYER 2 WON
        0 - game continues

  */
  const short end = check_player(*p1)
    ? PLAYER1_WIN_BIT
    : check_player(*p2)
    ? PLAYER2_WIN_BIT
    : ((*p1 | *p2) & 511) == 511
    ? DRAW_BIT
    : 0;
  
  if (end != 0)
    return end;

  short move;
  short  opponent_move = 0;
  short  best_value = (side == 1) ? PLAYER2_WIN_BIT : PLAYER1_WIN_BIT;

  for (short b = 1; b <= 256; b = b << 1) {
    move = ~(*p1 | *p2) & b;
    if (move) {
      short bit_position = transfer_sign(move, side);
      *p1 |= bit_position & -((bit_position >> 31) + 1);// & ~*p1; 
      *p2 |= -bit_position & (bit_position >> 31);      // & ~*p2;
    
      opponent_move = NegaMax(-side, p1, p2);
      
      best_value = transfer_sign(opponent_move & SCORE_MASK, side) 
        > transfer_sign(best_value & SCORE_MASK, side)
        ? ((opponent_move & SCORE_MASK) | move)
        : best_value;
      
      // clear move
      *p1 &= ~move;
      *p2 &= ~move;
    }
  }
  return best_value;
}

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
short generate_move(short side, short player1, short player2) { 
  return NegaMax(side, &player1, &player2) & 511; 
};

#ifndef __EMSCRIPTEN__ // DEBUG OR STANDALONE COMPILATION 

int main() {
  short player1 = 0; // side = 1 
  short player2 = 0; // side = -1
  short move = 0;

  time_t t;
  int seed = time(&t);
  srand(seed);
  
  player1 = 1 << rand() % 8; // First move is random.

  printf("TicTacToe AI. K!2020\n");
  printf("BOARD:\n");
  printf(" +-----+-----+-----+\n");
  printf(" | 001 | 002 | 004 |\n");
  printf(" +-----+-----+-----+\n");
  printf(" | 008 | 016 | 032 |\n");
  printf(" +-----+-----+-----+\n");
  printf(" | 064 | 128 | 256 |\n");
  printf(" +-----+-----+-----+\n\n");
  
  printf("Player1: %d\n", player1);
  do {
    move = generate_move(-1, player1, player2);
    player2 += move;
    printf("Player2: %d\n", move);

    move = generate_move(1, player1, player2);
    player1 += move;
    printf("Player1: %d\n", move);
  } while (move > 0);

  return 0;
}
#endif
