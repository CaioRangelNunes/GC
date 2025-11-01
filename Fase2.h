#ifndef FASE2_H
#define FASE2_H

#include "Fase.h"
#include <utility>
#include <vector>

class Fase2 : public Fase {
private:
    // Elementos especiais da fase 2
    int botaoCol = -1, botaoRow = -1;
    bool botaoAtivado = false;
    std::vector<std::pair<int,int>> portasParaAbrir; // posições de paredes que viram chão quando botão é ativado

    // Caminho principal e um caminho "hint" considerando portas abertas
    std::vector<std::pair<int,int>> pathCells;
    std::vector<std::pair<int,int>> hintPathCells;

    void gerarCaminho(bool fromCurrentPosition = false);
    void gerarHintComPortasAbertas();
    void inicializarEspinhos(const std::vector<std::pair<int,int>>& forbidden = {});
    std::pair<float,float> centroDaCelula(int col, int row) const;
    std::pair<int,int> cellDoJogador() const;

public:
    Fase2();
    void inicializar() override;
    void atualizar(float dt) override;
    void desenhar() override;
    bool verificarVitoria(const Jogador& jogador) override;
    bool verificarColisao(float x, float y) override;
    // Compatibilidade antiga: método estático sem efeito
    static void carregar();
};

#endif // FASE2_H