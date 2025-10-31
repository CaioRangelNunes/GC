#ifndef RENDERER_H
#define RENDERER_H

#include "GameManager.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

class Renderer {
public:
    static Renderer& getInstance();
    
    void initialize(int width, int height);
    void render();
    void reshape(int w, int h);

private:
    Renderer(); // Singleton
    
    void drawMaze();
    void drawPath();
    void drawSpikes();
    void drawPlayer();
    void setupViewport(int w, int h);
    
    void drawDashedLine(float x1, float y1, float x2, float y2, float dashLen = 10.0f, float gapLen = 5.0f);
    
    int windowWidth;
    int windowHeight;
};

#endif // RENDERER_H