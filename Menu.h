#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>
#include <functional>

struct BotaoUI {
    float x, y, w, h; // canto inferior esquerdo
    std::string texto;
    std::function<void()> onClick;
    bool hover = false;
    bool isSelector = false; // indica botão de seleção de personagem
};

class MenuPrincipal {
public:
    void inicializar(int screenW, int screenH);
    void desenhar() const;
    void atualizarHover(int mouseX, int mouseY);
    bool processarClick(int mouseX, int mouseY);
    std::vector<BotaoUI>& getBotoes() { return botoes; }
private:
    std::vector<BotaoUI> botoes; // 0 Iniciar 1 Como Jogar 2 Sair 3..6 seletores
};

class TelaInstrucoes {
public:
    void inicializar(int screenW, int screenH);
    void desenhar() const;
    void atualizarHover(int mouseX, int mouseY);
    bool processarClick(int mouseX, int mouseY);
    std::vector<BotaoUI>& getBotoes() { return botoes; }
private:
    std::vector<BotaoUI> botoes; // apenas voltar
};

class TelaSelecaoFase {
public:
    void inicializar(int screenW, int screenH);
    void desenhar() const;
    void atualizarHover(int mouseX, int mouseY);
    bool processarClick(int mouseX, int mouseY);
    std::vector<BotaoUI>& getBotoes() { return botoes; }
private:
    std::vector<BotaoUI> botoes; // Fase 1, Fase 2, Fase 3, Voltar
};

#endif // MENU_H
