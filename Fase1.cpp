#include "Fase1.h"
#include "Pathfinding.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

Fase1::Fase1()
{
    larguraLabirinto = 15;
    alturaLabirinto = 10;
    tamanhoCelula = 48.0f;

    // Inicializa o mapa da fase 1
    mapaLabirinto = std::vector<std::vector<char>>(alturaLabirinto, std::vector<char>(larguraLabirinto));
    const char *mapa[] = {
        "XXXXXXXXXXXXXXX",
        "X S X   X     X",
        "X X X X X XXX X",
        "X   X X   X X X",
        "X X XXXXX X X X",
        "X X     X X X X",
        "X XXX X X X X X",
        "X X X X   X   X",
        "X X   XXXXX X F",
        "XXXXXXXXXXXXXXX"};

    // Copia o mapa e encontra início/fim
    for (int i = 0; i < alturaLabirinto; i++)
    {
        for (int j = 0; j < larguraLabirinto; j++)
        {
            mapaLabirinto[i][j] = mapa[i][j];
            if (mapa[i][j] == 'S')
            {
                inicioCol = j;
                inicioRow = i;
            }
            else if (mapa[i][j] == 'F')
            {
                fimCol = j;
                fimRow = i;
            }
        }
    }
}

void Fase1::inicializar()
{
    inicializarEspinhos();
    gerarCaminhoVerde(); // Gera o caminho inicial
}

void Fase1::inicializarEspinhos()
{
    espinhos.clear();
    // Adiciona espinhos em posições válidas do labirinto (não parede)
    // Inclui alguns fixos e alguns aleatórios para dar variedade visual.
    std::srand((unsigned)std::time(nullptr));
    auto adicionaEspinho = [&](int c, int r)
    {
        if (r < 0 || r >= alturaLabirinto || c < 0 || c >= larguraLabirinto)
            return;
        if (mapaLabirinto[r][c] == 'X' || mapaLabirinto[r][c] == 'S' || mapaLabirinto[r][c] == 'F')
            return;
        float espW = tamanhoCelula * 0.6f;
        float espH = espW;
        float x = c * tamanhoCelula + (tamanhoCelula - espW) / 2.0f;
        float y = (alturaLabirinto - 1 - r) * tamanhoCelula + (tamanhoCelula - espH) / 2.0f;
        espinhos.emplace_back(x, y, espW, espH);
        // Todos os espinhos alternam entre ativo/inativo (evita alguns sempre ativos)
        float periodo = 1.2f + (std::rand() % 160) / 100.0f; // 1.2 a 2.8s
        espinhos.back().setPeriodo(periodo);
        float offset = (std::rand() % 100) / 100.0f * periodo;
        espinhos.back().setTimer(offset);
        // Estado inicial aleatório para variar padrões
        espinhos.back().setAtivo((std::rand() % 2) == 1);
    };

    // Candidatos fixos (checados contra paredes)
    const std::pair<int, int> fixos[] = {
        {3, 2}, {5, 3}, {7, 3}, {9, 5}, {11, 4}, {5, 6}, {8, 7}};
    for (auto &p : fixos)
        adicionaEspinho(p.first, p.second);

    // Adiciona alguns aleatórios fora do caminho (simplesmente aleatórios em espaços)
    int adicionados = 0;
    int tentativas = 0;
    while (adicionados < 5 && tentativas < 200)
    {
        ++tentativas;
        int c = std::rand() % larguraLabirinto;
        int r = std::rand() % alturaLabirinto;
        if (mapaLabirinto[r][c] == ' ')
        {
            adicionaEspinho(c, r);
            ++adicionados;
        }
    }
}

