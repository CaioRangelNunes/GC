#include "Fase2.h"
#include <vector>
#include <utility>
#include <iostream>
#include <cstdlib>
#include <ctime>

// Forward declarations of globals and functions defined in Jogo.cpp
#include "Jogador.h"
#include "Pathfinding.h"
#include <iostream>

// The main game defines these; use local constants here to match the game config
constexpr int LOCAL_LARGURA = 15;
constexpr int LOCAL_ALTURA = 10;
constexpr float LOCAL_TAMANHO_CELULA = 48.0f;
extern char mapaLabirinto[/*ALTURA*/10][/*LARGURA+1*/16];
extern std::vector<std::pair<float,float>> rastroVerde;
extern std::vector<std::pair<int,int>> pathCells;
extern std::vector<std::pair<float,float>> hintRastroVerde;
extern std::vector<std::pair<int,int>> hintPathCells;
extern Jogador jogador;
extern int inicioCol, inicioRow, fimCol, fimRow;
extern float inicioLabX, inicioLabY, fimLabX, fimLabY;
extern int botaoCol, botaoRow;
extern bool botaoAtivado;
extern std::vector<std::pair<int,int>> portasParaAbrir;

// Functions defined in Jogo.cpp that we call
extern void gerarRastroVerde(bool fromCurrentPosition = false);
extern void inicializarFaseEspinhos(const std::vector<std::pair<int,int>>& forbidden = {});
extern std::pair<float,float> getPosicaoCentro(int col, int row);

void Fase2::carregar() {
    // Define mapa da fase 2 (mais difícil)
    const char* mapa2[LOCAL_ALTURA] = {
        "XXXXXXXXXXXXXXX",
        "X S X X   X   X",
        "X X X X X XXX X",
        "X X   X   X X X",
        "X XXX XXXXX X X",
        "X   X     X   X",
        "X X XXX X X X X",
        "X X X   X X X X",
        "X   X XXX   X F",
        "XXXXXXXXXXXXXXX",
    };

    for (int r = 0; r < LOCAL_ALTURA; ++r) {
        for (int c = 0; c < LOCAL_LARGURA; ++c) {
            mapaLabirinto[r][c] = mapa2[r][c];
        }
    }

    // Atualiza inicio/fim e jogador (similar ao que main_proxy fazia)
    inicioCol = inicioRow = fimCol = fimRow = -1;
    for (int y = 0; y < LOCAL_ALTURA; ++y) {
        for (int x = 0; x < LOCAL_LARGURA; ++x) {
            if (mapaLabirinto[y][x] == 'S') {
                float startX = x * LOCAL_TAMANHO_CELULA + (LOCAL_TAMANHO_CELULA - jogador.getCaixaColisao().largura) / 2;
                float startY = (LOCAL_ALTURA - 1 - y) * LOCAL_TAMANHO_CELULA + (LOCAL_TAMANHO_CELULA - jogador.getCaixaColisao().altura) / 2;
                jogador = Jogador(startX, startY, jogador.getCaixaColisao().largura, jogador.getCaixaColisao().altura);
                inicioCol = x; inicioRow = y;
                inicioLabX = startX; inicioLabY = startY;
            } else if (mapaLabirinto[y][x] == 'F') {
                std::pair<float, float> posFim = getPosicaoCentro(x, y);
                fimLabX = posFim.first;
                fimLabY = posFim.second;
                fimCol = x; fimRow = y;
            }
        }
    }

    // Gera initial rastro
    gerarRastroVerde();

    // Try to set button on the path (preferably 1/3 along the path)
    botaoAtivado = false;
    portasParaAbrir.clear();
    if (!pathCells.empty()) {
        int idx = std::min((int)pathCells.size() - 1, (int) (pathCells.size() / 3));
        botaoCol = pathCells[idx].first;
        botaoRow = pathCells[idx].second;
    } else {
        // fallback: find any walkable cell
        botaoCol = botaoRow = -1;
        for (int r = 0; r < LOCAL_ALTURA; ++r) {
                for (int c = 0; c < LOCAL_LARGURA; ++c) {
                    if (mapaLabirinto[r][c] != 'X' && mapaLabirinto[r][c] != 'S' && mapaLabirinto[r][c] != 'F') {
                    botaoCol = c; botaoRow = r; break;
                }
            }
            if (botaoCol != -1) break;
        }
    }

    // Define example doors relative to map; ensure they are inside bounds and are walls
    std::vector<std::pair<int,int>> candidateDoors = {{4,2},{5,2},{6,2},{6,3}};
    for (auto &p : candidateDoors) {
        if (p.second >= 0 && p.second < LOCAL_ALTURA && p.first >= 0 && p.first < LOCAL_LARGURA) {
            if (mapaLabirinto[p.second][p.first] == 'X') portasParaAbrir.push_back(p);
        }
    }

    // Compute hint path: simulate opening doors on a copy of the map and run A*
    hintRastroVerde.clear();
    hintPathCells.clear();
    {
        // make grid string copy and open doors
        std::vector<std::string> gridOpen(LOCAL_ALTURA);
        for (int r = 0; r < LOCAL_ALTURA; ++r) {
            gridOpen[r].resize(LOCAL_LARGURA);
            for (int c = 0; c < LOCAL_LARGURA; ++c) gridOpen[r][c] = mapaLabirinto[r][c];
        }
        for (auto &pr : portasParaAbrir) {
            gridOpen[pr.second][pr.first] = ' ';
        }
        if (inicioCol >= 0 && fimCol >= 0) {
            auto hintPath = findPathAStar(gridOpen, inicioCol, inicioRow, fimCol, fimRow);
            for (const auto &cell : hintPath) {
                hintPathCells.push_back(cell);
                hintRastroVerde.push_back(getPosicaoCentro(cell.first, cell.second));
            }
        }
    }

    // Reinit spikes based on rastro but avoid placing spikes on the button
    std::vector<std::pair<int,int>> forbidden;
    if (botaoCol >= 0 && botaoRow >= 0) forbidden.emplace_back(botaoCol, botaoRow);
    inicializarFaseEspinhos(forbidden);

    std::cout << "Fase2 carregada. Botao em (" << botaoCol << "," << botaoRow << ")" << std::endl;
}
