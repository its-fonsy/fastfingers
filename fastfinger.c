#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#define N_WORD_PER_ROW 7
#define MAX_WORDS 255
#define WORD_LENGHT 16

#define KEY_ESC 27

// color stuff
#define RIGHT_WORD 1
#define WRONG_WORD 2
#define SELECT_WORD 3

#define ROUND_TIME 15

// variables
int x_offset, y_offset, tic = 0;
char *words[MAX_WORDS];

// functions
int init_curses();
int typing_word_correctly(char *user, char *word_to_type);
int word_typed_right(char *user, char *word_to_type);
int feed_words_into_array();
int shuffle();
int typing_round();
int time_track();
int view_score();
void second_elapsed();
void print_words_to_type();

int main()
{
	int time_child, ch;
	int play_again = 1;
	time_child = fork();

	// gui init
	init_curses();

	// malloc the words array
	for (int i = 0; i < MAX_WORDS; i++)
		words[i] = (char*)malloc(WORD_LENGHT*sizeof(char));
	
	// child take track of time
	if (time_child == 0)
		time_track();

	// father wait for child signals
	signal(SIGUSR1, second_elapsed);

	while(play_again)
	{
		// populate the array with words from a file
		feed_words_into_array();
		print_words_to_type();

		// type some words!
		if( typing_round() )
		{
			// view the score of a round
			view_score();

			// BACKSPACE to exit
			// TAB to play again
			while((ch = getch()))
				if(ch == '	' || ch == KEY_BACKSPACE)
					break;

			if (ch == KEY_BACKSPACE)
				play_again = 0;
		}
		clear();
	}
	
	endwin();
	kill(time_child, SIGKILL);

	return 0;
}

int init_curses()
{
	// initial curses setup
	initscr();
	raw();
	keypad(stdscr, TRUE);
	noecho();
	/* timeout(1); */

	if(has_colors() == FALSE)
	{	endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}

	start_color();
	init_pair(RIGHT_WORD, COLOR_GREEN, COLOR_BLACK);
	init_pair(WRONG_WORD, COLOR_RED, COLOR_BLACK);
	init_pair(SELECT_WORD, COLOR_BLACK, COLOR_WHITE);

	x_offset = (COLS/2) - 30;
	y_offset = 5;

	mvprintw(y_offset + 3, (COLS/2)-7, "-----------------");
	// ──────────────
	return 1;
}

int typing_word_correctly(char *user, char *word_to_type){
	for( ; *user != '\0' ; user++, word_to_type++ ){
		if (*user != *word_to_type)
			return 0;
	}
	return 1;
}

int word_typed_right(char *user, char *word_to_type){
	for( ; *user == *word_to_type ; user++, word_to_type++ ){
		if (*user == '\0')
			return 1;
	}
	return 0;
}

uint8_t print_raw = 0;
void print_words_to_type()
{
	size_t word_lenght = 0;
	int i;

	attroff(COLOR_PAIR(WRONG_WORD));
	attroff(COLOR_PAIR(RIGHT_WORD));

	move(y_offset, 0);
	clrtoeol();
	for (i = N_WORD_PER_ROW * print_raw; i < N_WORD_PER_ROW + N_WORD_PER_ROW * print_raw; i++) {
		mvprintw(y_offset, x_offset + word_lenght, words[i]);
		word_lenght += strlen(words[i]) + 1;
	}

	word_lenght = 0;
	print_raw++;

	move(y_offset + 1, 0);
	clrtoeol();

	for (i = N_WORD_PER_ROW * print_raw; i < N_WORD_PER_ROW * (1 + print_raw); i++) {
		mvprintw(y_offset + 1, x_offset + word_lenght, words[i]);
		word_lenght += strlen(words[i]) + 1;
	}
}

int feed_words_into_array()
{
	FILE *words_file;
	words_file = fopen("words.txt", "r");

	int i = 0;
	char *buffer = (char*)malloc(WORD_LENGHT*sizeof(char));

	srand((unsigned) time(NULL));

	// 1/3 of probabilty that the word will be added to the array
	while ((fscanf(words_file, "%s", buffer) != EOF) && i < MAX_WORDS )
		if (rand() % 4 == 1)
			strcpy(words[i++], buffer);
	
	free(buffer);
	fclose(words_file);

	shuffle();

	return 1;
}

int shuffle()
{
	int indx1, indx2;
	char *dummy;
	srand((unsigned) time(NULL));

	for (int i = 0; i < 500; i++){ 
		indx1 = rand() % 255;
		dummy = words[indx1];
		indx2 = rand() % 255;
		words[indx1] = words[indx2];
		words[indx2] = dummy;
	}
}

int n_ch = 0, playing_round = 0, playing_time = ROUND_TIME;
int correct_typed_words 	= 0;
int incorrect_typed_words 	= 0;
int index_word_to_type 		= 0;

/* playing_round has 3 state:			*/
/* 	  0 new game wait to start	 		*/
/* 	  1 game in progress				*/
/* 	 -1 round ended cause time expired	*/

