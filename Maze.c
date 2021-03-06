#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    int rows, columns;
} Matrix;

bool read_input(char * file_dest, Matrix * size, Matrix * entry, Matrix * exit, char *** maze);
void find_path(char ** maze, Matrix size, Matrix entry, Matrix exit);
void display_maze(char ** maze, Matrix size, Matrix curr);
void go_forward(Matrix * curr, int dir);
int turn_right(int dir);
int turn_left(int dir);
bool is_wall_next_to(char ** maze, Matrix size, Matrix * pos, int dir);
bool is_wall(char ** maze, Matrix size, Matrix pos);
bool is_in_maze(Matrix size, Matrix pos);


/* client */
int main(int argc, char ** argv) {
    char ** maze;
    Matrix size;
    Matrix entry;
    Matrix exit;
    int i;
    
    /* if no filename was entered, abort the program. */
    if (argc < 2) {
        printf("No maze name was entered. \n");
        return 1;
    }
    
    if (!read_input(argv[1], &size, &entry, &exit, &maze)) {
        printf("The file you tried to open, \"%s\" doesn't exist.\n", argv[1]);
        return 1;
    }
    
    printf("The unsolved maze: \n");
    printf("The starting position is (%d,%d) and the end is (%d,%d), from the upper left.\n", entry.rows,  entry.columns,  exit.rows, exit.columns);
    
    display_maze(maze, size, exit); /* display the maze before it is solved */
    find_path(maze, size, entry, exit); /* solve the maze */
    
    printf("The solved maze:\n");
    printf("The starting position is (%d,%d) and the end is (%d,%d), from the upper left.\n", entry.rows,  entry.columns,  exit.rows, exit.columns);
    display_maze(maze, size, exit); /* display the maze after it is solved */
    
    /* deallocate the memory of the maze */
    for (i = 0; i < size.rows; i++)
        free(maze[i]);
    free(maze);
    return 0;
}



/* read in the maze file and extract all information */
bool read_input(char * file_dest, Matrix * size, Matrix * entry, Matrix * exit, char *** maze) {
    FILE * file;
    char * next_line;
    int i, j, SIZE = 500;
    
    if ((file = fopen(file_dest, "r")) == NULL)
        return false;
    else {
        next_line = malloc(sizeof (char) * SIZE);
        fgets(next_line, SIZE, file); /* get maze's size */
        sscanf(next_line, "%d,%d", &(size->columns), &(size->rows));
        fgets(next_line, SIZE, file); /* get maze's entry */
        sscanf(next_line, "%d,%d", &(entry->columns), &(entry->rows));
        fgets(next_line, SIZE, file); /* get maze's exit */
        sscanf(next_line, "%d,%d", &(exit->columns), &(exit->rows));
        
        /* allocate the memory for the given maze */
        *maze = malloc(sizeof (char *) * size->rows);
        for (i = 0; i < size->rows; ++i)
            (*maze)[i] = malloc(sizeof (char) * size->columns);
        
        /* copy the maze from the file to the 2d-array */
        for (i = 0; i < size->rows; ++i) {
            for (j = 0; j < size->columns; ++j)
                (*maze)[i][j] = fgetc(file);
            fgetc(file);
        }
        fclose(file);
    }
	free(next_line);
    return true;
}



/* directions are stored as an int, with the convention 0 = north, 1 = east, 2 = south, 3 = west */
void find_path(char ** maze, Matrix size, Matrix entry, Matrix exit) {
    Matrix current; /* the current position of the walker within the maze */
    int dir = 2; /* south */
    
    current.rows = entry.rows;
    current.columns = entry.columns;
    
    while (!((current.rows == exit.rows) && (current.columns == exit.columns))) {
        maze[current.rows][current.columns] = 'W';
        if (!is_wall_next_to(maze, size, &current, turn_right(dir))) { /* no wall to the right */
            dir = turn_right(dir);
            go_forward(&current, dir);
        } else if (!is_wall_next_to(maze, size, &current, dir)) { /* no wall ahead */
            go_forward(&current, dir);
        } else if (!is_wall_next_to(maze, size, &current, turn_left(dir))) { /* no wall to the left */
            dir = turn_left(dir);
            go_forward(&current, dir);
        } else { /* no wall behind, turn 180 */
            dir = turn_left(dir);
            dir = turn_left(dir);
            go_forward(&current, dir);
        }
    }
    maze[current.rows][current.columns] = 'W';
}



/* Display the maze */
void display_maze(char ** maze, Matrix size, Matrix curr) {
    int i, j;
    for (i = 0; i < size.rows; ++i) {
        for (j = 0; j < size.columns; ++j)
            printf("%c", maze[i][j]);
        printf("\n");
    }
}



/* 
   take in a direction and a position, and move the position by one in that direction.
   for example, if the current position is (3,4) and the direction is south, the new position will be (3,5)
   make sure to always call is_in_maze() on the target postion to ensure you don't access memory outside of the array 
*/
void go_forward(Matrix * curr, int dir) {
    switch (dir) {
        case 0: /* north */
            --curr->rows;
            break;
        case 1: /* east */
            ++curr->columns;
            break;
        case 2: /* south */
            ++curr->rows;
            break;
        case 3: /* west */
            --curr->columns;
            break;
        default:
            break;
    }
}



/*
   increment the direction by 1 to turn clockwise or right.
   if the current direction is 3 or west, go back to 0 or north
*/
int turn_right(int dir) {
    return (dir + 1) % 4;
}



/*
   decrement the direction by 1 to turn counterclockwise or left.
   if the current direction is 0 or north, go back to 3 or west
*/
int turn_left(int dir) {
    return (dir - 1 + 4) % 4; /* add 4 to avoid % on a negative number */
}



/*
   returns true if ther is a wall one square ahead in the chosen direction.
   safe to call on target postions that might be outside of the maze.
*/
bool is_wall_next_to(char ** maze, Matrix size, Matrix * pos, int dir) {
    Matrix target;
    target.rows = pos->rows;
    target.columns = pos->columns;
    
    go_forward(&target, dir); /* go_forward is used on the temporary variable to get its neighbor*/
    return is_wall(maze, size, target);
    
}



/* 
   return true if the target position is outside the maze or is a wall.
   return false if the target position is not a wall.
   safe to use on coordinates outside of the maze. 
*/
bool is_wall(char ** maze, Matrix size, Matrix pos) {
    if (!is_in_maze(size, pos))
        return true;
    return (maze[pos.rows][pos.columns] == 'X');
}


/* 
   return false if the target position is outside the maze. otherwise return true.
   safe to use on coordinates outside of the maze. 
*/
bool is_in_maze(Matrix size, Matrix pos) {
    if (pos.rows < 0)
        return false;
    if (pos.rows >= size.rows)
        return false;
    if (pos.columns < 0)
        return false;
    if (pos.columns >= size.columns)
        return false;
    return true;
}
