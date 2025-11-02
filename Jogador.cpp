#include "Jogador.h"
#include "PlayerVariant.h" // acesso a g_selectedVariant
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath> // Para futura normalização de vetor
extern PlayerVariant g_selectedVariant; // definido em Jogo.cpp

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
      m_movendoDireita(false) {
}

// --- Loop de Jogo: Atualizar ---
void Jogador::atualizar(float deltaTime) {
    if (!estaVivo()) {
        return; 
    }

    // 1. Calcular o vetor de velocidade (4 direções)
    float velX = 0.0f;
    float velY = 0.0f;

    // Permite combinações (ex.: diagonais). Mantém velocidade base.
    if (m_movendoCima) velY += m_velocidadeBase;
    if (m_movendoBaixo) velY -= m_velocidadeBase;
    if (m_movendoEsquerda) velX -= m_velocidadeBase;
    if (m_movendoDireita) velX += m_velocidadeBase;

    // Normaliza velocidade diagonal para não ficar mais rápida
    if (velX != 0.0f && velY != 0.0f) {
        const float invSqrt2 = 0.70710678f; // 1/sqrt(2)
        velX *= invSqrt2;
        velY *= invSqrt2;
    }

    // 2. Aplicar o movimento
    m_x += velX * deltaTime;
    m_y += velY * deltaTime;
    
    // A colisão com paredes será tratada no Jogo.cpp
}

// --- Loop de Jogo: Desenhar ---
void Jogador::desenhar() const {
    // Usa g_selectedVariant para definir aparência persistente entre fases
    float x=m_x, y=m_y, w=m_largura, h=m_altura;
    float headH=h*0.24f, torsoH=h*0.40f, legsH=h-headH-torsoH;

    // Paletas por variante
    float skinR,skinG,skinB,hairR,hairG,hairB, shirtR,shirtG,shirtB,pantsR,pantsG,pantsB;
    switch(g_selectedVariant){
        case PlayerVariant::PADRAO: skinR=0.85f;skinG=0.72f;skinB=0.58f; hairR=0.25f;hairG=0.18f;hairB=0.08f; shirtR=0.10f;shirtG=0.45f;shirtB=0.90f; pantsR=0.05f;pantsG=0.18f;pantsB=0.35f; break;
        case PlayerVariant::VERMELHO: skinR=0.90f;skinG=0.78f;skinB=0.60f; hairR=0.80f;hairG=0.70f;hairB=0.30f; shirtR=0.75f;shirtG=0.15f;shirtB=0.15f; pantsR=0.28f;pantsG=0.06f;pantsB=0.06f; break;
        case PlayerVariant::VERDE: skinR=0.78f;skinG=0.64f;skinB=0.50f; hairR=0.12f;hairG=0.35f;hairB=0.15f; shirtR=0.15f;shirtG=0.65f;shirtB=0.25f; pantsR=0.10f;pantsG=0.32f;pantsB=0.12f; break;
        case PlayerVariant::ROXO: skinR=0.82f;skinG=0.70f;skinB=0.55f; hairR=0.32f;hairG=0.15f;hairB=0.40f; shirtR=0.45f;shirtG=0.20f;shirtB=0.65f; pantsR=0.20f;pantsG=0.07f;pantsB=0.32f; break;
    }
    if(!estaVivo()){ // escurece se morto
        shirtR*=0.5f; shirtG*=0.3f; shirtB*=0.3f; pantsR*=0.6f; pantsG*=0.4f; pantsB*=0.4f;
    }
    // Cabeça
    float headW=w*0.62f; float headX=x+(w-headW)/2.0f; float headY=y+legsH+torsoH;
    glColor3f(skinR,skinG,skinB); glBegin(GL_QUADS); glVertex2f(headX,headY); glVertex2f(headX+headW,headY); glVertex2f(headX+headW,headY+headH); glVertex2f(headX,headY+headH); glEnd();
    // Cabelo (parte superior)
    glColor3f(hairR,hairG,hairB); glBegin(GL_QUADS); glVertex2f(headX,headY+headH*0.70f); glVertex2f(headX+headW,headY+headH*0.70f); glVertex2f(headX+headW,headY+headH); glVertex2f(headX,headY+headH); glEnd();
    // Torso
    float torsoY=y+legsH; glColor3f(shirtR,shirtG,shirtB); glBegin(GL_QUADS); glVertex2f(x+w*0.10f,torsoY); glVertex2f(x+w*0.90f,torsoY); glVertex2f(x+w*0.90f,torsoY+torsoH); glVertex2f(x+w*0.10f,torsoY+torsoH); glEnd();
    // Pernas
    float legW=w*0.40f; glColor3f(pantsR,pantsG,pantsB); glBegin(GL_QUADS); glVertex2f(x+w*0.08f,y); glVertex2f(x+w*0.08f+legW,y); glVertex2f(x+w*0.08f+legW,y+legsH); glVertex2f(x+w*0.08f,y+legsH); glEnd(); glBegin(GL_QUADS); glVertex2f(x+w*0.52f,y); glVertex2f(x+w*0.52f+legW,y); glVertex2f(x+w*0.52f+legW,y+legsH); glVertex2f(x+w*0.52f,y+legsH); glEnd();
    // Sapatos
    glColor3f(0.05f,0.05f,0.05f); float shoeH=legsH*0.18f; glBegin(GL_QUADS); glVertex2f(x+w*0.08f,y); glVertex2f(x+w*0.08f+legW,y); glVertex2f(x+w*0.08f+legW,y+shoeH); glVertex2f(x+w*0.08f,y+shoeH); glEnd(); glBegin(GL_QUADS); glVertex2f(x+w*0.52f,y); glVertex2f(x+w*0.52f+legW,y); glVertex2f(x+w*0.52f+legW,y+shoeH); glVertex2f(x+w*0.52f,y+shoeH); glEnd();
    // Acessórios por variante
    switch(g_selectedVariant){
        case PlayerVariant::PADRAO: { // cinto
            glColor3f(0.25f,0.18f,0.05f); float beltY=torsoY+torsoH*0.55f; float beltH=torsoH*0.08f; glBegin(GL_QUADS); glVertex2f(x+w*0.10f,beltY); glVertex2f(x+w*0.90f,beltY); glVertex2f(x+w*0.90f,beltY+beltH); glVertex2f(x+w*0.10f,beltY+beltH); glEnd();
        } break;
        case PlayerVariant::VERMELHO: { // faixa na cabeça
            glColor3f(1.0f,0.9f,0.2f); float bandH=headH*0.22f; float bandY=headY+headH*0.45f; glBegin(GL_QUADS); glVertex2f(headX,bandY); glVertex2f(headX+headW,bandY); glVertex2f(headX+headW,bandY+bandH); glVertex2f(headX,bandY+bandH); glEnd();
        } break;
        case PlayerVariant::VERDE: { // espinhos na cabeça
            glColor3f(0.10f,0.50f,0.12f); float spikeBase=headY+headH; float spikeW=headW/4.0f; for(int i=0;i<3;++i){ float sx=headX+i*spikeW+spikeW*0.2f; glBegin(GL_TRIANGLES); glVertex2f(sx,spikeBase); glVertex2f(sx+spikeW*0.6f,spikeBase); glVertex2f(sx+spikeW*0.3f,spikeBase+headH*0.75f); glEnd(); }
        } break;
        case PlayerVariant::ROXO: { // capa atrás do torso
            glColor3f(0.35f,0.10f,0.5f); float capeTop=torsoY+torsoH*0.95f; float capeBottom=y+legsH*0.30f; glBegin(GL_QUADS); glVertex2f(x+w*0.10f,capeTop); glVertex2f(x+w*0.90f,capeTop); glVertex2f(x+w*0.74f,capeBottom); glVertex2f(x+w*0.26f,capeBottom); glEnd();
        } break;
    }
}

