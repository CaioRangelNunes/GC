#include "Jogador.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath> // usado futuramente para normalizar vetores ou cálculos adicionais

#include "PlayerVariant.h"

Jogador::Jogador(float spawnX, float spawnY, float largura, float altura)
    : m_x(spawnX), m_y(spawnY),
      m_largura(largura), m_altura(altura),
      m_spawnX(spawnX), m_spawnY(spawnY),
      m_velocidadeBase(150.0f), // 150 pixels por segundo
      m_vidaMaxima(100),
      m_vida(100),
      m_movendoCima(false),
      m_movendoBaixo(false),
      m_movendoEsquerda(false),
      m_movendoDireita(false)
{
}

void Jogador::atualizar(float deltaTime)
{
    if (!estaVivo())
    {
        return;
    }

    // Atualiza flags de movimento: cada direção exclusiva (prioriza ordem vertical > horizontal)
    float velX = 0.0f;
    float velY = 0.0f;

    if (m_movendoCima)
    {
        velY = m_velocidadeBase;
    }
    else if (m_movendoBaixo)
    {
        velY = -m_velocidadeBase;
    }
    else if (m_movendoEsquerda)
    {
        velX = -m_velocidadeBase;
    }
    else if (m_movendoDireita)
    {
        velX = m_velocidadeBase;
    }

    // Aplica deslocamento proporcional ao deltaTime (movimento uniforme independente de FPS)
    m_x += velX * deltaTime;
    m_y += velY * deltaTime;

    // Colisão com paredes é verificada externamente (Jogo.cpp) para manter responsabilidade separada
}

