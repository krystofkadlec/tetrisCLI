#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <string.h>
#include "../include/config.h"
#include "../include/game.h"

point l_block[4] = {{-1, -1}, {-1, 0}, {0, 0}, {1, 0}};
point rev_l_block[4] = {{-1, 0}, {0, 0}, {1, 0}, {1, -1}};
point t_block[4] = {{-1, 0}, {0, -1}, {0, 0}, {1, 0}};
point square[4] = {{-0.5, -0.5}, {0.5, -0.5}, {-0.5, 0.5}, {0.5, 0.5}};
point line_piece[4] = {{-1.5, -0.5}, {-0.5, -0.5}, {0.5, -0.5}, {1.5, -0.5}};
point squiggly[4] = {{-1, 0}, {0, 0}, {0, -1}, {1, -1}};
point rev_squiggly[4] = {{-1, -1}, {0, -1}, {0, 0}, {1, 0}};

point *get_shape_ptr(enum block_type type){
  switch(type){
    case L_BLOCK: return l_block;
    case REV_L_BLOCK: return rev_l_block;
    case T_BLOCK: return t_block;
    case SQUARE: return square;
    case LINE_PIECE: return line_piece;
    case SQUIGGLY: return squiggly;
    case REV_SQUIGGLY: return rev_squiggly;
    default: return l_block;
  }
}

uint8_t get_color(enum block_type type){
  switch(type){
    case L_BLOCK: return ORANGE;
    case REV_L_BLOCK: return BLUE;
    case T_BLOCK: return PURPLE;
    case SQUARE: return YELLOW;
    case LINE_PIECE: return LIGHT_BLUE;
    case SQUIGGLY: return GREEN;
    case REV_SQUIGGLY: return RED;
    default: return 88;
  }
}

uint8_t *get_spbag(){
  uint8_t *seven_piece_bag = calloc(BLOCK_TYPE_COUNT + 1, sizeof(int));
  for(int i = 0; i < BLOCK_TYPE_COUNT; i++) seven_piece_bag[i] = i;

  for(int i = 0; i < 30; i++){
    int a = rand() % BLOCK_TYPE_COUNT;
    int b = rand() % BLOCK_TYPE_COUNT;
    int tmp = seven_piece_bag[a];
    seven_piece_bag[a] = seven_piece_bag[b];
    seven_piece_bag[b] = tmp;
  }

  return seven_piece_bag;
}

void update_spbag(uint8_t *spbag){
  int next_piece = spbag[spbag[BLOCK_TYPE_COUNT]];

  for(int i = 0; i < 30; i++){
    int a = rand() % BLOCK_TYPE_COUNT;
    int b = rand() % BLOCK_TYPE_COUNT;
    int tmp = spbag[a];
    spbag[a] = spbag[b];
    spbag[b] = tmp;
  }

  int next_piece_index = 0;
  for(int i = 0; i < BLOCK_TYPE_COUNT; i++)
    if(spbag[i] == next_piece) next_piece_index = i;

  int tmp = spbag[0];
  spbag[0] = spbag[next_piece_index];
  spbag[next_piece_index] = tmp;

  spbag[BLOCK_TYPE_COUNT] = 0;
}

void init_block(active_block *ab, uint8_t *spbag){
  spbag[BLOCK_TYPE_COUNT]++;
  if(spbag[BLOCK_TYPE_COUNT] == BLOCK_TYPE_COUNT-1) update_spbag(spbag);
  ab->type = spbag[spbag[BLOCK_TYPE_COUNT]];

  point * wanted_shape = get_shape_ptr(ab->type);
  memcpy(ab->shape, wanted_shape, sizeof(ab->shape));

  ab->color = get_color(ab->type);

  float offset = 0;
  if(ab->type == SQUARE || ab->type == LINE_PIECE) offset = 0.5;
  ab->x = DEFAULT_SPAWN_X + offset;
  ab->y = DEFAULT_SPAWN_Y + offset;
}

bool is_colliding(uint8_t **f, active_block *ab, point *new_shape){
  point world_pos[sizeof(ab->shape) / sizeof(point)];

  for(int i = 0; i < sizeof(ab->shape) / sizeof(point); i++){
    world_pos[i].x = ab->x + new_shape[i].x;
    world_pos[i].y = ab->y + new_shape[i].y;
  }

  for(int i = 0; i < sizeof(ab->shape) / sizeof(point); i++){
    if(world_pos[i].y > FIELD_HEIGHT || world_pos[i].x < 0 || world_pos[i].x >= FIELD_WIDTH)
      return true;

    if(f[(int)(world_pos[i].y)][(int)(world_pos[i].x)] != 0)
      return true;
  }

  return false;
}