void Fase1::gerarCaminhoVerde()
{
    caminhoVerde.clear();

    // Converte o mapa para o formato esperado pelo A*
    std::vector<std::string> grid(alturaLabirinto);
    for (int i = 0; i < alturaLabirinto; i++)
    {
        grid[i].resize(larguraLabirinto);
        for (int j = 0; j < larguraLabirinto; j++)
        {
            grid[i][j] = mapaLabirinto[i][j];
        }
    }

    // Encontra o caminho usando A*
    std::vector<std::pair<int, int>> caminho = findPathAStar(
        grid,
        inicioCol, inicioRow,
        fimCol, fimRow);

    // Converte o caminho para coordenadas do mundo (Y invertido)
    for (const auto &pr : caminho)
    {
        int col = pr.first;
        int row = pr.second;
        caminhoVerde.emplace_back(
            col * tamanhoCelula + tamanhoCelula / 2,
            (alturaLabirinto - 1 - row) * tamanhoCelula + tamanhoCelula / 2);
    }
}

void Fase1::atualizar(float dt)
{
    for (auto &espinho : espinhos)
    {
        espinho.atualizar(dt);
    }

    // Recalcula o "GPS" (A*) dinamicamente a partir da célula atual do jogador
    if (jogadorRef)
    {
        static std::pair<int, int> lastCell = {-9999, -9999};
        CaixaColisao cj = jogadorRef->getCaixaColisao();
        int col = static_cast<int>((cj.x + cj.largura / 2) / tamanhoCelula);
        int row = (alturaLabirinto - 1) - static_cast<int>((cj.y + cj.altura / 2) / tamanhoCelula);
        // Só recalcula se a célula é válida e caminhável
        auto cellWalkable = [&](int c, int r)
        {
            return (r >= 0 && r < alturaLabirinto && c >= 0 && c < larguraLabirinto && mapaLabirinto[r][c] != 'X');
        };
        if ((col != lastCell.first || row != lastCell.second) && cellWalkable(col, row))
        {
            lastCell = {col, row};

            // Gera caminho da posição atual até o fim
            std::vector<std::string> grid(alturaLabirinto);
            for (int i = 0; i < alturaLabirinto; i++)
            {
                grid[i].resize(larguraLabirinto);
                for (int j = 0; j < larguraLabirinto; j++)
                    grid[i][j] = mapaLabirinto[i][j];
            }
            auto caminho = findPathAStar(grid, col, row, fimCol, fimRow);
            caminhoVerde.clear();
            for (const auto &p : caminho)
            {
                float x = p.first * tamanhoCelula + tamanhoCelula / 2;
                float y = (alturaLabirinto - 1 - p.second) * tamanhoCelula + tamanhoCelula / 2;
                caminhoVerde.emplace_back(x, y);
            }
        }
    }
}

