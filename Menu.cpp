// Implementação limpa do sistema de menu
#include "Menu.h"
#include "PlayerVariant.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

extern PlayerVariant g_selectedVariant;

static void drawText(float x, float y, const std::string &text, void *font = GLUT_BITMAP_HELVETICA_18)
{
    glRasterPos2f(x, y);
    for (char c : text)
        glutBitmapCharacter(font, c);
}

// ---------------- Menu Principal ----------------
void MenuPrincipal::inicializar(int screenW, int screenH)
{
    botoes.clear();
    float bw = (screenW < 700 ? screenW * 0.6f : 420.0f);
    float bh = 70.0f;
    float gap = 20.0f;
    float baseX = (screenW - bw) / 2.0f;
    float topY = screenH * 0.70f;
    botoes.push_back({baseX, topY, bw, bh, "Iniciar", nullptr, false, false});
    botoes.push_back({baseX, topY - (bh + gap), bw, bh, "Como Jogar", nullptr, false, false});
    botoes.push_back({baseX, topY - 2 * (bh + gap), bw, bh, "Sair", nullptr, false, false});
    float miniW = 90.0f;
    float selGap = 26.0f;
    float selY = screenH * 0.18f;
    float totalW = 4 * miniW + 3 * selGap;
    float startX = (screenW - totalW) / 2.0f;
    for (int i = 0; i < 4; ++i)
        botoes.push_back({startX + i * (miniW + selGap), selY, miniW, miniW, "", nullptr, false, true});
}
void MenuPrincipal::desenhar() const
{
    int sw = glutGet(GLUT_WINDOW_WIDTH), sh = glutGet(GLUT_WINDOW_HEIGHT);
    glColor3f(0.05f, 0.05f, 0.08f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(sw, 0);
    glVertex2f(sw, sh);
    glVertex2f(0, sh);
    glEnd();
    glColor3f(0.92f, 0.92f, 0.96f);
    drawText(sw * 0.5f - 160.0f, sh * 0.86f, "LABIRINTO DAS ARMADILHAS");
    for (size_t i = 0; i < botoes.size(); ++i)
    {
        const auto &b = botoes[i];
        float scale = b.hover ? 1.05f : 1.0f;
        float cx = b.x + b.w / 2, cy = b.y + b.h / 2;
        glPushMatrix();
        glTranslatef(cx, cy, 0);
        glScalef(scale, scale, 1);
        glTranslatef(-cx, -cy, 0);
        if (b.isSelector)
            glColor3f(b.hover ? 0.18f : 0.12f, b.hover ? 0.18f : 0.12f, b.hover ? 0.24f : 0.16f);
        else
            glColor3f(b.hover ? 0.28f : 0.18f, b.hover ? 0.50f : 0.35f, b.hover ? 0.90f : 0.65f);
        glBegin(GL_QUADS);
        glVertex2f(b.x, b.y);
        glVertex2f(b.x + b.w, b.y);
        glVertex2f(b.x + b.w, b.y + b.h);
        glVertex2f(b.x, b.y + b.h);
        glEnd();
        glColor3f(0.6f, 0.6f, 0.7f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(b.x, b.y);
        glVertex2f(b.x + b.w, b.y);
        glVertex2f(b.x + b.w, b.y + b.h);
        glVertex2f(b.x, b.y + b.h);
        glEnd();
        if (b.isSelector)
        {
            PlayerVariant variant = static_cast<PlayerVariant>(i - 3);
            float px = b.x + b.w * 0.15f, py = b.y + b.h * 0.12f, pw = b.w * 0.70f, ph = b.h * 0.76f;
            float headH = ph * 0.28f, torsoH = ph * 0.40f, legsH = ph - headH - torsoH, headW = pw * 0.55f, headX = px + (pw - headW) / 2.0f, headY = py + legsH + torsoH;
            float skinR, skinG, skinB, hairR, hairG, hairB, shirtR, shirtG, shirtB, pantsR, pantsG, pantsB;
            switch (variant)
            {
            case PlayerVariant::PADRAO:
                skinR = 0.85f;
                skinG = 0.72f;
                skinB = 0.58f;
                hairR = 0.25f;
                hairG = 0.18f;
                hairB = 0.08f;
                shirtR = 0.10f;
                shirtG = 0.45f;
                shirtB = 0.90f;
                pantsR = 0.05f;
                pantsG = 0.18f;
                pantsB = 0.35f;
                break;
            case PlayerVariant::VERMELHO:
                skinR = 0.90f;
                skinG = 0.78f;
                skinB = 0.60f;
                hairR = 0.80f;
                hairG = 0.70f;
                hairB = 0.30f;
                shirtR = 0.75f;
                shirtG = 0.15f;
                shirtB = 0.15f;
                pantsR = 0.28f;
                pantsG = 0.06f;
                pantsB = 0.06f;
                break;
            case PlayerVariant::VERDE:
                skinR = 0.78f;
                skinG = 0.64f;
                skinB = 0.50f;
                hairR = 0.12f;
                hairG = 0.35f;
                hairB = 0.15f;
                shirtR = 0.15f;
                shirtG = 0.65f;
                shirtB = 0.25f;
                pantsR = 0.10f;
                pantsG = 0.32f;
                pantsB = 0.12f;
                break;
            case PlayerVariant::ROXO:
                skinR = 0.82f;
                skinG = 0.70f;
                skinB = 0.55f;
                hairR = 0.32f;
                hairG = 0.15f;
                hairB = 0.40f;
                shirtR = 0.45f;
                shirtG = 0.20f;
                shirtB = 0.65f;
                pantsR = 0.20f;
                pantsG = 0.07f;
                pantsB = 0.32f;
                break;
            }
            glColor3f(skinR, skinG, skinB);
            glBegin(GL_QUADS);
            glVertex2f(headX, headY);
            glVertex2f(headX + headW, headY);
            glVertex2f(headX + headW, headY + headH);
            glVertex2f(headX, headY + headH);
            glEnd();
            glColor3f(hairR, hairG, hairB);
            glBegin(GL_QUADS);
            glVertex2f(headX, headY + headH * 0.70f);
            glVertex2f(headX + headW, headY + headH * 0.70f);
            glVertex2f(headX + headW, headY + headH);
            glVertex2f(headX, headY + headH);
            glEnd();
            float torsoY = py + legsH;
            glColor3f(shirtR, shirtG, shirtB);
            glBegin(GL_QUADS);
            glVertex2f(px + pw * 0.15f, torsoY);
            glVertex2f(px + pw * 0.85f, torsoY);
            glVertex2f(px + pw * 0.85f, torsoY + torsoH);
            glVertex2f(px + pw * 0.15f, torsoY + torsoH);
            glEnd();
            float legW = pw * 0.38f;
            glColor3f(pantsR, pantsG, pantsB);
            glBegin(GL_QUADS);
            glVertex2f(px + pw * 0.14f, py);
            glVertex2f(px + pw * 0.14f + legW, py);
            glVertex2f(px + pw * 0.14f + legW, py + legsH);
            glVertex2f(px + pw * 0.14f, py + legsH);
            glEnd();
            glBegin(GL_QUADS);
            glVertex2f(px + pw * 0.50f, py);
            glVertex2f(px + pw * 0.50f + legW, py);
            glVertex2f(px + pw * 0.50f + legW, py + legsH);
            glVertex2f(px + pw * 0.50f, py + legsH);
            glEnd();
            switch (variant)
            {
            case PlayerVariant::PADRAO:
                glColor3f(0.3f, 0.2f, 0.05f);
                {
                    float beltY = torsoY + torsoH * 0.55f;
                    glBegin(GL_QUADS);
                    glVertex2f(px + pw * 0.15f, beltY);
                    glVertex2f(px + pw * 0.85f, beltY);
                    glVertex2f(px + pw * 0.85f, beltY + torsoH * 0.08f);
                    glVertex2f(px + pw * 0.15f, beltY + torsoH * 0.08f);
                    glEnd();
                }
                break;
            case PlayerVariant::VERMELHO:
                glColor3f(1.0f, 0.9f, 0.2f);
                {
                    float bandH = headH * 0.25f;
                    glBegin(GL_QUADS);
                    glVertex2f(headX, headY + headH * 0.45f);
                    glVertex2f(headX + headW, headY + headH * 0.45f);
                    glVertex2f(headX + headW, headY + headH * 0.45f + bandH);
                    glVertex2f(headX, headY + headH * 0.45f + bandH);
                    glEnd();
                }
                break;
            case PlayerVariant::VERDE:
                glColor3f(0.1f, 0.5f, 0.12f);
                {
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
            case PlayerVariant::ROXO:
                glColor3f(0.35f, 0.10f, 0.5f);
                {
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
            if (variant == g_selectedVariant)
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
        else
        {
            glColor3f(1, 1, 1);
            drawText(b.x + b.w / 2 - (b.texto.size() * 9) / 2, b.y + b.h / 2 - 6, b.texto);
        }
        glPopMatrix();
    }
    if (botoes.size() >= 7)
    {
        const auto &fs = botoes[3];
        glColor3f(0.85f, 0.85f, 0.9f);
        drawText(fs.x - 40.0f, fs.y + fs.h + 22.0f, "Selecione seu personagem");
    }
}
void MenuPrincipal::atualizarHover(int mx, int my)
{
    for (auto &b : botoes)
        b.hover = (mx >= b.x && mx <= b.x + b.w && my >= b.y && my <= b.y + b.h);
}
bool MenuPrincipal::processarClick(int mx, int my)
{
    for (size_t i = 0; i < botoes.size(); ++i)
    {
        auto &b = botoes[i];
        if (mx >= b.x && mx <= b.x + b.w && my >= b.y && my <= b.y + b.h)
        {
            if (b.isSelector)
            {
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

// ---------------- Tela Instruções ----------------
void TelaInstrucoes::inicializar(int screenW, int screenH)
{
    botoes.clear();
    botoes.push_back({screenW * 0.5f - 130.0f, 140.0f, 260.0f, 60.0f, "Voltar", nullptr, false, false});
}
void TelaInstrucoes::desenhar() const
{
    int sw = glutGet(GLUT_WINDOW_WIDTH), sh = glutGet(GLUT_WINDOW_HEIGHT);
    glColor3f(0.07f, 0.07f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(sw, 0);
    glVertex2f(sw, sh);
    glVertex2f(0, sh);
    glEnd();
    glColor3f(0.9f, 0.9f, 0.95f);
    drawText(sw * 0.5f - 110.0f, sh - 130.0f, "COMO JOGAR");
    glColor3f(0.85f, 0.85f, 0.88f);
    drawText(160, 600, "Use WASD para mover; evite espinhos.");
    drawText(160, 570, "Chegue na celula F para vencer.");
    drawText(160, 540, "Selecione personagem no menu inicial.");
    drawText(160, 510, "Clique em 'Iniciar' para escolher a fase.");
    for (const auto &b : botoes)
    {
        float scale = b.hover ? 1.06f : 1.0f;
        float cx = b.x + b.w / 2, cy = b.y + b.h / 2;
        glPushMatrix();
        glTranslatef(cx, cy, 0);
        glScalef(scale, scale, 1);
        glTranslatef(-cx, -cy, 0);
        glColor3f(b.hover ? 0.30f : 0.22f, b.hover ? 0.55f : 0.45f, b.hover ? 0.30f : 0.18f);
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
void TelaInstrucoes::atualizarHover(int mx, int my)
{
    for (auto &b : botoes)
        b.hover = (mx >= b.x && mx <= b.x + b.w && my >= b.y && my <= b.y + b.h);
}
bool TelaInstrucoes::processarClick(int mx, int my)
{
    for (auto &b : botoes)
        if (mx >= b.x && mx <= b.x + b.w && my >= b.y && my <= b.y + b.h)
        {
            if (b.onClick)
                b.onClick();
            return true;
        }
    return false;
}

// ---------------- Tela Seleção de Fase ----------------
void TelaSelecaoFase::inicializar(int screenW, int screenH)
{
    botoes.clear();
    float bw = 260.0f, bh = 60.0f, gap = 18.0f;
    float baseX = screenW * 0.5f - bw / 2.0f;
    float startY = screenH * 0.65f;
    botoes.push_back({baseX, startY, bw, bh, "Fase 1", nullptr, false, false});
    botoes.push_back({baseX, startY - (bh + gap), bw, bh, "Fase 2", nullptr, false, false});
    botoes.push_back({baseX, startY - 2 * (bh + gap), bw, bh, "Fase 3", nullptr, false, false});
    botoes.push_back({baseX, startY - 3 * (bh + gap), bw, bh, "Voltar", nullptr, false, false});
}
void TelaSelecaoFase::desenhar() const
{
    int sw = glutGet(GLUT_WINDOW_WIDTH), sh = glutGet(GLUT_WINDOW_HEIGHT);
    glColor3f(0.05f, 0.05f, 0.08f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(sw, 0);
    glVertex2f(sw, sh);
    glVertex2f(0, sh);
    glEnd();
    glColor3f(0.9f, 0.9f, 0.95f);
    drawText(sw * 0.5f - 140.0f, sh - 130.0f, "SELECAO DE FASE");
    for (const auto &b : botoes)
    {
        float scale = b.hover ? 1.05f : 1.0f;
        float cx = b.x + b.w / 2, cy = b.y + b.h / 2;
        glPushMatrix();
        glTranslatef(cx, cy, 0);
        glScalef(scale, scale, 1);
        glTranslatef(-cx, -cy, 0);
        glColor3f(b.hover ? 0.28f : 0.18f, b.hover ? 0.50f : 0.35f, b.hover ? 0.90f : 0.65f);
        glBegin(GL_QUADS);
        glVertex2f(b.x, b.y);
        glVertex2f(b.x + b.w, b.y);
        glVertex2f(b.x + b.w, b.y + b.h);
        glVertex2f(b.x, b.y + b.h);
        glEnd();
        glColor3f(0.6f, 0.6f, 0.7f);
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
void TelaSelecaoFase::atualizarHover(int mx, int my)
{
    for (auto &b : botoes)
        b.hover = (mx >= b.x && mx <= b.x + b.w && my >= b.y && my <= b.y + b.h);
}
bool TelaSelecaoFase::processarClick(int mx, int my)
{
    for (auto &b : botoes)
        if (mx >= b.x && mx <= b.x + b.w && my >= b.y && my <= b.y + b.h)
        {
            if (b.onClick)
                b.onClick();
            return true;
        }
    return false;
}
