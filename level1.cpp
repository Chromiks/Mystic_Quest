
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include "include/level1.h"

float coinRotationAngle = 0.0f;
float coinTranslation = 0.0f;

extern int score;
bool level1Loaded = false;
float deltatimeday = 0.05f;
float pickupRange = 1.5f;

static GLTexture groundTexture;
static GLuint daySkyTex, nightSkyTex;
static Model_3DS playerModel;
static Model_3DS chestModel;

struct Placement {
    float x, z;
    const char* model;
    bool visible;
};

std::vector<Coin> goldCoins;
std::vector<Coin> silverCoins;
std::vector<Coin> bronzeCoins;
static std::vector<Object3D> trees;
static std::vector<Object3D> rocks;
static std::vector<Object3D> edgeTrees;
static std::vector<Object3D> caveRocks;

static float playerStartX = 0.0f;
static float playerStartZ = 0.0f;
static float chestX = 0.0f;
static float chestZ = 100.0f;

static GLfloat lightAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
static GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat lightPosition[] = { 0.0f, 100.0f, 50.0f, 1.0f };

static float timeOfDay = 0.0f;
extern float playerX, playerY, playerZ, playerRotY;
extern float hitAngle;
extern bool isJumping;

struct Pos {
    float x, z;
};
static std::vector<Pos> usedPositions; // To avoid collisions

int detectCoinInRange(float px, float py, float pz) {
    // Check all coins in gold, silver, bronze
    // Return a global index: first goldCoins, then silverCoins, then bronzeCoins
    // Or return just the index and figure out which list after.
    int index = 0;

    // goldCoins
    for (int i = 0; i < (int)goldCoins.size(); i++) {
        if (!goldCoins[i].collected && !goldCoins[i].beingPickedUp) {
            float dx = px - goldCoins[i].x;
            float dz = pz - goldCoins[i].z;
            float distSq = dx * dx + dz * dz;
            if (distSq < pickupRange * pickupRange) {
                return i; // gold coin index
            }
        }
    }

    index += (int)goldCoins.size();

    // silverCoins
    for (int i = 0; i < (int)silverCoins.size(); i++) {
        if (!silverCoins[i].collected && !silverCoins[i].beingPickedUp) {
            float dx = px - silverCoins[i].x;
            float dz = pz - silverCoins[i].z;
            float distSq = dx * dx + dz * dz;
            if (distSq < pickupRange * pickupRange) {
                return index + i;
            }
        }
    }

    index += (int)silverCoins.size();

    // bronzeCoins
    for (int i = 0; i < (int)bronzeCoins.size(); i++) {
        if (!bronzeCoins[i].collected && !bronzeCoins[i].beingPickedUp) {
            float dx = px - bronzeCoins[i].x;
            float dz = pz - bronzeCoins[i].z;
            float distSq = dx * dx + dz * dz;
            if (distSq < pickupRange * pickupRange) {
                return index + i;
            }
        }
    }

    return -1; // No coin in range
}
void enableCoinLight(float x, float y, float z) {
    glEnable(GL_LIGHT1);


    GLfloat yellowDiffuse[] = { 1.0f, 1.0f, 0.0f, 1.0f };

    GLfloat yellowAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    GLfloat pos[] = { x, y, z, 1.0f };

    glLightfv(GL_LIGHT1, GL_DIFFUSE, yellowDiffuse);
    glLightfv(GL_LIGHT1, GL_AMBIENT, yellowAmbient);
    glLightfv(GL_LIGHT1, GL_POSITION, pos);

    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.1f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.1f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.05f);
}

void disableCoinLight() {
    glDisable(GL_LIGHT1);
}

