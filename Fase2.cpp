// Implementação da Fase 2: labirinto diferente da Fase 1, com armadilhas ativas.
#include "Fase2.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include "Pathfinding.h"

Fase2::Fase2()
{
    larguraLabirinto = 15;
    alturaLabirinto = 10;
    tamanhoCelula = 48.0f;

    mapaLabirinto.assign(alturaLabirinto, std::vector<char>(larguraLabirinto));
    const char *mapa[] = {
        "XXXXXXXXXXXXXXX",
        "X S     X   X X",
        "X XXX X X X X X",
        "X   X X   X   X",
        "XXX X XXX XXX X",
        "X   X   X     X",
        "X XXX X XXXXX X",
        "X     X     X X",
        "X XXX XXXXX X F",
        "XXXXXXXXXXXXXXX"};

    for (int r = 0; r < alturaLabirinto; ++r)
    {
        for (int c = 0; c < larguraLabirinto; ++c)
        {
            mapaLabirinto[r][c] = mapa[r][c];
            if (mapa[r][c] == 'S')
            {
                inicioCol = c;
                inicioRow = r;
            }
            else if (mapa[r][c] == 'F')
            {
                fimCol = c;
                fimRow = r;
            }
        }
    }
}

void Fase2::inicializar()
{
    // Espinhos
    espinhos.clear();
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
        // Alternância garantida
        float periodo = 1.0f + (std::rand() % 180) / 100.0f; // 1.0 a 2.8s
        espinhos.back().setPeriodo(periodo);
        float offset = (std::rand() % 100) / 100.0f * periodo;
        espinhos.back().setTimer(offset);
        espinhos.back().setAtivo((std::rand() % 2) == 1);
    };

    // Alguns fixos e alguns aleatórios
    const std::pair<int, int> fixos[] = {{2, 3}, {5, 1}, {9, 4}, {12, 6}, {7, 7}, {3, 5}, {10, 2}};
    for (auto &p : fixos)
        adicionaEspinho(p.first, p.second);
    int adicionados = 0, tentativas = 0;
    while (adicionados < 7 && tentativas < 300)
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

void Fase2::atualizar(float dt)
{
    for (auto &e : espinhos)
        e.atualizar(dt);

    // Atualiza caminho (bússola) quando o jogador entra em uma nova célula
    if (jogadorRef)
    {
        CaixaColisao cj = jogadorRef->getCaixaColisao();
        int col = static_cast<int>((cj.x + cj.largura / 2) / tamanhoCelula);
        int row = (alturaLabirinto - 1) - static_cast<int>((cj.y + cj.altura / 2) / tamanhoCelula);
        auto inBounds = [&](int c, int r)
        { return r >= 0 && r < alturaLabirinto && c >= 0 && c < larguraLabirinto; };
        static int lastCol = -9999, lastRow = -9999;
        if ((col != lastCol || row != lastRow) && inBounds(col, row) && mapaLabirinto[row][col] != 'X')
        {
            lastCol = col;
            lastRow = row;
            // Monta grid para A*
            std::vector<std::string> grid(alturaLabirinto);
            for (int r = 0; r < alturaLabirinto; ++r)
            {
                grid[r].resize(larguraLabirinto);
                for (int c = 0; c < larguraLabirinto; ++c)
                    grid[r][c] = mapaLabirinto[r][c];
            }
            pathCells = findPathAStar(grid, col, row, fimCol, fimRow);
        }
    }
}

