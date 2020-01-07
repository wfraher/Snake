#include <curses.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#define LEFT_MOVE 1
#define RIGHT_MOVE 2
#define DOWN_MOVE 3
#define UP_MOVE 4
#define SCORE_DRAW_DISTANCE 5
#define BOARDER_COLOR 6
#define TROPHY_COLOR 7
#define SNAKE_COLOR 8
#define BACKGROUND_COLOR 9
#define SCORE_COLOR 10
#define SPEEDUP_PERGROWTH 98

/* Credits:
William Fraher did the core logic of the game: Handling/allocating the snake, drawing and moving the snake, growing the snake, handling collisions between the snake and walls, snake and the snake, etc.
Kelbin Rodgriguez handled the logic of the accessories of the game, such as making it fit to the terminal size, collisions with trophies, end game screens, and trophies randomly spawning with an alarm.
We each added our code to the spawn() method, the main method of the game.
We contributed together to some of the functions of the game, for instance, we each contributed code for making parts of the game appear in different colors.
We also each helped troubleshoot parts of the game when there were issues, regardless of who was in charge of which part.
We did not however write the curses or ncurses library. Whoever did saved us a lot of time.*/



/* Author: William Fraher. The body of the snake (its length, etc) is represented as a global variable. We also keep a blank character so we can delete the snake.	This way, we can keep updating the snake's position on the screen. */
char head_char = 'O';
char body_char = 'o';
char blank = ' ';
struct game_object{
	int x;
	int y;
	char symbol; //for the head this will b O, o otherwise
	struct game_object *next; //next piece of the snake, null for other objects
	struct game_object *previous;
};

/* Speed of the game */
int gamespeed = 100000;

/* The head is the part of the snake the user controls with the arrow keys. The body is the other parts of the snake.
 Tropies are the collectibles which grow the snake */
struct game_object *head;
int trophyX;
int trophyY;
int trophyVal;
int score = 0;


//Author: William Fraher /* Allocates memory for the start of the snake. The snake's head is a global variable so we need a separate method to do this. */ 
void init_snake(){
	head = (struct game_object *)malloc(sizeof(struct game_object));
	head->x = 0;
	head->y = 0;
	head->symbol = head_char;
	head->next = NULL;
	head->previous = NULL;
}

//Author: William Fraher 
/* Erases the snake. Starts with the head and recurses backwards.
	Must be called before move! */ 
void erase_snake(){
	struct game_object *iterator;
	iterator = head;
	while(iterator != NULL){
		move(iterator->y,iterator->x);
		addch(blank);
		iterator = iterator->next;
	}
}

//Author: William Fraher /* Grows the snake */ 
void grow_snake(int n){
	struct game_object *iterator;
	for(int i = 0; i < n; i++){
		iterator = head;
		while(iterator->next != NULL){
			iterator = iterator->next;
		}
		struct game_object *new_bodypart;
		new_bodypart = (struct game_object *)malloc(sizeof(struct game_object));
		new_bodypart->symbol = body_char;
		new_bodypart->previous = iterator;
		iterator->next = new_bodypart;
		gamespeed *= SPEEDUP_PERGROWTH; //can change this for the game to move more quickly/slowly
		gamespeed /= 100;
		score++;
	}
}

/* Draws the snake. Again, starts with the head and recurses backwards.
	Must be called after erase_snake! */ 
// Author: William Fraher
void draw_snake(){
	attron(COLOR_PAIR(SNAKE_COLOR));
	struct game_object *iterator;
	iterator = head;
	while(iterator != NULL){
		move(iterator->y,iterator->x);
		addch(iterator->symbol);
		iterator = iterator->next;
	}
	attroff(COLOR_PAIR(SNAKE_COLOR));
}

int dir = 1; //where he's going


 //Author: William Fraher /* Move the snake, according to a direction given*/