float getBoundingRadiusForModel(const char* modelPath) {
    std::string m = modelPath;
    // Just guess based on model type
    if (m.find("Tree_s") != std::string::npos) return 0.2f;
    if (m.find("Tree_m") != std::string::npos) return 0.7f;
    if (m.find("Tree_l") != std::string::npos) return 0.7f;
    if (m.find("Tree_c") != std::string::npos) return 0.5f;
    if (m.find("Tree_d") != std::string::npos) return 0.7f;
    if (m.find("log") != std::string::npos) return 1.0f;
    if (m.find("Tree1") != std::string::npos) return 5.0f;
    if (m.find("Stone") != std::string::npos) return 1.0f;
    if (m.find("Stones") != std::string::npos) return 1.0f;
    if (m.find("Rock") != std::string::npos) return 2.0f;
    if (m.find("rock1") != std::string::npos) return 2.0f;
    if (m.find("Rock2") != std::string::npos) return 2.0f;

    // Default
    return 1.0f;
}

bool positionsOverlap(float x1, float z1, float x2, float z2, float minDist = 3.0f) {
    float dx = x1 - x2;
    float dz = z1 - z2;
    return (dx * dx + dz * dz) < (minDist * minDist);
}

bool canPlaceHere(float x, float z, float minDist = 3.0f) {
    // Check against used positions
    for (auto& pos : usedPositions) {
        if (positionsOverlap(x, z, pos.x, pos.z, minDist)) {
            return false;
        }
    }
    // Check player start
    if (positionsOverlap(x, z, playerStartX, playerStartZ, minDist)) return false;
    // Check chest
    if (positionsOverlap(x, z, chestX, chestZ, minDist)) return false;

    return true;
}

void addUsedPosition(float x, float z) {
    Pos p = { x,z };
    usedPositions.push_back(p);
}

void placeEdgeTrees() {
    float xLeft = -70.0f, xRight = 70.0f, zBottom = -20.0f, zTop = 120.0f;

    for (float z = zBottom; z <= zTop; z += 10.0f) {
        Object3D et;
        et.model.Load((char*)"assets/models/edge_trees/Tree1.3ds");
        et.x = xLeft; et.y = 0; et.z = z;
        et.visible = true;
        edgeTrees.push_back(et);

        Object3D et2;
        et2.model.Load((char*)"assets/models/edge_trees/Tree1.3ds");
        et2.x = xRight; et2.y = 0; et2.z = z;
        et2.visible = true;
        edgeTrees.push_back(et2);
    }

    for (float x = xLeft + 10; x <= xRight - 10; x += 10.0f) {
        Object3D et3;
        et3.model.Load((char*)"assets/models/edge_trees/Tree1.3ds");
        et3.x = x; et3.y = 0; et3.z = zBottom;
        et3.visible = true;
        edgeTrees.push_back(et3);

        Object3D et4;
        et4.model.Load((char*)"assets/models/edge_trees/Tree1.3ds");
        et4.x = x; et4.y = 0; et4.z = zTop;
        et4.visible = true;
        edgeTrees.push_back(et4);
    }
}

