#ifndef GUI_H_
#define GUI_H_

#define MAX_WORDS_PER_ROW 7

// color stuff
#define COL_RIGHT_WORD 1
#define COL_WRONG_WORD 2
#define COL_SELECT_WORD 3

struct score {
	int correct_keystrokes;
	int correct_typed_words;
	int incorrect_keystrokes;
	int incorrect_typed_words;
};

extern int x_offset;
extern int y_offset;

int init_curses();
int draw_gui();
void print_words_to_type(int print_raw, char *array[]);
int view_score(struct score round);

#endif
