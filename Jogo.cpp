/*
 * COMANDO PARA COMPILAR (no terminal do VS Code, com MinGW 64-bit):
 * g++ Jogo.cpp Jogador.cpp ArmadilhaEspinho.cpp -o MeuJogo.exe -DFREEGLUT_STATIC -mwindows -lglew32 -lfreeglut -lglu32 -lopengl32
 */

// --- INCLUDES ESSENCIAIS ---
#include <windows.h>      // Para o Hack do WinMain
#include <GL/glew.h>      // DEVE vir antes do freeglut
#include <GL/freeglut.h>  // Para criar a janela e gerir eventos
#include <vector>
#include <iostream>
#include <utility>        // Para std::pair
#include <cmath>

// --- INCLUDES DO JOGO ---
#include "ArmadilhaEspinho.h"
#include "Jogador.h"
#include "Pathfinding.h"
#include <string>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "Fase2.h"

// Start/End in grid coords (col,row)
int inicioCol = -1, inicioRow = -1;
int fimCol = -1, fimRow = -1;

// Fase atual (1 = fase1, 2 = fase2)
int faseAtual = 1;

// Botão e portas da fase 2
int botaoCol = -1, botaoRow = -1;
bool botaoAtivado = false;
std::vector<std::pair<int,int>> portasParaAbrir; // células que serão convertidas de 'X' -> ' '

// --- VARIÁVEIS GLOBAIS DO JOGO ---
std::vector<ArmadilhaEspinho> meusEspinhos;
// Inicializa o jogador em (0,0) temporariamente, será movido no 'main_proxy'
// Aumentei o jogador para ficar proporcional ao novo TAMANHO_CELULA
Jogador jogador(0.0f, 0.0f, 32.0f, 32.0f); // Tamanho 32x32

// --- VARIÁVEIS DO LABIRINTO ---
const int LARGURA_LABIRINTO = 15;
const int ALTURA_LABIRINTO = 10;
const float TAMANHO_CELULA = 48.0f; // Cada parede/caminho terá 48x48 (melhor visual)

// 'X' = Parede, ' ' = Chão, 'S' = Início, 'F' = Fim
char mapaLabirinto[ALTURA_LABIRINTO][LARGURA_LABIRINTO + 1] = {
    "XXXXXXXXXXXXXXX",
    "X S X   X     X",
    "X X X X X XXX X",
    "X   X X   X X X",
    "X X XXXXX X X X",
    "X X     X X X X",
    "X XXX X X X X X",
    "X X X X   X   X",
    "X X   XXXXX X F",
    "XXXXXXXXXXXXXXX",
};

std::vector<std::pair<float, float>> rastroVerde;
std::vector<std::pair<int,int>> pathCells; // células do caminho (col,row)
// Hint path for Fase2 (path if doors opened)
std::vector<std::pair<float, float>> hintRastroVerde;
std::vector<std::pair<int,int>> hintPathCells;
int pathProgressIndex = 0; // índice do caminho já percorrido
float inicioLabX, inicioLabY;
float fimLabX, fimLabY;

