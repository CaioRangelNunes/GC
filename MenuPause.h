#ifndef MENU_PAUSE_H
#define MENU_PAUSE_H
#include <vector>
#include <functional>
#include <string>
#include <GL/glew.h>
#include <GL/freeglut.h>

struct BotaoPause
{
    float x, y, w, h;
    std::string texto;
    std::function<void()> onClick;
    bool hover = false;
};

class MenuPause
{
public:
    void inicializar(int screenW, int screenH)
    {
        botoes.clear();
        float bw = 340.0f, bh = 70.0f, gap = 22.0f;
        float baseX = (screenW - bw) / 2.0f;
        float topY = screenH * 0.65f;
        botoes.push_back({baseX, topY, bw, bh, "Continuar", nullptr});                   // 0
        botoes.push_back({baseX, topY - (bh + gap), bw, bh, "Menu Principal", nullptr}); // 1
    }
    void desenhar() const
    {
        int sw = glutGet(GLUT_WINDOW_WIDTH), sh = glutGet(GLUT_WINDOW_HEIGHT);
        glColor3f(0.06f, 0.06f, 0.09f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(sw, 0);
        glVertex2f(sw, sh);
        glVertex2f(0, sh);
        glEnd();
        glColor3f(0.92f, 0.92f, 0.96f);
        drawText(sw * 0.5f - 140.0f, sh * 0.82f, "PAUSA");
        for (const auto &b : botoes)
        {
            float scale = b.hover ? 1.06f : 1.0f;
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
            glColor3f(0.60f, 0.60f, 0.70f);
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
    void atualizarHover(int mx, int my)
    {
        for (auto &b : botoes)
            b.hover = (mx >= b.x && mx <= b.x + b.w && my >= b.y && my <= b.y + b.h);
    }
    bool processarClick(int mx, int my)
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
    std::vector<BotaoPause> &getBotoes() { return botoes; }

private:
    std::vector<BotaoPause> botoes;
    static void drawText(float x, float y, const std::string &t)
    {
        glRasterPos2f(x, y);
        for (char c : t)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
};

#endif // MENU_PAUSE_H
