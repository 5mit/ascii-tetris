#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define WIDTH 10
#define HEIGHT 20
#define BLOCK '#'
#define EMPTY ' '

int grid[HEIGHT][WIDTH];
int currentX, currentY, currentPiece, rotation;
int score = 0;
WINDOW *game, *info;

// Define each piece in its base (0°) orientation with relative positions
typedef struct {
    int shape[4][2];  // 4 blocks with (x, y) coordinates
} Piece;

Piece pieces[7] = {
    // Square
    {{{0, 0}, {1, 0}, {0, 1}, {1, 1}}},
    // Line
    {{{0, 0}, {1, 0}, {2, 0}, {3, 0}}},
    // T shape
    {{{1, 0}, {0, 1}, {1, 1}, {2, 1}}},
    // L shape
    {{{0, 0}, {1, 0}, {2, 0}, {2, 1}}},
    // J shape
    {{{0, 1}, {1, 1}, {2, 1}, {2, 0}}},
    // S shape
    {{{1, 0}, {2, 0}, {0, 1}, {1, 1}}},
    // Z shape
    {{{0, 0}, {1, 0}, {1, 1}, {2, 1}}}
};

void init();
void initGame();
void draw();
void spawnPiece();
int checkCollision(int x, int y, int rotation);
void lockPiece();
void clearLines();
void rotatePiece();
void movePiece(int dx, int dy);
void input();
void logic();
void drawScore();
void drawPiece(int x, int y, int piece, int rotation);
void rotateBlock(int *x, int *y, int rotation);
int kbhit(void);

// Function to detect if a key is hit or not
int kbhit(void)
{
    int ch = getch();
    if (ch != ERR) {
        ungetch(ch);
        return 1;
    } else {
        return 0;
    }
}

int main() {
    init();
    initGame();
    while (1) {
        input();
        logic();
        draw();
        usleep(150000);  // Lower delay for faster movement
    }
    endwin();
    return 0;
}

void init() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);

    keypad(stdscr, true);
    nodelay(stdscr, true);

    srand(time(NULL));
    game = newwin(HEIGHT + 2, WIDTH + 2, 1, 1);
    info = newwin(3, WIDTH + 2, HEIGHT + 3, 1);
    spawnPiece();
}

void initGame() {
    for (int y = 0; y < HEIGHT; y++) 
        for (int x = 0; x < WIDTH; x++)
            grid[y][x] = EMPTY;
}

void shiftGridDown() {
    // Shift every row down by one, starting from the bottom of the grid
    for (int y = HEIGHT - 1; y > 0; y--) {
        for (int x = 0; x < WIDTH; x++) {
            grid[y][x] = grid[y - 1][x];
        }
    }

    // Clear the top row
    for (int x = 0; x < WIDTH; x++) {
        grid[0][x] = EMPTY;
    }
}


void spawnPiece() {
    currentX = WIDTH / 2 - 1;  // Center the piece horizontally
    currentY = -1;  // Start slightly above the visible grid
    currentPiece = rand() % 7;
    rotation = 0;

    // Detect if we have an immediate collision at spawn
    if (checkCollision(currentX, currentY, rotation)) {
        shiftGridDown();
        

    }
}

int checkCollision(int x, int y, int rotation) {
    for (int i = 0; i < 4; i++) {
        int blockX = pieces[currentPiece].shape[i][0];
        int blockY = pieces[currentPiece].shape[i][1];

        // Rotate the block position
        rotateBlock(&blockX, &blockY, rotation);

        int newX = x + blockX;
        int newY = y + blockY;

        // Check boundaries and grid for collision
        if (newX < 0 || newX >= WIDTH || newY >= HEIGHT || (newY >= 0 && grid[newY][newX] != EMPTY)) {
            return 1;
        }
    }
    return 0;
}

void rotateBlock(int *x, int *y, int rotation) {
    int temp;
    for (int i = 0; i < rotation; i++) {
        temp = *x;
        *x = -*y;
        *y = temp;
    }
}

void lockPiece() {
    for (int i = 0; i < 4; i++) {
        int blockX = pieces[currentPiece].shape[i][0];
        int blockY = pieces[currentPiece].shape[i][1];
        rotateBlock(&blockX, &blockY, rotation);
        grid[currentY + blockY][currentX + blockX] = BLOCK;
        if (currentY + blockY < 0) {
            endwin();
            printf("Game Over! No space for new piece. Final Score: %d\n", score);
            exit(0);
        }
    }

    clearLines();
    spawnPiece();
}

void clearLines() {
    for (int i = HEIGHT - 1; i >= 0; i--) {
        int fullLine = 1;
        for (int j = 0; j < WIDTH; j++) {
            if (grid[i][j] == EMPTY) {
                fullLine = 0;
                break;
            }
        }
        if (fullLine) {
            for (int k = i; k > 0; k--)
                for (int l = 0; l < WIDTH; l++)
                    grid[k][l] = grid[k - 1][l];
            for (int l = 0; l < WIDTH; l++)
                grid[0][l] = EMPTY;
            score += 10;
            i++;
        }
    }
}

void rotatePiece() {
    int newRotation = (rotation + 1) % 4;
    if (!checkCollision(currentX, currentY, newRotation))
        rotation = newRotation;
}

void movePiece(int dx, int dy) {
    if (!checkCollision(currentX + dx, currentY + dy, rotation)) {
        currentX += dx;
        currentY += dy;
    } else if (dy == 1) {
        lockPiece();
    }
}

void input() {
    if (kbhit()) {
        int ch = getch();
        switch (ch) {
            case 'a': movePiece(-1, 0); break;
            case 'd': movePiece(1, 0); break;
            case 's': movePiece(0, 1); break;
            case 'w': rotatePiece(); break;
            case 'q': endwin(); exit(0);
        }
    }
    flushinp();
}

void logic() {
    movePiece(0, 1);  // Move piece down automatically
}

void drawPiece(int x, int y, int piece, int rotation) {
    for (int i = 0; i < 4; i++) {
        int blockX = pieces[piece].shape[i][0];
        int blockY = pieces[piece].shape[i][1];
        rotateBlock(&blockX, &blockY, rotation);
        if (y + blockY >= 0)
            mvwaddch(game, y + blockY + 1, x + blockX + 1, BLOCK);
    }
}

void draw() {
    wclear(game);
    box(game, 0, 0);

    // Draw the grid
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            mvwaddch(game, y + 1, x + 1, grid[y][x]);
        }
    }

    // Draw the current piece
    drawPiece(currentX, currentY, currentPiece, rotation);

    wrefresh(game);
    drawScore();
}

void drawScore() {
    wclear(info);
    mvwprintw(info, 1, 1, "Score: %d", score);
    wrefresh(info);
}
