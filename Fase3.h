#ifndef FASE3_H
#define FASE3_H

#include "Fase.h"
#include <utility>
#include <vector>

class Fase3 : public Fase {
public:
    Fase3();
    void inicializar() override;
    void atualizar(float dt) override;
    void desenhar() override;
    bool verificarVitoria(const Jogador& jogador) override;
    bool verificarColisao(float x, float y) override;
};

#endif // FASE3_H