void movement(int direction){
	/* Erases where the snake was on the screen */
	erase_snake();
	/* Moves the rest of the snake */
	struct game_object *iterator;
	iterator = head;
	/* Iterates to the end of the list to find where to start moving the snake's body */
	while(iterator->next != NULL){
		iterator = iterator->next;
	}
	while(iterator->previous != NULL){
		iterator->x = iterator->previous->x;
		iterator->y = iterator->previous->y;
		iterator = iterator->previous;
	}
	/* Moves the head, given a direction */
	switch(direction){
		case LEFT_MOVE:
			head->x -= 1;
			dir = LEFT_MOVE;
			break;
		case RIGHT_MOVE:
			head->x += 1;
			dir = RIGHT_MOVE;
			break;
		case UP_MOVE:
			head->y -= 1;
			dir = UP_MOVE;
			break;
		case DOWN_MOVE:
			head->y += 1;
			dir = DOWN_MOVE;
			break;
		default:
			break;
	}
	/* Draws the snake */
	draw_snake();
}

//Author: Kelbin Rodriguez //Lose condition. Will be exacuted if user hits body,reverses, or hits the boarder 
void endGame() {
	endwin();
	printf("OOPS You Lost.\nScore: %d\n",score);
	exit(-1);
}
//Author: Kelbin Rodriguez //win condition for the snake game. Half the perimeter of the boarder
 void gameWon() {
	int row = (LINES-1-SCORE_DRAW_DISTANCE)+SCORE_DRAW_DISTANCE;
	int col = (col-1)+1;
	if(score>=(row+col))
	{
		endwin();
		printf("Congratulations! You Have Won the Snake Game\nScore: %d\n",score);
		exit(-1);
	}
}

//Author: Kelbin Rodriguez
//Store the number of rows and columns found on the screen
void getWindowSize(int *row,int *col) {
	struct winsize window;
	if(ioctl(0,TIOCGWINSZ,&window)!=1)
	{
		*row = window.ws_row;
		*col = window.ws_col;
	}
}

//Author: Kelbin Rodriguez
//Spawn a trophy in a random location using a random number between 1-9. Trophy Value
//is used to update the scoreboard
	void randomTrophy(int row,int col) {
	srand(time(NULL));
	int i;
	char const random[] = "123456789";
	char trophy;
	trophy = random[(rand()%9)];
	trophyY = rand() % (row -1 -SCORE_DRAW_DISTANCE) + SCORE_DRAW_DISTANCE + 1;
	trophyX = rand() % (col-1) + 1;
	trophyVal = trophy - '0';
	move(trophyY,trophyX);
	attron(COLOR_PAIR(TROPHY_COLOR));
	addch(trophy);
	attroff(COLOR_PAIR(TROPHY_COLOR));
	refresh();
}

//Author: Kelbin Rodgiuez 
/* Determines if the trophy and snake have collided */ 
void trophyCollision() {
        if((head->x)==trophyX&&(head->y)==trophyY)
        {
                grow_snake(trophyVal);
                randomTrophy(LINES-SCORE_DRAW_DISTANCE,COLS);
        }
}

//Author: Kelbin Rodriguez //Creates a timer between 1-9 seconds that respawns a trophy. 
void trophyTimer(int dummy) {
	srand(time(NULL));
	move(trophyY,trophyX);
	addstr(" ");
	refresh();
	randomTrophy(LINES-SCORE_DRAW_DISTANCE-1,COLS-1);
	int time = (rand()%10)+1;
	alarm(time);
}

