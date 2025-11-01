#include "Jogador.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath> // Para futura normalização de vetor

#include "PlayerVariant.h"

// --- Construtor ---
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

// --- Loop de Jogo: Atualizar ---
void Jogador::atualizar(float deltaTime)
{
    if (!estaVivo())
    {
        return;
    }

    // 1. Calcular o vetor de velocidade (4 direções)
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

    // 2. Aplicar o movimento
    m_x += velX * deltaTime;
    m_y += velY * deltaTime;

    // A colisão com paredes será tratada no Jogo.cpp
}

// --- Loop de Jogo: Desenhar ---
void Jogador::desenhar() const
{
    // Desenha um personagem simples: cabeça (pele), camisa azul, calça jeans
    float x = m_x;
    float y = m_y;
    float w = m_largura;
    float h = m_altura;

    // Proporções
    float headH = h * 0.22f;
    float torsoH = h * 0.42f;
    float legsH = h - headH - torsoH;

    // Cores dependendo de vivo/morto
    float skinR = 1.0f, skinG = 0.85f, skinB = 0.7f;
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

    // Cabeça
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

    // Torso (camisa)
    float torsoY = y + legsH;
    glColor3f(shirtR, shirtG, shirtB);
    glBegin(GL_QUADS);
    glVertex2f(x + w * 0.1f, torsoY);
    glVertex2f(x + w * 0.9f, torsoY);
    glVertex2f(x + w * 0.9f, torsoY + torsoH);
    glVertex2f(x + w * 0.1f, torsoY + torsoH);
    glEnd();

    // Calças (duas pernas)
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

    // Sapatos
    glColor3f(0.05f, 0.05f, 0.05f);
    float shoeH = legsH * 0.18f;
    glBegin(GL_QUADS);
    glVertex2f(x + w * 0.08f, y);
    glVertex2f(x + w * 0.08f + legW, y);
    glVertex2f(x + w * 0.08f + legW, y + shoeH);
    glVertex2f(x + w * 0.08f, y + shoeH);
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(x + w * 0.5f, y);
    glVertex2f(x + w * 0.5f + legW, y);
    glVertex2f(x + w * 0.5f + legW, y + shoeH);
    glVertex2f(x + w * 0.5f, y + shoeH);
    glEnd();
}

// --- Ações de Jogo ---
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

// --- Getters ---
CaixaColisao Jogador::getCaixaColisao() const
{
    return {m_x, m_y, m_largura, m_altura};
}

// --- Setters de Movimento ---
void Jogador::setMovendoParaCima(bool movendo) { m_movendoCima = movendo; }
void Jogador::setMovendoParaBaixo(bool movendo) { m_movendoBaixo = movendo; }
void Jogador::setMovendoParaEsquerda(bool movendo) { m_movendoEsquerda = movendo; }
void Jogador::setMovendoParaDireita(bool movendo) { m_movendoDireita = movendo; }

// --- Função Auxiliar Estática de Colisão ---
bool Jogador::checkCollision(const CaixaColisao &box1, const CaixaColisao &box2)
{
    bool colisaoX = (box1.x < box2.x + box2.largura) &&
                    (box1.x + box1.largura > box2.x);

    bool colisaoY = (box1.y < box2.y + box2.altura) &&
                    (box1.y + box1.altura > box2.y);
    return colisaoX && colisaoY;
}