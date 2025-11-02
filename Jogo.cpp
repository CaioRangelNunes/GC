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

// ---------------- Jogo (Singleton) ----------------

Jogo* Jogo::instancia = nullptr;

Jogo::Jogo() :
    faseAtual(nullptr),
    jogador(0.0f, 0.0f, 32.0f, 32.0f),
    jogoVencido(false),
    larguraJanela(1024),
    alturaJanela(768)
{
    std::fill(std::begin(teclas), std::end(teclas), false);
}

Jogo* Jogo::getInstancia() {
    if (!instancia) instancia = new Jogo();
    return instancia;
}

void Jogo::inicializar() {
    carregarFase(1);
}

void Jogo::carregarFase(int numeroFase) {
    if (numeroFase == 1) {
        faseAtual = std::make_unique<Fase1>();
    } else if (numeroFase == 2) {
        faseAtual = std::make_unique<Fase2>();
    } else {
        faseAtual = std::make_unique<Fase3>();
    }
    faseAtual->setJogador(&jogador);
    faseAtual->inicializar();

    // posiciona jogador no centro da célula S
    float cx, cy; faseAtual->getPosicaoInicial(cx, cy);
    CaixaColisao cj = jogador.getCaixaColisao();
    jogador.setX(cx - cj.largura / 2.0f);
    jogador.setY(cy - cj.altura / 2.0f);
}

void Jogo::atualizar(float dt) {
    if (!faseAtual) return;

    // Guarda posição anterior e aplica input ao jogador
    CaixaColisao before = jogador.getCaixaColisao();
    jogador.atualizar(dt);

    // Colisão com cenário da fase: se colidir, reverte para posição anterior
    CaixaColisao after = jogador.getCaixaColisao();
    if (faseAtual->verificarColisao(after.x, after.y)) {
        jogador.setX(before.x);
        jogador.setY(before.y);
    }

    // Lógica interna da fase (botões, espinhos, etc.) — atualize antes de checar colisão com espinhos
    faseAtual->atualizar(dt);

    // Colisão com espinhos (agora usando o estado atualizado, evitando mortes quando espinho acabou de desativar)
    for (const auto &e : faseAtual->getEspinhos()) {
        if (e.verificarColisao(jogador.getCaixaColisao())) {
            // resetar para início da fase
            float cx, cy; faseAtual->getPosicaoInicial(cx, cy);
            CaixaColisao box = jogador.getCaixaColisao();
            jogador = Jogador(cx - box.largura/2, cy - box.altura/2, box.largura, box.altura);
            break;
        }
    }

    // Vitória e troca de fase
    if (faseAtual->verificarVitoria(jogador)) {
        // Fase 1 -> 2 -> 3 -> vitória
        if (dynamic_cast<Fase1*>(faseAtual.get()) != nullptr) carregarFase(2);
        else if (dynamic_cast<Fase2*>(faseAtual.get()) != nullptr) carregarFase(3);
        else jogoVencido = true;
    }
}

