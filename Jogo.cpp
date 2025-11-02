/*
 * COMANDO PARA COMPILAR (no terminal do VS Code, com MinGW 64-bit):
 * g++ Jogo.cpp Jogador.cpp ArmadilhaEspinho.cpp Fase1.cpp Fase2.cpp Pathfinding.cpp -o MeuJogo.exe -DFREEGLUT_STATIC -mwindows -lglew32 -lfreeglut -lglu32 -lopengl32
 */

#include <windows.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <algorithm>

#include "Jogo.h"
#include "Fase1.h"
#include "Fase2.h"
#include "Fase3.h"
#include "Menu.h"
#include "MenuPause.h"
#include "PlayerVariant.h"

// ---------------- Jogo (Singleton) ----------------

Jogo *Jogo::instancia = nullptr;

// --- Estados de UI ---
enum class GameState
{
    MENU_PRINCIPAL,
    INSTRUCOES,
    SELECAO_FASE,
    JOGANDO,
    PAUSE_MENU
};
static GameState g_gameState = GameState::MENU_PRINCIPAL;
static MenuPrincipal g_menuPrincipal;
static TelaInstrucoes g_telaInstrucoes;
static TelaSelecaoFase g_telaSelecaoFase;
static MenuPause g_menuPause;
PlayerVariant g_selectedVariant = PlayerVariant::PADRAO;
static int gScreenWidth = 1024, gScreenHeight = 768; // dimensão janela atual

Jogo::Jogo() : faseAtual(nullptr),
               jogador(0.0f, 0.0f, 32.0f, 32.0f),
               jogoVencido(false),
               larguraJanela(1024),
               alturaJanela(768)
{
    std::fill(std::begin(teclas), std::end(teclas), false);
    hoverPause = false;
}

Jogo *Jogo::getInstancia()
{
    if (!instancia)
        instancia = new Jogo();
    return instancia;
}

void Jogo::resetParaMenu()
{
    // Libera fase atual e limpa estado de vitória / hover
    faseAtual.reset();
    jogoVencido = false;
    hoverPause = false;
    // Opcional: centraliza jogador em origem neutra para evitar lixo visual
    jogador.setX(0.0f);
    jogador.setY(0.0f);
}

void Jogo::inicializar()
{
    // Inicializa telas de menu
    g_menuPrincipal.inicializar(gScreenWidth, gScreenHeight);
    g_telaInstrucoes.inicializar(gScreenWidth, gScreenHeight);
    g_telaSelecaoFase.inicializar(gScreenWidth, gScreenHeight);
    g_menuPause.inicializar(gScreenWidth, gScreenHeight);
    // Liga callbacks principais
    // Botões do Menu Principal: 0 Iniciar, 1 Como Jogar, 2 Sair, 3..6 seletores de personagem
    auto &botoesMenu = g_menuPrincipal.getBotoes();
    if (botoesMenu.size() >= 4)
    {
        // Iniciar: escolher fase (reinicia fase ao voltar do menu)
        botoesMenu[0].onClick = []()
        { g_gameState = GameState::SELECAO_FASE; };
        // Como Jogar
        botoesMenu[1].onClick = []()
        { g_gameState = GameState::INSTRUCOES; };
        // Sair
        botoesMenu[2].onClick = []()
        { glutLeaveMainLoop(); };
        // Continuar: volta para jogo se havia fase carregada
        botoesMenu[3].onClick = []()
        {
            if (Jogo::getInstancia()->faseAtual)
                g_gameState = GameState::JOGANDO;
        };
    }
    // Botão voltar instruções
    auto &botoesInstr = g_telaInstrucoes.getBotoes();
    if (!botoesInstr.empty())
        botoesInstr[0].onClick = []()
        { g_gameState = GameState::MENU_PRINCIPAL; };
    // Seleção de fase
    auto &botoesFase = g_telaSelecaoFase.getBotoes(); // 0 Fase1 1 Fase2 2 Fase3 (se existir) 3 Voltar
    if (botoesFase.size() >= 2)
        botoesFase[0].onClick = []()
        { Jogo::getInstancia()->carregarFase(1); g_gameState = GameState::JOGANDO; };
    if (botoesFase.size() >= 3)
        botoesFase[1].onClick = []()
        { Jogo::getInstancia()->carregarFase(2); g_gameState = GameState::JOGANDO; };
    if (botoesFase.size() >= 4)
        botoesFase[2].onClick = []()
        { Jogo::getInstancia()->carregarFase(3); g_gameState = GameState::JOGANDO; };
    if (!botoesFase.empty())
        botoesFase.back().onClick = []()
        { g_gameState = GameState::MENU_PRINCIPAL; };

    // Callbacks do menu de pausa
    auto &botoesPause = g_menuPause.getBotoes();
    if (botoesPause.size() >= 2)
    {
        // Continuar
        botoesPause[0].onClick = []()
        { g_gameState = GameState::JOGANDO; };
        // Menu Principal: limpar fase e voltar ao menu
        botoesPause[1].onClick = []()
        {
            Jogo::getInstancia()->resetParaMenu();
            g_gameState = GameState::MENU_PRINCIPAL;
        };
    }
}