// Forward declarations
void inicializarFaseEspinhos(const std::vector<std::pair<int,int>>& forbidden);
void gerarRastroVerde(bool fromCurrentPosition);
std::pair<float, float> getPosicaoCentro(int col, int row);
void desenharJogo();
void atualizarJogo(float deltaTime);
// Helper to load phase 2 cleanly (used on finishing phase1 and for debug key)
void carregarFase2() {
    // Reset state before loading phase 2
    botaoAtivado = false;
    portasParaAbrir.clear();
    rastroVerde.clear();
    pathCells.clear();
    meusEspinhos.clear();

    // Load phase 2 and initialize player position
    Fase2::carregar();

    // Make sure player is in starting position
    for (int y = 0; y < ALTURA_LABIRINTO; ++y) {
        for (int x = 0; x < LARGURA_LABIRINTO; ++x) {
            if (mapaLabirinto[y][x] == 'S') {
                float startX = x * TAMANHO_CELULA + (TAMANHO_CELULA - jogador.getCaixaColisao().largura) / 2;
                float startY = (ALTURA_LABIRINTO - 1 - y) * TAMANHO_CELULA + (TAMANHO_CELULA - jogador.getCaixaColisao().altura) / 2;
                jogador = Jogador(startX, startY, jogador.getCaixaColisao().largura, jogador.getCaixaColisao().altura);
                inicioCol = x; inicioRow = y;
                inicioLabX = startX; inicioLabY = startY;
                break;
            }
        }
    }

    // Find finish position
    for (int y = 0; y < ALTURA_LABIRINTO; ++y) {
        for (int x = 0; x < LARGURA_LABIRINTO; ++x) {
            if (mapaLabirinto[y][x] == 'F') {
                auto posFim = getPosicaoCentro(x, y);
                fimLabX = posFim.first;
                fimLabY = posFim.second;
                fimCol = x; fimRow = y;
                break;
            }
        }
    }

    // Update phase and regenerate path
    faseAtual = 2;
    pathProgressIndex = 0;
    gerarRastroVerde(false);

    // Initialize spikes avoiding the button position
    std::vector<std::pair<int,int>> forbidden;
    if (botaoCol >= 0 && botaoRow >= 0) {
        forbidden.emplace_back(botaoCol, botaoRow);
    }
    inicializarFaseEspinhos(forbidden);

    // Debug log
    std::cout << "[DEBUG] carregarFase2: player at (" << jogador.getCaixaColisao().x << "," << jogador.getCaixaColisao().y << ")" << std::endl;
    std::cout << "[DEBUG] carregarFase2: meusEspinhos.size=" << meusEspinhos.size() << " pathCells.size=" << pathCells.size() << std::endl;
}
// --- FUNÇÕES DE LÓGICA DO JOGO ---

// Retorna a célula (col,row) onde o jogador está atualmente
std::pair<int,int> getJogadorCell() {
    CaixaColisao caixa = jogador.getCaixaColisao();
    float centerX = caixa.x + caixa.largura / 2.0f;
    float centerY = caixa.y + caixa.altura / 2.0f;
    int col = (int)(centerX / TAMANHO_CELULA);
    int row = (ALTURA_LABIRINTO - 1) - (int)(centerY / TAMANHO_CELULA);
    return {col, row};
}

// Função auxiliar para converter (coluna, linha) da matriz para (x, y) do jogo
std::pair<float, float> getPosicaoCentro(int col, int row) {
    float x = col * TAMANHO_CELULA + TAMANHO_CELULA / 2;
    float y = (ALTURA_LABIRINTO - 1 - row) * TAMANHO_CELULA + TAMANHO_CELULA / 2;
    return {x, y};
}

