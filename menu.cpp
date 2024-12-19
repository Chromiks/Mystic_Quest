#include <GL/glew.h>
#include "include/game.h"
#include "include/menu.h" // Include the menu header // Include necessary declarations for `startGame`
#include "include/texture.h" // Include the texture loading header
#include <GL/freeglut.h>
#include <iostream>


// Define global variables (declared as extern in the header)
GLuint backgroundTexture, titleTexture, playButtonTexture, exitButtonTexture;
bool isHoveringPlay = false;
bool isHoveringExit = false;

// Load all menu textures
void loadMenuTextures() {
    backgroundTexture = loadTextures("assets/image/menubg.png");
    titleTexture = loadTextures("assets/image/menu_title.png");
    playButtonTexture = loadTextures("assets/image/play.png");
    exitButtonTexture = loadTextures("assets/image/exit.png");
}

// Render a textured quad
void drawQuad(float x, float y, float width, float height, GLuint texture) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x + width, y);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x + width, y + height);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y + height);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

void cleanupMenuTextures() {
    GLuint textures[] = { backgroundTexture, titleTexture, playButtonTexture, exitButtonTexture };
    glDeleteTextures(4, textures);
    backgroundTexture = titleTexture = playButtonTexture = exitButtonTexture = 0;
}


// Render the menu
void renderMenu() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Grey for the menu
    // Enable blending for transparency
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set up orthographic projection for 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 0, 720); // Window size
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
    glLoadIdentity();

    // Render background
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);
    drawQuad(0, 0, 1280, 720, backgroundTexture);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind background texture

    // Render title
    glBindTexture(GL_TEXTURE_2D, titleTexture);
    float titleWidth = 375;
    float titleHeight = 375;
    float titleX = (1280 - titleWidth) / 2;
    float titleY = 720 - titleHeight;
    drawQuad(titleX, titleY, titleWidth, titleHeight, titleTexture);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind title texture

    // Render Play button (hover effect)
    glBindTexture(GL_TEXTURE_2D, playButtonTexture);
    float playWidth = 193 * 1.25;
    float playHeight = 95.5 * 1.25;
    float playX = (1280 - playWidth) / 2;
    float playY = 360 - playHeight / 2 - 70;

    if (isHoveringPlay) {
        playWidth *= 1.1; // Scale up by 10% on hover
        playHeight *= 1.1;
        playX = (1280 - playWidth) / 2; // Re-center
    }
    drawQuad(playX, playY, playWidth, playHeight, playButtonTexture);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind play button texture

    // Render Exit button (hover effect)
    glBindTexture(GL_TEXTURE_2D, exitButtonTexture);
    float exitWidth = 160 * 1.3;
    float exitHeight = 122.5 * 1.3;
    float exitX = (1280 - exitWidth) / 2 + 3;
    float exitY = playY - exitHeight - 30;

    if (isHoveringExit) {
        exitWidth *= 1.1; // Scale up by 10% on hover
        exitHeight *= 1.1;
        exitX = (1280 - exitWidth) / 2 + 3; // Re-center
    }
    drawQuad(exitX, exitY, exitWidth, exitHeight, exitButtonTexture);
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind exit button texture

    glPopMatrix();

    // Restore previous projection
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glFlush();
}

// Handle mouse clicks on the menu
void handleMenuClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Map mouse coordinates to window coordinates
        int windowY = 720 - y; // Invert y-axis to match OpenGL's coordinate system

        // Play Button Bounds
        float playWidth = 193 * 1.25;
        float playHeight = 95.5 * 1.25;
        float playX = (1280 - playWidth) / 2;
        float playY = 360 - playHeight / 2 - 70;

        if (x >= playX && x <= playX + playWidth && windowY >= playY && windowY <= playY + playHeight) {
            // Play button clicked
            startGame(); // Switch to gameplay
            return;
        }

        // Exit Button Bounds
        float exitWidth = 160 * 1.3;
        float exitHeight = 122.5 * 1.3;
        float exitX = (1280 - exitWidth) / 2 + 3;
        float exitY = playY - exitHeight - 30;

        if (x >= exitX && x <= exitX + exitWidth && windowY >= exitY && windowY <= exitY + exitHeight) {
            // Exit button clicked
            cleanupMenuTextures();
            exit(0);
            return;
        }
    }
}


void handleMenuMouseMotion(int x, int y) {
    // Map mouse coordinates to window coordinates
    int windowY = 720 - y; // Invert y-axis

    // Play Button Bounds
    float playWidth = 193 * 1.25;
    float playHeight = 95.5 * 1.25;
    float playX = (1280 - playWidth) / 2;
    float playY = 360 - playHeight / 2 - 70;

    // Exit Button Bounds
    float exitWidth = 160 * 1.3;
    float exitHeight = 122.5 * 1.3;
    float exitX = (1280 - exitWidth) / 2 + 3;
    float exitY = playY - exitHeight - 30;

    // Check if hovering over Play button
    isHoveringPlay = (x >= playX && x <= playX + playWidth && windowY >= playY && windowY <= playY + playHeight);

    // Check if hovering over Exit button
    isHoveringExit = (x >= exitX && x <= exitX + exitWidth && windowY >= exitY && windowY <= exitY + exitHeight);
}
