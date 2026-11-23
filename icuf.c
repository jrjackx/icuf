/* includes */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <string.h>


  /* defines */

#define CTRL_KEY(k) ((k) & 0x1f)


/* OG data */

struct editorConfig {
	int cx, cy;
	int screenrows;
	int screencols;
	struct termios orig_termios;
};

struct editorConfig E;

/* terminal (low-level processing) */

void die(const char *s) {
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	perror(s);
	exit(1);
}

void disableRawMode() {
//error check: runs what's inside the if statement, THEN checks the outputted value. if != -1, the command executes like normal. this kind of syntax is regularly used.
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) 
	die("tcsetattr");
}

void enableRawMode() {
	if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) {
		die("tcgetattr");
	}
	atexit(disableRawMode);

	struct termios raw = E.orig_termios;
	tcgetattr(STDIN_FILENO, &raw);

//all of these flags have something to do with turning off extra crap that the terminal does by default to cook the data. full* list found in demonstrating_understanding.

	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | ISIG);

//causes the program to read() every 10th of a second inside the main loop.
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
	die("tcsetattr");
}

//this actually doesn't do anything because i commented out the function that uses it.
//it's editorProcessKeypress. i turned it off because it was messing up the animation.
//char editorReadKey() {
//	int nread;
//	char c;
//	while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
//		if (nread == -1 && errno != EAGAIN) die("read");
//	}
//	return c;
//}

int getWindowSize(int *rows, int *cols) {
	struct winsize ws;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
		return -1;
	} else {
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}

int getCursorPosition(int *rows, int *cols) {
	char buf[32];
	unsigned int i = 0;

	if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

	while (i < sizeof(buf) - 1) {
		if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
		if (buf[i] == 'R') break;
		i++;
	}
	buf[i] = '\0';

	if (buf[0] != '\x1b' || buf[1] != '[') return -1;
	if (sscanf(&buf[2], "%d,%d", rows, cols) != 2) return -1;

	return 0;
}


/*output */

/* LOOK HERE!!! THIS IS WHERE YOU WILL PLUG IN ALL OF THE DISPLAY LOGIC FOR ICUF */

//All of these todos are finished, leaving them here for documentation / archival purposes
//TODO increment phase from within draw floor.
//TODO implement some kind of wait system so the animation actually plays out
//TODO all logic surrounding planting the seeds, saving their locations, growing the stem, terminating stem to grow flower head

struct flower {
	int pos;
	int height;
	int growing;
};

struct garden_info {
	struct flower garden[8];
	int size;
};

struct garden_info garden_info;

void draw_floor(int *phase) {
	char buff[16] = {0};
	int size = sprintf(buff, "\x1b[%d;0H", E.screenrows);
	write(STDOUT_FILENO, buff, size);

	int x;
	for (x = 0; x < E.screencols; x++) {
		write(STDOUT_FILENO, "=", 1);
	}
	
	(*phase)++;
}

void plant_seeds(int *phase) {
	int upper_bound = (E.screencols/3); //makes an average of 3 seeds i think.
	int plant_chance; 
	
	for(int i = 11; i < E.screencols-11; i++) {
		plant_chance = rand() % (upper_bound + 1);
		if(plant_chance == upper_bound){
			garden_info.garden[garden_info.size] = (struct flower) {
				.pos = i,
				.height = 0,
				.growing = 1
			};
			garden_info.size++;
			i += 20;
		}
	}

	//obsolete testing code
//	char buff[32] = {0};
//	for(int i = 0; i < garden_info.size; i++) {
//		sprintf(buff + strlen(buff), "%d, ", garden_info.garden[i].pos);
//	}
//
//	write(STDOUT_FILENO, buff, strlen(buff));
	if(garden_info.garden[0].growing) {
		(*phase)++;
	}
}

