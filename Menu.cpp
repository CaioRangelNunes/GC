#include "Menu.h"
#include "PlayerVariant.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

// Desenha texto usando fonte bitmap do GLUT (coordenadas já em ortho 2D)
static void drawText(float x, float y, const std::string &text, void *font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos2f(x, y);
    for (char c : text)
    {
        glutBitmapCharacter(font, c);
    }
}

extern int gWindowWidth;
extern int gWindowHeight;
void MenuPrincipal::inicializar()
{
    botoes.clear();
    // Calcula layout de botões baseado nas dimensões atuais da janela
    float bw = 300.0f;
    float bh = 60.0f;
    float buttonGap = 18.0f;
    float centerX = gWindowWidth * 0.5f;
    float centerY = gWindowHeight * 0.55f; // desloca para cima para abrir espaço à seleção de personagem
    float baseX = centerX - bw / 2.0f;
    float startY = centerY + bh;

    BotaoUI iniciar{baseX, startY, bw, bh, "Iniciar", nullptr, false};
    BotaoUI instrucoes{baseX, startY - (bh + buttonGap), bw, bh, "Como Jogar", nullptr, false};
    BotaoUI sair{baseX, startY - 2 * (bh + buttonGap), bw, bh, "Sair", nullptr, false};

    botoes.push_back(iniciar);
    botoes.push_back(instrucoes);
    botoes.push_back(sair);

    // Área de seleção de personagem: quatro slots alinhados na parte inferior
    float miniW = 72.0f;
    float gap = 22.0f;
    float selY = gWindowHeight * 0.16f;
    // Reduz tamanho das miniaturas se largura insuficiente
    if (gWindowWidth < 4 * miniW + 3 * gap + 120)
    {
        miniW = std::max(56.0f, (gWindowWidth - 120 - 3 * gap) / 4.0f);
    }
    float startSelX = centerX - (4 * miniW + 3 * gap) / 2.0f;

    botoes.push_back({startSelX + 0 * (miniW + gap), selY, miniW, miniW, "", nullptr, false});
    botoes.push_back({startSelX + 1 * (miniW + gap), selY, miniW, miniW, "", nullptr, false});
    botoes.push_back({startSelX + 2 * (miniW + gap), selY, miniW, miniW, "", nullptr, false});
    botoes.push_back({startSelX + 3 * (miniW + gap), selY, miniW, miniW, "", nullptr, false});
}

