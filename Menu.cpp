#include "Menu.h"
#include "PlayerVariant.h" // acesso à enum e variável global
#include <GL/glew.h>
#include <GL/freeglut.h>

// Utilidade simples para desenhar texto (bitmap)
static void drawText(float x, float y, const std::string &text, void *font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos2f(x, y);
    for (char c : text)
    {
        glutBitmapCharacter(font, c);
    }
}

void MenuPrincipal::inicializar()
{
    botoes.clear();
    // Layout básico centralizado (valores de ortho serão tratados externamente; aqui assumimos 1024x768 base)
    float baseX = 1024.0f / 2 - 150.0f;
    float startY = 768.0f / 2 + 40.0f;
    float bw = 300.0f;
    float bh = 60.0f;

    // Linha principal de ações
    BotaoUI iniciar{baseX, startY, bw, bh, "Iniciar", nullptr, false};
    BotaoUI instrucoes{baseX, startY - 80.0f, bw, bh, "Como Jogar", nullptr, false};
    BotaoUI sair{baseX, startY - 160.0f, bw, bh, "Sair", nullptr, false};

    botoes.push_back(iniciar);
    botoes.push_back(instrucoes);
    botoes.push_back(sair);

    // Área de seleção de personagem (4 mini botões horizontais)
    float selY = startY - 260.0f;
    float miniW = 60.0f;
    float gap = 20.0f;
    float startSelX = 1024.0f / 2 - (4 * miniW + 3 * gap) / 2.0f;

    botoes.push_back({startSelX + 0 * (miniW + gap), selY, miniW, miniW, "P", nullptr, false});
    botoes.push_back({startSelX + 1 * (miniW + gap), selY, miniW, miniW, "R", nullptr, false});
    botoes.push_back({startSelX + 2 * (miniW + gap), selY, miniW, miniW, "V", nullptr, false});
    botoes.push_back({startSelX + 3 * (miniW + gap), selY, miniW, miniW, "X", nullptr, false});
}

