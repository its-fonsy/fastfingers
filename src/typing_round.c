#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "gui.h"
#include "util.h"

#define KEY_ESC 27
#define DEFAULT_ROUND_TIME 59

/* playing_round has 3 state:			*/
/* 	  0 new game wait to start	 		*/
/* 	  1 game in progress				*/
/* 	 -1 round ended cause time expired	*/

int n_ch = 0, playing_round = 0, playing_time = DEFAULT_ROUND_TIME;
char *user_word;

int typing_round(char* array_words[], struct score* user_score)
{

	int cursor_x, cursor_y, ch, index_word_to_type = 0;
	int print_raw = 0;
	size_t word_lenght = 0;

	// reset the score
	user_score->correct_typed_words 	= 0;
	user_score->correct_keystrokes		= 0;
	user_score->incorrect_typed_words	= 0;
	user_score->incorrect_keystrokes	= 0;

	char *word_to_type;
	user_word = (char*)malloc(MAX_WORD_LENGHT*sizeof(char));
	word_to_type = (char*)malloc(MAX_WORD_LENGHT*sizeof(char));

	// copy the first word to type
	strcpy(word_to_type, array_words[index_word_to_type]);

	// select the first word to type
	attron(A_BOLD);
	mvprintw(y_offset, x_offset, word_to_type);
	attroff(A_BOLD);

	// reset the round variables
	playing_round 	= 0;
	n_ch 			= 0;
	playing_time	= DEFAULT_ROUND_TIME;

	// print the start time
	attron(A_BOLD);
	mvprintw(y_offset - 2, (COLS/2) - 3, "01:00");
	attroff(A_BOLD);

	// clear the cursor line and move on position
	move(y_offset + 2, 0);
	clrtoeol();
	move(y_offset + 2, (COLS/2) - 5);

	// print a line under user input
	mvaddstr(y_offset + 4, (COLS/2) - 5 - 1, "────────────────");

	// move the cursor to user input
	move(y_offset + 3, (COLS/2) - 5);
	*user_word = '\0';

	while( playing_round != -1 && (ch = getch()))
	{
		switch (ch)
		{
			case KEY_BACKSPACE:
				if(n_ch > 0)
				{
					// delete character from the word
					getyx(stdscr, cursor_y, cursor_x);
					mvdelch(cursor_y, cursor_x - 1);

					user_word--;
					n_ch--;
					user_score->incorrect_keystrokes--;

					// dealing with accent letters
					if(*(--user_word) == -61)
						n_ch--;
					else
						user_word++;

					*user_word = '\0';
				}
				break;

			case ' ': // space pressed

				// missclick
				if(!n_ch)
					break;

				// end the string
				*user_word = '\0';
				user_word -= n_ch;
				n_ch = 0;

				// update the word to be typed
				index_word_to_type++;
				if (index_word_to_type == (MAX_WORDS_PER_ROW * (print_raw + 1)))
				{
					print_raw++;
					print_words_to_type(print_raw, array_words);
					word_lenght = 0;

					if (word_typed_right(user_word, word_to_type))
						user_score->correct_typed_words++;
					else
						user_score->incorrect_typed_words++;

				} else {
					if (word_typed_right(user_word, word_to_type))
					{
						/* correct_typed_words++; */
						user_score->correct_typed_words++;
						attron(COLOR_PAIR(COL_RIGHT_WORD));

					} else
					{
						user_score->incorrect_typed_words++;
						attron(COLOR_PAIR(COL_WRONG_WORD));
					}

					// mark as correct or wrong the current typed word
					mvprintw(y_offset, x_offset + word_lenght, word_to_type);
					attroff(COLOR_PAIR(COL_WRONG_WORD));
					attroff(COLOR_PAIR(COL_RIGHT_WORD));

					word_lenght += string_len(word_to_type) + 1;
				}
				strcpy(word_to_type, array_words[index_word_to_type]);

				// select next word to be typed
				attron(A_BOLD);
				mvprintw(y_offset, x_offset + word_lenght, word_to_type);
				attroff(A_BOLD);

				// reset the cursor and user word
				move(y_offset + 3, (COLS/2) - 5);
				*user_word = '\0';
				clrtoeol();

				break;

			case '\n' || KEY_ESC:
				break;

			case '	': // TAB pressed
				print_raw = 0;
				return 0; break;

			default:
				break;
		}

		// keypress is alphabetical character
		if ( ((ch >=97)   && (ch <= 122))||	// a to z
			  (ch == 160) || (ch == 168) ||	// à and è
			  (ch == 172) || (ch == 178) ||	// ì and ò
		 	  (ch == 185)             	  )	// ù
		{

			// start the timer
			if(!playing_round)
				playing_round = 1;

			// deal with accent word
			switch(ch)
			{
				case 160: // à
					*(user_word++) = -61;
					*(user_word++) = -96;
					n_ch += 2;
					break;

				case 168: // è
					*(user_word++) = -61;
					*(user_word++) = -88;
					n_ch += 2;
					break;

				case 172: // ì
					*(user_word++) = -61;
					*(user_word++) = -84;
					n_ch += 2;
					break;

				case 178: // ò
					*(user_word++) = -61;
					*(user_word++) = -78;
					n_ch += 2;
					break;

				case 185: // ù
					*(user_word++) = -61;
					*(user_word++) = -71;
					n_ch += 2;
					break;

				default: // a to z
					*(user_word++) = ch;
					n_ch++;
					break;
				}

			*user_word = '\0';

			if( typing_word_correctly(user_word - n_ch, word_to_type) )
			{
				user_score->correct_keystrokes++;
				attroff(COLOR_PAIR(COL_WRONG_WORD));
			}
			else
			{
				attron(COLOR_PAIR(COL_WRONG_WORD));
				user_score->incorrect_keystrokes++;
			}

			switch(ch)
			{
				case 160:
					addstr("à");
					break;
				case 168:
					addstr("è");
					break;
				case 172:
					addstr("ì");
					break;
				case 178:
					addstr("ò");
					break;
				case 185:
					addstr("ù");
					break;
				default:
					addch(ch);
					break;
			}
		}
	}

	// round ended because the time expired
	print_raw = 0;

	return 1;
}
