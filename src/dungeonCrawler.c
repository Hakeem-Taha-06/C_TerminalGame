#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>

#define MAX_ROWS 16
#define MAX_COLS 16
#define MAX_ENEMIES 8

struct entity {
    char type;
    int x, y;
};
struct MoveCommand {
    char input;
    int dx, dy;
};
struct enemy {
    char type;
    int x, y;
    struct MoveCommand dir;
    int startX, startY, limitX, limitY;
};

///////////////BFS
struct node{
    int x, y, visited;
    struct node *next, *prev, *parent;
};

struct queue{
    struct node *front;
    struct node *rear;
};

//MoveCommands
const struct MoveCommand up = {'w', 0, -1};
const struct MoveCommand down = {'s', 0, 1};
const struct MoveCommand left = {'a', -1, 0};
const struct MoveCommand right = {'d', 1, 0};
const struct MoveCommand none = {'n', 0, 0};
struct MoveCommand commands[] = {up, down, left, right, none};
int numCommands = sizeof(commands) / sizeof(commands[0]);

//game data (obtained from a level file)
struct entity Player;
struct enemy enemies[MAX_ENEMIES];
int numEnemies = 0;
char tiles[MAX_ROWS][MAX_COLS];
int rows;
int cols;
int currentSwitches;
int maxSwitches;

void levelLoad(char levelName[]);
void levelSave();
void printScreen(char tiles[MAX_ROWS][MAX_COLS], int rows, int cols);
int checkTile(int x, int y);
void updatePosition(struct entity *e, struct MoveCommand move);
void updateEnemyPosition(struct enemy *e, struct MoveCommand move);
void moveEnemy(struct enemy *e);
int movePlayer(struct MoveCommand move);
/////////////BFS
void initQueue(struct queue *q);
void initNodes(struct node n[MAX_ROWS][MAX_COLS]);
int enqueue(struct queue *q, struct node *n);
int dequeue(struct queue *q, int *x, int *y);
int isEmpty(struct queue *q);
void printQueue(struct queue *q);
struct node *BFS(int startX, int startY, int goalX, int goalY);

int main()
{
    char level[100];
    printf("enter level name : ");
    scanf("%s", level);
    levelLoad(level);
    char in;
    int enemyTimer = 0;

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 160 * 1000000L;
    
    
    
    while(1)
    {
        
        //printf("enter a direction (w/a/s/d) or save/load (u/i) : ");
        //scanf(" %c", &in);
        if (_kbhit())
        {
            char in = getch();
            in = tolower(in);
            if (in == 'u')
            {
                levelSave();
            }
            else if (in == 'i')
            {
                levelLoad("save_level.txt");
            }
            else
            {
                //loop to find the right MoveCommand
                for(int i = 0; i < numCommands; i++)
                {
                    if (commands[i].input == in)
                    {
                        //move player in the found direction and check if they died or won, end if either happens
                        if(!movePlayer(commands[i]))
                        {
                        return 0;
                        }
                    }
                    continue;
                }
            }
        }
            nanosleep(&ts, NULL);
            //loop through each enemy and move them after the player moves
            if (enemyTimer == 3)
            {
                for (int i = 0; i < numEnemies; i++)
                {
                    moveEnemy(&enemies[i]);
                }
                enemyTimer = 0;
            }
            printScreen(tiles, rows, cols);
            enemyTimer++;
    }
    
    return 0;
}


void printScreen(char tiles[MAX_ROWS][MAX_COLS], int rows, int cols)
{
    //special line to display the switch count
    for (int j = 0; j < cols; j++)
        {
            printf("%c ", tiles[0][j]);
        }
        printf("Switches : %i/%i", currentSwitches, maxSwitches);
        printf("\n");
    for(int i = 1; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            printf("%c ", tiles[i][j]);
        }
        printf("\n");
    }
}

int checkTile(int x, int y)
{
    char tile = tiles[y][x];
    if (tile != '#')
    {
        if (tile == '+')
        {
            if(currentSwitches == maxSwitches)
            return 1;
            else 
            return 2;
        }
        else if (tile == 'v' || tile == '^' || tile == '<' || tile == '>' || tile == 'X')
        return 3;
        else if (tile == 'o')
        return 4;
        else return 5;
    }
    return 0;
}

void updatePosition(struct entity *e, struct MoveCommand move)
{
    tiles[e->y][e->x] = ' ';
    e->x = e->x + move.dx;
    e->y = e->y + move.dy;
    tiles[e->y][e->x] = e->type;
}

