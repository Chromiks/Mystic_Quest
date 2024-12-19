#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
 // Include game header
#include "include/menu.h" // Include menu header
#include "include/game.h"

void initializeOpenGL() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);      // Enable depth testing
    glEnable(GL_LIGHTING);        // Enable lighting
    glEnable(GL_LIGHT0);          // Example light source
    glEnable(GL_COLOR_MATERIAL);  // Enable material coloring
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Background color
    glEnable(GL_NORMALIZE); // Ensure normals are normalized for correct lighting
    glShadeModel(GL_SMOOTH); // Smooth shading
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Set the lighting model to local viewer
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
}

void timer(int value) {
    if (!isMenuActive) {
        updateGame();
    }

    glutPostRedisplay(); // Always request a redraw
    glutTimerFunc(16, timer, 0); // Continue 60 FPS updates
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT); // Clear buffers

    if (isMenuActive) {
        glDisable(GL_DEPTH_TEST);
        renderMenu();
    }
    else {
        glEnable(GL_DEPTH_TEST);
        renderGame();
    }
    glutSwapBuffers(); // Double buffering
}

void handleKeyboard(unsigned char key, int x, int y) {
    if (isMenuActive) {
        // If you're in the menu, you can handle menu keys here if needed.
        // Currently nothing is done in the menu with keys.
    }
    else {
        // Pass input to the game
        handleGameKeyboard(key, x, y);
    }
}

void handleMouse(int button, int state, int x, int y) {
    if (isMenuActive) {
        handleMenuClick(button, state, x, y);
    }
    else {
        // Gameplay mouse handling
        handleGameMouse(button, state, x, y);
    }
}

void handleMouseMotion(int x, int y) {
    if (isMenuActive) {
        handleMenuMouseMotion(x, y);
    }
    else {
        // Gameplay mouse handling (movement)
        handleGameMouseMotion(x, y);
    }
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("Mystic Quest");

    glewInit();

    // Load menu textures
    loadMenuTextures();

    initializeOpenGL();

    // Register callback functions
    glutDisplayFunc(display);
    glutMouseFunc(handleMouse);
    glutPassiveMotionFunc(handleMouseMotion);
    glutKeyboardFunc(handleKeyboard);
    glutTimerFunc(16, timer, 0); // Start the timer for 60 FPS updates

    glutMainLoop(); // Enter the GLUT main loop
    return 0;
}