void inicializarFaseEspinhos(const std::vector<std::pair<int,int>>& forbidden = {}) {
    meusEspinhos.clear();
    // Adiciona espinhos apenas em células caminháveis (não 'X').
    float espW = TAMANHO_CELULA * 0.6f;
    float espH = espW;

    // Recolhe células caminháveis (exclui S e F)
    std::vector<std::pair<int,int>> walkables;
    for (int r = 0; r < ALTURA_LABIRINTO; ++r) {
        for (int c = 0; c < LARGURA_LABIRINTO; ++c) {
            char ch = mapaLabirinto[r][c];
            if (ch != 'X' && ch != 'S' && ch != 'F') {
                walkables.emplace_back(c, r);
            }
        }
    }

    // Queremos um número maior de espinhos para dinamizar a fase
    int maxSpikes = std::min(14, (int)walkables.size());

    // Se houver um rastro gerado (A*), prefira posicionar vários espinhos ao longo do rastro
    std::vector<std::pair<int,int>> placedCells;
    if (rastroVerde.size() >= 3) {
        // converte rastroVerde em células únicas
        std::vector<std::pair<int,int>> pathCellsLocal;
        for (const auto &p : rastroVerde) {
            int col = (int)(p.first / TAMANHO_CELULA);
            int row = (ALTURA_LABIRINTO - 1) - (int)(p.second / TAMANHO_CELULA);
            if (std::find(pathCellsLocal.begin(), pathCellsLocal.end(), std::make_pair(col,row)) == pathCellsLocal.end())
                pathCellsLocal.emplace_back(col,row);
        }

        // Escolhe alguns pontos ao longo do caminho (cada Nth) para colocar espinhos
        int n = std::max(1, (int)pathCellsLocal.size() / 6);
        int placed = 0;
        for (size_t i = n; i < pathCellsLocal.size() && placed < maxSpikes/2; i += n) {
            auto pr = pathCellsLocal[i];
            // evita colocar em S/F
            if (mapaLabirinto[pr.second][pr.first] == 'X') continue;
            // evita células proibidas (ex: botão)
            if (std::find(forbidden.begin(), forbidden.end(), pr) != forbidden.end()) continue;
            placedCells.push_back(pr);
            // remove this cell from walkables
            auto it = std::find(walkables.begin(), walkables.end(), pr);
            if (it != walkables.end()) walkables.erase(it);
            ++placed;
        }
    }

    // Preenche o restante com células aleatórias caminháveis
    std::srand((unsigned)std::time(nullptr));
    // Preenche o restante com células aleatórias caminháveis, evitando forbiddens
    while ((int)placedCells.size() < maxSpikes && !walkables.empty()) {
        int idx = std::rand() % (int)walkables.size();
        auto cell = walkables[idx];
        if (std::find(forbidden.begin(), forbidden.end(), cell) == forbidden.end()) {
            placedCells.push_back(cell);
        }
        walkables.erase(walkables.begin() + idx);
    }

    // Cria os espinhos nas células escolhidas com períodos aleatórios e offsets
    for (size_t i = 0; i < placedCells.size(); ++i) {
        int col = placedCells[i].first;
        int row = placedCells[i].second;
        float x = col * TAMANHO_CELULA + (TAMANHO_CELULA - espW) / 2.0f;
        float y = (ALTURA_LABIRINTO - 1 - row) * TAMANHO_CELULA + (TAMANHO_CELULA - espH) / 2.0f;
        meusEspinhos.push_back(ArmadilhaEspinho(x, y, espW, espH));
        // período aleatório entre 1.0 e 4.0 segundos
        float periodo = 1.0f + (std::rand() % 300) / 100.0f; // 1.00 .. 4.00
        meusEspinhos.back().setPeriodo(periodo);
        // offset aleatório inicial no timer para desincronizar
        float offset = (std::rand() % 100) / 100.0f * periodo;
        meusEspinhos.back().setTimer(offset);
    }
}