// We'll place a large number of trees and rocks randomly
void placeRandomObstacles() {
    const char* treeModels[] = {
        "assets/models/trees/Tree_s.3ds",
        "assets/models/trees/Tree_m.3ds",
        "assets/models/trees/Tree_l.3ds",
        "assets/models/trees/Tree_c.3ds",
        "assets/models/trees/Tree_d.3ds",
        "assets/models/trees/log.3ds"
    };
    int numTreeModels = 6;

    const char* rockModels[] = {
        "assets/models/rocks/Stone.3ds",
        "assets/models/rocks/Stones1.3ds",
        "assets/models/rocks/Stones2.3ds",
        "assets/models/rocks/Stones3.3ds",
        "assets/models/rocks/Stones4.3ds",
        "assets/models/rocks/Stones5.3ds"
    };
    int numRockModels = 6;

    const char* rockModels2[] = {
        "assets/models/cave_rocks/Rock.3DS"
    };
    int numRockModels2 = 1;

    float startX = -60.0f, endX = 60.0f;
    float startZ = -10.0f, endZ = 110.0f;

    int treesToPlace = 50;
    int attempts = 0;
    int placedTrees = 0;
    while (placedTrees < treesToPlace && attempts < 2000) {
        attempts++;
        float x = startX + (rand() / (float)RAND_MAX) * (endX - startX);
        float z = startZ + (rand() / (float)RAND_MAX) * (endZ - startZ);
        if (canPlaceHere(x, z, 4.0f)) {
            Object3D t;
            const char* chosenModel = treeModels[rand() % numTreeModels];
             t.model.Load((char*)chosenModel);
             if (chosenModel == "assets/models/trees/Tree_s.3ds") {
                 t.x = x + 20;
                 t.y = 0;
                 t.z = z;
             }
             if (chosenModel == "assets/models/trees/Tree_m.3ds") {
                 t.x = x + 12;
                 t.y = 0;
                 t.z = z;
             }
             if (chosenModel == "assets/models/trees/Tree_l.3ds") {
                 t.x = x + 2;
                 t.y = 0;
                 t.z = z;
             }
             if (chosenModel == "assets/models/trees/Tree_c.3ds") {
                 t.x = x + 20;
                 t.y = 0;
                 t.z = z + 6;
             }
             if (chosenModel == "assets/models/trees/Tree_d.3ds") {
                 t.x = x - 19;
                 t.y = 0;
                 t.z = z;
             }
             if (chosenModel == "assets/models/trees/log.3ds") {
                 t.x = x - 19;
                 t.y = 0;
                 t.z = z + 6;
             }
            t.visible = true;
            t.boundingRadius = getBoundingRadiusForModel(chosenModel);
            trees.push_back(t);
            addUsedPosition(x, z);
            placedTrees++;
        }
    }

    // Place about 20 rocks
    int rocksToPlace2 = 20;
    attempts = 0;
    int placedRocks2 = 0;
    while (placedRocks2 < rocksToPlace2 && attempts < 1000) {
        attempts++;
        float x = startX + (rand() / (float)RAND_MAX) * (endX - startX);
        float z = startZ + (rand() / (float)RAND_MAX) * (endZ - startZ);
        if (canPlaceHere(x, z, 4.0f)) {
            Object3D r;
            const char* chosenRock = rockModels2[rand() % numRockModels2];
            r.model.Load((char*)chosenRock);
            r.x = x + 1.52864f; r.y = 0; r.z = z + 3.31507;
            r.visible = true;
            r.boundingRadius = getBoundingRadiusForModel(chosenRock);
            caveRocks.push_back(r);
            addUsedPosition(x, z);
            placedRocks2++;
        }
    }

    // Place about 10 stalagmites

    int rocksToPlace = 50;
    attempts = 0;
    int placedRocks = 0;
    while (placedRocks < rocksToPlace && attempts < 2000) {
        attempts++;
        float x = startX + (rand() / (float)RAND_MAX) * (endX - startX);
        float z = startZ + (rand() / (float)RAND_MAX) * (endZ - startZ);
        if (canPlaceHere(x, z, 3.0f)) {
            Object3D r;
            const char* chosenRock = rockModels[rand() % numRockModels];
            r.model.Load((char*)chosenRock);
            r.x = x; r.y = 0; r.z = z;
            r.visible = true;
            r.boundingRadius = getBoundingRadiusForModel(chosenRock);
            rocks.push_back(r);
            addUsedPosition(x, z);
            placedRocks++;
        }
    }
}


void loadObstacles() {
    // Place edge boundary trees
    placeEdgeTrees();

    // Place random obstacles inside
    placeRandomObstacles();
}

