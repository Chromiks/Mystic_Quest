#include <GL/glew.h>
#include "include/level1.h"
#include "include/camera.h"
#include <GL/freeglut.h>
#include "include/level2.h"
#include <iostream>
#include <cmath>
#include "include/game.h"
#include "TextureBuilder.h"
#include <windows.h>
#include <playsoundapi.h>

// From previous code
bool isMenuActive = true;

float playerX = 0.0f;
float playerY = 0.0f;
float playerZ = 0.0f;
float playerRotY = 0.0f;

static float totalTime = 0.0f;
bool gamelose = false;
bool gameWin = false;
int health = 5;    // start health
int score = 0;     // start score
float immunityTimer = 0.0f;
float collisionMessageTimer = 0.0f;
float pickupMessageTimer = 0.0f;
int currentLevel = 1;

// Hit animation angle
float hitAngle = 0.0f;

static float gravity = -20.0f;
static float jumpVelocity = 0.0f;
static bool isJumping = false;
static float jumpImpulse = 8.0f;
void renderBitmapString(float x, float y, void* font, const char* string) {
    const char* c;
    glRasterPos2f(x, y);
    for (c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

void renderWin() {
    // Set up a 2D orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 0, 720);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Disable lighting and depth for HUD rendering
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set text color to white
    glColor3f(1.0f, 1.0f, 1.0f);

    // Display "Game Win"
    const char* winMsg = "Game Win";
    int winMsgWidth = (int)strlen(winMsg) * 9;
    float winX = (1280 - winMsgWidth) / 2.0f;
    float winY = 400.0f; // Center vertically
    renderBitmapString(winX, winY, GLUT_BITMAP_HELVETICA_18, winMsg);

    // Display Score
    char scoreStr[64];
    sprintf(scoreStr, "Score: %d", score);
    int scoreMsgWidth = (int)strlen(scoreStr) * 9;
    float scoreX = (1280 - scoreMsgWidth) / 2.0f;
    float scoreY = 360.0f; // slightly below "Game Win"
    renderBitmapString(scoreX, scoreY, GLUT_BITMAP_HELVETICA_18, scoreStr);

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void renderLose() {
    // Set up a 2D orthographic projection
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 1280, 0, 720);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Disable lighting and depth for HUD rendering
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Clear the screen to black
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set text color to white
    glColor3f(1.0f, 1.0f, 1.0f);

    // Display "Game Lose"
    const char* loseMsg = "Game Lose";
    int loseMsgWidth = (int)strlen(loseMsg) * 9;
    float loseX = (1280 - loseMsgWidth) / 2.0f;
    float loseY = 400.0f; // Center vertically
    renderBitmapString(loseX, loseY, GLUT_BITMAP_HELVETICA_18, loseMsg);

    // Display Score
    char scoreStr[64];
    sprintf(scoreStr, "Score: %d", score);
    int scoreMsgWidth = (int)strlen(scoreStr) * 9;
    float scoreX = (1280 - scoreMsgWidth) / 2.0f;
    float scoreY = 360.0f; // slightly below "Game Lose"
    renderBitmapString(scoreX, scoreY, GLUT_BITMAP_HELVETICA_18, scoreStr);

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}
void startGame() {
    isMenuActive = false;
    initLevel1();
    initCamera();
    totalTime = 0.0f;
}


void loaderror(GLuint* textureID, char* strFileName, int wrap) {
    loadBMP(textureID, strFileName, wrap);
}
void renderHUD() {
    // Display:
    // Time: (time) s
    // Score: 00(score)
    // Health: 5
    // Level 1
    char hudStr[64];

    int seconds = (int)totalTime;

    // We'll position text starting from top-left corner
    float startX = 10.0f;
    float startY = 700.0f;

    sprintf(hudStr, "Time: %d s", seconds);
    renderBitmapString(startX, startY, GLUT_BITMAP_HELVETICA_18, hudStr);

    sprintf(hudStr, "Score: %d", score);
    renderBitmapString(startX, startY - 20, GLUT_BITMAP_HELVETICA_18, hudStr);

    sprintf(hudStr, "Health: %d", health);
    renderBitmapString(startX, startY - 40, GLUT_BITMAP_HELVETICA_18, hudStr);

    sprintf(hudStr, "Level %d", currentLevel);
    renderBitmapString(startX, startY - 60, GLUT_BITMAP_HELVETICA_18, hudStr);
    if (pickupMessageTimer > 0.0f) {
        glColor3f(1.0f, 0.0f, 0.0f);
        const char* msg = "You can only pickup coins or gems you are close to.";
        // position near top
        renderBitmapString(400.0f, 680.0f, GLUT_BITMAP_HELVETICA_18, msg);
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    // If collisionMessageTimer > 0 and immunityTimer <= 0, show message
    if (collisionMessageTimer > 0.0f) {
        glColor3f(1.0f, 0.0f, 0.0f);
        // Center message at bottom
        const char* msg = "You stumbled! You lost some health!";
        int msgWidth = (int)strlen(msg) * 9; // approximate width
        float msgX = (1280 - msgWidth) / 2.0f;
        float msgY = 100.0f;
        renderBitmapString(msgX, msgY, GLUT_BITMAP_HELVETICA_18, msg);
        glColor3f(1.0f, 1.0f, 1.0f); // reset color
    }
}


void renderGame() {
    // Apply hitAngle to playerRotX if desired (small forward tilt)
    // We'll temporarily store original rotations in level1 rendering
    if (gameWin) {
        renderWin();
    }
    else if (gamelose) {
        renderLose();
    }
    else {
        updateCamera(playerX, playerY, playerZ, playerRotY);
        if (currentLevel == 1) {
            renderLevel1();
        }
        else if (currentLevel == 2) {
            renderLevel2();
        }
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 1280, 0, 720);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glColor3f(1.0f, 1.0f, 1.0f);

        renderHUD();

        glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
}

static void resolveHitAngle(float deltaTime) {
    if (hitAngle > 0.0f) {
        hitAngle -= 30.0f * deltaTime; // decrease angle over time
        if (hitAngle < 0.0f) hitAngle = 0.0f;
    }
}

void updateGame() {
    float deltaTime;
    if (currentLevel == 1)
        deltaTime = 4.0f / 60.0f;
    else
         deltaTime = 1.0f / 60.0f;
    totalTime += deltaTime;
    if (immunityTimer > 0.0f) {
        immunityTimer -= deltaTime;
        if (immunityTimer < 0.0f) immunityTimer = 0.0f;
    }

    // Decrement collisionMessageTimer if active
    if (collisionMessageTimer > 0.0f) {
        collisionMessageTimer -= deltaTime;
        if (collisionMessageTimer < 0.0f) collisionMessageTimer = 0.0f;
    }
    if (pickupMessageTimer > 0.0f) {
        pickupMessageTimer -= deltaTime;
        if (pickupMessageTimer < 0.0f) pickupMessageTimer = 0.0f;
    }
    // Jump physics
    if (isJumping) {
        playerY += jumpVelocity * deltaTime;
        jumpVelocity += gravity * deltaTime;
        if (playerY <= 0.0f) {
            playerY = 0.0f;
            isJumping = false;
            jumpVelocity = 0.0f;
        }
    }

    resolveHitAngle(deltaTime);
    if (currentLevel == 1) {
        updateLevel1(deltaTime);
    }
    else if (currentLevel == 2) {
        updateLevel2(deltaTime);
    }
}

// Forward declaration for collision check
bool detectCollision(float newX, float newZ, float newY);
bool detectCollisionBound(float newX, float newZ);

void handleGameKeyboard(unsigned char key, int x, int y) {
    float moveSpeed = 1.0f;
    float rad = playerRotY * 3.14159f / 180.0f;

    float oldX = playerX;
    float oldZ = playerZ;

    switch (key) {
    case 'w':
    case 's':
    {
        // Moving forward or backward
        float direction = (key == 'w') ? 1.0f : -1.0f;
        float newX = playerX + sin(rad) * moveSpeed * direction;
        float newZ = playerZ + cos(rad) * moveSpeed * direction;
        if (health == 0) {
            gamelose = true;
            return;
        }
        if (currentLevel == 1) {
            // Level 1 checks
            if (checkChestCollision(playerX, playerY, playerZ)) {
                cleanupLevel1();
                PlaySoundA("assets/sounds/win", NULL, SND_FILENAME | SND_ASYNC);
                health = 5; // reset health maybe
                currentLevel = 2;
                playerX = 0.0f;
                playerY = 0.0f;
                playerZ = 0.0f;
                playerRotY = 0.0f;
                updateCamera(0, 0, 0, 0);
                initLevel2();
                break;
            }

            if (detectCollision(newX, newZ, playerY)) {
                if (immunityTimer <= 0.0f) {
                    if (health > 0) health--;
                    PlaySoundA("assets/sounds/grunt", NULL, SND_FILENAME | SND_ASYNC);
                    score -= 10;
                    collisionMessageTimer = 1.0f;
                    immunityTimer = 2.0f;
                    
                }
                hitAngle = 15.0f;
            }
            else {
                if (!detectCollisionBound(newX, newZ)) {
                    playerX = newX;
                    playerZ = newZ;
                }
            }
        }
        else if (currentLevel == 2) {
            // Level 2 checks
            // Check altar collision
            if (checkAltarCollision(playerX, playerY, playerZ)) {
                cleanupLevel2();
                PlaySoundA("assets/sounds/win", NULL, SND_FILENAME | SND_ASYNC);
                printf("YOU WIN");
                gameWin = true; // show game win screen
                return; // after winning, maybe stop movement?
            }

            // Check cave collision (like detectCaveCollision())
            if (detectCaveCollision(newX, newZ, playerY)) {
                if (immunityTimer <= 0.0f) {
                    if (health > 0) health--;
                    PlaySoundA("assets/sounds/grunt", NULL, SND_FILENAME | SND_ASYNC);
                    score -= 10;
                    collisionMessageTimer = 1.0f;
                    immunityTimer = 2.0f;
                    if (health == 0) {
                        renderLose();
                        return;
                    }
                }
                hitAngle = 15.0f;
            }
            else {
                // Bound check for cave: ensure player stays inside x=250..350, z=-20..120
                if (newX < -50 || newX > 50 || newZ < -20 || newZ > 120) {
                    // don't move if out of cave bounds
                }
                else {
                    playerX = newX;
                    playerZ = newZ;
                }
            }
        }
        break;
    }
    case 'a':
        playerRotY += 10.0f;
        break;
    case 'd':
        playerRotY -= 10.0f;
        break;
    case ' ':
        if (!isJumping && playerY <= 0.0f) {
            isJumping = true;
            jumpVelocity = jumpImpulse;
        }
        break;
    case 'e':
    {
        if (currentLevel == 1) {
            int coinIndex = detectCoinInRange(playerX, playerY, playerZ);
            if (coinIndex == -1) {
                pickupMessageTimer = 1.5f;
            }
            else {
                // Coin found
                Coin& c = (coinIndex < (int)goldCoins.size()) ? goldCoins[coinIndex] :
                    (coinIndex < (int)(goldCoins.size() + silverCoins.size())) ? silverCoins[coinIndex - goldCoins.size()] :
                    bronzeCoins[coinIndex - goldCoins.size() - silverCoins.size()];

                if (!c.beingPickedUp) {
                    c.beingPickedUp = true;
                    c.pickupTimer = 0.0f;
                    PlaySoundA("assets/sounds/PickUp", NULL, SND_FILENAME | SND_ASYNC);
                    enableCoinLight(c.x, c.y, c.z);
                }
            }
        }
        else if (currentLevel == 2) {
            // Gems pickup
            int gemIndex = detectGemInRange(playerX, playerY, playerZ);
            if (gemIndex != -1) {
                Gem& g = (gemIndex < (int)gem1Gems.size()) ? gem1Gems[gemIndex]
                    : (gemIndex < (int)(gem1Gems.size() + gem2Gems.size())) ? gem2Gems[gemIndex - (int)gem1Gems.size()]
                    : gem3Gems[gemIndex - (int)gem1Gems.size() - (int)gem2Gems.size()];

                if (!g.beingPickedUp) {
                    g.beingPickedUp = true;
                    PlaySoundA("assets/sounds/PickUp", NULL, SND_FILENAME | SND_ASYNC);
                    g.pickupTimer = 0.0f;
                    enableGemLight(g.x, g.y, g.z);
                }
            }
            else {
                pickupMessageTimer = 1.5f; // No gem in range, show message
            }
        }
        break;
    }
    default:
        break;
    }
}

void handleGameMouse(int button, int state, int x, int y) {
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        cycleCameraMode();
    }
}

void handleGameMouseMotion(int x, int y) {
    // Implement mouse look if desired
}

