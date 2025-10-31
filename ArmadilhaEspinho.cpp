#include "ArmadilhaEspinho.h"

// Inclua aqui os cabeçalhos do OpenGL
#include <GL/glew.h>
#include <GL/freeglut.h>

// --- Construtor ---
ArmadilhaEspinho::ArmadilhaEspinho(float posX, float posY, float largura, float altura)
        : m_x(posX), m_y(posY), m_largura(largura), m_altura(altura),
            m_ativo(true), m_timer(0.0f), m_periodo(0.0f) {
        // Construtor: por padrão ativo e sem alternância (periodo = 0)
}

// --- Atualizar ---
void ArmadilhaEspinho::atualizar(float deltaTime) {
    // Se houver um período configurado, alterna o estado automaticamente
    if (m_periodo > 0.0f) {
        m_timer += deltaTime;
        if (m_timer >= m_periodo) {
            m_timer = 0.0f;
            m_ativo = !m_ativo;
        }
    }
}

// --- Desenhar ---
void ArmadilhaEspinho::desenhar() const {
    // 1. Desenha a base do espinho (quadrado)
    // Se ativo: vermelho / laranja. Se inativo: cinza escuro.
    if (m_ativo) {
        glColor3f(0.6f, 0.1f, 0.1f);
    } else {
        glColor3f(0.25f, 0.25f, 0.25f);
    }
    glBegin(GL_QUADS);
        glVertex2f(m_x, m_y);
        glVertex2f(m_x + m_largura, m_y);
        glVertex2f(m_x + m_largura, m_y + m_altura);
        glVertex2f(m_x, m_y + m_altura);
    glEnd();

    // 2. Desenha os "espinhos" (um "X" cinza claro)
    // Desenho do X: cor mais clara quando ativo
    if (m_ativo) glColor3f(1.0f, 0.8f, 0.2f);
    else glColor3f(0.6f, 0.6f, 0.6f);
    glLineWidth(2.0f);
    
    glBegin(GL_LINES);
        // Linha 1: ( \ )
        glVertex2f(m_x, m_y);
        glVertex2f(m_x + m_largura, m_y + m_altura);
        
        // Linha 2: ( / )
        glVertex2f(m_x + m_largura, m_y);
        glVertex2f(m_x, m_y + m_altura);
    glEnd();
    
    glLineWidth(1.0f); // Reseta a largura da linha
}

// --- Verificar Colisão ---
bool ArmadilhaEspinho::verificarColisao(const CaixaColisao& caixaDoJogador) const {
    // Se inativo, não colide
    if (!m_ativo) return false;

    // Lógica de colisão AABB (Axis-Aligned Bounding Box)
    bool colisaoX = (caixaDoJogador.x < m_x + m_largura) &&
                    (caixaDoJogador.x + caixaDoJogador.largura > m_x);

    bool colisaoY = (caixaDoJogador.y < m_y + m_altura) &&
                    (caixaDoJogador.y + caixaDoJogador.altura > m_y);

    return colisaoX && colisaoY;
}

void ArmadilhaEspinho::setTimer(float t) {
    m_timer = t;
}