void loadCoins() {
    // We'll scatter about 20 coins in random spots behind obstacles.
    // Different types of coins: gold, silver, bronze
    const char* goldModel = "assets/models/coin/Coin.3ds";
    const char* silverModel = "assets/models/coin/Coin_silv.3ds";
    const char* bronzeModel = "assets/models/coin/Coin_bronz.3ds";

    const char* coinModels[] = { goldModel, silverModel, bronzeModel };
    int numCoinModels = 3;

    float startX = -60.0f, endX = 60.0f;
    float startZ = -10.0f, endZ = 110.0f;

    int coinsToPlace = 20;
    int attempts = 0;
    int placedCoins = 0;
    while (placedCoins < coinsToPlace && attempts < 2000) {
        attempts++;
        float x = startX + (rand() / (float)RAND_MAX) * (endX - startX);
        float z = startZ + (rand() / (float)RAND_MAX) * (endZ - startZ);
        // Coins can be placed closer, so minDist = 3.0f is still good
        if (canPlaceHere(x, z, 3.0f)) {
            Coin c;
            const char* chosenCoin = coinModels[rand() % numCoinModels];
            c.model.Load((char*)chosenCoin);
            c.collected = false;
            c.beingPickedUp = false;
            c.pickupTimer = 0.0f;
            c.x = x;
            c.y = 1.0f; // Float above ground
            c.z = z;

            std::string m = chosenCoin;
            if (m.find("silv") != std::string::npos) {
                c.type = COIN_SILVER;
                silverCoins.push_back(c);
            }
            else if (m.find("bronz") != std::string::npos) {
                c.type = COIN_BRONZE;
                bronzeCoins.push_back(c);
            }
            else {
                c.type = COIN_GOLD;
                goldCoins.push_back(c);
            }

            addUsedPosition(x, z);
            placedCoins++;
        }
    }
}

void initLights() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}

void updateDayNightCycle(float deltaTime) {
    if (timeOfDay >= 1.0f) deltatimeday = -0.05f;
    if (timeOfDay <= 0.0f) deltatimeday = 0.05f;;
    timeOfDay += deltaTime * deltatimeday;

    float brightness;
    brightness = 1.0f - (timeOfDay * 1.0f);
   

    GLfloat newDiffuse[] = {
        0.2f + 0.8f * brightness,
        0.2f + 0.8f * brightness,
        0.2f + 0.8f * (0.5f + 0.5f * brightness),
        1.0f
    };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, newDiffuse);
}

void initLevel1() {
    if (level1Loaded) return;

    srand((unsigned)time(NULL));

    groundTexture.Load((char*)"assets/textures/ground.bmp");
    groundTexture.Use();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    playerModel.Load((char*)"assets/models/player/Finished-adventurer.3ds");
    chestModel.Load((char*)"assets/models/chest/treasure.3ds");

    addUsedPosition(playerStartX, playerStartZ);
    addUsedPosition(chestX, chestZ);

    loadObstacles();
    loadCoins();

    loaderror(&daySkyTex, "assets/textures/blu-sky-3.bmp", true);
    loaderror(&nightSkyTex, "assets/textures/orange-sky-3.bmp", true);

    initLights();

    level1Loaded = true;
}


void renderGround() {
    glEnable(GL_TEXTURE_2D);
    groundTexture.Use();
    glPushMatrix();
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glTexCoord2f(0, 0);
    glVertex3f(-250, 0, -250);
    glTexCoord2f(62.5, 0);
    glVertex3f(250, 0, -250);
    glTexCoord2f(62.5, 62.5);
    glVertex3f(250, 0, 250);
    glTexCoord2f(0, 62.5);
    glVertex3f(-250, 0, 250);
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void renderSkybox() {
    bool isDay = (timeOfDay < 0.5f);
    GLuint tex = isDay ? daySkyTex : nightSkyTex;

    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex);

    float size = 500.0f;
    glPushMatrix();
    glTranslatef(playerX, playerY, playerZ);
    // front
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(-size, size, -size);
    glTexCoord2f(1, 1); glVertex3f(size, size, -size);
    glTexCoord2f(1, 0); glVertex3f(size, -size, -size);
    glTexCoord2f(0, 0); glVertex3f(-size, -size, -size);
    glEnd();

    // back
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(size, size, size);
    glTexCoord2f(1, 1); glVertex3f(-size, size, size);
    glTexCoord2f(1, 0); glVertex3f(-size, -size, size);
    glTexCoord2f(0, 0); glVertex3f(size, -size, size);
    glEnd();

    // left
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(-size, size, size);
    glTexCoord2f(1, 1); glVertex3f(-size, size, -size);
    glTexCoord2f(1, 0); glVertex3f(-size, -size, -size);
    glTexCoord2f(0, 0); glVertex3f(-size, -size, size);
    glEnd();

    // right
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex3f(size, size, -size);
    glTexCoord2f(1, 1); glVertex3f(size, size, size);
    glTexCoord2f(1, 0); glVertex3f(size, -size, size);
    glTexCoord2f(0, 0); glVertex3f(size, -size, -size);
    glEnd();

    // top
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-size, size, size);
    glTexCoord2f(1, 0); glVertex3f(size, size, size);
    glTexCoord2f(1, 1); glVertex3f(size, size, -size);
    glTexCoord2f(0, 1); glVertex3f(-size, size, -size);
    glEnd();

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
}