void Jogo::desenhar() {
    if (!faseAtual) return;
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    // Fundo com gradiente sutil
    float larguraMundo = (faseAtual ? faseAtual->getLargura() : 15) * (faseAtual ? faseAtual->getTamanhoCelula() : 48.0f);
    float alturaMundo = (faseAtual ? faseAtual->getAltura() : 10) * (faseAtual ? faseAtual->getTamanhoCelula() : 48.0f);
    glBegin(GL_QUADS);
        glColor3f(0.06f, 0.08f, 0.12f);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(larguraMundo, 0.0f);
        glColor3f(0.12f, 0.14f, 0.22f);
        glVertex2f(larguraMundo, alturaMundo);
        glVertex2f(0.0f, alturaMundo);
    glEnd();

    faseAtual->desenhar();
    jogador.desenhar();
    
        // Overlay de vitória após terminar Fase 3
        if (jogoVencido) {
            // caixa semi-transparente central
            float w = larguraMundo * 0.6f;
            float h = alturaMundo * 0.3f;
            float cx = larguraMundo / 2.0f;
            float cy = alturaMundo / 2.0f;
            glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
            glBegin(GL_QUADS);
                glVertex2f(cx - w/2, cy - h/2);
                glVertex2f(cx + w/2, cy - h/2);
                glVertex2f(cx + w/2, cy + h/2);
                glVertex2f(cx - w/2, cy + h/2);
            glEnd();
            // moldura dourada
            glColor3f(0.98f, 0.82f, 0.30f);
            glLineWidth(3.0f);
            glBegin(GL_LINE_LOOP);
                glVertex2f(cx - w/2, cy - h/2);
                glVertex2f(cx + w/2, cy - h/2);
                glVertex2f(cx + w/2, cy + h/2);
                glVertex2f(cx - w/2, cy + h/2);
            glEnd();
            glLineWidth(1.0f);
            // texto
            const char* msg = "Voce ganhou!";
            glColor3f(0.98f, 0.90f, 0.50f);
            // centraliza aproximadamente: calcula largura do texto
            int len = 0; for (const char* p = msg; *p; ++p) ++len;
            float textW = len * 10.0f; // approx para HELVETICA_18
            float tx = cx - textW/2;
            float ty = cy - 6.0f;
            glRasterPos2f(tx, ty);
            for (const char* p = msg; *p; ++p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
        }

    glutSwapBuffers();
}

void Jogo::redimensionarJanela(int w, int h) {
    if (h == 0) h = 1;
    larguraJanela = w; alturaJanela = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float larguraMundo = (faseAtual ? faseAtual->getLargura() : 15) * (faseAtual ? faseAtual->getTamanhoCelula() : 48.0f);
    float alturaMundo = (faseAtual ? faseAtual->getAltura() : 10) * (faseAtual ? faseAtual->getTamanhoCelula() : 48.0f);
    float aspectoJanela = (float)w / (float)h;
    float aspectoMundo = larguraMundo / alturaMundo;

    float orthoW, orthoH;
    if (aspectoJanela > aspectoMundo) { orthoH = alturaMundo + 100; orthoW = orthoH * aspectoJanela; }
    else { orthoW = larguraMundo + 100; orthoH = orthoW / aspectoJanela; }

    float centroX = larguraMundo / 2.0f;
    float centroY = alturaMundo / 2.0f;
    gluOrtho2D(centroX - orthoW / 2.0f, centroX + orthoW / 2.0f,
               centroY - orthoH / 2.0f, centroY + orthoH / 2.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void Jogo::teclaPressionada(unsigned char key, int, int) {
    switch(key) {
        case 'w': case 'W': jogador.setMovendoParaCima(true); break;
        case 's': case 'S': jogador.setMovendoParaBaixo(true); break;
        case 'a': case 'A': jogador.setMovendoParaEsquerda(true); break;
        case 'd': case 'D': jogador.setMovendoParaDireita(true); break;
        case 27: glutLeaveMainLoop(); break;
    }
}

void Jogo::teclaSolta(unsigned char key, int, int) {
    switch(key) {
        case 'w': case 'W': jogador.setMovendoParaCima(false); break;
        case 's': case 'S': jogador.setMovendoParaBaixo(false); break;
        case 'a': case 'A': jogador.setMovendoParaEsquerda(false); break;
        case 'd': case 'D': jogador.setMovendoParaDireita(false); break;
    }
}

void Jogo::teclaEspecialPressionada(int key, int, int) {
    // Suporte às setas do teclado
    switch(key) {
        case GLUT_KEY_UP:    jogador.setMovendoParaCima(true); break;
        case GLUT_KEY_DOWN:  jogador.setMovendoParaBaixo(true); break;
        case GLUT_KEY_LEFT:  jogador.setMovendoParaEsquerda(true); break;
        case GLUT_KEY_RIGHT: jogador.setMovendoParaDireita(true); break;
    }
}

void Jogo::teclaEspecialSolta(int key, int, int) {
    switch(key) {
        case GLUT_KEY_UP:    jogador.setMovendoParaCima(false); break;
        case GLUT_KEY_DOWN:  jogador.setMovendoParaBaixo(false); break;
        case GLUT_KEY_LEFT:  jogador.setMovendoParaEsquerda(false); break;
        case GLUT_KEY_RIGHT: jogador.setMovendoParaDireita(false); break;
    }
}

// ---------------- GLUT Callbacks ----------------

static void cbDisplay() { Jogo::getInstancia()->desenhar(); }
static void cbTimer(int) {
    Jogo::getInstancia()->atualizar(16.0f/1000.0f);
    glutPostRedisplay();
    glutTimerFunc(16, cbTimer, 0);
}
static void cbReshape(int w, int h) { Jogo::getInstancia()->redimensionarJanela(w,h); }
static void cbKeyDown(unsigned char k, int x, int y) { Jogo::getInstancia()->teclaPressionada(k,x,y); }
static void cbKeyUp(unsigned char k, int x, int y) { Jogo::getInstancia()->teclaSolta(k,x,y); }
static void cbSpecialDown(int k, int x, int y) { Jogo::getInstancia()->teclaEspecialPressionada(k,x,y); }
static void cbSpecialUp(int k, int x, int y) { Jogo::getInstancia()->teclaEspecialSolta(k,x,y); }

int main_proxy(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_MULTISAMPLE);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Jogo de Orientacao - Fases");

    GLenum err = glewInit();
    if (GLEW_OK != err) {
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

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    char* argv[1]; argv[0] = (char*)"MeuJogo"; int argc = 1; return main_proxy(argc, argv);
}