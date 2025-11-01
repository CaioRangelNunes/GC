#ifndef JOGO_H
#define JOGO_H

#include "Jogador.h"
#include "Fase.h"
#include <memory>

class Jogo {
private:
    static Jogo* instancia;
    std::unique_ptr<Fase> faseAtual;
    Jogador jogador;
    bool jogoVencido;
    
    // Variáveis de estado do jogo
    int larguraJanela;
    int alturaJanela;
    bool teclas[256];
    
    // Construtor privado (Singleton)
    Jogo();

public:
    static Jogo* getInstancia();
    
    void inicializar();
    void carregarFase(int numeroFase);
    void atualizar(float dt);
    void desenhar();
    
    void redimensionarJanela(int w, int h);
    void teclaPressionada(unsigned char key, int x, int y);
    void teclaSolta(unsigned char key, int x, int y);
    void teclaEspecialPressionada(int key, int x, int y);
    void teclaEspecialSolta(int key, int x, int y);
    
    
    bool getJogoVencido() const { return jogoVencido; }
};

#endif // JOGO_H