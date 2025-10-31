#ifndef FASE_H
#define FASE_H

#include <vector>
#include "ArmadilhaEspinho.h"
#include "Jogador.h"

class Fase {
protected:
    int larguraLabirinto;
    int alturaLabirinto;
    float tamanhoCelula;
    std::vector<std::vector<char>> mapaLabirinto;
    std::vector<ArmadilhaEspinho> espinhos;
    int inicioCol, inicioRow;
    int fimCol, fimRow;
    std::vector<std::pair<float, float>> caminhoVerde;

public:
    virtual ~Fase() = default;
    
    virtual void inicializar() = 0;
    virtual void atualizar(float dt) = 0;
    virtual void desenhar() = 0;
    virtual bool verificarVitoria(const Jogador& jogador) = 0;
    virtual bool verificarColisao(float x, float y) = 0;
    
    // Getters úteis
    float getTamanhoCelula() const { return tamanhoCelula; }
    const std::vector<ArmadilhaEspinho>& getEspinhos() const { return espinhos; }
    void getPosicaoInicial(float& x, float& y) const {
        x = inicioCol * tamanhoCelula + tamanhoCelula/2;
        y = inicioRow * tamanhoCelula + tamanhoCelula/2;
    }
};

#endif // FASE_H