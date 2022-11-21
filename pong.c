#include <ncurses.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

/* objects and etc */
char playmsg[] = "Hit enter to play Pong!";
char exitmsg[] = "Press any other key to exit...";

typedef struct {
    int cur_x, cur_y; 
} OBJECT;

typedef enum {
    NONE,
    UP,
    DOWN,
    BOUNCE,
    LEFT,
    RIGHT,
} collision_type;

typedef enum {
    WHITE = 1,
    FONT_BLACK
} colors;

/* evil global variables */
WINDOW *game_field;
WINDOW *status_line;

OBJECT *player_one, *player_two;
OBJECT *ball;

int gravity = 0;
int velocity;

int score_one = 0;
int score_two = 0;

/* status line functions */
void update_score()
{
    wattron(status_line, COLOR_PAIR(FONT_BLACK));
    mvwprintw(status_line, 0, COLS/2, "%d       %d", score_one, score_two);
    wattroff(status_line, COLOR_PAIR(FONT_BLACK));
}

/* init functions */
void init_object(OBJECT *obj, int cur_y, int cur_x)
{
    obj->cur_y = cur_y;
    obj->cur_x = cur_x;
}

void init_game_objects()
{
    init_object(player_one, LINES/2, 1);
    init_object(player_two, LINES/2, COLS - 2);
    init_object(ball, LINES/2, COLS/2);
}

void init_status_line()
{
    int i;
    status_line = newwin(1, COLS, LINES - 1, 0);
    wrefresh(status_line);
    init_pair(WHITE, COLOR_RED, COLOR_WHITE);
    init_pair(FONT_BLACK, COLOR_BLACK, COLOR_WHITE);
    for (i = 0; i < COLS - 1; i++) { 
        wattron(status_line, COLOR_PAIR(WHITE));
        mvwaddch(status_line, 0, i, ' ');
    }
    wattroff(status_line, COLOR_PAIR(WHITE));
    update_score();
}

void init_pong()
{
    initscr();
    start_color();
    noecho();
    curs_set(false);
    refresh();

    /* player_one       ball           player_two */
    player_one = malloc(sizeof(OBJECT));
    player_two = malloc(sizeof(OBJECT));
    ball = malloc(sizeof(OBJECT));
    init_game_objects(); 
    
    srand(time(NULL));
    velocity = (rand() % 2 == 0 ? 1 : -1); 
    /* Game field is window where all stuff happens */
    game_field = newwin(LINES - 2, COLS, 0, 0);
    keypad(game_field, true);

    init_status_line();
}

/* object functions */
void inverse_velocity()
{
    if (velocity == -1)
        velocity = 1;
    else 
        velocity = -1;
}

void bounce()
{
    if (ball->cur_y - 1 < LINES/2) {
        gravity = 1;
        inverse_velocity();
    }
    if (ball->cur_y - 1 > LINES/2) {
        gravity = -1;
        inverse_velocity();
    }
}

void draw_player(const OBJECT obj)
{
    int i = obj.cur_y - 1;
    for (; i < obj.cur_y + 1; i++) {
        mvwaddch(game_field, i, obj.cur_x, ACS_CKBOARD);
    }
}

void hide_player(const OBJECT obj)
{
    int i = obj.cur_y - 1;
    for (; i < obj.cur_y + 1; i++) {
        mvwaddch(game_field, i, obj.cur_x, ' ');
    }
}

void move_player(OBJECT *obj, const int ay, const int ax)
{
    hide_player(*obj);
    obj->cur_y += ay;
    obj->cur_x += ax;
    draw_player(*obj);
}

void move_ball(const int ay, const int ax) 
{
    mvwaddch(game_field, ball->cur_y, ball->cur_x, ' ');
    ball->cur_y += ay;
    ball->cur_x += ax;
    mvwaddch(game_field, ball->cur_y, ball->cur_x, ACS_DIAMOND);
}

