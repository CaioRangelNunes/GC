#include "Fase3.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <utility>
#include <vector>

Fase3::Fase3() {
    larguraLabirinto = 15;
    alturaLabirinto = 10;
    tamanhoCelula = 48.0f;

    mapaLabirinto.assign(alturaLabirinto, std::vector<char>(larguraLabirinto));
    // Labirinto diferente (mais aberto nas bordas e com corredor central alternado)
    const char* mapa[] = {
        "XXXXXXXXXXXXXXX",
        "X S     X     X",
        "X XXX X XXX X X",
        "X   X X   X X X",
        "X X X XXX X   X",
        "X X   X   XXX X",
        "X XXX XXX X   X",
        "X     X   X X X",
        "X XXX XXX   X F",
        "XXXXXXXXXXXXXXX"
    };
    for (int r = 0; r < alturaLabirinto; ++r) {
        for (int c = 0; c < larguraLabirinto; ++c) {
            mapaLabirinto[r][c] = mapa[r][c];
            if (mapa[r][c] == 'S') { inicioCol = c; inicioRow = r; }
            else if (mapa[r][c] == 'F') { fimCol = c; fimRow = r; }
        }
    }
}

void Fase3::inicializar() {
    // Inicializa espinhos
    espinhos.clear();
    std::srand((unsigned)std::time(nullptr));
    auto adicionaEspinho = [&](int c, int r) {
        if (r < 0 || r >= alturaLabirinto || c < 0 || c >= larguraLabirinto) return;
        if (mapaLabirinto[r][c] == 'X' || mapaLabirinto[r][c] == 'S' || mapaLabirinto[r][c] == 'F') return;
        float espW = tamanhoCelula * 0.6f;
        float espH = espW;
        float x = c * tamanhoCelula + (tamanhoCelula - espW) / 2.0f;
        float y = (alturaLabirinto - 1 - r) * tamanhoCelula + (tamanhoCelula - espH) / 2.0f;
        espinhos.emplace_back(x, y, espW, espH);
        float periodo = 1.0f + (std::rand() % 160) / 100.0f; // 1.0 a 2.6s
        espinhos.back().setPeriodo(periodo);
        float offset = (std::rand() % 100) / 100.0f * periodo;
        espinhos.back().setTimer(offset);
    };

    const std::pair<int,int> fixos[] = { {4,2},{6,2},{9,2},{3,3},{7,4},{11,5},{5,6},{10,7} };
    for (auto &p : fixos) adicionaEspinho(p.first, p.second);
    int adicionados = 0, tentativas = 0;
    while (adicionados < 8 && tentativas < 300) {
        ++tentativas;
        int c = std::rand() % larguraLabirinto;
        int r = std::rand() % alturaLabirinto;
        if (mapaLabirinto[r][c] == ' ') { adicionaEspinho(c, r); ++adicionados; }
    }
}

void Fase3::atualizar(float dt) {
    for (auto &e : espinhos) e.atualizar(dt);
}