void Fase1::desenhar()
{
    // Desenha o labirinto
    for (int i = 0; i < alturaLabirinto; i++)
    {
        for (int j = 0; j < larguraLabirinto; j++)
        {
            float x = j * tamanhoCelula;
            // Inverte Y para coordenadas de mundo (origem inferior)
            float y = (alturaLabirinto - 1 - i) * tamanhoCelula;

            // Paredes e chão
            if (mapaLabirinto[i][j] == 'X')
            {
                glColor3f(0.45f, 0.45f, 0.45f); // parede
            }
            else
            {
                glColor3f(0.20f, 0.20f, 0.20f); // chão
            }
            glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + tamanhoCelula, y);
            glVertex2f(x + tamanhoCelula, y + tamanhoCelula);
            glVertex2f(x, y + tamanhoCelula);
            glEnd();

            // Contorno da célula: quadriculado clássico
            glColor3f(0.06f, 0.06f, 0.08f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x, y);
            glVertex2f(x + tamanhoCelula, y);
            glVertex2f(x + tamanhoCelula, y + tamanhoCelula);
            glVertex2f(x, y + tamanhoCelula);
            glEnd();
        }
    }

    // Desenha o caminho verde tracejado (manual dash ao invés de pontilhado)
    if (!caminhoVerde.empty())
    {
        const float dashLen = 24.0f;
        const float gapLen = 10.0f;
        glLineWidth(4.0f);
        // contorno preto para destacar
        glColor3f(0.0f, 0.0f, 0.0f);
        for (size_t i = 0; i + 1 < caminhoVerde.size(); ++i)
        {
            float x1 = caminhoVerde[i].first, y1 = caminhoVerde[i].second;
            float x2 = caminhoVerde[i + 1].first, y2 = caminhoVerde[i + 1].second;
            float dx = x2 - x1, dy = y2 - y1;
            float dist = static_cast<float>(std::sqrt(dx * dx + dy * dy));
            if (dist < 0.001f)
                continue;
            float ux = dx / dist, uy = dy / dist;
            float t = 0.0f;
            while (t < dist)
            {
                float t0 = t;
                float t1 = std::min(dist, t + dashLen);
                glBegin(GL_LINES);
                glVertex2f(x1 + ux * t0, y1 + uy * t0);
                glVertex2f(x1 + ux * t1, y1 + uy * t1);
                glEnd();
                t += dashLen + gapLen;
            }
        }
        glLineWidth(2.0f);
        glColor3f(0.0f, 1.0f, 0.0f);
        for (size_t i = 0; i + 1 < caminhoVerde.size(); ++i)
        {
            float x1 = caminhoVerde[i].first, y1 = caminhoVerde[i].second;
            float x2 = caminhoVerde[i + 1].first, y2 = caminhoVerde[i + 1].second;
            float dx = x2 - x1, dy = y2 - y1;
            float dist = static_cast<float>(std::sqrt(dx * dx + dy * dy));
            if (dist < 0.001f)
                continue;
            float ux = dx / dist, uy = dy / dist;
            float t = 0.0f;
            while (t < dist)
            {
                float t0 = t;
                float t1 = std::min(dist, t + dashLen);
                glBegin(GL_LINES);
                glVertex2f(x1 + ux * t0, y1 + uy * t0);
                glVertex2f(x1 + ux * t1, y1 + uy * t1);
                glEnd();
                t += dashLen + gapLen;
            }
        }
        glLineWidth(1.0f);
    }

    // Desenha os espinhos
    for (auto &espinho : espinhos)
    {
        espinho.desenhar();
    }
}

bool Fase1::verificarVitoria(const Jogador &jogador)
{
    float centroX = fimCol * tamanhoCelula + tamanhoCelula / 2;
    float centroY = (alturaLabirinto - 1 - fimRow) * tamanhoCelula + tamanhoCelula / 2;

    CaixaColisao caixaJogador = jogador.getCaixaColisao();
    float distX = (caixaJogador.x + caixaJogador.largura / 2) - centroX;
    float distY = (caixaJogador.y + caixaJogador.altura / 2) - centroY;

    return (distX * distX + distY * distY) < (tamanhoCelula * tamanhoCelula / 4);
}

bool Fase1::verificarColisao(float x, float y)
{
    // Usa dimensões reais do jogador, se disponíveis
    float larguraJog = tamanhoCelula * 0.66f;
    float alturaJog = tamanhoCelula * 0.66f;
    if (jogadorRef)
    {
        CaixaColisao cj = jogadorRef->getCaixaColisao();
        larguraJog = cj.largura;
        alturaJog = cj.altura;
    }

    // Verifica os quatro cantos do jogador
    float offsetsX[] = {0.0f, larguraJog};
    float offsetsY[] = {0.0f, alturaJog};

    for (float offsetX : offsetsX)
    {
        for (float offsetY : offsetsY)
        {
            int col = static_cast<int>((x + offsetX) / tamanhoCelula);
            // Converte Y do mundo (origem inferior) para índice de linha
            int row = (alturaLabirinto - 1) - static_cast<int>((y + offsetY) / tamanhoCelula);

            // Verifica limites
            if (row < 0 || row >= alturaLabirinto || col < 0 || col >= larguraLabirinto)
            {
                return true;
            }

            if (mapaLabirinto[row][col] == 'X')
            {
                return true;
            }
        }
    }

    return false;
}