void gerarRastroVerde(bool fromCurrentPosition = false) {
    rastroVerde.clear();
    pathCells.clear();

    // Gera o rastro automaticamente com A* usando o mapa
    if (fimCol >= 0) { // Só precisamos ter um fim válido
        int startCol, startRow;
        if (fromCurrentPosition) {
            auto [col, row] = getJogadorCell();
            // Verifica se a posição atual é válida
            if (col < 0 || col >= LARGURA_LABIRINTO || 
                row < 0 || row >= ALTURA_LABIRINTO || 
                mapaLabirinto[row][col] == 'X') {
                return; // Não gera caminho se estiver em posição inválida
            }
            startCol = col;
            startRow = row;
        } else {
            startCol = inicioCol;
            startRow = inicioRow;
        }
        
        // Converte mapa para vetor<string>
        std::vector<std::string> grid;
        for (int r = 0; r < ALTURA_LABIRINTO; ++r) {
            grid.emplace_back(mapaLabirinto[r]);
        }
        
        // Tenta encontrar o caminho
        std::cout << "[A*] Procurando caminho start=(" << startCol << "," << startRow << ") -> goal=(" << fimCol << "," << fimRow << ")" << std::endl;

        // Primeiro checa conectividade simples (ignora regras de quinas) para ajudar debug
        auto simpleReachable = [&]() {
            int rows = (int)grid.size();
            int cols = (int)grid[0].size();
            std::vector<char> vis(rows*cols, 0);
            std::vector<std::pair<int,int>> q;
            q.emplace_back(startRow, startCol);
            vis[startRow*cols + startCol] = 1;
            for (size_t qi = 0; qi < q.size(); ++qi) {
                auto [r,c] = q[qi];
                if (r == fimRow && c == fimCol) return true;
                const int d[4][2] = {{1,0},{-1,0},{0,1},{0,-1}};
                for (int k=0;k<4;++k) {
                    int nr = r + d[k][0];
                    int nc = c + d[k][1];
                    if (nr>=0 && nr<rows && nc>=0 && nc<cols && !vis[nr*cols+nc] && grid[nr][nc] != 'X') {
                        vis[nr*cols+nc] = 1;
                        q.emplace_back(nr,nc);
                    }
                }
            }
            return false;
        }();
        std::cout << "[A* debug] Conectividade (ignorando quinas): " << (simpleReachable?"SIM":"NAO") << std::endl;

        auto path = findPathAStar(grid, startCol, startRow, fimCol, fimRow);
        std::cout << "[A*] Caminho retornou tamanho: " << path.size() << std::endl;

    // Só atualiza o rastro se encontrou um caminho válido
    if (!path.empty()) {
            for (const auto &cell : path) {
                // Verifica se cada célula do caminho é válida
                if (cell.first >= 0 && cell.first < LARGURA_LABIRINTO &&
                    cell.second >= 0 && cell.second < ALTURA_LABIRINTO &&
                    mapaLabirinto[cell.second][cell.first] != 'X') {
                    pathCells.push_back(cell);
                    rastroVerde.push_back(getPosicaoCentro(cell.first, cell.second));
                } else {
                    // Se encontrar uma célula inválida, limpa o caminho
                    rastroVerde.clear();
                    pathCells.clear();
                    return;
                }
            }
            pathProgressIndex = 0;
        }
    }
}

