#ifndef MENU_H
#define MENU_H

#include <GL/glew.h>
#include <GL/freeglut.h>

// Global variables for menu hover states
extern bool isHoveringPlay;
extern bool isHoveringExit;

// Menu-related global variables
extern GLuint backgroundTexture;
extern GLuint titleTexture;
extern GLuint playButtonTexture;
extern GLuint exitButtonTexture;

// Function declarations
void loadMenuTextures();                  // Load all menu textures
void renderMenu();                        // Render the menu
void handleMenuClick(int button, int state, int x, int y); // Handle mouse clicks
void handleMenuMouseMotion(int x, int y);     // Handle mouse movement for hover effect
void cleanupMenuTextures();

#endif // MENU_H
