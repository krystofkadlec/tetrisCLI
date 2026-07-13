#pragma once

#include <stdint.h>

enum block_type {
  L_BLOCK,
  REV_L_BLOCK,
  T_BLOCK,
  SQUARE,
  LINE_PIECE,
  SQUIGGLY,
  REV_SQUIGGLY,
  BLOCK_TYPE_COUNT
};

typedef struct point{
  float x;
  float y;
} point;

typedef struct active_block{
  float x;
  float y;
  enum block_type type;
  point shape[4];
  uint8_t color;
} active_block;
