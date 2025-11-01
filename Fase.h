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
    // Referência opcional ao jogador para lógica interna da fase (ex.: botões)
    Jogador* jogadorRef = nullptr;

public:
    virtual ~Fase() = default;
    
    virtual void inicializar() = 0;
    virtual void atualizar(float dt) = 0;
    virtual void desenhar() = 0;
    virtual bool verificarVitoria(const Jogador& jogador) = 0;
    // Verifica colisão do jogador com o cenário da fase usando a caixa atual do jogador (top-left em x,y)
    virtual bool verificarColisao(float x, float y) = 0;
    
    // Injeta o jogador para a fase poder ler seu estado durante atualizar()
    virtual void setJogador(Jogador* j) { jogadorRef = j; }
    
    // Getters úteis
    float getTamanhoCelula() const { return tamanhoCelula; }
    const std::vector<ArmadilhaEspinho>& getEspinhos() const { return espinhos; }
    void getPosicaoInicial(float& x, float& y) const {
        x = inicioCol * tamanhoCelula + tamanhoCelula/2;
        // origem inferior: inverte Y para converter de linha -> mundo
        y = (alturaLabirinto - 1 - inicioRow) * tamanhoCelula + tamanhoCelula/2;
    }
    int getLargura() const { return larguraLabirinto; }
    int getAltura() const { return alturaLabirinto; }
};

#endif // FASE_H