/* Starts the game */ //Authors: William Fraher and Kelbin Rodriguez 
void spawn() {
	signal(SIGALRM,trophyTimer);
	int row;
	int col;
	int spawned = 1; //not sure if we need this
	getWindowSize(&row,&col);
	randomTrophy(LINES-SCORE_DRAW_DISTANCE,COLS);

	/* Author: William Fraher. Checks if the head is colliding with the walls, or the snake
		We only need to check if the head collides with objects. The body parts will already not be in a collision, unless the head hits one of them.*/
	void checkcollisions(){
        	if(head->x >= col-1 || head->x <= 1 || head->y >= row-1 || head->y <= SCORE_DRAW_DISTANCE){
			endGame();
        	}
        	else{
            		struct game_object *iterator;
            	iterator = head->next;
            	while(iterator != NULL){
                	if(head->x == iterator->x && head->y == iterator->y){
                    		endGame();
                	}
                	iterator = iterator->next;
            	}
        }
    }

	/* Allocates memory for the snake, its head, and its rudimentary components */
    init_snake();

	/* Starts moving the snake in a random direction */
	int moves[4] = {1,2,3,4};
	srand(time(0));
	dir = moves[rand() % 4]; //currently chooses one of LEFT_MOVE, RIGHT_MOVE, UP_MOVE, DOWN_MOVE. Will change if LEFT_MOVE, RIGHT_MOVE, UP_MOVE, or DOWN_MOVE changes.

	/* Declares the starting x and starting y positions of the snake. */
	head->x = col/2;
	head->y = row/2;
	int timer = (rand()%10)+1;
	alarm(timer);

	/* Prints out the border around the screen */
	/* Left border */
	attron(COLOR_PAIR(BOARDER_COLOR));
	move(0,0);
	for(int r = SCORE_DRAW_DISTANCE; r < row; r++){
		move(r,0);
		addstr("X");
	}
	/* Right border */
	move(0,col);
	for(int r = SCORE_DRAW_DISTANCE; r < row; r++){
		move(r,col-1);
        	addstr("X");
    	}
    	/* Top border */
	move(0,0);
    	for(int i = 0; i < col; i++){
        	move(SCORE_DRAW_DISTANCE,i);
        	addstr("X");
    	}
	/* Bottom border */
	move(row-1,0);
    	for(int i = 0; i < col; i++){
        	move(row,i);
        	addstr("X");
    	}
	attroff(COLOR_PAIR(BOARDER_COLOR));
	int tmp[2] = {-1, 1}; //for making the snake move at random
	int c; // noecho(); // nodelay(stdscr, TRUE); // printf("test"); // system("stty -icanon;");
	/* Main game loop. Currently causes the snake's head to dance around randomly. */
	move(head->y,head->x);
	char buffer[50]; //used for printing out the score
	while(1){
		c = getch();
		usleep(gamespeed);
		switch(c){
			case KEY_UP:
				movement(UP_MOVE);
				break;
			case KEY_DOWN:
				movement(DOWN_MOVE);
				break;
			case KEY_LEFT:
				movement(LEFT_MOVE);
				break;
			case KEY_RIGHT:
				movement(RIGHT_MOVE);
				break;
			default:
				movement(dir);
				break;
		}
		checkcollisions();
		trophyCollision();
		move(SCORE_DRAW_DISTANCE/2,col/2);
		attron(COLOR_PAIR(SCORE_COLOR));
		sprintf(buffer,"Score: %d",score);
		addstr(buffer);
		attroff(COLOR_PAIR(SCORE_COLOR));
		gameWon(); //checks to see if score has reached half the distance of perimetert
		refresh();
	}
}
//Authors: William Fraher and Kelbin Rodriguez 
void main() {
	initscr();
	start_color();
	init_pair(BOARDER_COLOR,COLOR_BLUE,COLOR_BLACK);
	init_pair(TROPHY_COLOR,COLOR_YELLOW,COLOR_BLACK);
	init_pair(SNAKE_COLOR,COLOR_GREEN,COLOR_BLACK);
	init_pair(SCORE_COLOR,COLOR_YELLOW,COLOR_BLACK);
	init_pair(BACKGROUND_COLOR,COLOR_BLACK,COLOR_BLACK); //unused
	curs_set(0);
	nonl();
	cbreak();
	noecho();
	keypad(stdscr,1);
	nodelay(stdscr,1);
	spawn();
	endwin();
}
