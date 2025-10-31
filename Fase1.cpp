#include "Fase1.h"
#include "Pathfinding.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

Fase1::Fase1() {
    larguraLabirinto = 15;
    alturaLabirinto = 10;
    tamanhoCelula = 48.0f;
    
    // Inicializa o mapa da fase 1
    mapaLabirinto = std::vector<std::vector<char>>(alturaLabirinto, std::vector<char>(larguraLabirinto));
    const char* mapa[] = {
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

    // Copia o mapa e encontra início/fim
    for (int i = 0; i < alturaLabirinto; i++) {
        for (int j = 0; j < larguraLabirinto; j++) {
            mapaLabirinto[i][j] = mapa[i][j];
            if (mapa[i][j] == 'S') {
                inicioCol = j;
                inicioRow = i;
            }
            else if (mapa[i][j] == 'F') {
                fimCol = j;
                fimRow = i;
            }
        }
    }
}

void Fase1::inicializar() {
    inicializarEspinhos();
    gerarCaminhoVerde();  // Gera o caminho inicial
}

void Fase1::inicializarEspinhos() {
    espinhos.clear();
    // Adiciona espinhos em posições específicas
    espinhos.emplace_back(3 * tamanhoCelula + tamanhoCelula/2, 
                         2 * tamanhoCelula + tamanhoCelula/2,
                         tamanhoCelula * 0.6f,
                         tamanhoCelula * 0.6f);
    // Adicione mais espinhos conforme necessário
}

void Fase1::gerarCaminhoVerde() {
    caminhoVerde.clear();
    
    // Converte o mapa para o formato esperado pelo A*
    std::vector<std::string> grid(alturaLabirinto);
    for (int i = 0; i < alturaLabirinto; i++) {
        grid[i].resize(larguraLabirinto);
        for (int j = 0; j < larguraLabirinto; j++) {
            grid[i][j] = mapaLabirinto[i][j];
        }
    }
    
    // Encontra o caminho usando A*
    std::vector<std::pair<int,int>> caminho = findPathAStar(
        grid,
        inicioCol, inicioRow,
        fimCol, fimRow
    );
    
    // Converte o caminho para coordenadas do mundo
    for (const auto& [col, row] : caminho) {
        caminhoVerde.emplace_back(
            col * tamanhoCelula + tamanhoCelula/2,
            row * tamanhoCelula + tamanhoCelula/2
        );
    }
}

void Fase1::atualizar(float dt) {
    for (auto& espinho : espinhos) {
        espinho.atualizar(dt);
    }
}

void Fase1::desenhar() {
    // Desenha o labirinto
    for (int i = 0; i < alturaLabirinto; i++) {
        for (int j = 0; j < larguraLabirinto; j++) {
            float x = j * tamanhoCelula;
            float y = i * tamanhoCelula;
            
            if (mapaLabirinto[i][j] == 'X') {
                // Desenha parede
                glColor3f(0.5f, 0.5f, 0.5f);
                glBegin(GL_QUADS);
                glVertex2f(x, y);
                glVertex2f(x + tamanhoCelula, y);
                glVertex2f(x + tamanhoCelula, y + tamanhoCelula);
                glVertex2f(x, y + tamanhoCelula);
                glEnd();
            }
            else {
                // Desenha chão
                glColor3f(0.2f, 0.2f, 0.2f);
                glBegin(GL_QUADS);
                glVertex2f(x, y);
                glVertex2f(x + tamanhoCelula, y);
                glVertex2f(x + tamanhoCelula, y + tamanhoCelula);
                glVertex2f(x, y + tamanhoCelula);
                glEnd();
            }
        }
    }

    // Desenha o caminho verde tracejado
    if (!caminhoVerde.empty()) {
        // Desenha linha de contorno preta
        glColor3f(0.0f, 0.0f, 0.0f);
        glLineWidth(4.0f);
        glBegin(GL_LINE_STRIP);
        for (const auto& ponto : caminhoVerde) {
            glVertex2f(ponto.first, ponto.second);
        }
        glEnd();

        // Desenha linha tracejada verde por cima
        glColor3f(0.0f, 1.0f, 0.0f);
        glLineWidth(2.0f);
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(2, 0x5555);  // Padrão mais visível de tracejado
        
        glBegin(GL_LINE_STRIP);
        for (const auto& ponto : caminhoVerde) {
            glVertex2f(ponto.first, ponto.second);
        }
        glEnd();
        
        glDisable(GL_LINE_STIPPLE);
        glLineWidth(1.0f);
    }

    // Desenha os espinhos
    for (auto& espinho : espinhos) {
        espinho.desenhar();
    }
}

bool Fase1::verificarVitoria(const Jogador& jogador) {
    float centroX = fimCol * tamanhoCelula + tamanhoCelula/2;
    float centroY = fimRow * tamanhoCelula + tamanhoCelula/2;
    
    CaixaColisao caixaJogador = jogador.getCaixaColisao();
    float distX = (caixaJogador.x + caixaJogador.largura/2) - centroX;
    float distY = (caixaJogador.y + caixaJogador.altura/2) - centroY;
    
    return (distX * distX + distY * distY) < (tamanhoCelula * tamanhoCelula / 4);
}

bool Fase1::verificarColisao(float x, float y) {
    // Tamanho do jogador (ajuste conforme necessário)
    const float tamanhoJogador = tamanhoCelula * 0.6f;
    
    // Verifica os quatro cantos do jogador
    float offsetsX[] = {0.0f, tamanhoJogador};
    float offsetsY[] = {0.0f, tamanhoJogador};
    
    for (float offsetX : offsetsX) {
        for (float offsetY : offsetsY) {
            int col = static_cast<int>((x + offsetX) / tamanhoCelula);
            int row = static_cast<int>((y + offsetY) / tamanhoCelula);
            
            // Verifica limites
            if (row < 0 || row >= alturaLabirinto || col < 0 || col >= larguraLabirinto) {
                return true;
            }
            
            if (mapaLabirinto[row][col] == 'X') {
                return true;
            }
        }
    }
    
    return false;
}