void draw_objects()
{
    draw_player(*player_one);
    draw_player(*player_two);
    mvwaddch(game_field, ball->cur_y, ball->cur_x, ACS_DIAMOND);
}

void reset_objects()
{
    wclear(game_field);
    init_game_objects();
    draw_objects();
    wrefresh(game_field);
    velocity = -1;
    gravity = 0;
}

/* collision checks */
collision_type check_player_collision(OBJECT *obj)
{
    if (obj->cur_y == LINES - 1)
        return DOWN;
    if (obj->cur_y == 0) 
        return UP;
    else 
        return NONE;
}

bool check_bp_collision(OBJECT *obj)
{
    /* check collison with players
     *             | 
     *  ball --> * | <-- player
     *             |
     *  To check properly, we need to check three coordinates, because player have three parts */
    int check_x = obj->cur_x - 1;
    if (check_x == 0)
        check_x += 2;
    if ((ball->cur_x == check_x) && (ball->cur_y == obj->cur_y - 1))
        return true;
    if ((ball->cur_x == check_x) && (ball->cur_y == obj->cur_y))
        return true;
    if ((ball->cur_x == check_x) && (ball->cur_y == obj->cur_y + 1))
        return true;
    else 
        return false;
}

collision_type check_ball_collision()
{
    if (check_bp_collision(player_one))
        return BOUNCE;
    if (check_bp_collision(player_two))
        return BOUNCE;
    if (ball->cur_x <= 0)
        return LEFT;
    if (ball->cur_x >= COLS)
        return RIGHT;
    if (ball->cur_y >= getmaxy(game_field))
        return DOWN;
    if (ball->cur_y <= 0) 
        return UP;
    return NONE;
}

void check_objects_collision()
{
    collision_type ball_collision;
    /* Firstly we check player collision */
    if (check_player_collision(player_one) == UP) 
        move_player(player_one, 1, 0);
    if (check_player_collision(player_one) == DOWN) 
        move_player(player_one, -1, 0);
    if (check_player_collision(player_two) == UP) 
        move_player(player_two, 1, 0);
    if (check_player_collision(player_two) == DOWN) 
        move_player(player_two, -1, 0);

    /* Then we check ball collision */
    ball_collision = check_ball_collision();
    switch(ball_collision) {
    case BOUNCE:
        bounce();
        break;
    case LEFT:
        score_two++;
        update_score();
        reset_objects();
        break;
    case RIGHT:
        score_one++;
        update_score();
        reset_objects();
        break;
    case UP:
        gravity = 1;
        break;
    case DOWN:
        gravity = -1;
        break;
    default:
        ;
    }
}

/* main loop functions */
void procces_events()
{
    int ch;
    ch = wgetch(game_field);
    switch(ch) {
    case ERR:
        return;
        break;
    case 'W':
    case 'w':
        move_player(player_one, -1, 0);
        break;
    case 'S':
    case 's':
        move_player(player_one, 1, 0);
        break;
    case KEY_UP:
        move_player(player_two, -1, 0);
        break;
    case KEY_DOWN:
        move_player(player_two, 1, 0);
        break;
    }
}

void update()
{
    check_objects_collision();
    move_ball(gravity, velocity);
    wrefresh(game_field);
    wrefresh(status_line);
}

void run()
{
    halfdelay(1);
    wclear(game_field);
    draw_objects();
    while (true) {
        procces_events();
        update();
    }
}

void exit_pong()
{
    endwin();
    exit(0);
}

int main (int argc, char *argv[])
{
    int ch;
    init_pong();
    atexit(*exit_pong);
    mvwaddstr(game_field, LINES/2, (COLS - sizeof(exitmsg))/2, playmsg);
    mvwaddstr(game_field, LINES/2 + 1, (COLS - sizeof(exitmsg))/2, exitmsg);
    wrefresh(game_field);
    
    ch = wgetch(game_field);
    if (ch == '\n')
        run();
    return 0;
}
