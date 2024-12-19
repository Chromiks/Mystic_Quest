#ifndef GAME_H
#define GAME_H
#include "../Model_3DS.h"
#include "../GLTexture.h"

extern bool isMenuActive;

// Player state
extern float playerX;
extern float playerY;
extern float playerZ;
extern float playerRotY;

struct Object3D {
    Model_3DS model;
    float x, y, z;
    bool visible;
    float boundingRadius;
};

// Collision and game state variables
extern int health;
extern int score;
extern float hitAngle;
extern float immunityTimer;
extern float collisionMessageTimer;
extern float pickupMessageTimer; // Added for pickup out-of-range message
extern int currentLevel;

// Called by the menu when play is clicked
void startGame();
void renderGame();
void updateGame();
void loaderror(GLuint* textureID, char* strFileName, int wrap);

// Input handling
void handleGameKeyboard(unsigned char key, int x, int y);
void handleGameMouse(int button, int state, int x, int y);
void handleGameMouseMotion(int x, int y);

#endif