void Fase2::desenhar()
{
    // Desenha labirinto com grade (quadriculado)
    // Calcula a célula do jogador para limitar campo de visão (raio 3 células)
    int pcol = -100, prow = -100;
    if (jogadorRef)
    {
        CaixaColisao cj = jogadorRef->getCaixaColisao();
        pcol = static_cast<int>((cj.x + cj.largura / 2) / tamanhoCelula);
        prow = (alturaLabirinto - 1) - static_cast<int>((cj.y + cj.altura / 2) / tamanhoCelula);
    }

    for (int i = 0; i < alturaLabirinto; ++i)
    {
        for (int j = 0; j < larguraLabirinto; ++j)
        {
            float x = j * tamanhoCelula;
            float y = (alturaLabirinto - 1 - i) * tamanhoCelula;
            if (mapaLabirinto[i][j] == 'X') {
                glColor3f(0.45f, 0.45f, 0.45f);
            } else if (j == fimCol && i == fimRow) {
                glColor3f(0.75f, 0.08f, 0.08f);
            } else {
                glColor3f(0.20f, 0.20f, 0.20f);
            }
            glBegin(GL_QUADS);
            glVertex2f(x, y);
            glVertex2f(x + tamanhoCelula, y);
            glVertex2f(x + tamanhoCelula, y + tamanhoCelula);
            glVertex2f(x, y + tamanhoCelula);
            glEnd();
            if (j == fimCol && i == fimRow) glColor3f(0.90f, 0.15f, 0.15f); else glColor3f(0.06f,0.06f,0.08f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x, y);
            glVertex2f(x + tamanhoCelula, y);
            glVertex2f(x + tamanhoCelula, y + tamanhoCelula);
            glVertex2f(x, y + tamanhoCelula);
            glEnd();
        }
    }

    // Desenha espinhos
    for (const auto &e : espinhos)
        e.desenhar();

    // X vermelho adicional na célula final (reforço visual dentro do blackout)
    float fx = fimCol * tamanhoCelula;
    float fy = (alturaLabirinto - 1 - fimRow) * tamanhoCelula;
    glColor3f(0.95f, 0.25f, 0.25f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
        glVertex2f(fx + tamanhoCelula*0.25f, fy + tamanhoCelula*0.25f);
        glVertex2f(fx + tamanhoCelula*0.75f, fy + tamanhoCelula*0.75f);
        glVertex2f(fx + tamanhoCelula*0.75f, fy + tamanhoCelula*0.25f);
        glVertex2f(fx + tamanhoCelula*0.25f, fy + tamanhoCelula*0.75f);
    glEnd();
    glLineWidth(1.0f);

    // Blackout fora do raio de visão (3 células ao redor do jogador)
    if (pcol > -50 && prow > -50)
    {
        const float raio = 3.0f;
        glColor3f(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < alturaLabirinto; ++i)
        {
            for (int j = 0; j < larguraLabirinto; ++j)
            {
                float dx = static_cast<float>(j - pcol);
                float dy = static_cast<float>(i - prow);
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist > raio)
                {
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

    // Desenha bússola dourada no canto inferior esquerdo (fora do labirinto)
    // Posição fixa na margem: usa a margem do ortho (ver Jogo::redimensionarJanela)
    const float hudX = -40.0f; // centro da bússola
    const float hudY = -10.0f; // um pouco mais pra cima
    const float radiusOuter = 26.0f;
    const float radiusInner = 21.0f;
    const int segments = 48;
    // Face com leve gradiente (golden)
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.75f, 0.55f, 0.15f); // centro (ouro mais escuro)
    glVertex2f(hudX, hudY);
    glColor3f(0.95f, 0.76f, 0.24f); // borda (ouro mais claro)
    for (int i = 0; i <= segments; ++i)
    {
        float a = (float)i / segments * 2.0f * 3.1415926f;
        glVertex2f(hudX + std::cos(a) * radiusInner, hudY + std::sin(a) * radiusInner);
    }
    glEnd();
    // Aro externo (bezel) dourado
    glColor3f(0.98f, 0.82f, 0.30f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; ++i)
    {
        float a = (float)i / segments * 2.0f * 3.1415926f;
        glVertex2f(hudX + std::cos(a) * radiusOuter, hudY + std::sin(a) * radiusOuter);
    }
    glEnd();
    glLineWidth(1.0f);
    // Marcas cardeais (ticks)
    glColor3f(0.98f, 0.82f, 0.30f);
    glLineWidth(2.0f);
    for (int k = 0; k < 4; ++k)
    {
        float a = 3.1415926f * 0.5f * k; // 0,90,180,270 graus
        float x1 = hudX + std::cos(a) * (radiusInner - 2.0f);
        float y1 = hudY + std::sin(a) * (radiusInner - 2.0f);
        float x2 = hudX + std::cos(a) * (radiusOuter - 1.0f);
        float y2 = hudY + std::sin(a) * (radiusOuter - 1.0f);
        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
    }
    glLineWidth(1.0f);

    // Letras dos pontos cardeais (N, E, S, W)
    auto drawText = [&](float x, float y, const char *txt)
    {
        glRasterPos2f(x, y);
        for (const char *p = txt; *p; ++p)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *p);
    };
    glColor3f(0.98f, 0.90f, 0.50f);
    float tOff = 6.0f;
    drawText(hudX - 3.0f, hudY + radiusOuter + tOff, "N");
    drawText(hudX + radiusOuter + tOff, hudY - 4.0f, "E");
    drawText(hudX - 3.5f, hudY - radiusOuter - (tOff + 8.0f), "S");
    drawText(hudX - radiusOuter - (tOff + 8.0f), hudY - 4.0f, "W");

    // Leve highlight (brilho) no aro
    glColor4f(1.0f, 1.0f, 1.0f, 0.25f);
    glBegin(GL_LINE_STRIP);
    for (int i = -segments / 6; i <= segments / 6; ++i)
    {
        float a = (float)i / segments * 2.0f * 3.1415926f - 2.2f; // arco superior-esquerdo
        glVertex2f(hudX + std::cos(a) * (radiusOuter - 1.0f), hudY + std::sin(a) * (radiusOuter - 1.0f));
    }
    glEnd();

    // Calcula direção da seta
    float ang = 0.0f;
    bool temDirecao = false;
    if (pcol > -50 && prow > -50)
    {
        // Se temos pathCells, usa próximo passo; senão, aponta para o objetivo
        if (!pathCells.empty())
        {
            // encontra índice do jogador no caminho
            int idx = -1;
            for (int i = 0; i < (int)pathCells.size(); ++i)
            {
                if (pathCells[i].first == pcol && pathCells[i].second == prow)
                {
                    idx = i;
                    break;
                }
            }
            int nx = fimCol, ny = fimRow;
            if (idx >= 0 && idx + 1 < (int)pathCells.size())
            {
                nx = pathCells[idx + 1].first;
                ny = pathCells[idx + 1].second;
            }
            else if (idx == -1 && pathCells.size() >= 2)
            {
                // se o primeiro nó já é o próximo
                nx = pathCells[1].first;
                ny = pathCells[1].second;
            }
            float dx = static_cast<float>(nx - pcol);
            float dy = static_cast<float>(-(ny - prow)); // invertido para coordenadas de mundo (y pra cima)
            if (dx != 0.0f || dy != 0.0f)
            {
                ang = std::atan2(dy, dx);
                temDirecao = true;
            }
        }
        else
        {
            float dx = static_cast<float>(fimCol - pcol);
            float dy = static_cast<float>(-(fimRow - prow));
            if (dx != 0.0f || dy != 0.0f)
            {
                ang = std::atan2(dy, dx);
                temDirecao = true;
            }
        }
    }

    if (temDirecao)
    {
        // Parâmetros da seta
        const float arrowLen = 18.0f;
        const float arrowWidth = 10.0f;
        float cx = hudX, cy = hudY;
        float tipX = cx + std::cos(ang) * (arrowLen + 4.0f);
        float tipY = cy + std::sin(ang) * (arrowLen + 4.0f);
        float baseX = cx - std::cos(ang) * 6.0f;
        float baseY = cy - std::sin(ang) * 6.0f;
        float leftAng = ang + 2.6f; // abas da ponta
        float rightAng = ang - 2.6f;
        float leftX = tipX + std::cos(leftAng) * arrowWidth;
        float leftY = tipY + std::sin(leftAng) * arrowWidth;
        float rightX = tipX + std::cos(rightAng) * arrowWidth;
        float rightY = tipY + std::sin(rightAng) * arrowWidth;

        // haste com contorno preto
        glColor3f(0.0f, 0.0f, 0.0f);
        glLineWidth(5.0f);
        glBegin(GL_LINES);
        glVertex2f(baseX, baseY);
        glVertex2f(tipX, tipY);
        glEnd();
        glLineWidth(1.0f);
        glColor3f(0.90f, 0.12f, 0.12f); // seta vermelha
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glVertex2f(baseX, baseY);
        glVertex2f(tipX, tipY);
        glEnd();
        glLineWidth(1.0f);

        // ponta com contorno
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(tipX, tipY);
        glVertex2f(leftX, leftY);
        glVertex2f(rightX, rightY);
        glEnd();
        glColor3f(0.90f, 0.12f, 0.12f); // ponta vermelha
        glBegin(GL_TRIANGLES);
        glVertex2f(tipX - std::cos(ang) * 1.5f, tipY - std::sin(ang) * 1.5f);
        glVertex2f(leftX + std::cos(ang) * 1.5f, leftY + std::sin(ang) * 1.5f);
        glVertex2f(rightX + std::cos(ang) * 1.5f, rightY + std::sin(ang) * 1.5f);
        glEnd();
    }
}

bool Fase2::verificarVitoria(const Jogador &jogador)
{
    float centroX = fimCol * tamanhoCelula + tamanhoCelula / 2;
    float centroY = (alturaLabirinto - 1 - fimRow) * tamanhoCelula + tamanhoCelula / 2;
    CaixaColisao cj = jogador.getCaixaColisao();
    float dx = (cj.x + cj.largura / 2) - centroX;
    float dy = (cj.y + cj.altura / 2) - centroY;
    return (dx * dx + dy * dy) < (tamanhoCelula * tamanhoCelula / 4);
}

bool Fase2::verificarColisao(float x, float y)
{
    float larguraJog = tamanhoCelula * 0.66f;
    float alturaJog = tamanhoCelula * 0.66f;
    if (jogadorRef)
    {
        CaixaColisao cj = jogadorRef->getCaixaColisao();
        larguraJog = cj.largura;
        alturaJog = cj.altura;
    }
    float offsetsX[] = {0.0f, larguraJog};
    float offsetsY[] = {0.0f, alturaJog};
    for (float ox : offsetsX)
    {
        for (float oy : offsetsY)
        {
            int col = static_cast<int>((x + ox) / tamanhoCelula);
            int row = (alturaLabirinto - 1) - static_cast<int>((y + oy) / tamanhoCelula);
            if (row < 0 || row >= alturaLabirinto || col < 0 || col >= larguraLabirinto)
                return true;
            if (mapaLabirinto[row][col] == 'X')
                return true;
        }
    }
    return false;
}

void Fase2::carregar()
{
    // Compatibilidade antiga: sem efeito
}