void atualizarJogo(float deltaTime) {
    static std::pair<int,int> lastCell = getJogadorCell();
    float oldX = jogador.getCaixaColisao().x;
    float oldY = jogador.getCaixaColisao().y;

    // 1. Atualiza posição do jogador
    jogador.atualizar(deltaTime);
    CaixaColisao novaCaixaJogador = jogador.getCaixaColisao();

    // Verifica se o jogador mudou de célula
    auto currentCell = getJogadorCell();
    if (currentCell != lastCell) {
        // Verifica se a nova posição está no caminho atual
        bool onPath = false;
        for (const auto& cell : pathCells) {
            if (cell.first == currentCell.first && cell.second == currentCell.second) {
                onPath = true;
                break;
            }
        }
        // Se não estiver no caminho, recalcula
        if (!onPath) {
            gerarRastroVerde(true);
        }
        lastCell = currentCell;
    }

    // Atualiza espinhos (para permitir alternância ativa/inativa)
    for (auto &esp : meusEspinhos) {
        esp.atualizar(deltaTime);
    }

    // Botão (apenas na fase 2): ao pisar, abre portas definidas
    if (faseAtual == 2 && !botaoAtivado && botaoCol >= 0 && botaoRow >= 0) {
        auto [pcol, prow] = getJogadorCell();
        if (pcol == botaoCol && prow == botaoRow) {
            botaoAtivado = true;
            std::cout << "Botao pressionado! Abrindo portas..." << std::endl;
            
            // Clear current state
            rastroVerde.clear();
            pathCells.clear();
            meusEspinhos.clear();
            
            // Open doors
            for (const auto &pr : portasParaAbrir) {
                int dc = pr.first;
                int dr = pr.second;
                if (dr >= 0 && dr < ALTURA_LABIRINTO && dc >= 0 && dc < LARGURA_LABIRINTO) {
                    mapaLabirinto[dr][dc] = ' ';
                }
            }
            
            // Reset progress and regenerate path from current position
            pathProgressIndex = 0;
            gerarRastroVerde(true);
            
            // Initialize spikes avoiding the button
            std::vector<std::pair<int,int>> forbidden;
            forbidden.emplace_back(botaoCol, botaoRow);
            inicializarFaseEspinhos(forbidden);
        }
    }

    // 2. Lógica de Colisão com Paredes do Labirinto
    bool colidiuComParede = false;
    for (int y = 0; y < ALTURA_LABIRINTO; ++y) {
        for (int x = 0; x < LARGURA_LABIRINTO; ++x) {
            if (mapaLabirinto[y][x] == 'X') { // Se for uma parede
                float paredeX = (float)x * TAMANHO_CELULA;
                float paredeY = (float)(ALTURA_LABIRINTO - 1 - y) * TAMANHO_CELULA;
                CaixaColisao caixaParede = {paredeX, paredeY, TAMANHO_CELULA, TAMANHO_CELULA};

                if (Jogador::checkCollision(novaCaixaJogador, caixaParede)) {
                    colidiuComParede = true;
                    break;
                }
            }
        }
        if (colidiuComParede) break;
    }

    if (colidiuComParede) {
        // Reverte o movimento
        jogador.setX(oldX);
        jogador.setY(oldY);
    }

    // Atualiza progresso do rastro conforme posição do jogador
    {
        auto [col, row] = getJogadorCell();
        // procura primeiro índice em pathCells que corresponda a (col,row)
        bool foundPosition = false;
        for (size_t i = 0; i < pathCells.size(); ++i) {
            if (pathCells[i].first == col && pathCells[i].second == row) {
                pathProgressIndex = (int)i;
                foundPosition = true;
                break;
            }
        }

        if (!foundPosition) {
            pathProgressIndex = 0; // Reset progress while off path
        }
    }

    // 3. Colisão com Espinhos
    for (const auto& espinho : meusEspinhos) {
        if (espinho.verificarColisao(jogador.getCaixaColisao())) {
            std::cout << "Colidiu com espinho!" << std::endl;
            jogador.reiniciarPosicao(); 
            // Ao morrer, restaura visual completo do rastro
            pathProgressIndex = 0;
            break; 
        }
    }

    // 4. Verificação de Chegada ao Fim
    CaixaColisao caixaFim = {fimLabX - TAMANHO_CELULA / 2, fimLabY - TAMANHO_CELULA / 2, TAMANHO_CELULA, TAMANHO_CELULA};
    if (Jogador::checkCollision(jogador.getCaixaColisao(), caixaFim)) {
        if (faseAtual == 1) {
            std::cout << "VOCE VENCEU A FASE 1! Carregando Fase 2..." << std::endl;
            carregarFase2();
        } else {
                // Debug info to help diagnose visibility issues
                std::cout << "[DEBUG] Fase2 carregada: inicioCol=" << inicioCol << " inicioRow=" << inicioRow
                          << " fimCol=" << fimCol << " fimRow=" << fimRow << std::endl;
                std::cout << "[DEBUG] Player pos: x=" << jogador.getCaixaColisao().x << " y=" << jogador.getCaixaColisao().y
                          << " w=" << jogador.getCaixaColisao().largura << " h=" << jogador.getCaixaColisao().altura << std::endl;
                std::cout << "[DEBUG] pathCells.size=" << pathCells.size() << " rastroVerde.size=" << rastroVerde.size()
                          << " meusEspinhos.size=" << meusEspinhos.size() << std::endl;
                std::cout << "[DEBUG] botaoCol=" << botaoCol << " botaoRow=" << botaoRow << " portasParaAbrir=" << portasParaAbrir.size() << std::endl;
        } 
    }
}