void renderCoin(Coin& c) {
    if (c.collected) return;
    glPushMatrix();
    glTranslatef(c.x, c.y + coinTranslation, c.z);
    glRotatef(90.0f + coinRotationAngle, 1.0f, 0.0f, 0.0f);
    // Scale coin to half size
    glScalef(0.2f, 0.2f, 0.2f);
    c.model.Draw();
    glPopMatrix();

    coinTranslation += 0.02;
    if (coinTranslation >= 1.0f) coinTranslation -= 1.0f;

    coinRotationAngle += 0.5f;
    if (coinRotationAngle >= 360.0) coinRotationAngle -= 360.0f;
}

void renderObject(Object3D& obj) {
    if (!obj.visible) return;
    glPushMatrix();
    glTranslatef(obj.x, obj.y, obj.z);
    obj.model.Draw();
    glPopMatrix();
}

bool detectCollision(float newX, float newZ, float newY) {
    // Check all obstacles
    // Bounding sphere collision: distance < sum of radii
    // Player radius ~1.0f (adjust as needed)
    float playerRadius = 0.5f;

    // Check trees
    for (auto& t : trees) {
        if (!t.visible) continue;
        float dx = abs(newX - t.x);
        float dz = abs(newZ - t.z);
        float dy = abs(newY - t.y);
        float distSq = dx * dx * dx + dz * dz * dz + dy * dy *dy;
        float sumR = playerRadius + t.boundingRadius;
        if (distSq < sumR * sumR * sumR) {
            return true;
        }
    }
    // Check rocks
    for (auto& r : rocks) {
        if (!r.visible) continue;
        float dx = abs(newX - r.x);
        float dz = abs(newZ - r.z);
        float dy = abs(newY - r.y);
        float distSq = dx * dx * dx + dz * dz * dz + dy * dy * dy;
        float sumR = playerRadius + r.boundingRadius;
        if (distSq < sumR * sumR * sumR) {
            return true;
        }


    }

    for (auto& r : caveRocks) {
        if (!r.visible) continue;
        float dx = fabs(newX - r.x);
        float dz = fabs(newZ - r.z);
        float dy = fabs(newY - r.y);

        // Following the same logic as detectCollision in level1,
        // using cubic distance comparison:
        float distSq = dx * dx * dx + dz * dz * dz + dy * dy * dy;
        float sumR = playerRadius + r.boundingRadius;
        if (distSq < sumR * sumR * sumR) {
            return true;
        }
    }
    return false;
}
void displayBoundingSphere(float x, float y,float z,float radius) {
    glPushMatrix();

    GLUquadricObj* qobj;
    qobj = gluNewQuadric();
    glTranslated(x, y, z);
    glRotated(90, 1, 0, 1);
    gluQuadricTexture(qobj, true);
    gluQuadricNormals(qobj, GL_SMOOTH);
    gluSphere(qobj, radius, 100, 100);
    gluDeleteQuadric(qobj);
    glPopMatrix();
}

void renderCaveObject4(Object3D& obj) {
    if (!obj.visible) return;
    glPushMatrix();
    glTranslatef(obj.x, obj.y, obj.z);
    glScalef(0.05, 0.05, 0.05);
    obj.model.Draw();
    glPopMatrix();
}