void grow(int *phase) {
	int upper_bound = (E.screenrows/4);
	int terminalize_chance;


	char buff[16] = {0};
	for(int i = 0; i < garden_info.size; i++) {
		sprintf(buff, "\x1b[%d;%dH", ((E.screenrows - garden_info.garden[i].height) - 1), garden_info.garden[i].pos);
		write(STDOUT_FILENO, buff, strlen(buff));
		if (garden_info.garden[i].growing) {
			write(STDOUT_FILENO, "||", 2);

			if(garden_info.garden[i].height % 10 == 0 && garden_info.garden[i].height != 0){
		
				const char *leaf_rows[] = {
				"     __||__      ",
				"     //||\\\\	  ",
				"   _// || \\\\_  ",
				"  /  / || \\  \\",
				" /__/  ||  \\__\\",
				};

				int leaf_row_count = 5;

				char leaf_buff[32] = {0};
				int leaf_base_row = E.screenrows - garden_info.garden[i].height + 1;
				int leaf_base_col = garden_info.garden[i].pos - 7;

				for (int j = 0; j < leaf_row_count; j++){
					int size = sprintf(leaf_buff, "\x1b[%d;%dH", leaf_base_row - leaf_row_count + j, leaf_base_col);
					write(STDOUT_FILENO, leaf_buff, size);
					write(STDOUT_FILENO, leaf_rows[j], strlen(leaf_rows[j]));
				}
			}

			garden_info.garden[i].height++;
		}
		
		terminalize_chance = rand() % (upper_bound + 1);
		if ((((terminalize_chance == upper_bound) && garden_info.garden[i].height >=5) || garden_info.garden[i].height >= E.screenrows - 11) && garden_info.garden[i].growing == 1){
			garden_info.garden[i].growing = 0;
			
			const char *flower_rows[] = {
			"    ___      ___      ",
			"   /   \\    /   \\   ",
			"   \\    \\  /    /   ",
			" ___\\____\\/____/___ ",
			"/     /      \\     \\",
			"\\____/        \\____/",
			"    /\\        /\\    ",
			"   /  \\______/  \\   ",
			"  /   /  ||  \\   \\  ",
			"  \\__/   ||   \\__/  ",
			};
			int num_rows = 10;

			char flower_buff[32] = {0};
			int base_row = E.screenrows - garden_info.garden[i].height;
			int base_col = garden_info.garden[i].pos - 9;

			for (int j = 0; j < num_rows; j++) {
				int size = sprintf(flower_buff, "\x1b[%d;%dH", base_row - num_rows + j, base_col);
				write(STDOUT_FILENO, flower_buff, size);
				write(STDOUT_FILENO, flower_rows[j], strlen(flower_rows[j]));
			}
		}
	}

	int still_growing = 0;
	for(int i = 0; i < garden_info.size; i++) {
		if(garden_info.garden[i].growing == 1) {
			still_growing = 1;
		}
	}

	if(!still_growing){
		(*phase)++;
	}
}

void editorRefreshScreen(int *phase) {
	switch (*phase){
		case 0: draw_floor(phase); break;	
		case 1: plant_seeds(phase); break; 
		case 2: grow(phase); break;
		case 3: char buff[32] = {0};
				char exit_message[32] = "I love you kat!! <3";
				int size = sprintf(buff, "\x1b[2;%dH", ((E.screencols/2)-(int)(strlen(exit_message)/2)));
				write(STDOUT_FILENO, buff, size);
				write(STDOUT_FILENO, exit_message, strlen(exit_message));
				write(STDOUT_FILENO, "\x1b[H", 3);
				exit(0);
				break;
	}

}




/* input */

/* LOOK HERE!!! THIS IS WHERE ALL OF THE COMMANDS ARE THAT YOU WILL WANT TO CHANGE LATER! */
//actually it doesn't do anything because it was messing up the animation.
//causing it to hang on every phase of the growth cycle. so i commented it out.
//void editorProcessKeypress() {
//	char c = editorReadKey();
//
//	switch (c) {
//		case CTRL_KEY('q'):
//		write(STDOUT_FILENO, "\x1b[2J]", 4);
//		exit(0);
//		break;
//	}
//}


/* init */

void initEditor() {
	E.cx = 0;
	E.cy = 0;

	write(STDOUT_FILENO, "\x1b[2J", 4);
	if (getWindowSize(&E.screenrows, &E.screencols) == -1) die ("getWindowSize");
}

int main() {
	enableRawMode();
	initEditor();

	int phase = 0;
	srand(time(NULL));

	while (1) {
		editorRefreshScreen(&phase);

		char c;
		int quit = read(STDIN_FILENO, &c, 1);
		if (quit == 1 && c == CTRL_KEY('q')) {
			write(STDOUT_FILENO,"\x1b[H", 3);
			exit(0);
		}
	}

	return 0;
}
