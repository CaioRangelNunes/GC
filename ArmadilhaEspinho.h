#ifndef ARMADILHAESPINHO_H
#define ARMADILHAESPINHO_H

// Uma estrutura simples para representar uma caixa de colisão 2D.
// Usada pelo Jogador, Armadilha, e Labirinto.
struct CaixaColisao {
    float x, y;
    float largura, altura;
};

class ArmadilhaEspinho {
public:
    // Construtor: Define a posição (x, y) e o tamanho (largura, altura)
    ArmadilhaEspinho(float posX, float posY, float largura, float altura);

    // Método de Atualização (pode ser usado para animações)
    void atualizar(float deltaTime);

    // Método de Renderização (Desenha o espinho na tela)
    void desenhar() const;

    // Método de Colisão
    bool verificarColisao(const CaixaColisao& caixaDoJogador) const;

    // Ativação / desativação (quando inativo não causa colisão)
    void setAtivo(bool ativo) { m_ativo = ativo; }
    bool isAtivo() const { return m_ativo; }

    // Define período de alternância (segundos). Se 0 -> sempre ativo
    void setPeriodo(float segundos) { m_periodo = segundos; }

    // Define o timer interno (útil para criar offsets iniciais)
    void setTimer(float t);

    // Métodos "Get" para obter a posição
    float getX() const { return m_x; }
    float getY() const { return m_y; }

private:
    float m_x, m_y;             // Posição no mundo 2D (canto inferior esquerdo)
    float m_largura, m_altura;  // Dimensões do espinho
    bool m_ativo;
    float m_timer;    // contador interno
    float m_periodo;  // intervalo para alternar estado (em segundos). 0 = estático
};

#endif // ARMADILHAESPINHO_H