bool rotate(uint8_t **f, active_block *ab){
  point new_shape[sizeof(ab->shape) / sizeof(point)];
  for(int i = 0; i < sizeof(ab->shape) / sizeof(point); i++){
    float x = 0.0 * (float)ab->shape[i].x - 1.0 * (float)ab->shape[i].y;
    float y = (float)ab->shape[i].x + 0.0 * (float)ab->shape[i].y;
    new_shape[i].x = x;
    new_shape[i].y = y;
  }

  // basic wallkick logic
  if(is_colliding(f, ab, new_shape)){
    ab->x += 1;
    if(is_colliding(f, ab, new_shape)){
      ab->x -= 2;
      if(is_colliding(f, ab, new_shape)){
        ab->x += 1;
        return false;
      }
    }
  }

  for(int i = 0; i < sizeof(ab->shape) / sizeof(point); i++)
    ab->shape[i] = new_shape[i];

  return true;
}

bool move_block_sideways(uint8_t **f, active_block *ab, int dir){
  point world_pos[sizeof(ab->shape) / sizeof(point)];

  for(int i = 0; i < sizeof(ab->shape) / sizeof(point); i++){
    world_pos[i].x = ab->x + ab->shape[i].x + dir;
    world_pos[i].y = ab->y + ab->shape[i].y;
    if(world_pos[i].x < 0 || world_pos[i].x >= FIELD_WIDTH) return false;
  }

  for(int i = 0; i < sizeof(ab->shape) / sizeof(point); i++)
    if(f[(int)world_pos[i].y][(int)world_pos[i].x] != 0) return false;

  ab->x += dir;
}

bool move_block_down(uint8_t **f, active_block *ab){
  point world_pos[sizeof(ab->shape) / sizeof(point)];

  for(int i = 0; i < sizeof(ab->shape) / sizeof(point); i++){
    world_pos[i].x = ab->x + ab->shape[i].x;
    world_pos[i].y = ab->y + ab->shape[i].y + 1;
    if(world_pos[i].y >= FIELD_HEIGHT) return false;
  }

  for(int i = 0; i < sizeof(ab->shape) / sizeof(point); i++){
    if(world_pos[i].y < 0) continue;
    if(f[(int)world_pos[i].y][(int)world_pos[i].x] != 0) return false;
  }
  ab->y++;

  return true;
}

// helper function for draw_field()
bool is_block_pos(point *p, int p_size, int x, int y){
  for(int i = 0; i < p_size; i++)
    if((int)p[i].x == x && (int)p[i].y == y) return true;
  return false;
}

