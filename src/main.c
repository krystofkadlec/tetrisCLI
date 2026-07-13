#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include "../include/input_handling.h"
#include "../include/game.h"
#include "../include/utils.h"
#include "../include/config.h"

int main(){
  srand(time(NULL));

  // setting conio terminal mode for specific input
  set_conio_terminal_mode();
  printf("\n");

  // this is where blocks are stored after they drop
  uint8_t **field = calloc(FIELD_HEIGHT, sizeof(char *));
  for(int i = 0; i < FIELD_HEIGHT; i++)
    field[i] = calloc(FIELD_WIDTH, sizeof(char));

  // seven piece bag for getting the random block type, last item in the bag is index of which item was last used
  uint8_t *spbag = get_spbag();

  // this currently falling block
  active_block ab = { 0 };
  init_block(&ab, spbag);

  int interval_counter = 0;
  bool run = true;
  bool gameover = false;
  int score = 0;

  // start screen
  draw_field(field, &ab, spbag, -1, false, false);
  draw_text_inside_frame("PRESS", 9);
  draw_text_inside_frame("SPACE", 10);
  draw_text_inside_frame("TO  START", 11);
  while(!(kbhit() && getchar() == ' '));

  // game loop
  while(run){
    if(kbhit()) {
      char ch = getchar();
      switch(ch){
        case '\n':
          if(gameover){
            for(int i = 0; i < FIELD_HEIGHT; i++) for(int j = 0; j < FIELD_WIDTH; j++) field[i][j] = 0;
            free(spbag);
            spbag = get_spbag();
            init_block(&ab, spbag);
            gameover = false;
          }
          break;
        case 'q':
          run = false;
          break;
        case 'a':
          move_block_sideways(field, &ab, -1);
          break;
        case 'd':
          move_block_sideways(field, &ab, 1);
          break;
        case 'w':
          rotate(field, &ab);
          break;
        case 's':
          move_block_down(field, &ab);
          interval_counter = 0;
          break;
        case ' ':
          while(move_block_down(field, &ab));
          interval_counter = 0;
          break;
        case '\x1b':
          getchar();
          int ch = getchar();
          if(ch == 'A') rotate(field, &ab);
          else if(ch == 'B') {
            move_block_down(field, &ab);
            interval_counter = 0;
          }
          else if(ch == 'C') move_block_sideways(field, &ab, 1);
          else if(ch == 'D') move_block_sideways(field, &ab, -1);
      }
    }

    // skip game logic if the game is over
    if(gameover){
      continue;
    }

    // move the block once in a while
    if(interval_counter >= DEFAULT_SPEED){
      if(!move_block_down(field, &ab)){
        merge_block_to_field(field, &ab);
        init_block(&ab, spbag);
        if(is_top_out(field, &ab)){
          gameover = true;
          draw_text_inside_frame("GAME OVER", 9);
          char buf[20] = { 0 };
          sprintf(buf, "SCORE: %d", score);
          draw_text_inside_frame(buf, 10);
        }
      }
      interval_counter = 0;
      if(gameover) continue;
    } else interval_counter++;

    // go back up and redraw the game
    printf("\x1b[%dA",FIELD_HEIGHT - SPAWN_ZONE + 1);
    draw_field(field, &ab, spbag, score, true, true);

    lines_disappearing(field, &score);

    usleep(16000);
  }

  for(int i = 0; i < FIELD_HEIGHT; i++) free(field[i]);
  free(field);
  set_canon_terminal_mode();
  return 0;
}