void Jogo::carregarFase(int numeroFase)
{
    if (numeroFase == 1)
    {
        faseAtual = std::make_unique<Fase1>();
    }
    else if (numeroFase == 2)
    {
        faseAtual = std::make_unique<Fase2>();
    }
    else
    {
        faseAtual = std::make_unique<Fase3>();
    }
    faseAtual->setJogador(&jogador);
    faseAtual->inicializar();

    // posiciona jogador no centro da célula S
    float cx, cy;
    faseAtual->getPosicaoInicial(cx, cy);
    CaixaColisao cj = jogador.getCaixaColisao();
    float startX = cx - cj.largura / 2.0f;
    float startY = cy - cj.altura / 2.0f;
    jogador.definirSpawn(startX, startY);
    jogador.setX(startX);
    jogador.setY(startY);
    jogador.reiniciarPosicao(); // garante vida cheia e flags limpas já na posição correta
}

void Jogo::atualizar(float dt)
{
    if (g_gameState != GameState::JOGANDO || !faseAtual)
        return;

    // Movimento com colisão por eixo usando top-left da caixa
    CaixaColisao antes = jogador.getCaixaColisao();
    jogador.atualizar(dt); // aplica intenção de movimento completa
    CaixaColisao depois = jogador.getCaixaColisao();

    // Primeiro testa eixo X isolado
    float tentativaX = depois.x;        // novo x proposto
    float tentativaYOriginal = antes.y; // mantém y original
    if (faseAtual->verificarColisao(tentativaX, tentativaYOriginal))
    {
        // bloqueia movimento horizontal
        jogador.setX(antes.x);
    }

    // Atualiza caixa após possível ajuste X
    CaixaColisao aposX = jogador.getCaixaColisao();

    // Testa eixo Y isolado (com X definitivo já aplicado)
    float tentativaY = depois.y; // novo y proposto
    if (faseAtual->verificarColisao(aposX.x, tentativaY))
    {
        // bloqueia movimento vertical
        jogador.setY(antes.y);
    }

    // Atualizações internas da fase (espinhos, caminho, etc.) depois da resolução de colisão
    faseAtual->atualizar(dt);

    // Colisão com espinhos ativos: reinicia jogador se encostar
    CaixaColisao caixaJog = jogador.getCaixaColisao();
    for (const auto &esp : faseAtual->getEspinhos())
    {
        if (esp.verificarColisao(caixaJog))
        {
            jogador.reiniciarPosicao();
            break; // basta primeira colisão
        }
    }

    // Vitória usando posição final
    if (faseAtual->verificarVitoria(jogador))
        jogoVencido = true;
}

