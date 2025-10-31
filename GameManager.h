#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "Jogador.h"
#include "Labirinto.h"
#include "ArmadilhaEspinho.h"
#include <vector>
#include <memory>

class GameManager {
public:
    static GameManager& getInstance();
    
    void initialize();
    void update(float deltaTime);
    void handleKeyPress(unsigned char key, bool isPressed);
    bool isGameOver() const;
    
    // Getters
    int getCurrentPhase() const { return faseAtual; }
    Jogador* getPlayer() { return &jogador; }
    const std::vector<ArmadilhaEspinho>& getSpikes() const { return meusEspinhos; }
    const std::vector<std::pair<float, float>>& getPath() const { return rastroVerde; }
    Labirinto& getLabirinth() { return labirinto; }
    
    // Phase management
    void loadPhase1();
    void loadPhase2();
    void transitionToNextPhase();

private:
    GameManager(); // Singleton
    
    // Game state
    int faseAtual;
    Labirinto labirinto;
    Jogador jogador;
    std::vector<ArmadilhaEspinho> meusEspinhos;
    std::vector<std::pair<float, float>> rastroVerde;
    std::vector<std::pair<int,int>> pathCells;
    
    // Phase 2 specific
    bool botaoAtivado;
    std::pair<int,int> botaoPos;
    std::vector<std::pair<int,int>> portasParaAbrir;
    
    // Helper methods
    void initializeSpikes(const std::vector<std::pair<int,int>>& forbidden = {});
    void generatePath(bool fromCurrentPosition = false);
    void checkCollisions();
};

#endif // GAME_MANAGER_H