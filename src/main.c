#define WIDTH 100
#define HEIGHT 40
#include<ncurses.h>
#include<math.h>
#include<stdlib.h>

char canvas[HEIGHT][WIDTH];
void canvas_draw(void); 
int cursor_x = 0;
int cursor_y = 0;
int rect_mode = 0;
int circ_mode = 0;
int start_x = 0;
int start_y = 0;
enum Tool {
    TOOL_PEN,
    TOOL_RECT,
    TOOL_LINE,
    TOOL_CIRCLE,
    TOOL_ERASER,
    TOOL_TRI,

};

typedef struct {
    int active;
    int startx;
    int starty;
    int curtool;
}PendingShape;

PendingShape pend = {0};


char* tool_name(enum Tool tool ){
    switch(tool){
        case(TOOL_PEN):
            return "Pencil";
            break;
        case(TOOL_RECT):
            return "Rectangle";
            break;
        case(TOOL_LINE):
            return "Line";
            break;
        case(TOOL_CIRCLE):
            return "Circle";
            break;
        case(TOOL_ERASER):
            return "Eraser";
            break;
        case(TOOL_TRI):
            return "Triangle";
            break;
    }

}
enum Tool cur_tool = TOOL_PEN;

void draw_rect(int x0 , int y0 , int x1 , int y1){
    int left = x0<x1? x0: x1;
    int right = x1>x0 ? x1:x0;
    int top = y0<y1? y0:y1;
    int bottom = y1>y0 ? y1: y0;
    for(int x = left ; x<=right;x++){
        if(top >= 0 && top<HEIGHT && x>=0 && x<WIDTH){
            canvas[top][x] = '*';

        }
        if(bottom >= 0 && bottom<HEIGHT && x>=0 && x<WIDTH){
            canvas[bottom][x] = '*';
        }

    }
    for(int y = top ;y<=bottom;y++){
        if(y >=0 && y<HEIGHT){
            if(left>=0 &&left<WIDTH) canvas[y][left] = '*';
            if(right>=0 && right<WIDTH) canvas[y][right] = '*';
        }

    }
}
void circ_draw(int startx , int starty , int endx , int endy){
    int dx = endx-startx;
    int dy = endy - start_y;

    float r = sqrt(dx*dx + dy*dy);

    for(int i = starty-r;i<=starty+r;i++){
        for(int j = startx-r;j<=endx+r;j++){
            dx = i-starty;
            dy = j-startx;

            if (dx*dx + dy*dy <= r) canvas[i][j] = '*';

        }
    }
}

void draw_line(int x0 , int y0 , int x1 , int y1 ){
    int dx = abs(x1-x0);
    int dy = abs(y1-y0);

    int err = dx-dy; //you get the intial error between the coordinates of your source and destination

    int sx = (x0<x1)? 1 : -1; //move one step forward if x1 lies ahead of x0 , else one step backwarrd
    int sy = (y0<y1) ? 1 : -1;// same as backward

    while(1){
        if (x0 >= 0 && x0 < WIDTH && y0 >= 0 && y0<HEIGHT){
            canvas[y0][x0] = '*';
        }
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err; // only deal with integers

        if (e2 > -dy){ // checking to see if the error accumulated is small enough in the negative y direction , that moving in x , will not effect the line
            x0 += sx; // we move in x , if the error accumulated in the negative y direction isn't too small.
            err -= dy; //moving one step in x , puts us below the actual line that you need , therefore we increase the error in the negative x direction , so we can move correctly later on
            
        }
        if(e2 < dx){ // checking to see , if the error accumulated in the x direction is small enough that we can move upwards
            y0 += sy; //if the error accumulated is lesser in the x direction , we move in y , which gains us x coordinates
            err += dx; // Since moving 1 step in y , we are above the actual line that we need to be on , we increase the amount of error in the x direction , to force us to draw correctly later on.
        }


    }

}


