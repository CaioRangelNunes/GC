#include "GameManager.h"
#include "Fase2.h"
#include "Pathfinding.h"
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <algorithm>

GameManager& GameManager::getInstance() {
    static GameManager instance;
    return instance;
}

GameManager::GameManager() : 
    faseAtual(1), 
    jogador(0, 0, 32, 32),
    botaoAtivado(false) {
}

void GameManager::initialize() {
    loadPhase1();
}

void GameManager::loadPhase1() {
    faseAtual = 1;
    labirinto.reset();
    
    // Clear state
    rastroVerde.clear();
    pathCells.clear();
    meusEspinhos.clear();
    
    // Setup player position
    auto startPos = labirinto.getStartPosition();
    float startX = startPos.first * Labirinto::TAMANHO_CELULA + 
                   (Labirinto::TAMANHO_CELULA - jogador.getCaixaColisao().largura) / 2;
    float startY = (Labirinto::ALTURA - 1 - startPos.second) * Labirinto::TAMANHO_CELULA + 
                   (Labirinto::TAMANHO_CELULA - jogador.getCaixaColisao().altura) / 2;
    
    jogador = Jogador(startX, startY, jogador.getCaixaColisao().largura, jogador.getCaixaColisao().altura);
    
    // Generate initial path
    generatePath();
    
    // Setup spikes
    initializeSpikes();
}

void GameManager::loadPhase2() {
    faseAtual = 2;
    botaoAtivado = false;
    portasParaAbrir.clear();
    rastroVerde.clear();
    pathCells.clear();
    meusEspinhos.clear();
    
    // Load phase 2 map and initialize positions
    Fase2::carregar();
    
    // Re-initialize player at start position
    auto startPos = labirinto.getStartPosition();
    float startX = startPos.first * Labirinto::TAMANHO_CELULA + 
                   (Labirinto::TAMANHO_CELULA - jogador.getCaixaColisao().largura) / 2;
    float startY = (Labirinto::ALTURA - 1 - startPos.second) * Labirinto::TAMANHO_CELULA + 
                   (Labirinto::TAMANHO_CELULA - jogador.getCaixaColisao().altura) / 2;
    
    jogador = Jogador(startX, startY, jogador.getCaixaColisao().largura, jogador.getCaixaColisao().altura);
    
    // Generate new path and spikes for phase 2
    generatePath();
    initializeSpikes();
}

void GameManager::update(float deltaTime) {
    // Update player
    jogador.atualizar(deltaTime);
    
    // Check collisions
    checkCollisions();
    
    // Check for phase transition
    auto endPos = labirinto.getEndPosition();
    float fimX = endPos.first * Labirinto::TAMANHO_CELULA + Labirinto::TAMANHO_CELULA / 2;
    float fimY = (Labirinto::ALTURA - 1 - endPos.second) * Labirinto::TAMANHO_CELULA + Labirinto::TAMANHO_CELULA / 2;
    
    // If player reaches the end point
    auto playerBox = jogador.getCaixaColisao();
    float dx = playerBox.x - fimX;
    float dy = playerBox.y - fimY;
    if (std::sqrt(dx*dx + dy*dy) < 20.0f) {
        if (faseAtual == 1) {
            loadPhase2();
        }
    }
}

void GameManager::generatePath(bool fromCurrentPosition) {
    auto endPos = labirinto.getEndPosition();
    std::vector<std::pair<int, int>> path;
    
    if (fromCurrentPosition) {
        auto playerBox = jogador.getCaixaColisao();
        int playerCol = static_cast<int>(playerBox.x / Labirinto::TAMANHO_CELULA);
        int playerRow = Labirinto::ALTURA - 1 - static_cast<int>(playerBox.y / Labirinto::TAMANHO_CELULA);
        // Convert labirinto to grid format for pathfinding
        std::vector<std::string> grid;
        for (int y = 0; y < Labirinto::ALTURA; y++) {
            std::string row;
            for (int x = 0; x < Labirinto::LARGURA; x++) {
                row += labirinto.getCell(y, x);
            }
            grid.push_back(row);
        }
        path = findPathAStar(grid, playerCol, playerRow, endPos.first, endPos.second);
    } else {
        auto startPos = labirinto.getStartPosition();
        // Convert labirinto to grid format for pathfinding
        std::vector<std::string> grid;
        for (int y = 0; y < Labirinto::ALTURA; y++) {
            std::string row;
            for (int x = 0; x < Labirinto::LARGURA; x++) {
                row += labirinto.getCell(y, x);
            }
            grid.push_back(row);
        }
        path = findPathAStar(grid, startPos.first, startPos.second, endPos.first, endPos.second);
    }
    
    rastroVerde.clear();
    pathCells = path;
    
    // Convert grid positions to world coordinates
    for (const auto& pos : path) {
        auto worldPos = labirinto.getPosicaoCentro(pos.first, pos.second);
        rastroVerde.push_back(worldPos);
    }
}

