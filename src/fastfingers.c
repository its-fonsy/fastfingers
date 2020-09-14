#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <locale.h>
#include <unistd.h>
#include <time.h>

#include "util.h"
#include "gui.h"
#include "typing_round.h"

// variables
char *words[MAX_WORDS];

// functions
int time_track();
void second_elapsed();

int main(int argc, char *argv[])
{
	int ch;
	if( argc == 2 )
	{
		ch = *++argv[1];
		switch(ch)
		{
			case 'h':
				// print usage message
				printf("usage: %s [-h]\n\n", argv[0]);
				printf("Practice your touch typing in your terminal\n\n");
				printf("OPTION\n");
				printf("  -h\t\tPrint this help message\n");
				break;
		}
		return 0;
	}

	int time_child;
	struct score user_score;

	// gui init
	init_curses(&y_offset, &x_offset);

	// malloc the words array
	for (int i = 0; i < MAX_WORDS; i++)
		words[i] = (char*)malloc(MAX_WORD_LENGHT*sizeof(char));
	
	// child take track of time
	time_child = fork();
	if (time_child == 0)
		time_track();

	// father handle child signals
	signal(SIGUSR1, second_elapsed);

	// start the game
	while(1)
	{
		clear();
		draw_gui();

		// populate the array with words from a file
		feed_words_into_array("italian.txt", words);
		print_words_to_type(0, words);

		// type some words
		if( typing_round(words, &user_score) )
			view_score(user_score);
	}
	
	endwin();
	kill(time_child, SIGKILL);

	return 0;
}

void second_elapsed()
{
	// disabling all coloring fancy
	attroff(COLOR_PAIR(COL_WRONG_WORD));
	attroff(COLOR_PAIR(COL_RIGHT_WORD));
	attroff(COLOR_PAIR(COL_SELECT_WORD));

	// decrease the time only if the user is playing
	attron(A_BOLD);
	if(playing_round == 1)
		mvprintw(y_offset - 2, (COLS/2) - 3, (playing_time-- >= 10) ? "00:%d" : "00:0%d", playing_time);
	attroff(A_BOLD);

	// when time reach 0 then end the round
	playing_round = (playing_time <= 0 && playing_round != 0) ? -1 : playing_round;

	// reset the cursor to the input box
	move(y_offset + 3, (COLS/2) - 5 + string_len(user_word - n_ch));
	refresh();
}

// Child send SIGUSR1 every second 
int time_track()
{
	clock_t begin = clock();
	double time_spent;

	while(1)
	{
		time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;

		if(time_spent >= 1.0)
		{
			begin = clock();
			kill(getppid(), SIGUSR1);
		}
	}
}
