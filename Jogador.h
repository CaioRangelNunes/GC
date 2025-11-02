#ifndef JOGADOR_H
#define JOGADOR_H

// Inclui para ter acesso à struct 'CaixaColisao'
#include "ArmadilhaEspinho.h" 

class Jogador {
public:
    // Construtor
    Jogador(float spawnX, float spawnY, float largura, float altura);

    // --- MÉTODOS DO LOOP PRINCIPAL ---
    void atualizar(float deltaTime);
    void desenhar() const;

    // --- MÉTODOS DE INPUT (MOVIMENTAÇÃO 2D) ---
    void setMovendoParaCima(bool movendo);
    void setMovendoParaBaixo(bool movendo);
    void setMovendoParaEsquerda(bool movendo);
    void setMovendoParaDireita(bool movendo);

    // --- MÉTODOS DE AÇÃO / ESTADO ---
    void sofrerDano(int quantidade);
    void reiniciarPosicao();
    // Define dinamicamente a posição de spawn (ex.: ao carregar nova fase)
    void definirSpawn(float sx, float sy) { m_spawnX = sx; m_spawnY = sy; }

    // --- GETTERS (Informações para outras partes do jogo) ---
    CaixaColisao getCaixaColisao() const;
    int getVida() const { return m_vida; }
    bool estaVivo() const { return m_vida > 0; }

    // --- SETTERS (Para colisão com o labirinto) ---
    void setX(float newX) { m_x = newX; }
    void setY(float newY) { m_y = newY; }

    // --- FUNÇÃO AUXILIAR DE COLISÃO ESTÁTICA ---
    // Verifica colisão entre quaisquer duas caixas
    static bool checkCollision(const CaixaColisao& box1, const CaixaColisao& box2);

private:
    // --- Posição e Dimensões ---
    float m_x, m_y;             
    float m_largura, m_altura;
    float m_spawnX, m_spawnY;   // Posição inicial (para respawn)

    // --- Estado de Movimento ---
    float m_velocidadeBase;     
    bool m_movendoCima;
    bool m_movendoBaixo;
    bool m_movendoEsquerda;
    bool m_movendoDireita;
    
    // --- Estado de Jogo ---
    int m_vida;
    int m_vidaMaxima;
};

#endif // JOGADOR_H