void draw_field(uint8_t **f, active_block *ab, uint8_t *spbag, int score, bool show_next_block, bool show_active_block){
  point world_pos[sizeof(ab->shape) / sizeof(point)];

  for(int i = 0; i < sizeof(ab->shape) / sizeof(point); i++){
    world_pos[i].x = ab->x + ab->shape[i].x;
    world_pos[i].y = ab->y + ab->shape[i].y;
  }

  // ghost piece
  active_block gp = {ab->x, ab->y, ab->type, 0, ab->color};
  memcpy(gp.shape, ab->shape, sizeof(ab->shape));
  while(move_block_down(f, &gp));
  point gp_world_pos[sizeof(ab->shape) / sizeof(point)];
  for(int i = 0; i < sizeof(ab->shape) / sizeof(point); i++){
    gp_world_pos[i].x = gp.x + gp.shape[i].x;
    gp_world_pos[i].y = gp.y + gp.shape[i].y;
  }

  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  int start = w.ws_col / 2 - (FIELD_WIDTH + 2);

  for(int i = SPAWN_ZONE; i < FIELD_HEIGHT; i++){
    printf("\x1B[%dC", start);
    printf("\x1B[38;5;231m%s\x1B[0m", BLOCK_SYM);
    for(int j = 0; j < FIELD_WIDTH; j++){
      if(f[i][j] != 0) printf("\x1B[38;5;%dm%s\x1B[0m", f[i][j], BLOCK_SYM);
      else if(is_block_pos(world_pos, sizeof(world_pos) / sizeof(point), j, i) && show_active_block)
        printf("\x1B[38;5;%dm%s\x1B[0m", ab->color, BLOCK_SYM);
      else if(is_block_pos(gp_world_pos, sizeof(gp_world_pos) / sizeof(point), j, i) && show_active_block)
        printf("\x1B[38;5;%dm%s\x1B[0m", GHOST_COLOR, BLOCK_SYM);
      else printf("  ");
    }
    printf("\x1B[38;5;231m%s\x1B[0m", BLOCK_SYM);

    // if score is negative, dont show it (used for start screen)
    if(score >= 0 && i - SPAWN_ZONE == SCORE_POS) printf("\tSCORE:");
    else if(score >= 0 && i - SPAWN_ZONE == SCORE_POS + 1) printf("\t\t %d", score);
    printf("\n");
  }
  for(int k = 0; k < start; k++) printf(" ");
  printf("\x1B[38;5;231m%s", BLOCK_SYM);
  for(int i = 0; i < FIELD_WIDTH; i++) printf("%s", BLOCK_SYM);
  printf("%s\x1B[0m\n", BLOCK_SYM);

  if(show_next_block){
    printf("\x1B[%dA\x1B[%dCNEXT:\x1B[1B\x1B[3D", (FIELD_HEIGHT - SPAWN_ZONE + 1) - NEXT_BLOCK_POS, start - 14);
    active_block next_block = { 0 };
    next_block.type = spbag[spbag[BLOCK_TYPE_COUNT] + 1];
    point *wanted_shape = get_shape_ptr(next_block.type);
    memcpy(next_block.shape, wanted_shape, sizeof(next_block.shape));
    next_block.color = get_color(next_block.type);
    float offset = 0;
    if(next_block.type == LINE_PIECE || next_block.type == SQUARE) offset = 0.5;
    next_block.x = 1 + offset;
    next_block.y = 1 + offset;
    for(int i = 0; i < 4; i++){
      for(int j = 0; j < 4; j++){
        for(int k = 0; k < sizeof(next_block.shape) / sizeof(point); k++)
          if(j == next_block.x + next_block.shape[k].x && i == next_block.y + next_block.shape[k].y){
            printf("\x1B[38;5;%dm%s\x1B[0m", next_block.color, BLOCK_SYM);
            break;
          }else if(k == sizeof(next_block.shape) / sizeof(point) - 1) printf("  ");
      }
      printf("\x1B[1B\x1B[%dD", 8);
    }
    printf("\x1B[%dB\x1B[0G", (FIELD_HEIGHT - SPAWN_ZONE + 1) - NEXT_BLOCK_POS - 5);
  }
  return;
}

void merge_block_to_field(uint8_t **f, active_block *ab){
  for(int i = 0; i < sizeof(ab->shape) / sizeof(point); i++){
    f[(int)(ab->y + ab->shape[i].y)][(int)(ab->x + ab->shape[i].x)] = ab->color;
  }
}

// helper function for lines_disappearing()
void del_line(uint8_t **f, int line){
  for(int i = line; i >= 1; i--)
    for(int j = 0; j < FIELD_WIDTH; j++) f[i][j] = f[i-1][j];

  for(int j = 0; j < FIELD_WIDTH; j++) f[0][j] = 0;
}

void lines_disappearing(uint8_t **f, int *score){
  for(int i = 0; i < FIELD_HEIGHT; i++){
    for(int j = 0; j < FIELD_WIDTH; j++){
      if(f[i][j] == 0) break;
      if(j == FIELD_WIDTH - 1) {
        del_line(f, i);
        (*score) += 1;
      }
    }
  }
}

bool is_top_out(uint8_t **f, active_block *ab){
  for(int i = 0; i < FIELD_HEIGHT; i++){
    for(int j = 0; j < FIELD_WIDTH; j++){
      for(int k = 0; k < sizeof(ab->shape) / sizeof(point); k++){
        if(j == ab->shape[k].x + ab->x && i == ab->shape[k].y + ab->y && f[i][j] != 0)
          return true;
      }
    }
  }

  return false;
}

void draw_text_inside_frame(char *text, int row){
  struct winsize w;
  ioctl(0, TIOCGWINSZ, &w);
  int start = w.ws_col / 2 - strlen(text) / 2;

  printf("\x1b[%dA\x1b[%dC%s", (FIELD_HEIGHT - SPAWN_ZONE + 1) - row, start, text);
  printf("\x1b[%dB\n", (FIELD_HEIGHT - SPAWN_ZONE) - row);
}