void GameManager::initializeSpikes(const std::vector<std::pair<int,int>>& forbidden) {
    meusEspinhos.clear();
    
    // Add spikes avoiding path and forbidden positions
    for (int y = 0; y < Labirinto::ALTURA; y++) {
        for (int x = 0; x < Labirinto::LARGURA; x++) {
            if (labirinto.getCell(y, x) == ' ') {
                bool isOnPath = false;
                for (const auto& pathCell : pathCells) {
                    if (pathCell.first == x && pathCell.second == y) {
                        isOnPath = true;
                        break;
                    }
                }
                
                bool isForbidden = false;
                for (const auto& pos : forbidden) {
                    if (pos.first == x && pos.second == y) {
                        isForbidden = true;
                        break;
                    }
                }
                
                if (!isOnPath && !isForbidden && (rand() % 100 < 30)) {  // 30% chance
                    float spikeX = x * Labirinto::TAMANHO_CELULA;
                    float spikeY = (Labirinto::ALTURA - 1 - y) * Labirinto::TAMANHO_CELULA;
                    meusEspinhos.emplace_back(spikeX, spikeY, Labirinto::TAMANHO_CELULA, Labirinto::TAMANHO_CELULA);
                }
            }
        }
    }
}

void GameManager::checkCollisions() {
    auto playerBox = jogador.getCaixaColisao();
    int playerRow = Labirinto::ALTURA - 1 - static_cast<int>(playerBox.y / Labirinto::TAMANHO_CELULA);
    int playerCol = static_cast<int>(playerBox.x / Labirinto::TAMANHO_CELULA);
    
    // Check surrounding cells for walls
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (labirinto.isWall(playerRow + dy, playerCol + dx)) {
                float wallX = (playerCol + dx) * Labirinto::TAMANHO_CELULA;
                float wallY = (Labirinto::ALTURA - 1 - (playerRow + dy)) * Labirinto::TAMANHO_CELULA;
                CaixaColisao wallBox = {wallX, wallY, Labirinto::TAMANHO_CELULA, Labirinto::TAMANHO_CELULA};
                
                // If collision detected, adjust player position
                if (Jogador::checkCollision(playerBox, wallBox)) {
                    // Push player back
                    if (dx > 0) jogador.setX(wallBox.x - playerBox.largura);
                    if (dx < 0) jogador.setX(wallBox.x + wallBox.largura);
                    if (dy > 0) jogador.setY(wallBox.y - playerBox.altura);
                    if (dy < 0) jogador.setY(wallBox.y + wallBox.altura);
                }
            }
        }
    }
    
    // Check spike collisions
    for (const auto& espinho : meusEspinhos) {
        if (espinho.verificarColisao(playerBox)) {
            // Reset to current phase's start position
            auto startPos = labirinto.getStartPosition();
            float startX = startPos.first * Labirinto::TAMANHO_CELULA + 
                          (Labirinto::TAMANHO_CELULA - jogador.getCaixaColisao().largura) / 2;
            float startY = (Labirinto::ALTURA - 1 - startPos.second) * Labirinto::TAMANHO_CELULA + 
                          (Labirinto::TAMANHO_CELULA - jogador.getCaixaColisao().altura) / 2;
            jogador = Jogador(startX, startY, jogador.getCaixaColisao().largura, jogador.getCaixaColisao().altura);
            generatePath();
            break;
        }
    }
}

void GameManager::handleKeyPress(unsigned char key, bool isPressed) {
    switch(key) {
        case 'w': case 'W': jogador.setMovendoParaCima(isPressed); break;
        case 's': case 'S': jogador.setMovendoParaBaixo(isPressed); break;
        case 'a': case 'A': jogador.setMovendoParaEsquerda(isPressed); break;
        case 'd': case 'D': jogador.setMovendoParaDireita(isPressed); break;
    }
}

bool GameManager::isGameOver() const {
    return false;  // Implement game over conditions
}