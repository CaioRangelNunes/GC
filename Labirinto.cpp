#include "Labirinto.h"
#include <GL/freeglut.h>

const float Labirinto::TAMANHO_CELULA = 48.0f;

Labirinto::Labirinto() {
    initializeDefaultMap();
    findStartAndEnd();
}

void Labirinto::initializeDefaultMap() {
    const char defaultMap[ALTURA][LARGURA + 1] = {
        "XXXXXXXXXXXXXXX",
        "X S X   X     X",
        "X X X X X XXX X",
        "X   X X   X X X",
        "X X XXXXX X X X",
        "X X     X X X X",
        "X XXX X X X X X",
        "X X X X   X   X",
        "X X   XXXXX X F",
        "XXXXXXXXXXXXXXX"
    };

    for (int i = 0; i < ALTURA; i++) {
        for (int j = 0; j < LARGURA + 1; j++) {
            mapa[i][j] = defaultMap[i][j];
        }
    }
}

void Labirinto::findStartAndEnd() {
    for (int y = 0; y < ALTURA; y++) {
        for (int x = 0; x < LARGURA; x++) {
            if (mapa[y][x] == 'S') {
                startPos = {x, y};
            } else if (mapa[y][x] == 'F') {
                endPos = {x, y};
            }
        }
    }
}

char Labirinto::getCell(int row, int col) const {
    if (row < 0 || row >= ALTURA || col < 0 || col >= LARGURA) {
        return 'X';  // Fora dos limites é considerado parede
    }
    return mapa[row][col];
}

void Labirinto::setCell(int row, int col, char value) {
    if (row >= 0 && row < ALTURA && col >= 0 && col < LARGURA) {
        mapa[row][col] = value;
    }
}

std::pair<float, float> Labirinto::getPosicaoCentro(int col, int row) const {
    float x = col * TAMANHO_CELULA + TAMANHO_CELULA / 2;
    float y = (ALTURA - 1 - row) * TAMANHO_CELULA + TAMANHO_CELULA / 2;
    return {x, y};
}

std::pair<int, int> Labirinto::getStartPosition() const {
    return startPos;
}

std::pair<int, int> Labirinto::getEndPosition() const {
    return endPos;
}

bool Labirinto::isWall(int row, int col) const {
    return getCell(row, col) == 'X';
}

void Labirinto::reset() {
    initializeDefaultMap();
    findStartAndEnd();
}