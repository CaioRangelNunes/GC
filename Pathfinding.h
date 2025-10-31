#ifndef PATHFINDING_H
#define PATHFINDING_H

#include <vector>
#include <string>
#include <utility>

// Retorna uma lista de pares (col, row) do caminho do start até goal inclusive.
// A grade é um vetor de strings onde 'X' = parede (não navegável), qualquer outro char = navegável.
std::vector<std::pair<int,int>> findPathAStar(const std::vector<std::string>& grid,
                                              int startCol, int startRow,
                                              int goalCol, int goalRow);

#endif // PATHFINDING_H