int main(){
    
    
    for(int i = 0;i<HEIGHT;i++){
        for(int j = 0;j<WIDTH;j++){
            canvas[i][j] = '.';
        }
    }

    initscr();
    curs_set(0);

    if(!has_colors()){
        endwin();
        printf("Terminal does not support coloring\n");
        return 1;
    }

    start_color();
    init_pair(1 , COLOR_GREEN , COLOR_BLUE);



    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    MEVENT mouse_ev;


    refresh();

    int running = 1;
    mousemask(ALL_MOUSE_EVENTS , NULL);
    while (running)
    {
        clear();
        canvas_draw();

        attron(A_REVERSE);
        mvaddch(cursor_y , cursor_x , canvas[cursor_y][cursor_x]);
        attroff(A_REVERSE);

        

        mvprintw(HEIGHT+1 , 0 , "Current Tool: %s" , tool_name(cur_tool));
    
        mvprintw(HEIGHT+3, 0 , "Press 'l' for line tool\nPress 'r' for Rectangle tool\nPress 'c' for Circle tool\nPress 't' for Triangle\nPress 'e' for Eraser tool");
        refresh();
        int ch = getch();

      

        switch(ch){
            case KEY_UP:
                cursor_y--;
                break;
            case KEY_DOWN:
                cursor_y++;
                break;
            case KEY_LEFT:
                cursor_x--;
                break;
            case KEY_RIGHT:
                cursor_x++;
                break;
            case 'q':
                running = 0;
                break;
            case 'p':
                cur_tool = TOOL_PEN;
                break;
            case 'r':
                cur_tool = TOOL_RECT;
                break;
            case 'l':
                cur_tool = TOOL_LINE;
                break;
            case 'c':
                cur_tool = TOOL_CIRCLE;
                break;
            case 'e':
                cur_tool = TOOL_ERASER;
                break;
            case 't':
                cur_tool = TOOL_TRI;
                break;
            case KEY_MOUSE:
                if(getmouse(&mouse_ev) == OK){
                    mvprintw(0 , WIDTH+3 , "Mouse event: %lu" , mouse_ev.bstate);
                    if (mouse_ev.bstate & BUTTON1_PRESSED) canvas[mouse_ev.y][mouse_ev.x] = '*';

                }
                break;

            case ' ':
            {
             
                if(cur_tool == TOOL_PEN){
                    canvas[cursor_y][cursor_x] = '*';

                }
                else if(cur_tool == TOOL_ERASER){
                    canvas[cursor_y][cursor_x] = '.';
                }
                else{
                    if(pend.active == 0){
                        pend.startx = cursor_x;
                        pend.starty = cursor_y;
                        pend.curtool = cur_tool;
                        pend.active = 1;
                        canvas[cursor_y][cursor_x] = '*';
                    }
                    else{
                        switch(pend.curtool){
                            case TOOL_LINE:
                                draw_line(pend.startx , pend.starty , cursor_x , cursor_y);
                                break;
                            case TOOL_RECT:
                                draw_rect(pend.startx , pend.starty , cursor_x , cursor_y);
                                break;
                            case TOOL_CIRCLE:
                                circ_draw(pend.startx , pend.starty , cursor_x , cursor_y);
                          
                            
                        }
                        pend.active = 0;
                    }
                }
            }
            
                

        }
        if(cursor_x<0) cursor_x = 0;
        if(cursor_x >= WIDTH) cursor_x = WIDTH-1;
        if(cursor_y >= HEIGHT) cursor_y = HEIGHT-1;
        if(cursor_y<0) cursor_y = 0;


        
    }
    
    endwin();
    return 0;

}

void canvas_draw(){
    for(int i = 0;i<HEIGHT;i++){
        for(int j = 0;j<WIDTH;j++){
            if(canvas[i][j] == '*')
            {
                attron(COLOR_PAIR(1));
                mvaddch(i , j ,'*');
                attroff(COLOR_PAIR(1));
            }
            else{
                mvaddch(i , j ,'.');
            }
            
        }
    }
}
