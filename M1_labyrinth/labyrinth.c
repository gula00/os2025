#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <testkit.h>
#include "labyrinth.h"

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printUsage();
        return 1;
    }

    char *mapFile = NULL;
    char playerId = '\0';
    char *moveDirection = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--version") == 0) {
            if (argc > 2) {
                return 1;
            }
            printf("%s\n", VERSION_INFO);
            return 0;
        } else if (strcmp(argv[i], "--map") == 0 || strcmp(argv[i], "-m") == 0) {
            if (i + 1 >= argc) {
                return 1;
            }
            mapFile = argv[++i];
        } else if (strcmp(argv[i], "--player") == 0 || strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc) {
                return 1;
            }
            char *playerStr = argv[++i];
            if (strlen(playerStr) != 1 || !isValidPlayer(playerStr[0])) {
                return 1;
            }
            playerId = playerStr[0];
        } else if (strcmp(argv[i], "--move") == 0) {
            if (i + 1 >= argc) {
                return 1;
            }
            moveDirection = argv[++i];
            if (strcmp(moveDirection, "up") != 0 &&
                strcmp(moveDirection, "down") != 0 &&
                strcmp(moveDirection, "left") != 0 &&
                strcmp(moveDirection, "right") != 0) {
                return 1;
            }
        } else {
            return 1;
        }
    }

    if (mapFile == NULL || playerId == '\0') {
        return 1;
    }

    Labyrinth labyrinth;
    if (!loadMap(&labyrinth, mapFile)) {
        return 1;
    }

    Position playerPos = findPlayer(&labyrinth, playerId);
    if (playerPos.row == -1) {
        Position emptyPos = findFirstEmptySpace(&labyrinth);
        if (emptyPos.row == -1) {
            return 1;
        }
        labyrinth.map[emptyPos.row][emptyPos.col] = playerId;
        playerPos = emptyPos;
    }

    // If move direction is specified, move the player
    if (moveDirection != NULL) {
        if (!movePlayer(&labyrinth, playerId, moveDirection)) {
            return 1;
        }
    }

    if (!saveMap(&labyrinth, mapFile)) {
        return 1;
    }

    for (int i = 0; i < labyrinth.rows; i++) {
        for (int j = 0; j < labyrinth.cols; j++) {
            printf("%c", labyrinth.map[i][j]);
        }
        printf("\n");
    }

    return 0;
}

void printUsage() {
    printf("Usage:\n");
    printf("  labyrinth --map map.txt --player id\n");
    printf("  labyrinth -m map.txt -p id\n");
    printf("  labyrinth --map map.txt --player id --move direction\n");
    printf("  labyrinth --version\n");
}

bool isValidPlayer(char playerId) {
    return playerId >= '0' && playerId <= '9';
}

bool loadMap(Labyrinth *labyrinth, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return false;
    }

    labyrinth->rows = 0;
    labyrinth->cols = 0;

    // +2 FOR '\n' and '\0'
    char line[MAX_COLS +2];
    while (fgets(line, sizeof(line), file) && labyrinth->rows < MAX_ROWS) {
        int len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }

        if (len > MAX_COLS) {
            fclose(file);
            return false;
        }

        if (labyrinth->rows == 0) {
            labyrinth->cols = len; // init
        } else if (len != labyrinth->cols) {
            // All rows should have the same length
            fclose(file);
            return false;
        }

        strcpy(labyrinth->map[labyrinth->rows], line);
        labyrinth->rows++;
    }

    fclose(file);
    return labyrinth->rows > 0 && labyrinth->cols > 0;
}

Position findPlayer(Labyrinth *labyrinth, char playerId) {
    Position pos = {-1, -1};
    for (int i = 0; i < labyrinth->rows; i++) {
        for (int j = 0; j < labyrinth->cols; j++) {
            if (labyrinth->map[i][j] == playerId) {
                pos.row = i;
                pos.col = j;
                return pos;
            }
        }
    }
    return pos;
}

Position findFirstEmptySpace(Labyrinth *labyrinth) {
    Position pos = {-1, -1};
    for (int i = 0; i < labyrinth->rows; i++) {
        for (int j = 0; j < labyrinth->cols; j++) {
            if (isEmptySpace(labyrinth, i, j)) {
                pos.row = i;
                pos.col = j;
                return pos;
            }
        }
    }
    return pos;
}

bool isEmptySpace(Labyrinth *labyrinth, int row, int col) {
    if (row < 0 || row >= labyrinth->rows || col < 0 || col >= labyrinth->cols) {
        return false;
    }
    char cell = labyrinth->map[row][col];
    return cell == '.' || cell == ' ';
}

bool movePlayer(Labyrinth *labyrinth, char playerId, const char *direction) {
    Position currentPos = findPlayer(labyrinth, playerId);
    if (currentPos.row == -1) return false;
    
    int newRow = currentPos.row;
    int newCol = currentPos.col;
    if (strcmp(direction, "up") == 0) {
        newRow--;
    } else if (strcmp(direction, "down") == 0) {
        newRow++;
    }  else if (strcmp(direction, "left") == 0) {
        newCol--;
    }  else if (strcmp(direction, "right") == 0) {
        newCol++;
    } else {
        return false;
    }
    
    if (!isEmptySpace(labyrinth, newRow, newCol)) {
        return false;
    }

    labyrinth->map[currentPos.row][currentPos.col] = '.';
    labyrinth->map[newRow][newCol] = playerId;

    return true;
}

bool saveMap(Labyrinth *labyrinth, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        return false;
    }
    for (int i = 0; i < labyrinth->rows; i++) {
        fprintf(file, "%s\n", labyrinth->map[i]);
    }
    fclose(file);
    return true;
}

// Check if all empty spaces are connected using DFS
void dfs(Labyrinth *labyrinth, int row, int col, bool visited[MAX_ROWS][MAX_COLS]) {
    if (row < 0 || row >= labyrinth->rows || col < 0 || col >= labyrinth->cols) {
        return;
    }
    if (visited[row][col] || !isEmptySpace(labyrinth, row, col)) {
        return;
    }
    visited[row][col] = true;
    
    dfs(labyrinth, row - 1, col, visited);
    dfs(labyrinth, row + 1, col, visited);
    dfs(labyrinth, row, col - 1, visited);
    dfs(labyrinth, row, col + 1, visited);
}

bool isConnected(Labyrinth *labyrinth) {
    bool visited[MAX_ROWS][MAX_COLS];

    for (int i = 0; i < MAX_ROWS; i++) {
        for (int j = 0; j < MAX_COLS; j++) {
            visited[i][j] = false;
        }
    }
    Position firstEmpty = findFirstEmptySpace(labyrinth);
    if (firstEmpty.row == -1) {
        return true;
    }
    dfs(labyrinth, firstEmpty.row, firstEmpty.col, visited);
    for (int i = 0; i < labyrinth->rows; i++) {
        for (int j = 0; j < labyrinth->cols; j++) {
            if (isEmptySpace(labyrinth, i, j) && !visited[i][j]) {
                return false;
            }
        }
    }
    return true;
}