void Fase3::desenhar() {
    // Calcula célula do jogador para FOV
    int pcol = -100, prow = -100;
    if (jogadorRef) {
        CaixaColisao cj = jogadorRef->getCaixaColisao();
        pcol = static_cast<int>((cj.x + cj.largura/2) / tamanhoCelula);
        prow = (alturaLabirinto - 1) - static_cast<int>((cj.y + cj.altura/2) / tamanhoCelula);
    }

    // Labirinto quadriculado
    for (int i = 0; i < alturaLabirinto; ++i) {
        for (int j = 0; j < larguraLabirinto; ++j) {
            float x = j * tamanhoCelula;
            float y = (alturaLabirinto - 1 - i) * tamanhoCelula;
            if (mapaLabirinto[i][j] == 'X') glColor3f(0.45f, 0.45f, 0.45f);
            else glColor3f(0.20f, 0.20f, 0.20f);
            glBegin(GL_QUADS);
                glVertex2f(x, y);
                glVertex2f(x + tamanhoCelula, y);
                glVertex2f(x + tamanhoCelula, y + tamanhoCelula);
                glVertex2f(x, y + tamanhoCelula);
            glEnd();
            glColor3f(0.06f, 0.06f, 0.08f);
            glBegin(GL_LINE_LOOP);
                glVertex2f(x, y);
                glVertex2f(x + tamanhoCelula, y);
                glVertex2f(x + tamanhoCelula, y + tamanhoCelula);
                glVertex2f(x, y + tamanhoCelula);
            glEnd();
        }
    }

    // Espinhos
    for (const auto &e : espinhos) e.desenhar();

    // Blackout fora do raio (3 células) igual à Fase 2 (tudo preto fora do FOV)
    if (pcol > -50 && prow > -50) {
        const float raio = 3.0f;
        glColor3f(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < alturaLabirinto; ++i) {
            for (int j = 0; j < larguraLabirinto; ++j) {
                float dx = static_cast<float>(j - pcol);
                float dy = static_cast<float>(i - prow);
                float dist = std::sqrt(dx*dx + dy*dy);
                if (dist > raio) {
                    float x = j * tamanhoCelula;
                    float y = (alturaLabirinto - 1 - i) * tamanhoCelula;
                    glBegin(GL_QUADS);
                        glVertex2f(x, y);
                        glVertex2f(x + tamanhoCelula, y);
                        glVertex2f(x + tamanhoCelula, y + tamanhoCelula);
                        glVertex2f(x, y + tamanhoCelula);
                    glEnd();
                }
            }
        }
    }

    // Mapa dourado (mini-mapa) no canto inferior esquerdo
    // Mostra o labirinto e marca spawn (S) e destino (X) sem posição do jogador
    const float mapCX = -40.0f; // centro
    const float mapCY =  32.0f; // um pouco mais acima para comportar maior altura
    const float cellSize = 12.0f; // ainda maior para melhor visualização
    const float pad = 12.0f; // padding proporcional
    float mapW = larguraLabirinto * cellSize;
    float mapH = alturaLabirinto * cellSize;
    float left = mapCX - mapW/2 - pad;
    float right = mapCX + mapW/2 + pad;
    float bottom = mapCY - mapH/2 - pad;
    float top = mapCY + mapH/2 + pad;

    // Moldura dourada com gradiente
    glBegin(GL_TRIANGLE_FAN);
        glColor3f(0.75f, 0.55f, 0.15f);
        glVertex2f(mapCX, mapCY);
        glColor3f(0.95f, 0.76f, 0.24f);
        glVertex2f(left, bottom);
        glVertex2f(right, bottom);
        glVertex2f(right, top);
        glVertex2f(left, top);
    glEnd();
    glColor3f(0.98f, 0.82f, 0.30f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(left, bottom);
        glVertex2f(right, bottom);
        glVertex2f(right, top);
        glVertex2f(left, top);
    glEnd();

    // Área interna do mini-mapa (fundo)
    float innerL = mapCX - mapW/2;
    float innerR = mapCX + mapW/2;
    float innerB = mapCY - mapH/2;
    float innerT = mapCY + mapH/2;
    glColor3f(0.12f, 0.12f, 0.12f);
    glBegin(GL_QUADS);
        glVertex2f(innerL, innerB);
        glVertex2f(innerR, innerB);
        glVertex2f(innerR, innerT);
        glVertex2f(innerL, innerT);
    glEnd();

    // Desenha o grid do mini-mapa
    for (int i = 0; i < alturaLabirinto; ++i) {
        for (int j = 0; j < larguraLabirinto; ++j) {
            float mx = innerL + j * cellSize;
            float my = innerB + (alturaLabirinto - 1 - i) * cellSize;
            if (mapaLabirinto[i][j] == 'X') glColor3f(0.35f, 0.35f, 0.35f);
            else glColor3f(0.20f, 0.20f, 0.20f);
            glBegin(GL_QUADS);
                glVertex2f(mx, my);
                glVertex2f(mx + cellSize, my);
                glVertex2f(mx + cellSize, my + cellSize);
                glVertex2f(mx, my + cellSize);
            glEnd();
            glColor3f(0.06f, 0.06f, 0.08f);
            glBegin(GL_LINE_LOOP);
                glVertex2f(mx, my);
                glVertex2f(mx + cellSize, my);
                glVertex2f(mx + cellSize, my + cellSize);
                glVertex2f(mx, my + cellSize);
            glEnd();
        }
    }

    // Marca spawn (S) com símbolo distinto (círculo verde)
    float sx = innerL + inicioCol * cellSize + cellSize/2;
    float sy = innerB + (alturaLabirinto - 1 - inicioRow) * cellSize + cellSize/2;
    glColor3f(0.10f, 0.85f, 0.25f);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(sx, sy);
        for (int i = 0; i <= 16; ++i) {
            float a = (float)i/16 * 2.0f * 3.1415926f;
            glVertex2f(sx + std::cos(a) * (cellSize*0.45f), sy + std::sin(a) * (cellSize*0.45f));
        }
    glEnd();

    // Marca objetivo (F) com X vermelho
    float fx = innerL + fimCol * cellSize;
    float fy = innerB + (alturaLabirinto - 1 - fimRow) * cellSize;
    glColor3f(0.9f, 0.1f, 0.1f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glVertex2f(fx + 0.3f*cellSize, fy + 0.2f*cellSize);
        glVertex2f(fx + 0.7f*cellSize, fy + 0.8f*cellSize);
        glVertex2f(fx + 0.7f*cellSize, fy + 0.2f*cellSize);
        glVertex2f(fx + 0.3f*cellSize, fy + 0.8f*cellSize);
    glEnd();
    glLineWidth(1.0f);
}

bool Fase3::verificarVitoria(const Jogador& jogador) {
    float centroX = fimCol * tamanhoCelula + tamanhoCelula/2;
    float centroY = (alturaLabirinto - 1 - fimRow) * tamanhoCelula + tamanhoCelula/2;
    CaixaColisao cj = jogador.getCaixaColisao();
    float dx = (cj.x + cj.largura/2) - centroX;
    float dy = (cj.y + cj.altura/2) - centroY;
    return (dx*dx + dy*dy) < (tamanhoCelula * tamanhoCelula / 4);
}

bool Fase3::verificarColisao(float x, float y) {
    float larguraJog = tamanhoCelula * 0.66f;
    float alturaJog = tamanhoCelula * 0.66f;
    if (jogadorRef) { auto cj = jogadorRef->getCaixaColisao(); larguraJog=cj.largura; alturaJog=cj.altura; }
    float offsetsX[] = {0.0f, larguraJog};
    float offsetsY[] = {0.0f, alturaJog};
    for (float ox : offsetsX) {
        for (float oy : offsetsY) {
            int col = static_cast<int>((x + ox) / tamanhoCelula);
            int row = (alturaLabirinto - 1) - static_cast<int>((y + oy) / tamanhoCelula);
            if (row < 0 || row >= alturaLabirinto || col < 0 || col >= larguraLabirinto) return true;
            if (mapaLabirinto[row][col] == 'X') return true;
        }
    }
    return false;
}
