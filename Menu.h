#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>
#include <functional>

struct BotaoUI {
    float x, y, w, h; // posição canto inferior esquerdo
    std::string texto;
    std::function<void()> onClick;
    bool hover = false;
};

class MenuPrincipal {
public:
    void inicializar();
    void desenhar() const; // desenha os botões
    void atualizarHover(int mouseX, int mouseY); // mouse em coordenadas da janela (convertido para ortho)
    bool processarClick(int mouseX, int mouseY);

    // Acesso aos botões (para integrar com estados externos)
    std::vector<BotaoUI>& getBotoes() { return botoes; }
private:
    std::vector<BotaoUI> botoes;
};

class TelaInstrucoes {
public:
    void inicializar();
    void desenhar() const;
    void atualizarHover(int mouseX, int mouseY);
    bool processarClick(int mouseX, int mouseY);
    std::vector<BotaoUI>& getBotoes() { return botoes; }
private:
    std::vector<BotaoUI> botoes; // só botão voltar
};

#endif // MENU_H
