#pragma once

void draw_field(uint8_t **f, active_block *ab, uint8_t *spbag, int score, bool show_next_block, bool show_active_block);
uint8_t *get_spbag();
void init_block(active_block *ab, uint8_t *spbag);
bool rotate(uint8_t **f, active_block *ab);
bool move_block_sideways(uint8_t **f, active_block *ab, int dir);
bool move_block_down(uint8_t **f, active_block *ab);
void merge_block_to_field(uint8_t **f, active_block *ab);
void lines_disappearing(uint8_t **f, int *score);
bool is_top_out(uint8_t **f, active_block *ab);
void draw_text_inside_frame(char *text, int row);
