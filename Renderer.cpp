#include "Renderer.h"
#include <cmath>
#include <algorithm>

Renderer& Renderer::getInstance() {
    static Renderer instance;
    return instance;
}

Renderer::Renderer() : windowWidth(1024), windowHeight(768) {}

void Renderer::initialize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
}

void Renderer::render() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    
    drawMaze();
    drawPath();
    drawSpikes();
    drawPlayer();
    
    glutSwapBuffers();
}

void Renderer::drawMaze() {
    auto& gameManager = GameManager::getInstance();
    auto& labirinto = gameManager.getLabirinth();
    
    // Draw maze walls
    glColor3f(0.7f, 0.7f, 0.7f);
    for (int y = 0; y < Labirinto::ALTURA; y++) {
        for (int x = 0; x < Labirinto::LARGURA; x++) {
            if (labirinto.isWall(y, x)) {
                float posX = x * Labirinto::TAMANHO_CELULA;
                float posY = (Labirinto::ALTURA - 1 - y) * Labirinto::TAMANHO_CELULA;
                
                glBegin(GL_QUADS);
                glVertex2f(posX, posY);
                glVertex2f(posX + Labirinto::TAMANHO_CELULA, posY);
                glVertex2f(posX + Labirinto::TAMANHO_CELULA, posY + Labirinto::TAMANHO_CELULA);
                glVertex2f(posX, posY + Labirinto::TAMANHO_CELULA);
                glEnd();
            }
        }
    }
}

void Renderer::drawPath() {
    auto& gameManager = GameManager::getInstance();
    const auto& rastroVerde = gameManager.getPath();
    
    if (rastroVerde.empty()) return;
    
    glColor3f(0.0f, 1.0f, 0.0f);
    glLineWidth(2.0f);
    
    for (size_t i = 1; i < rastroVerde.size(); i++) {
        drawDashedLine(
            rastroVerde[i-1].first, rastroVerde[i-1].second,
            rastroVerde[i].first, rastroVerde[i].second
        );
    }
    
    glLineWidth(1.0f);
}

void Renderer::drawSpikes() {
    auto& gameManager = GameManager::getInstance();
    const auto& spikes = gameManager.getSpikes();
    
    for (const auto& spike : spikes) {
        spike.desenhar();
    }
}

void Renderer::drawPlayer() {
    auto& gameManager = GameManager::getInstance();
    gameManager.getPlayer()->desenhar();
}

void Renderer::drawDashedLine(float x1, float y1, float x2, float y2, float dashLen, float gapLen) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance < 0.001f) return;
    
    float unitX = dx / distance;
    float unitY = dy / distance;
    
    float currentX = x1;
    float currentY = y1;
    bool drawing = true;
    float remainingDistance = distance;
    
    while (remainingDistance > 0) {
        if (drawing) {
            float segmentLength = std::min(dashLen, remainingDistance);
            glBegin(GL_LINES);
            glVertex2f(currentX, currentY);
            glVertex2f(currentX + unitX * segmentLength, currentY + unitY * segmentLength);
            glEnd();
            remainingDistance -= segmentLength;
            currentX += unitX * segmentLength;
            currentY += unitY * segmentLength;
        } else {
            float gapDistance = std::min(gapLen, remainingDistance);
            remainingDistance -= gapDistance;
            currentX += unitX * gapDistance;
            currentY += unitY * gapDistance;
        }
        drawing = !drawing;
    }
}

void Renderer::reshape(int w, int h) {
    if (h == 0) h = 1;
    windowWidth = w;
    windowHeight = h;
    setupViewport(w, h);
}

void Renderer::setupViewport(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float larguraMundo = Labirinto::LARGURA * Labirinto::TAMANHO_CELULA;
    float alturaMundo = Labirinto::ALTURA * Labirinto::TAMANHO_CELULA;
    float aspectoJanela = (float)w / (float)h;
    float aspectoMundo = larguraMundo / alturaMundo;
    
    float orthoW, orthoH;
    if (aspectoJanela > aspectoMundo) {
        orthoH = alturaMundo + 100;
        orthoW = orthoH * aspectoJanela;
    } else {
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