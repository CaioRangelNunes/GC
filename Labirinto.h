#ifndef LABIRINTO_H
#define LABIRINTO_H

#include <vector>
#include <utility>

class Labirinto {
public:
    static const int LARGURA = 15;
    static const int ALTURA = 10;
    static const float TAMANHO_CELULA;

    Labirinto();
    char getCell(int row, int col) const;
    void setCell(int row, int col, char value);
    std::pair<float, float> getPosicaoCentro(int col, int row) const;
    std::pair<int, int> getStartPosition() const;
    std::pair<int, int> getEndPosition() const;
    bool isWall(int row, int col) const;
    void reset();

private:
    char mapa[ALTURA][LARGURA + 1];
    std::pair<int, int> startPos;
    std::pair<int, int> endPos;
    void initializeDefaultMap();
    void findStartAndEnd();
};

#endif // LABIRINTO_H