void MenuPrincipal::desenhar() const
{
    extern int gWindowWidth;
    extern int gWindowHeight;
    // Fundo do menu cobrindo tela inteira
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.05f, 0.05f, 0.08f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(gWindowWidth, 0);
    glVertex2f(gWindowWidth, gWindowHeight);
    glVertex2f(0, gWindowHeight);
    glEnd();

    // Título
    glColor3f(0.9f, 0.9f, 0.95f);
    drawText(gWindowWidth * 0.5f - 140.0f, gWindowHeight * 0.82f, "LABIRINTO DAS ARMADILHAS");

    extern PlayerVariant g_selectedVariant; // definido em Jogo.cpp
    for (size_t i = 0; i < botoes.size(); ++i)
    {
        const auto &b = botoes[i];
    // Hover: aplica leve escala para feedback visual
        float scale = b.hover ? 1.05f : 1.0f;
        float cx = b.x + b.w / 2.0f;
        float cy = b.y + b.h / 2.0f;
        glPushMatrix();
        glTranslatef(cx, cy, 0);
        glScalef(scale, scale, 1);
        glTranslatef(-cx, -cy, 0);
    bool isSelector = (i >= 3);
        if (isSelector)
        {
            // Fundo do slot (diferente da cor dos botões principais)
            glColor3f(0.12f, 0.12f, 0.16f);
            if (b.hover)
                glColor3f(0.18f, 0.18f, 0.24f);
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
    // Borda base (todos os botões e slots)
        glColor3f(0.6f, 0.6f, 0.7f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(b.x, b.y);
        glVertex2f(b.x + b.w, b.y);
        glVertex2f(b.x + b.w, b.y + b.h);
        glVertex2f(b.x, b.y + b.h);
        glEnd();
    // Miniatura: desenho simplificado do personagem dentro do slot
        if (isSelector)
        {
            PlayerVariant variantForButton = static_cast<PlayerVariant>(i - 3);
            float px = b.x + b.w * 0.15f;
            float py = b.y + b.h * 0.10f;
            float pw = b.w * 0.70f;
            float ph = b.h * 0.80f;
            // Calcula dimensões internas relativas ao slot
            float headH = ph * 0.28f;
            float torsoH = ph * 0.40f;
            float legsH = ph - headH - torsoH;
            float headW = pw * 0.55f;
            float headX = px + (pw - headW) / 2.0f;
            float headY = py + legsH + torsoH;
            // Define paleta da variante
            float skinR = 0.80f, skinG = 0.66f, skinB = 0.52f;
            float shirtR, shirtG, shirtB, pantsR, pantsG, pantsB;
            switch (variantForButton)
            {
            case PlayerVariant::PADRAO:
                shirtR = 0.1f;
                shirtG = 0.45f;
                shirtB = 0.9f;
                pantsR = 0.05f;
                pantsG = 0.18f;
                pantsB = 0.35f;
                break;
            case PlayerVariant::VERMELHO:
                shirtR = 0.75f;
                shirtG = 0.15f;
                shirtB = 0.15f;
                pantsR = 0.25f;
                pantsG = 0.05f;
                pantsB = 0.05f;
                break;
            case PlayerVariant::VERDE:
                shirtR = 0.15f;
                shirtG = 0.65f;
                shirtB = 0.25f;
                pantsR = 0.08f;
                pantsG = 0.30f;
                pantsB = 0.10f;
                break;
            case PlayerVariant::ROXO:
                shirtR = 0.45f;
                shirtG = 0.20f;
                shirtB = 0.65f;
                pantsR = 0.18f;
                pantsG = 0.07f;
                pantsB = 0.30f;
                break;
            }
            // Cabeça
            glColor3f(skinR, skinG, skinB);
            glBegin(GL_QUADS);
            glVertex2f(headX, headY);
            glVertex2f(headX + headW, headY);
            glVertex2f(headX + headW, headY + headH);
            glVertex2f(headX, headY + headH);
            glEnd();
            // Torso
            float torsoY = py + legsH;
            glColor3f(shirtR, shirtG, shirtB);
            glBegin(GL_QUADS);
            glVertex2f(px + pw * 0.15f, torsoY);
            glVertex2f(px + pw * 0.85f, torsoY);
            glVertex2f(px + pw * 0.85f, torsoY + torsoH);
            glVertex2f(px + pw * 0.15f, torsoY + torsoH);
            glEnd();
            // Pernas (duas para simular separação)
            float legW = pw * 0.38f;
            glColor3f(pantsR, pantsG, pantsB);
            glBegin(GL_QUADS); // esquerda
            glVertex2f(px + pw * 0.14f, py);
            glVertex2f(px + pw * 0.14f + legW, py);
            glVertex2f(px + pw * 0.14f + legW, py + legsH);
            glVertex2f(px + pw * 0.14f, py + legsH);
            glEnd();
            glBegin(GL_QUADS); // direita
            glVertex2f(px + pw * 0.50f, py);
            glVertex2f(px + pw * 0.50f + legW, py);
            glVertex2f(px + pw * 0.50f + legW, py + legsH);
            glVertex2f(px + pw * 0.50f, py + legsH);
            glEnd();
            // Acessórios específicos
            switch (variantForButton)
            {
            case PlayerVariant::PADRAO: {
                glColor3f(0.3f, 0.2f, 0.05f);
                float beltY = torsoY + torsoH * 0.45f;
                glBegin(GL_QUADS);
                glVertex2f(px + pw * 0.15f, beltY);
                glVertex2f(px + pw * 0.85f, beltY);
                glVertex2f(px + pw * 0.85f, beltY + torsoH * 0.08f);
                glVertex2f(px + pw * 0.15f, beltY + torsoH * 0.08f);
                glEnd();
            }
            break;
            case PlayerVariant::VERMELHO: {
                glColor3f(1.0f, 0.9f, 0.2f);
                float bandH = headH * 0.25f;
                glBegin(GL_QUADS);
                glVertex2f(headX, headY + headH * 0.55f);
                glVertex2f(headX + headW, headY + headH * 0.55f);
                glVertex2f(headX + headW, headY + headH * 0.55f + bandH);
                glVertex2f(headX, headY + headH * 0.55f + bandH);
                glEnd();
            }
            break;
            case PlayerVariant::VERDE: {
                glColor3f(0.1f, 0.5f, 0.12f);
                float spikeBaseY = headY + headH;
                float spikeW = headW / 4.0f;
                for (int si = 0; si < 3; ++si)
                {
                    float sx = headX + si * spikeW + spikeW * 0.2f;
                    glBegin(GL_TRIANGLES);
                    glVertex2f(sx, spikeBaseY);
                    glVertex2f(sx + spikeW * 0.6f, spikeBaseY);
                    glVertex2f(sx + spikeW * 0.3f, spikeBaseY + headH * 0.8f);
                    glEnd();
                }
            }
            break;
            case PlayerVariant::ROXO: {
                glColor3f(0.35f, 0.10f, 0.5f);
                float capeTop = torsoY + torsoH * 0.95f;
                float capeBottom = py + legsH * 0.3f;
                glBegin(GL_QUADS);
                glVertex2f(px + pw * 0.15f, capeTop);
                glVertex2f(px + pw * 0.85f, capeTop);
                glVertex2f(px + pw * 0.70f, capeBottom);
                glVertex2f(px + pw * 0.30f, capeBottom);
                glEnd();
            }
            break;
            }
        }
    // Destaque: borda externa dourada no personagem selecionado
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
    // Texto centralizado nos botões (slots não têm texto)
        glColor3f(1, 1, 1);
        // Texto
        if (!isSelector)
        {
            drawText(b.x + b.w / 2 - (b.texto.size() * 9) / 2, b.y + b.h / 2 - 6, b.texto);
        }
        glPopMatrix();
    }

    // Legenda indicando ação de seleção
    glColor3f(0.85f, 0.85f, 0.9f);
    // Usa posição do primeiro slot
    if (botoes.size() >= 7)
    {
        const BotaoUI &slot0 = botoes[3];
        drawText(slot0.x - 20.0f, slot0.y + slot0.h + 18.0f, "Selecione seu personagem");
    }
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
    extern PlayerVariant g_selectedVariant;
    for (size_t i = 0; i < botoes.size(); ++i)
    {
        auto &b = botoes[i];
        if (mouseX >= b.x && mouseX <= b.x + b.w && mouseY >= b.y && mouseY <= b.y + b.h)
        {
            if (i >= 3) {
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

// Tela Instrucoes
extern int gWindowWidth;
extern int gWindowHeight;
void TelaInstrucoes::inicializar()
{
    botoes.clear();
    BotaoUI voltar{gWindowWidth * 0.5f - 120.0f, 140.0f, 240.0f, 55.0f, "Voltar ao Menu", nullptr, false};
    botoes.push_back(voltar);
}

void TelaInstrucoes::desenhar() const
{
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.07f, 0.07f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(gWindowWidth, 0);
    glVertex2f(gWindowWidth, gWindowHeight);
    glVertex2f(0, gWindowHeight);
    glEnd();

    glColor3f(0.9f, 0.9f, 0.95f);
    drawText(gWindowWidth * 0.5f - 100.0f, gWindowHeight - 128.0f, "COMO JOGAR");

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