int typing_round()
{
	int cursor_x, cursor_y, ch;
	size_t word_lenght = 0;

	char *user_word, *word_to_type;
	user_word = (char*)malloc(WORD_LENGHT*sizeof(char));
	word_to_type = (char*)malloc(WORD_LENGHT*sizeof(char));

	strcpy(word_to_type, words[index_word_to_type]);

	attron(COLOR_PAIR(SELECT_WORD));
	mvprintw(y_offset, x_offset, word_to_type);
	attroff(COLOR_PAIR(SELECT_WORD));

	// print the start time
	mvprintw(y_offset - 2, (COLS/2) - 3, "01:00");

	// clear the cursor line and move on position
	move(y_offset + 2, 0);
	clrtoeol();
	move(y_offset + 2, (COLS/2) - 5);

	while( playing_round != -1 && (ch = getch()))
	{
		move(y_offset + 2, (COLS/2) - 5 + n_ch);
		switch (ch)
		{
			case KEY_BACKSPACE:
				if(n_ch > 0)
				{
					getyx(stdscr, cursor_y, cursor_x);
					mvdelch(cursor_y, cursor_x - 1);
					user_word--;
					n_ch--;
				}
				break;
			case ' ':
				// end the string
				*user_word = '\0';
				user_word -= n_ch;
				n_ch = 0;

				// update the word that must be typed
				index_word_to_type++;
				if (index_word_to_type == (N_WORD_PER_ROW * print_raw))
				{
					print_words_to_type();
					word_lenght = 0;

					if (word_typed_right(user_word, word_to_type))
						correct_typed_words++;
					else
						incorrect_typed_words++;

				} else {
					if (word_typed_right(user_word, word_to_type))
					{
						correct_typed_words++;
						attron(COLOR_PAIR(RIGHT_WORD));

					} else
					{
						incorrect_typed_words++;
						attron(COLOR_PAIR(WRONG_WORD));
					}

					// mark as correct or wrong the current typed word
					mvprintw(y_offset, x_offset + word_lenght, word_to_type);
					attroff(COLOR_PAIR(WRONG_WORD));
					attroff(COLOR_PAIR(RIGHT_WORD));

					word_lenght += strlen(word_to_type) + 1;
				}
				strcpy(word_to_type, words[index_word_to_type]);

				// select next word to be typed
				attron(COLOR_PAIR(SELECT_WORD));
				mvprintw(y_offset, x_offset + word_lenght, word_to_type);
				attroff(COLOR_PAIR(SELECT_WORD));

				// reset the cursor
				move(y_offset + 2, (COLS/2) - 5);
				clrtoeol();

				break;

			case '\n':
				break;

			case '	':

				// reset the round variables
				playing_round 		= 0;
				print_raw 			= 0;
				index_word_to_type 	= 0;
				n_ch 				= 0;
				playing_time 		= ROUND_TIME;

				// reset the score
				correct_typed_words 	= 0;
				incorrect_typed_words 	= 0;
				index_word_to_type 		= 0;

				return 0; break;

			default:

				*(user_word++) = ch;
				*user_word = '\0';
			 	n_ch++;

				if(playing_round == 0)
					playing_round = 1;

				if( typing_word_correctly(user_word - n_ch, word_to_type) )
					attron(COLOR_PAIR(RIGHT_WORD));
				else
					attron(COLOR_PAIR(WRONG_WORD));

				addch(ch);
				break;
		}
	}
	// round ended because of time expired
	
	// reset the round variables
	playing_round 		= 0;
	print_raw 			= 0;
	index_word_to_type 	= 0;
	n_ch 				= 0;
	playing_time 		= ROUND_TIME;

	// reset the score
	correct_typed_words 	= 0;
	incorrect_typed_words 	= 0;
	index_word_to_type 		= 0;

	return 1;
}

int view_score()
{
	clear();
	
	attroff(COLOR_PAIR(WRONG_WORD));
	attroff(COLOR_PAIR(RIGHT_WORD));
	attroff(COLOR_PAIR(SELECT_WORD));

	mvprintw(y_offset,     (COLS/2) - 3, "%d WPM", index_word_to_type);
	mvprintw(y_offset + 1, (COLS/2) - 8, "Correct words: %d", correct_typed_words);
	mvprintw(y_offset + 2, (COLS/2) - 8, "Mistyped words: %d", incorrect_typed_words);
	mvprintw(y_offset + 4, (COLS/2) - 8, "Press BACKSPACE to exit", incorrect_typed_words);

	return 1;
}

void second_elapsed()
{
	// disabling all coloring fancy
	attroff(COLOR_PAIR(WRONG_WORD));
	attroff(COLOR_PAIR(RIGHT_WORD));
	attroff(COLOR_PAIR(SELECT_WORD));

	// decrease the time only if the user is playing
	if(playing_round == 1)
		mvprintw(y_offset - 2, (COLS/2) - 3, (playing_time-- >= 10) ? "00:%d" : "00:0%d", playing_time);
	else if(playing_round == -1) {
		move(y_offset - 2, (COLS/2) - 3);
		clrtoeol();
	}

	// when time reach 0 then end the round
	playing_round = (playing_time <= 0 && playing_round != 0) ? -1 : playing_round;

	// reset the cursor to the input box
	move(y_offset + 2, (COLS/2) - 5 + n_ch);
	refresh();
}

// Child function that send SIGUSR1 every second 
int time_track()
{
	clock_t begin = clock();
	double time_spent;

	while(1)
	{
		time_spent = (double)(clock() - begin) / CLOCKS_PER_SEC;

		if(time_spent > 1.0)
		{
			begin = clock();
			kill(getppid(), SIGUSR1);
		}
	}
}
