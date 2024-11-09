#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

typedef enum {
    WHITE,
    BLACK
} Color;

typedef struct {
    bool isKing;
    Color color;
} Piece;

typedef struct {
    Piece* board[8][8];
} Board;

typedef struct {
    Color color;
    bool isLost;
} Player;

// Clear console screen
void ClearConsole() {
    #if defined(_WIN32)
        system("cls"); // For Windows
    #else
        system("clear"); // For Linux and macOS
    #endif
}

// Initialize the board with pieces in their starting positions
void InitializeBoard(Board* board) {
    // Set all squares to NULL initially
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board->board[i][j] = NULL;
        }
    }

    // Place BLACK pieces on the first three rows, on alternating squares
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 8; j++) {
            if ((i + j) % 2 == 1) {
                board->board[i][j] = (Piece*)malloc(sizeof(Piece));
                board->board[i][j]->color = BLACK;
                board->board[i][j]->isKing = false;
            }
        }
    }

    // Place WHITE pieces on the last three rows, on alternating squares
    for (int i = 5; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if ((i + j) % 2 == 1) {
                board->board[i][j] = (Piece*)malloc(sizeof(Piece));
                board->board[i][j]->color = WHITE;
                board->board[i][j]->isKing = false;
            }
        }
    }
    //checking if the king movement is working!!!
    board->board[5][2]->isKing = true;
    board->board[1][6] =NULL;

}

void ConvertAlgebraicToIndices(const char* position, int* x, int* y) {
    *y = toupper(position[0]) - 'A'; // Column (A-H) to index (0-7)
    *x = 8 - (position[1] - '0'); // Row (1-8) to index (0-7)
}

void PrintBoard(Board* board) {
    ClearConsole();
    printf("     ");
    for (int col = 0; col < 8; col++) {
        printf("  %d    ", col); // Wider column headers
    }
    printf("\n");

    printf("   +");
    for (int col = 0; col < 8; col++) {
        printf("------+");
    }
    printf("\n");

    for (int i = 0; i < 8; i++) {
        printf(" %d |", i); // Row number
        for (int j = 0; j < 8; j++) {
            if (board->board[i][j] == NULL) {
                printf("      |"); // Empty square, wider
            } else if (board->board[i][j]->color == BLACK) {
                if(board->board[i][j]->isKing)
                {
                    printf("  K-B |"); // White piece
                }
                else
                {
                    printf("   B  |"); // Black piece
                }
            } else if (board->board[i][j]->color == WHITE) {
                if(board->board[i][j]->isKing)
                {
                    printf("  K-W |"); // White King piece
                }
                else
                {
                    printf("   W  |"); // White piece
                }
            }
        }
        printf(" %d\n", i); // Row number on the other side

        printf("   +");
        for (int col = 0; col < 8; col++) {
            printf("------+");
        }
        printf("\n");
    }

    printf("     ");
    for (int col = 0; col < 8; col++) {
        printf("  %d    ", col); // Wider column headers
    }
    printf("\n");
}

void FreeBoard(Board* board) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board->board[i][j] != NULL) {
                free(board->board[i][j]);
                board->board[i][j] = NULL;
            }
        }
    }
}

void InitializePlayers(Player *player1, Player *player2) {
    player1->color = WHITE;
    player1->isLost = false;

    player2->color = BLACK;
    player2->isLost = false;
}

bool IsMoveValid(Board* board, int startX, int startY, int endX, int endY) {
    Piece* piece = board->board[startX][startY];
    if (!piece) return false; // No piece at the starting position

    int dx = endX - startX;
    int dy = endY - startY;

    if (!piece->isKing) {
        // Regular piece movement logic here
        int direction = (piece->color == WHITE) ? -1 : 1;

        // Regular move (1 square diagonally)
        if (dx == direction && abs(dy) == 1) {
            return board->board[endX][endY] == NULL; // Only allow move if the end is empty
        }

        // Capture move (2 squares diagonally)
        if (dx == 2 * direction && abs(dy) == 2 &&
            board->board[startX + direction][startY + dy / 2] != NULL &&
            board->board[startX + direction][startY + dy / 2]->color != piece->color) {
            // Capture the opponent's piece
            free(board->board[startX + direction][startY + dy / 2]);
            board->board[startX + direction][startY + dy / 2] = NULL;
            return true;
        }
    } else { // King movement: can move multiple squares diagonally
        if (abs(dx) == abs(dy)) {
            // Check if the path is clear (no pieces in between)
            int stepX = (dx > 0) ? 1 : -1;
            int stepY = (dy > 0) ? 1 : -1;
            int x = startX + stepX;
            int y = startY + stepY;

            while (x != endX && y != endY) {
                if (board->board[x][y] != NULL) {
                    return false; // Path is blocked
                }
                x += stepX;
                y += stepY;
            }

            // If destination has opponent’s piece, allow capture
            if (board->board[endX][endY] != NULL &&
                board->board[endX][endY]->color != piece->color) {
                // Capture opponent’s piece
                free(board->board[endX][endY]);
                board->board[endX][endY] = NULL;
                return true;
            }

            // Move without capture if destination is empty
            return board->board[endX][endY] == NULL;
        }
    }
    return false;
}





void MovePiece(Board* board, int startX, int startY, int endX, int endY) {
    Piece* piece = board->board[startX][startY];

    // Move the piece
    board->board[endX][endY] = piece;
    board->board[startX][startY] = NULL;

    // Check if piece should be promoted to king (reaching the opposite side)
    if ((endX == 0 && piece->color == WHITE) || (endX == 7 && piece->color == BLACK)) {
        piece->isKing = true;
    }
}

void PlayTurn(Board* board, Player* player) {
    int startX, startY, endX, endY;

    printf("Player %s, enter move (startY startX endY endX): ",
           player->color == WHITE ? "WHITE" : "BLACK");
    scanf("%d %d %d %d", &startX, &startY, &endX, &endY);

    if (IsMoveValid(board, startX, startY, endX, endY)) {
        MovePiece(board, startX, startY, endX, endY);
        printf("Move successful!\n");
    } else {
        printf("Invalid move. Try again.\n");
        PlayTurn(board, player); // Retry turn
    }
}

bool IsGameOver(Board* board, Player* player1, Player* player2) {
    bool whiteHasPieces = false, blackHasPieces = false;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board->board[i][j] != NULL) {
                if (board->board[i][j]->color == WHITE) whiteHasPieces = true;
                if (board->board[i][j]->color == BLACK) blackHasPieces = true;
            }
        }
    }
    if (!whiteHasPieces || !blackHasPieces) {
        printf("Game over! %s wins!\n", whiteHasPieces ? "WHITE" : "BLACK");
        return true;
    }
    return false;
}

int main() {
    Board board;
    Player player1 = { WHITE, false };
    Player player2 = { BLACK, false };
    InitializeBoard(&board);
    PrintBoard(&board);

    Player* currentPlayer = &player1;

    while (!IsGameOver(&board, &player1, &player2)) {
        PrintBoard(&board);
        PlayTurn(&board, currentPlayer);

        // Switch players
        currentPlayer = (currentPlayer == &player1) ? &player2 : &player1;
    }

    FreeBoard(&board); // Clean up allocated memory
    return 0;
}
