#ifndef JOGO_H
#define JOGO_H

#include "Jogador.h"
#include "Fase.h"
#include <memory>

class Jogo
{
private:
    static Jogo *instancia;
    std::unique_ptr<Fase> faseAtual;
    Jogador jogador;
    bool jogoVencido;
    bool hoverPause; // hover do botão Pause

    // Variáveis de estado do jogo
    int larguraJanela;
    int alturaJanela;
    bool teclas[256];

    // Construtor privado (Singleton)
    Jogo();

public:
    static Jogo *getInstancia();

    void inicializar();
    void carregarFase(int numeroFase);
    void atualizar(float dt);
    void desenhar();

    void redimensionarJanela(int w, int h);
    void teclaPressionada(unsigned char key, int x, int y);
    void teclaSolta(unsigned char key, int x, int y);
    void teclaEspecialPressionada(int key, int x, int y);
    void teclaEspecialSolta(int key, int x, int y);

    Fase *getFaseAtual() { return faseAtual.get(); }

    bool getJogoVencido() const { return jogoVencido; }
    void setHoverPause(bool v) { hoverPause = v; }
    bool isHoverPause() const { return hoverPause; }
};

#endif // JOGO_H