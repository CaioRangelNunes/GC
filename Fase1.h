#ifndef FASE1_H
#define FASE1_H

#include "Fase.h"
#include "Pathfinding.h"

class Fase1 : public Fase {
private:
    void gerarCaminhoVerde();
    void inicializarEspinhos();

public:
    Fase1();
    void inicializar() override;
    void atualizar(float dt) override;
    void desenhar() override;
    bool verificarVitoria(const Jogador& jogador) override;
    bool verificarColisao(float x, float y) override;
};

#endif // FASE1_H