// --- Ações de Jogo ---
void Jogador::sofrerDano(int quantidade) {
    if (estaVivo()) {
        m_vida -= quantidade;
        if (m_vida < 0) {
            m_vida = 0;
        }
    }
}

void Jogador::reiniciarPosicao() {
    m_x = m_spawnX;
    m_y = m_spawnY;
    m_vida = m_vidaMaxima;
    m_movendoCima = false;
    m_movendoBaixo = false;
    m_movendoEsquerda = false;
    m_movendoDireita = false;
}

// --- Getters ---
CaixaColisao Jogador::getCaixaColisao() const {
    return { m_x, m_y, m_largura, m_altura };
}

// --- Setters de Movimento ---
void Jogador::setMovendoParaCima(bool movendo) { m_movendoCima = movendo; }
void Jogador::setMovendoParaBaixo(bool movendo) { m_movendoBaixo = movendo; }
void Jogador::setMovendoParaEsquerda(bool movendo) { m_movendoEsquerda = movendo; }
void Jogador::setMovendoParaDireita(bool movendo) { m_movendoDireita = movendo; }

// --- Função Auxiliar Estática de Colisão ---
bool Jogador::checkCollision(const CaixaColisao& box1, const CaixaColisao& box2) {
    bool colisaoX = (box1.x < box2.x + box2.largura) &&
                    (box1.x + box1.largura > box2.x);

    bool colisaoY = (box1.y < box2.y + box2.altura) &&
                    (box1.y + box1.altura > box2.y);
    return colisaoX && colisaoY;
}