void updateEnemyPosition(struct enemy *e, struct MoveCommand move)
{
    tiles[e->y][e->x] = ' ';
    e->x = e->x + move.dx;
    e->y = e->y + move.dy;
    tiles[e->y][e->x] = e->type;
}

void moveEnemy(struct enemy *e)
{
    if(e->type == 'X')
    {
        struct node *n = BFS(e->x, e->y, Player.x, Player.y);
        //printf("enemy will move to x=%i, y=%i\n", n->x, n->y);
        int nextX = n->x;
        int nextY = n->y;
        if (nextX == Player.x && nextY == Player.y)
        {
            printf("YOU DIED");
            exit(0);
        }
        tiles[e->y][e->x] = ' ';
        e->x = n->x;
        e->y = n->y;
        tiles[e->y][e->x] = e->type;
        
    }
    else
    {
        int nextX = e->x + e->dir.dx;
        int nextY = e->y + e->dir.dy;
        if (nextX == Player.x && nextY == Player.y)
        {
            printf("YOU DIED");
            exit(0);
        }
        if (e->x == e->limitX && e->y == e->limitY)
            {
                tiles[e->y][e->x] = ' ';
                e->x = e->startX;
                e->y = e->startY;
                tiles[e->y][e->x] = e->type;
            } else 
            {
                updateEnemyPosition(e, e->dir);
            }
    }
}

int movePlayer(struct MoveCommand move)
{
    int newX = Player.x + move.dx;
    int newY = Player.y + move.dy;
    int state = checkTile(newX, newY);
    
    switch (state)
    {
        case 1:
            printf("Congrats, you won!!!\n");
            return 0;
        case 2:
            printf("Activate all the switches first.\n");
            break;
        case 3:
            printf("YOU DIED\n");
            return 0;
        case 4:
            currentSwitches++;
            printf("%i switches out of %i activated.\n", currentSwitches, maxSwitches);
            // fallthrough intended
        case 5:
            updatePosition(&Player, move);
            break;
        default:
            break; // wall or invalid move
    }
    return 1;
}

void levelSave()
{
    FILE *save = fopen("save_level.txt", "w");
            
            if (save == NULL) 
            {
                printf("Could not open save file.\n");
                exit(1);
            }
            
            fprintf(save, "MAP %i %i\n", rows, cols);
            
            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols; j++)
                {
                    fprintf(save, "%c", tiles[i][j]);
                }
                fprintf(save, "\n");
            }
            
            fprintf(save, "SWITCH %i %i\n", currentSwitches, maxSwitches);
            fprintf(save, "PLAYER %i %i\n", Player.x, Player.y);
            
            for(int i = 0; i < numEnemies; i++)
            {
                //temporary variables to fill in each enemy with
                char t;
                int x, y, sx, sy, lx, ly;
                char dir[8];
                struct enemy e = enemies[i];
                t = e.type;
                x = e.x;
                y = e.y;
                sx = e.startX;
                sy = e.startY;
                lx = e.limitX;
                ly = e.limitY;
                
                switch (e.dir.input) 
                {
                    case 'w': strcpy(dir, "up"); break;
                    case 's': strcpy(dir, "down"); break;
                    case 'a': strcpy(dir, "left"); break;
                    case 'd': strcpy(dir, "right"); break;
                    default: strcpy(dir, "unknown"); break;
                }
                
                fprintf(save, "ENEMY %c %i %i %s %i %i %i %i\n", t, x, y, dir, sx, sy, lx, ly);
            }
            fclose(save);
            printf("Game saved!\n");
}