void Jogo::desenhar()
{
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // Telas de menu em coordenadas de pixel
    if (g_gameState == GameState::MENU_PRINCIPAL || g_gameState == GameState::INSTRUCOES || g_gameState == GameState::SELECAO_FASE || g_gameState == GameState::PAUSE_MENU)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, gScreenWidth, 0, gScreenHeight);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        if (g_gameState == GameState::MENU_PRINCIPAL)
            g_menuPrincipal.desenhar();
        else if (g_gameState == GameState::INSTRUCOES)
            g_telaInstrucoes.desenhar();
        else if (g_gameState == GameState::SELECAO_FASE)
            g_telaSelecaoFase.desenhar();
        else if (g_gameState == GameState::PAUSE_MENU)
            g_menuPause.desenhar();
        glutSwapBuffers();
        return;
    }
    if (!faseAtual)
    {
        glutSwapBuffers();
        return;
    }
    // Coordenadas do mundo para fase
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float larguraMundo = faseAtual->getLargura() * faseAtual->getTamanhoCelula();
    float alturaMundo = faseAtual->getAltura() * faseAtual->getTamanhoCelula();
    // Expande a área ortográfica para permitir HUD em coordenadas negativas (ex.: bussola em -40,-10)
    // Mini-mapa da Fase 3 pode ir até aproximadamente -142 no eixo X (15*12/2 + padding partindo de -40).
    // Expandimos mais a margem para garantir visibilidade completa de HUDs negativos futuros.
    const float margemHUD = 220.0f; // espaço extra em cada lado (negativo e positivo)
    gluOrtho2D(-margemHUD, larguraMundo + margemHUD, -margemHUD, alturaMundo + margemHUD);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Fundo gradiente simples
    glBegin(GL_QUADS);
    glColor3f(0.06f, 0.08f, 0.12f);
    glVertex2f(0, 0);
    glVertex2f(larguraMundo, 0);
    glColor3f(0.12f, 0.14f, 0.22f);
    glVertex2f(larguraMundo, alturaMundo);
    glVertex2f(0, alturaMundo);
    glEnd();
    faseAtual->desenhar();
    jogador.desenhar();

    // Botão Pause fora do mapa (HUD negativo) com hover
    float btnW = 140.0f, btnH = 46.0f;
    float bx = -180.0f, by = alturaMundo - btnH - 20.0f; // área negativa
    if (hoverPause)
        glColor3f(0.22f, 0.22f, 0.30f);
    else
        glColor3f(0.15f, 0.15f, 0.22f);
    glBegin(GL_QUADS);
    glVertex2f(bx, by);
    glVertex2f(bx + btnW, by);
    glVertex2f(bx + btnW, by + btnH);
    glVertex2f(bx, by + btnH);
    glEnd();
    if (hoverPause)
        glColor3f(0.85f, 0.85f, 0.95f);
    else
        glColor3f(0.70f, 0.70f, 0.80f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(bx, by);
    glVertex2f(bx + btnW, by);
    glVertex2f(bx + btnW, by + btnH);
    glVertex2f(bx, by + btnH);
    glEnd();
    if (hoverPause)
        glColor3f(0.98f, 0.98f, 1.0f);
    else
        glColor3f(0.92f, 0.92f, 0.96f);
    glRasterPos2f(bx + 28.0f, by + btnH / 2 - 6.0f);
    const char *txt = "Pause";
    for (const char *p = txt; *p; ++p)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
    if (jogoVencido)
    {
        float w = larguraMundo * 0.6f;
        float h = alturaMundo * 0.3f;
        float cx = larguraMundo / 2.0f;
        float cy = alturaMundo / 2.0f;
        glColor4f(0, 0, 0, 0.55f);
        glBegin(GL_QUADS);
        glVertex2f(cx - w / 2, cy - h / 2);
        glVertex2f(cx + w / 2, cy - h / 2);
        glVertex2f(cx + w / 2, cy + h / 2);
        glVertex2f(cx - w / 2, cy + h / 2);
        glEnd();
        glColor3f(0.98f, 0.82f, 0.30f);
        glLineWidth(3);
        glBegin(GL_LINE_LOOP);
        glVertex2f(cx - w / 2, cy - h / 2);
        glVertex2f(cx + w / 2, cy - h / 2);
        glVertex2f(cx + w / 2, cy + h / 2);
        glVertex2f(cx - w / 2, cy + h / 2);
        glEnd();
        glLineWidth(1);
        const char *msg = "Voce ganhou!";
        int len = 0; for (const char *p = msg; *p; ++p) ++len;
        float textW = len * 10.0f;
        float tx = cx - textW / 2;
        float ty = cy + h * 0.15f;
        glColor3f(0.98f, 0.90f, 0.50f);
        glRasterPos2f(tx, ty);
        for (const char *p = msg; *p; ++p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
        const char *msg2 = "Clique M para voltar ao menu";
        int len2 = 0; for(const char* p=msg2;*p;++p) ++len2;
        float textW2 = len2 * 9.0f;
        float tx2 = cx - textW2 / 2;
        float ty2 = cy - h * 0.10f;
        glColor3f(0.92f,0.92f,0.96f);
        glRasterPos2f(tx2, ty2);
        for(const char* p=msg2;*p;++p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
    }
    glutSwapBuffers();
}

void Jogo::redimensionarJanela(int w, int h)
{
    if (h == 0)
        h = 1;
    larguraJanela = w;
    alturaJanela = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float larguraMundo = (faseAtual ? faseAtual->getLargura() : 15) * (faseAtual ? faseAtual->getTamanhoCelula() : 48.0f);
    float alturaMundo = (faseAtual ? faseAtual->getAltura() : 10) * (faseAtual ? faseAtual->getTamanhoCelula() : 48.0f);
    float aspectoJanela = (float)w / (float)h;
    float aspectoMundo = larguraMundo / alturaMundo;

    float orthoW, orthoH;
    if (aspectoJanela > aspectoMundo)
    {
        orthoH = alturaMundo + 100;
        orthoW = orthoH * aspectoJanela;
    }
    else
    {
        orthoW = larguraMundo + 100;
        orthoH = orthoW / aspectoJanela;
    }

    float centroX = larguraMundo / 2.0f;
    float centroY = alturaMundo / 2.0f;
    gluOrtho2D(centroX - orthoW / 2.0f, centroX + orthoW / 2.0f,
               centroY - orthoH / 2.0f, centroY + orthoH / 2.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Jogo::teclaPressionada(unsigned char key, int, int)
{
    switch (key)
    {
    case 'w':
    case 'W':
        jogador.setMovendoParaCima(true);
        break;
    case 's':
    case 'S':
        jogador.setMovendoParaBaixo(true);
        break;
    case 'a':
    case 'A':
        jogador.setMovendoParaEsquerda(true);
        break;
    case 'd':
    case 'D':
        jogador.setMovendoParaDireita(true);
        break;
    case 'm':
    case 'M':
        if (jogoVencido) {
            resetParaMenu();
            g_gameState = GameState::MENU_PRINCIPAL;
        }
        break;
    case 27:
        glutLeaveMainLoop();
        break;
    }
}

void Jogo::teclaSolta(unsigned char key, int, int)
{
    switch (key)
    {
    case 'w':
    case 'W':
        jogador.setMovendoParaCima(false);
        break;
    case 's':
    case 'S':
        jogador.setMovendoParaBaixo(false);
        break;
    case 'a':
    case 'A':
        jogador.setMovendoParaEsquerda(false);
        break;
    case 'd':
    case 'D':
        jogador.setMovendoParaDireita(false);
        break;
    }
}

void Jogo::teclaEspecialPressionada(int key, int, int)
{
    // Suporte às setas do teclado
    switch (key)
    {
    case GLUT_KEY_UP:
        jogador.setMovendoParaCima(true);
        break;
    case GLUT_KEY_DOWN:
        jogador.setMovendoParaBaixo(true);
        break;
    case GLUT_KEY_LEFT:
        jogador.setMovendoParaEsquerda(true);
        break;
    case GLUT_KEY_RIGHT:
        jogador.setMovendoParaDireita(true);
        break;
    }
}

void Jogo::teclaEspecialSolta(int key, int, int)
{
    switch (key)
    {
    case GLUT_KEY_UP:
        jogador.setMovendoParaCima(false);
        break;
    case GLUT_KEY_DOWN:
        jogador.setMovendoParaBaixo(false);
        break;
    case GLUT_KEY_LEFT:
        jogador.setMovendoParaEsquerda(false);
        break;
    case GLUT_KEY_RIGHT:
        jogador.setMovendoParaDireita(false);
        break;
    }
}

// ---------------- GLUT Callbacks ----------------

static void cbDisplay() { Jogo::getInstancia()->desenhar(); }
static void cbMouse(int button, int state, int x, int y)
{
    if (state != GLUT_DOWN || button != GLUT_LEFT_BUTTON)
        return;
    int convY = gScreenHeight - y;
    int convX = x;
    if (g_gameState == GameState::MENU_PRINCIPAL)
    {
        if (g_menuPrincipal.processarClick(convX, convY))
            glutPostRedisplay();
    }
    else if (g_gameState == GameState::INSTRUCOES)
    {
        if (g_telaInstrucoes.processarClick(convX, convY))
            glutPostRedisplay();
    }
    else if (g_gameState == GameState::SELECAO_FASE)
    {
        if (g_telaSelecaoFase.processarClick(convX, convY))
            glutPostRedisplay();
    }
    else if (g_gameState == GameState::JOGANDO)
    {
        // Converte coordenadas de tela (convX,convY) para o espaço ortográfico usado em Jogo::desenhar
        Fase *fasePtr = Jogo::getInstancia()->getFaseAtual();
        if (fasePtr)
        {
            float larguraMundo = fasePtr->getLargura() * fasePtr->getTamanhoCelula();
            float alturaMundo = fasePtr->getAltura() * fasePtr->getTamanhoCelula();
            const float margemHUD = 220.0f; // deve bater com valor em desenhar()
            // Mapeamento linear: pixel 0 -> -margemHUD, pixel larguraJanela -> larguraMundo + margemHUD
            float worldX = -margemHUD + (static_cast<float>(convX) / gScreenWidth) * (larguraMundo + 2.0f * margemHUD);
            float worldY = -margemHUD + (static_cast<float>(convY) / gScreenHeight) * (alturaMundo + 2.0f * margemHUD);

            // Área do botão Pause (desenhado em coordenadas negativas)
            float btnW = 140.0f, btnH = 46.0f;
            float bx = -180.0f;
            float by = alturaMundo - btnH - 20.0f;
            if (worldX >= bx && worldX <= bx + btnW && worldY >= by && worldY <= by + btnH)
            {
                g_gameState = GameState::PAUSE_MENU;
                glutPostRedisplay();
            }
            // Clique na mensagem de vitória
            if (Jogo::getInstancia()->getJogoVencido()) {
                const char* msg2 = "Clique M para voltar ao menu";
                int len2=0; for(const char* p=msg2;*p;++p) ++len2;
                float textW2 = len2 * 9.0f;
                float cx = larguraMundo / 2.0f; float cy = alturaMundo / 2.0f; float h = alturaMundo * 0.3f;
                float tx2 = cx - textW2 / 2; float ty2 = cy - h * 0.10f;
                float vBoxX1 = tx2 - 12.0f; float vBoxY1 = ty2 - 12.0f; float vBoxX2 = tx2 + textW2 + 12.0f; float vBoxY2 = ty2 + 28.0f;
                if (worldX >= vBoxX1 && worldX <= vBoxX2 && worldY >= vBoxY1 && worldY <= vBoxY2) {
                    Jogo::getInstancia()->resetParaMenu();
                    g_gameState = GameState::MENU_PRINCIPAL;
                    glutPostRedisplay();
                }
            }
        }
    }
    else if (g_gameState == GameState::PAUSE_MENU)
    {
        if (g_menuPause.processarClick(convX, convY))
            glutPostRedisplay();
    }
}
static void cbMotionPassive(int x, int y)
{
    int convY = gScreenHeight - y;
    int convX = x;
    if (g_gameState == GameState::MENU_PRINCIPAL)
        g_menuPrincipal.atualizarHover(convX, convY);
    else if (g_gameState == GameState::INSTRUCOES)
        g_telaInstrucoes.atualizarHover(convX, convY);
    else if (g_gameState == GameState::SELECAO_FASE)
        g_telaSelecaoFase.atualizarHover(convX, convY);
    else if (g_gameState == GameState::PAUSE_MENU)
        g_menuPause.atualizarHover(convX, convY);
    else if (g_gameState == GameState::JOGANDO)
    {
        Fase *fasePtr = Jogo::getInstancia()->getFaseAtual();
        bool hover = false;
        if (fasePtr)
        {
            float larguraMundo = fasePtr->getLargura() * fasePtr->getTamanhoCelula();
            float alturaMundo = fasePtr->getAltura() * fasePtr->getTamanhoCelula();
            const float margemHUD = 220.0f;
            float worldX = -margemHUD + (static_cast<float>(convX) / gScreenWidth) * (larguraMundo + 2.0f * margemHUD);
            float worldY = -margemHUD + (static_cast<float>(convY) / gScreenHeight) * (alturaMundo + 2.0f * margemHUD);
            float btnW = 140.0f, btnH = 46.0f;
            float bx = -180.0f;
            float by = alturaMundo - btnH - 20.0f;
            if (worldX >= bx && worldX <= bx + btnW && worldY >= by && worldY <= by + btnH)
                hover = true;
        }
        Jogo::getInstancia()->setHoverPause(hover);
    }
}
static void cbTimer(int)
{
    Jogo::getInstancia()->atualizar(16.0f / 1000.0f);
    glutPostRedisplay();
    glutTimerFunc(16, cbTimer, 0);
    glutMouseFunc(cbMouse);
    glutPassiveMotionFunc(cbMotionPassive);
}
static void cbReshape(int w, int h) { Jogo::getInstancia()->redimensionarJanela(w, h); }
static void cbKeyDown(unsigned char k, int x, int y) { Jogo::getInstancia()->teclaPressionada(k, x, y); }
static void cbKeyUp(unsigned char k, int x, int y) { Jogo::getInstancia()->teclaSolta(k, x, y); }
static void cbSpecialDown(int k, int x, int y) { Jogo::getInstancia()->teclaEspecialPressionada(k, x, y); }
static void cbSpecialUp(int k, int x, int y) { Jogo::getInstancia()->teclaEspecialSolta(k, x, y); }

int main_proxy(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Jogo de Orientacao - Fases");

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        std::cerr << "Erro ao inicializar GLEW: " << glewGetErrorString(err) << std::endl;
        return 1;
    }

    // Melhoria de qualidade visual
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    Jogo::getInstancia()->inicializar();

    glutDisplayFunc(cbDisplay);
    glutReshapeFunc(cbReshape);
    glutKeyboardFunc(cbKeyDown);
    glutKeyboardUpFunc(cbKeyUp);
    glutSpecialFunc(cbSpecialDown);
    glutSpecialUpFunc(cbSpecialUp);
    glutTimerFunc(16, cbTimer, 0);

    glutMainLoop();
    return 0;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    char *argv[1];
    argv[0] = (char *)"MeuJogo";
    int argc = 1;
    return main_proxy(argc, argv);
}