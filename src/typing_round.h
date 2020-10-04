#ifndef TYPING_ROUND_H_
#define TYPING_ROUND_H_

extern int n_ch;
extern int playing_round;
extern int playing_time;
extern char *user_word;

int typing_round(char* words[], struct score *user_score);
int reset_score(struct score* user_score);

#endif