void levelLoad(char levelName[])
{
    char line[64];
    numEnemies = 0;
    
    FILE *file = fopen(levelName, "r");
    
    //check if file opened and exit if not
    if (file == NULL)
    {
        printf("file not open");
        exit(1);
    }
    
    
    //loop for reading file
    while(fgets(line, sizeof(line), file))
    {   
        
        if (strncmp(line, "MAP", 3) == 0)
        {
            sscanf(line, "MAP %d %d", &rows, &cols);
            //copy each line from the file into the tiles array
            for (int i = 0; i < rows; i++)
            {
                fgets(line, sizeof(line), file);
                for(int j = 0; j < cols; j++)
                {
                    tiles[i][j]=line[j];    
                }
            }
        } 
        else if (strncmp(line, "SWITCH", 6) == 0)
        {   //get max switches
            sscanf(line, "SWITCH %d %d",&currentSwitches , &maxSwitches);
        }
        else if (strncmp(line, "PLAYER", 6) == 0)
        {   //get player coordinates
            sscanf(line, "PLAYER %d %d", &Player.x, &Player.y);
            Player.type = 'P';//<--- you can change the charachter symbol if you want
        }
        else if (strncmp(line, "ENEMY", 5) == 0)
        {   //temporary variables to fill in each enemy with
            char t;
            int x, y, sx, sy, lx, ly;
            char dir[8];
            //get enemy attributes
            sscanf(line, "ENEMY %c %d %d %s %d %d %d %d", &t, &x, &y, dir, &sx, &sy, &lx, &ly);
            
            //fill them in and increment the number of enemies
            struct enemy *e = &enemies[numEnemies++];
            e->type = t;
            e->x = x;
            e->y = y;
            e->startX = sx;
            e->startY = sy;
            e->limitX = lx;
            e->limitY = ly;
            
            //convert from string to a defined MoveCommand
            if (!strcmp(dir, "up")) 
            {
                e->dir = up;
            }
            else if (!strcmp(dir, "down"))
            {
                e->dir = down;
            }
            else if (!strcmp(dir, "left"))
            {
                e->dir = left;
            }
            else if (!strcmp(dir, "right"))
            {
                e->dir = right;
            }
        }
    }
    fclose(file);
}

////////////////BFS
void initQueue(struct queue *q)
{
    q->front = NULL;
    q->rear = NULL;
}

void initNodes(struct node nodes[MAX_ROWS][MAX_COLS])
{
    for (int i = 0; i < MAX_ROWS; i ++)
    {
        for (int j = 0; j < MAX_COLS; j++)
        {
            nodes[i][j].x = j;
            nodes[i][j].y = i;
            nodes[i][j].visited = 0;
            nodes[i][j].parent = NULL;
            nodes[i][j].next = nodes[i][j].prev = NULL;
        }
    }
}


int enqueue(struct queue *q, struct node *n)
{
    if (q->front == NULL)
    {
        q->front = q->rear = n;
    }
    else
    {
        q->rear->next = n;
        n->prev = q->rear;
        q->rear = n;
    }
    
    //printf("Value Enqueued\n");
    return 0;
}

int dequeue(struct queue *q, int *x, int *y)
{
    if (q->front == NULL)
    {
        printf("Queue is empty");
        return 1;
    }
    
    *x = q->front->x;
    *y = q->front->y;
    
    q->front = q->front->next;
    
    if(q->front == NULL)
    {
        q->rear = NULL;
    }
    else 
    {
        q->front->prev = NULL;
    }
    
    //printf("Front dequeued\n");
    return 0;
}

int isEmpty(struct queue *q)
{
    return (q->front == NULL);
}

void printQueue(struct queue *q)
{
    printf("Queue : {");
    for(struct node *ptr = q->front; ptr != NULL; ptr = ptr->next)
    {
        printf("(%i, %i)", ptr->x, ptr->y);
        if (ptr->next != NULL)
            printf(",");
    }
    printf("}\n");
}

struct node *BFS(int startX, int startY, int goalX, int goalY)
{
    struct node nodes[MAX_ROWS][MAX_COLS];
    initNodes(nodes);
    struct queue frontier;
    initQueue(&frontier);
    enqueue(&frontier, &nodes[startY][startX]);
    nodes[startY][startX].visited = 1;
    
    int dx[] = {-1, 1, 0, 0}; //up, down, left, right
    int dy[] = {0, 0, -1, 1};
    
    while(!isEmpty(&frontier))
    {
        int x, y;
        dequeue(&frontier, &x, &y);
        
        //check if current node is the goal
        if(x == goalX && y == goalY)
        {
            struct node *n = &nodes[y][x];
            for(; n->parent != NULL && !(n->parent->x == startX && n->parent->y == startY); n = n->parent)
            {
                // tiles[n->y][n->x] = '*';
            }
            return n;
        }
        //if not, then create the child nodes and link them to the parent node
        else 
        {
            for(int i = 0; i < 4; i++)
            {
                int nx = x + dx[i];
                int ny = y + dy[i];
                
                if(nx >= 0 && ny >= 0 && nx < MAX_COLS && ny < MAX_ROWS)
                {
                    if(!nodes[ny][nx].visited && tiles[ny][nx] != '#')
                    {
                        enqueue(&frontier, &nodes[ny][nx]);
                        nodes[ny][nx].visited = 1;
                        nodes[ny][nx].parent = &nodes[y][x];
                    }
                }
            }
        }
    }
    return NULL;
}