void Jogador::desenhar() const
{
    float x = m_x;
    float y = m_y;
    float w = m_largura;
    float h = m_altura;

    // Calcula proporções do sprite para desenhar segmentos (facilita alterar estilo só mudando fatores)
    float headH = h * 0.22f;
    float torsoH = h * 0.42f;
    float legsH = h - headH - torsoH;

    // Define cores iniciais; variantes ajustam camisa/calça; morte escurece paleta
    float skinR = 0.80f, skinG = 0.66f, skinB = 0.52f;
    // Cores base da variante (variação por PlayerVariant)
    extern PlayerVariant g_selectedVariant; // definido em Jogo.cpp

    float shirtR = 0.1f, shirtG = 0.45f, shirtB = 0.9f;   // PADRAO azul
    float pantsR = 0.05f, pantsG = 0.18f, pantsB = 0.35f; // jeans escuro

    switch (g_selectedVariant)
    {
    case PlayerVariant::PADRAO:
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
    if (!estaVivo())
    {
        // tom mais escuro / avermelhado
        shirtR = 0.4f;
        shirtG = 0.1f;
        shirtB = 0.1f;
        pantsR = 0.2f;
        pantsG = 0.05f;
        pantsB = 0.05f;
    }

    // Desenha cabeça centralizada sobre torso
    float headW = w * 0.6f;
    float headX = x + (w - headW) / 2.0f;
    float headY = y + legsH + torsoH;
    glColor3f(skinR, skinG, skinB);
    glBegin(GL_QUADS);
    glVertex2f(headX, headY);
    glVertex2f(headX + headW, headY);
    glVertex2f(headX + headW, headY + headH);
    glVertex2f(headX, headY + headH);
    glEnd();

    // Desenha torso como bloco principal de cor da camisa
    float torsoY = y + legsH;
    glColor3f(shirtR, shirtG, shirtB);
    glBegin(GL_QUADS);
    glVertex2f(x + w * 0.1f, torsoY);
    glVertex2f(x + w * 0.9f, torsoY);
    glVertex2f(x + w * 0.9f, torsoY + torsoH);
    glVertex2f(x + w * 0.1f, torsoY + torsoH);
    glEnd();

    // Desenha duas pernas separadas para dar aparência de volume simples
    float legW = w * 0.42f;
    glColor3f(pantsR, pantsG, pantsB);
    // perna esquerda
    glBegin(GL_QUADS);
    glVertex2f(x + w * 0.08f, y);
    glVertex2f(x + w * 0.08f + legW, y);
    glVertex2f(x + w * 0.08f + legW, y + legsH);
    glVertex2f(x + w * 0.08f, y + legsH);
    glEnd();
    // perna direita
    glBegin(GL_QUADS);
    glVertex2f(x + w * 0.5f, y);
    glVertex2f(x + w * 0.5f + legW, y);
    glVertex2f(x + w * 0.5f + legW, y + legsH);
    glVertex2f(x + w * 0.5f, y + legsH);
    glEnd();

    // Sapatos: pequena barra inferior escura
    glColor3f(0.05f, 0.05f, 0.05f);
    float shoeH = legsH * 0.18f;
    glBegin(GL_QUADS);
    glVertex2f(x + w * 0.08f, y);
    glVertex2f(x + w * 0.08f + legW, y);
    glVertex2f(x + w * 0.08f + legW, y + shoeH);
    glVertex2f(x + w * 0.08f, y + shoeH);
    glEnd();

    // Acessórios: incrementam identidade visual sem alterar hitbox
    switch (g_selectedVariant)
    {
    case PlayerVariant::PADRAO:
    {
        // Cinto central
        glColor3f(0.3f, 0.2f, 0.05f);
        float beltY = torsoY + torsoH * 0.45f;
        glBegin(GL_QUADS);
        glVertex2f(x + w * 0.1f, beltY);
        glVertex2f(x + w * 0.9f, beltY);
        glVertex2f(x + w * 0.9f, beltY + torsoH * 0.08f);
        glVertex2f(x + w * 0.1f, beltY + torsoH * 0.08f);
        glEnd();
    }
    break;
    case PlayerVariant::VERMELHO:
    {
        // Headband (faixa) + fita lateral
        glColor3f(1.0f, 0.9f, 0.2f);
        float bandH = headH * 0.25f;
        glBegin(GL_QUADS);
        glVertex2f(headX, headY + headH * 0.55f);
        glVertex2f(headX + headW, headY + headH * 0.55f);
        glVertex2f(headX + headW, headY + headH * 0.55f + bandH);
        glVertex2f(headX, headY + headH * 0.55f + bandH);
        glEnd();
        // Fita lateral simulada como pequeno retângulo inclinado
        glBegin(GL_QUADS);
        glVertex2f(headX + headW * 0.75f, headY + headH * 0.55f);
        glVertex2f(headX + headW * 0.75f + bandH * 0.8f, headY + headH * 0.55f);
        glVertex2f(headX + headW * 0.75f + bandH * 0.6f, headY + headH * 0.55f - bandH * 0.9f);
        glVertex2f(headX + headW * 0.75f, headY + headH * 0.55f - bandH * 0.9f);
        glEnd();
    }
    break;
    case PlayerVariant::VERDE:
    {
        // Cabelo espigado: 3 triângulos acima da cabeça
        glColor3f(0.1f, 0.5f, 0.12f);
        float spikeBaseY = headY + headH;
        float spikeW = headW / 4.0f;
        for (int i = 0; i < 3; i++)
        {
            float sx = headX + i * spikeW + spikeW * 0.2f;
            glBegin(GL_TRIANGLES);
            glVertex2f(sx, spikeBaseY);
            glVertex2f(sx + spikeW * 0.6f, spikeBaseY);
            glVertex2f(sx + spikeW * 0.3f, spikeBaseY + headH * 0.9f);
            glEnd();
        }
    }
    break;
    case PlayerVariant::ROXO:
    {
        // Capa trapezoidal atrás do torso
        glColor3f(0.35f, 0.10f, 0.5f);
        float capeTop = torsoY + torsoH * 0.95f;
        float capeBottom = y + legsH * 0.3f;
        glBegin(GL_QUADS);
        glVertex2f(x + w * 0.15f, capeTop);
        glVertex2f(x + w * 0.85f, capeTop);
        glVertex2f(x + w * 0.70f, capeBottom);
        glVertex2f(x + w * 0.30f, capeBottom);
        glEnd();
        // Visor retangular frontal
        glColor3f(0.85f, 0.85f, 1.0f);
        float visorY = headY + headH * 0.35f;
        glBegin(GL_QUADS);
        glVertex2f(headX + headW * 0.1f, visorY);
        glVertex2f(headX + headW * 0.9f, visorY);
        glVertex2f(headX + headW * 0.9f, visorY + headH * 0.22f);
        glVertex2f(headX + headW * 0.1f, visorY + headH * 0.22f);
        glEnd();
    }
    break;
    }
    glBegin(GL_QUADS);
    glVertex2f(x + w * 0.5f, y);
    glVertex2f(x + w * 0.5f + legW, y);
    glVertex2f(x + w * 0.5f + legW, y + shoeH);
    glVertex2f(x + w * 0.5f, y + shoeH);
    glEnd();
}

// Aplica dano (clampa em zero); evita valores negativos de vida
void Jogador::sofrerDano(int quantidade)
{
    if (estaVivo())
    {
        m_vida -= quantidade;
        if (m_vida < 0)
        {
            m_vida = 0;
        }
    }
}

void Jogador::reiniciarPosicao()
{
    m_x = m_spawnX;
    m_y = m_spawnY;
    m_vida = m_vidaMaxima;
    m_movendoCima = false;
    m_movendoBaixo = false;
    m_movendoEsquerda = false;
    m_movendoDireita = false;
}

// Retorna caixa axis-aligned usada para colisões com paredes/espinhos
CaixaColisao Jogador::getCaixaColisao() const
{
    return {m_x, m_y, m_largura, m_altura};
}

void Jogador::setMovendoParaCima(bool movendo) { m_movendoCima = movendo; }
void Jogador::setMovendoParaBaixo(bool movendo) { m_movendoBaixo = movendo; }
void Jogador::setMovendoParaEsquerda(bool movendo) { m_movendoEsquerda = movendo; }
void Jogador::setMovendoParaDireita(bool movendo) { m_movendoDireita = movendo; }

// Verifica colisão entre dois retângulos axis-aligned
bool Jogador::checkCollision(const CaixaColisao &box1, const CaixaColisao &box2)
{
    bool colisaoX = (box1.x < box2.x + box2.largura) &&
                    (box1.x + box1.largura > box2.x);

    bool colisaoY = (box1.y < box2.y + box2.altura) &&
                    (box1.y + box1.altura > box2.y);
    return colisaoX && colisaoY;
}