bool detectCollisionBound(float newX, float newZ) {
    // Check all obstacles
    // Bounding sphere collision: distance < sum of radii
    // Player radius ~1.0f (adjust as needed)
    float playerRadius = 0.5f;

    // Edge trees
    for (auto& et : edgeTrees) {
        if (!et.visible) continue;
        float dx = newX - et.x;
        float dz = newZ - et.z;
        float distSq = dx * dx + dz * dz;
        // Edge trees also a radius guess
        float sumR = playerRadius + 5.0f; // edge trees guess radius
        if (distSq < sumR * sumR) {
            return true;
        }
    }
    return false;
}

void renderLevel1() {
    if (!level1Loaded) return;

    renderSkybox();
    renderGround();

    // Player
    glPushMatrix();
    glTranslatef(playerX, playerY, playerZ);
    glRotatef(playerRotY, 0, 1, 0);
    /*glRotatef((hitAngle > 0 ? hitAngle : 0), 1, 0, 0);*/
    glScalef((hitAngle > 0 ? 0.3 : 1), (hitAngle > 0 ? 0.3 : 1), (hitAngle > 0 ? 0.3 : 1));
    playerModel.Draw();
    glPopMatrix();

    // Chest
    glPushMatrix();
    glTranslatef(chestX, 0.0f, chestZ);
    glRotatef(180, 0, 1, 0);
    chestModel.Draw();
    
    glPopMatrix();

    // Coins
    for (auto& c : goldCoins)   renderCoin(c);
    for (auto& c : silverCoins) renderCoin(c);
    for (auto& c : bronzeCoins) renderCoin(c);

    // Trees
    for (auto& t : trees) {
        renderObject(t);
    }
    // Rocks
    for (auto& r : rocks) {
        renderObject(r);
}
    // Edge Trees
    for (auto& et : edgeTrees) {
        renderObject(et);
    }

    for (auto& r : caveRocks) renderCaveObject4(r);
}
void updateCoinPickup(Coin& c, float deltaTime) {
    if (c.beingPickedUp) {
        c.pickupTimer += deltaTime;
        // Move the coin upwards only during pickup
        c.y += deltaTime * 0.5f;
        if (c.pickupTimer > 1.0f) {
            // After 1 second, coin disappears
            c.collected = true;
            c.beingPickedUp = false;
            c.pickupTimer = 0.0f;
            disableCoinLight();

            // Add score depending on coin type
            if (c.type == COIN_GOLD) {
                score += 100;
            }
            else if (c.type == COIN_SILVER) {
                score += 50;
            }
            else if (c.type == COIN_BRONZE) {
                score += 25;
            }
        }
    }
}



void updateLevel1(float deltaTime) {
    updateDayNightCycle(deltaTime);
    for (auto& c : goldCoins) updateCoinPickup(c, deltaTime);
    for (auto& c : silverCoins) updateCoinPickup(c, deltaTime);
    for (auto& c : bronzeCoins) updateCoinPickup(c, deltaTime);
}

bool checkChestCollision(float px, float py, float pz) {
    // Determine a radius for chest collision. Let's say the player must be within 2 units
    float dx = px - chestX;
    float dz = pz - chestZ;
    float distSq = dx * dx + dz * dz;
    float radius = 2.5f; // adjust as needed
    return distSq < (radius * radius);
}

void cleanupLevel1() {
    // Clear all game objects and data related to level 1
    goldCoins.clear();
    silverCoins.clear();
    bronzeCoins.clear();
    trees.clear();
    rocks.clear();
    edgeTrees.clear();
    usedPositions.clear(); // Clear position tracking

    // Delete the sky textures
    if (daySkyTex) {
        glDeleteTextures(1, &daySkyTex);
        daySkyTex = 0;
    }
    if (nightSkyTex) {
        glDeleteTextures(1, &nightSkyTex);
        nightSkyTex = 0;
    }
    level1Loaded = false;
}