void desenharJogo() {
    // 0. Fundo com gradiente sutil
    float larguraMundo = LARGURA_LABIRINTO * TAMANHO_CELULA;
    float alturaMundo = ALTURA_LABIRINTO * TAMANHO_CELULA;
    glBegin(GL_QUADS);
        glColor3f(0.06f, 0.08f, 0.12f);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(larguraMundo, 0.0f);
        glColor3f(0.12f, 0.14f, 0.22f);
        glVertex2f(larguraMundo, alturaMundo);
        glVertex2f(0.0f, alturaMundo);
    glEnd();

    // 1. Desenha o Labirinto (Paredes e Chão)
    for (int y = 0; y < ALTURA_LABIRINTO; ++y) {
        for (int x = 0; x < LARGURA_LABIRINTO; ++x) {
            float drawX = (float)x * TAMANHO_CELULA;
            float drawY = (float)(ALTURA_LABIRINTO - 1 - y) * TAMANHO_CELULA; // Inverte Y

            if (mapaLabirinto[y][x] == 'X') {
                glColor3f(0.45f, 0.42f, 0.5f); // Cor da parede (cinza azulado)
            } else {
                // piso com leve variação dependendo se é caminho do rastro
                glColor3f(0.18f, 0.18f, 0.18f); // Cor do chão (cinza escuro)
            }
            glBegin(GL_QUADS);
                glVertex2f(drawX, drawY);
                glVertex2f(drawX + TAMANHO_CELULA, drawY);
                glVertex2f(drawX + TAMANHO_CELULA, drawY + TAMANHO_CELULA);
                glVertex2f(drawX, drawY + TAMANHO_CELULA);
            glEnd();

            // Se estivermos na fase 2 e esta célula for o botão, desenha um botão no chão
            if (faseAtual == 2 && botaoCol == x && botaoRow == y) {
                float bx = drawX + TAMANHO_CELULA * 0.25f;
                float by = drawY + TAMANHO_CELULA * 0.25f;
                float bw = TAMANHO_CELULA * 0.5f;
                if (botaoAtivado) glColor3f(0.2f, 0.8f, 0.2f); // verde quando ativado
                else glColor3f(0.8f, 0.2f, 0.2f); // vermelho quando não ativado
                glBegin(GL_QUADS);
                    glVertex2f(bx, by);
                    glVertex2f(bx + bw, by);
                    glVertex2f(bx + bw, by + bw);
                    glVertex2f(bx, by + bw);
                glEnd();
            }
            // linhas de grid sutis
            glColor3f(0.06f, 0.06f, 0.08f);
            glBegin(GL_LINE_LOOP);
                glVertex2f(drawX, drawY);
                glVertex2f(drawX + TAMANHO_CELULA, drawY);
                glVertex2f(drawX + TAMANHO_CELULA, drawY + TAMANHO_CELULA);
                glVertex2f(drawX, drawY + TAMANHO_CELULA);
            glEnd();
        }
    }

    // 2. Desenha o Traço Verde (de nó em nó — centros das células)
    // For phase2: draw the hint path (as if button pressed) always; draw the actual path only if button activated.
    if (faseAtual == 2) {
        if (!hintPathCells.empty()) {
            // draw hint path (lighter/fainter green to indicate it's a hint)
            glColor3f(0.2f, 0.8f, 0.4f);
            glLineWidth(3.0f);
            const float dashLen = 12.0f;
            const float gapLen = 8.0f;
            for (int i = 0; i + 1 < (int)hintPathCells.size(); ++i) {
                int dxcell = hintPathCells[i+1].first - hintPathCells[i].first;
                int dycell = hintPathCells[i+1].second - hintPathCells[i].second;
                if (abs(dxcell) + abs(dycell) != 1) continue;
                auto [x1, y1] = getPosicaoCentro(hintPathCells[i].first, hintPathCells[i].second);
                auto [x2, y2] = getPosicaoCentro(hintPathCells[i+1].first, hintPathCells[i+1].second);
                float segLen = sqrtf((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
                if (segLen < 0.001f) continue;
                float ux = (x2-x1)/segLen;
                float uy = (y2-y1)/segLen;
                float t = 0.0f;
                while (t < segLen) {
                    float t0 = t;
                    float t1 = std::min(segLen, t + dashLen);
                    float sx = x1 + ux * t0;
                    float sy = y1 + uy * t0;
                    float ex = x1 + ux * t1;
                    float ey = y1 + uy * t1;
                    glBegin(GL_LINES);
                        glVertex2f(sx, sy);
                        glVertex2f(ex, ey);
                    glEnd();
                    t += dashLen + gapLen;
                }
            }
            glLineWidth(1.0f);
        }
        // draw actual path only if button pressed
        if (botaoAtivado && !pathCells.empty()) {
            glColor3f(0.0f, 0.9f, 0.2f);
            glLineWidth(4.0f);
            const float dashLen = 12.0f;
            const float gapLen = 8.0f;
            for (int i = 0; i + 1 < (int)pathCells.size(); ++i) {
                int dxcell = pathCells[i+1].first - pathCells[i].first;
                int dycell = pathCells[i+1].second - pathCells[i].second;
                if (abs(dxcell) + abs(dycell) != 1) continue;
                auto [x1, y1] = getPosicaoCentro(pathCells[i].first, pathCells[i].second);
                auto [x2, y2] = getPosicaoCentro(pathCells[i+1].first, pathCells[i+1].second);
                float segLen = sqrtf((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
                if (segLen < 0.001f) continue;
                float ux = (x2-x1)/segLen;
                float uy = (y2-y1)/segLen;
                float t = 0.0f;
                while (t < segLen) {
                    float t0 = t;
                    float t1 = std::min(segLen, t + dashLen);
                    float sx = x1 + ux * t0;
                    float sy = y1 + uy * t0;
                    float ex = x1 + ux * t1;
                    float ey = y1 + uy * t1;
                    glBegin(GL_LINES);
                        glVertex2f(sx, sy);
                        glVertex2f(ex, ey);
                    glEnd();
                    t += dashLen + gapLen;
                }
            }
            glLineWidth(1.0f);
        }
    } else {
        if (!pathCells.empty()) {
        // Encontra índice inicial para desenhar (começa do progresso atual)
        int startIndex = pathProgressIndex;

        // Se o jogador está em alguma célula do path, comece dali
        auto [pcol, prow] = getJogadorCell();
        for (size_t i = 0; i < pathCells.size(); ++i) {
            if (pathCells[i].first == pcol && pathCells[i].second == prow) {
                startIndex = (int)i;
                break;
            }
        }

        if (startIndex >= 0 && startIndex + 1 < (int)pathCells.size()) {
            glColor3f(0.0f, 0.9f, 0.2f);
            glLineWidth(4.0f);

            const float dashLen = 12.0f;
            const float gapLen = 8.0f;

            for (int i = startIndex; i + 1 < (int)pathCells.size(); ++i) {
                int dxcell = pathCells[i+1].first - pathCells[i].first;
                int dycell = pathCells[i+1].second - pathCells[i].second;
                // Só conecta células adjacentes (sem diagonal)
                if (abs(dxcell) + abs(dycell) != 1) continue;

                auto [x1, y1] = getPosicaoCentro(pathCells[i].first, pathCells[i].second);
                auto [x2, y2] = getPosicaoCentro(pathCells[i+1].first, pathCells[i+1].second);

                float segLen = sqrtf((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
                if (segLen < 0.001f) continue;
                float ux = (x2-x1)/segLen;
                float uy = (y2-y1)/segLen;

                float t = 0.0f;
                while (t < segLen) {
                    float t0 = t;
                    float t1 = std::min(segLen, t + dashLen);
                    float sx = x1 + ux * t0;
                    float sy = y1 + uy * t0;
                    float ex = x1 + ux * t1;
                    float ey = y1 + uy * t1;
                    glBegin(GL_LINES);
                        glVertex2f(sx, sy);
                        glVertex2f(ex, ey);
                    glEnd();
                    t += dashLen + gapLen;
                }
            }
            glLineWidth(1.0f);
        }
    }


    // 3. Desenha os espinhos
    for (const auto& espinho : meusEspinhos) {
        espinho.desenhar();
    }

    // 4. Desenha o jogador (por último, para ficar por cima)
    jogador.desenhar();
} // <-- fim de desenharJogo()
}

// --- FUNÇÕES DE "CALLBACK" DO GLUT ---
// Forward declarations
void desenharJogo();
void atualizarJogo(float deltaTime);

// GLUT callback functions
void callbackDisplay() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    desenharJogo();
    glutSwapBuffers();
}

void callbackUpdate(int value) {
    float deltaTime = 16.0f / 1000.0f;
    atualizarJogo(deltaTime);
    glutPostRedisplay();
    glutTimerFunc(16, callbackUpdate, 0); 
}

void callbackKeyboardDown(unsigned char key, int x, int y) {
    switch(key) {
        case 'w': case 'W': jogador.setMovendoParaCima(true); break;
        case 's': case 'S': jogador.setMovendoParaBaixo(true); break;
        case 'a': case 'A': jogador.setMovendoParaEsquerda(true); break;
        case 'd': case 'D': jogador.setMovendoParaDireita(true); break;
        case 27: glutLeaveMainLoop(); break;
    }
}

void callbackKeyboardUp(unsigned char key, int x, int y) {
     switch(key) {
        case 'w': case 'W': jogador.setMovendoParaCima(false); break;
        case 's': case 'S': jogador.setMovendoParaBaixo(false); break;
        case 'a': case 'A': jogador.setMovendoParaEsquerda(false); break;
        case 'd': case 'D': jogador.setMovendoParaDireita(false); break;
    }
}

void callbackReshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float larguraMundo = LARGURA_LABIRINTO * TAMANHO_CELULA;
    float alturaMundo = ALTURA_LABIRINTO * TAMANHO_CELULA;
    float aspectoJanela = (float)w / (float)h;
    float aspectoMundo = larguraMundo / alturaMundo;
    
    float orthoW, orthoH;
    if (aspectoJanela > aspectoMundo) {
        orthoH = alturaMundo + 100;
        orthoW = orthoH * aspectoJanela;
    } else {
        orthoW = larguraMundo + 100;
        orthoH = orthoW / aspectoJanela;
    }
    
    float centroX = larguraMundo / 2.0f;
    float centroY = alturaMundo / 2.0f;

    gluOrtho2D(centroX - orthoW / 2.0f, centroX + orthoW / 2.0f, 
               centroY - orthoH / 2.0f, centroY + orthoH / 2.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main_proxy(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Jogo de Orientacao - Fase Labirinto");

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "Erro ao inicializar GLEW: " << glewGetErrorString(err) << std::endl;
        return 1;
    }

    // Inicializa os componentes do jogo
    // Ordem: gerar rastro (A*), depois posicionar espinhos com base no rastro
    gerarRastroVerde();
    inicializarFaseEspinhos();

    // Encontra 'S' e 'F' e define posições
    for (int y = 0; y < ALTURA_LABIRINTO; ++y) {
        for (int x = 0; x < LARGURA_LABIRINTO; ++x) {
            if (mapaLabirinto[y][x] == 'S') {
                float startX = x * TAMANHO_CELULA + (TAMANHO_CELULA - jogador.getCaixaColisao().largura) / 2;
                float startY = (ALTURA_LABIRINTO - 1 - y) * TAMANHO_CELULA + (TAMANHO_CELULA - jogador.getCaixaColisao().altura) / 2;
                // Recria o jogador na posição correta
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

    // Re-generate rastro now that we discovered start/end
    gerarRastroVerde();
    // Re-init espinhos based on new rastro
    inicializarFaseEspinhos();

    // Registra as Callbacks
    glutDisplayFunc(callbackDisplay);
    glutReshapeFunc(callbackReshape);
    glutKeyboardFunc(callbackKeyboardDown);
    glutKeyboardUpFunc(callbackKeyboardUp);
    glutTimerFunc(16, callbackUpdate, 0);

    glutMainLoop(); 
    return 0;
}

// O Hack do WinMain que o Linker do Windows procura
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
    char* argv[1];
    argv[0] = (char*)"MeuJogo"; 
    int argc = 1;
    return main_proxy(argc, argv);
}