void MenuPrincipal::desenhar() const
{
    // Fundo
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.05f, 0.05f, 0.08f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(1024, 0);
    glVertex2f(1024, 768);
    glVertex2f(0, 768);
    glEnd();

    // Título
    glColor3f(0.9f, 0.9f, 0.95f);
    drawText(1024.0f / 2 - 140.0f, 768.0f / 2 + 160.0f, "LABIRINTO DAS ARMADILHAS");

    extern PlayerVariant g_selectedVariant; // definido em Jogo.cpp
    for (size_t i = 0; i < botoes.size(); ++i)
    {
        const auto &b = botoes[i];
        // Efeito hover: cor mais clara e leve escala
        float scale = b.hover ? 1.05f : 1.0f;
        float cx = b.x + b.w / 2.0f;
        float cy = b.y + b.h / 2.0f;
        glPushMatrix();
        glTranslatef(cx, cy, 0);
        glScalef(scale, scale, 1);
        glTranslatef(-cx, -cy, 0);
        bool isSelector = (i >= 3); // índices 3..6 são seletores de personagem
        if (isSelector)
        {
            // Diferenciar cada variante pela cor de fundo
            switch (i - 3)
            {
            case 0:
                glColor3f(0.1f, 0.45f, 0.9f);
                break; // PADRAO
            case 1:
                glColor3f(0.75f, 0.15f, 0.15f);
                break; // VERMELHO
            case 2:
                glColor3f(0.15f, 0.65f, 0.25f);
                break; // VERDE
            case 3:
                glColor3f(0.45f, 0.20f, 0.65f);
                break; // ROXO
            }
            if (b.hover)
                glColor3f(0.9f, 0.9f, 0.3f);
        }
        else
        {
            if (b.hover)
                glColor3f(0.25f, 0.45f, 0.85f);
            else
                glColor3f(0.15f, 0.25f, 0.55f);
        }
        glBegin(GL_QUADS);
        glVertex2f(b.x, b.y);
        glVertex2f(b.x + b.w, b.y);
        glVertex2f(b.x + b.w, b.y + b.h);
        glVertex2f(b.x, b.y + b.h);
        glEnd();
        // Borda
        glColor3f(0.9f, 0.9f, 0.95f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(b.x, b.y);
        glVertex2f(b.x + b.w, b.y);
        glVertex2f(b.x + b.w, b.y + b.h);
        glVertex2f(b.x, b.y + b.h);
        glEnd();
        // Destaque se este seletor é o personagem atual
        if (isSelector)
        {
            PlayerVariant variantForButton = static_cast<PlayerVariant>(i - 3); // ordem mantém
            if (variantForButton == g_selectedVariant)
            {
                glColor3f(1.0f, 0.85f, 0.2f);
                glBegin(GL_LINE_LOOP);
                glVertex2f(b.x - 4, b.y - 4);
                glVertex2f(b.x + b.w + 4, b.y - 4);
                glVertex2f(b.x + b.w + 4, b.y + b.h + 4);
                glVertex2f(b.x - 4, b.y + b.h + 4);
                glEnd();
            }
        }
        // Texto centralizado
        glColor3f(1, 1, 1);
        // Texto
        drawText(b.x + b.w / 2 - (b.texto.size() * 9) / 2, b.y + b.h / 2 - 6, b.texto);
        glPopMatrix();
    }

    // Legenda seleção personagem
    glColor3f(0.85f, 0.85f, 0.9f);
    drawText(1024.0f / 2 - 140.0f, 768.0f / 2 - 300.0f, "Selecione seu personagem: P R V X");
}

void MenuPrincipal::atualizarHover(int mouseX, int mouseY)
{
    for (auto &b : botoes)
    {
        b.hover = (mouseX >= b.x && mouseX <= b.x + b.w && mouseY >= b.y && mouseY <= b.y + b.h);
    }
}

bool MenuPrincipal::processarClick(int mouseX, int mouseY)
{
    extern PlayerVariant g_selectedVariant; // global
    for (size_t i = 0; i < botoes.size(); ++i)
    {
        auto &b = botoes[i];
        if (mouseX >= b.x && mouseX <= b.x + b.w && mouseY >= b.y && mouseY <= b.y + b.h)
        {
            if (i >= 3)
            { // seletores
                g_selectedVariant = static_cast<PlayerVariant>(i - 3);
                return true;
            }
            if (b.onClick)
                b.onClick();
            return true;
        }
    }
    return false;
}

// ---------------- Tela Instrucoes -----------------
void TelaInstrucoes::inicializar()
{
    botoes.clear();
    BotaoUI voltar{1024.0f / 2 - 120.0f, 140.0f, 240.0f, 55.0f, "Voltar ao Menu", nullptr, false};
    botoes.push_back(voltar);
}

void TelaInstrucoes::desenhar() const
{
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.07f, 0.07f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(1024, 0);
    glVertex2f(1024, 768);
    glVertex2f(0, 768);
    glEnd();

    glColor3f(0.9f, 0.9f, 0.95f);
    drawText(1024.0f / 2 - 100.0f, 640.0f, "COMO JOGAR");

    glColor3f(0.85f, 0.85f, 0.88f);
    drawText(160.0f, 600.0f, "Este jogo e um labirinto de armadilhas dinamicas.");
    drawText(160.0f, 570.0f, "Escolha um dos quatro personagens no menu inicial.");
    drawText(160.0f, 540.0f, "Cada cor representa apenas estilo visual (por enquanto).");
    drawText(160.0f, 510.0f, "Use W A S D para mover o jogador pelo labirinto.");
    drawText(160.0f, 480.0f, "Evite espinhos que alternam ativos/inativos.");
    drawText(160.0f, 450.0f, "Chegue na celula F para concluir a fase. S e o inicio.");
    drawText(160.0f, 420.0f, "Pressione ESC para sair do jogo, M para voltar ao menu.");

    for (const auto &b : botoes)
    {
        float scale = b.hover ? 1.06f : 1.0f;
        float cx = b.x + b.w / 2.0f;
        float cy = b.y + b.h / 2.0f;
        glPushMatrix();
        glTranslatef(cx, cy, 0);
        glScalef(scale, scale, 1);
        glTranslatef(-cx, -cy, 0);
        if (b.hover)
            glColor3f(0.35f, 0.55f, 0.25f);
        else
            glColor3f(0.25f, 0.45f, 0.15f);
        glBegin(GL_QUADS);
        glVertex2f(b.x, b.y);
        glVertex2f(b.x + b.w, b.y);
        glVertex2f(b.x + b.w, b.y + b.h);
        glVertex2f(b.x, b.y + b.h);
        glEnd();
        glColor3f(0.95f, 0.95f, 0.95f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(b.x, b.y);
        glVertex2f(b.x + b.w, b.y);
        glVertex2f(b.x + b.w, b.y + b.h);
        glVertex2f(b.x, b.y + b.h);
        glEnd();
        glColor3f(1, 1, 1);
        drawText(b.x + b.w / 2 - (b.texto.size() * 9) / 2, b.y + b.h / 2 - 6, b.texto);
        glPopMatrix();
    }
}

void TelaInstrucoes::atualizarHover(int mouseX, int mouseY)
{
    for (auto &b : botoes)
    {
        b.hover = (mouseX >= b.x && mouseX <= b.x + b.w && mouseY >= b.y && mouseY <= b.y + b.h);
    }
}

bool TelaInstrucoes::processarClick(int mouseX, int mouseY)
{
    for (auto &b : botoes)
    {
        if (mouseX >= b.x && mouseX <= b.x + b.w && mouseY >= b.y && mouseY <= b.y + b.h)
        {
            if (b.onClick)
                b.onClick();
            return true;
        }
    }
    return false;
}
