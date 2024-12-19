
#include <GL/glew.h>// for playerX, playerY, playerZ, playerRotY, etc.
#include "include/level2.h"
#include <GL/freeglut.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>



extern int score;
bool level2Loaded = false;
float pickupRange2 = 1.5f;

static GLTexture groundTexture2;
static GLuint wallTexture;
static std::vector<Object3D> trees;
static Model_3DS playerModel2;
static Model_3DS altarModel;
static Model_3DS torchModel;

// Gems analogous to coins
std::vector<Gem> gem1Gems;    // like goldCoins
std::vector<Gem> gem2Gems;    // like silverCoins
std::vector<Gem> gem3Gems;    // like bronzeCoins

// Obstacles: rocks and stalagmites
static std::vector<Object3D> caveRocks;
static std::vector<Object3D> stalagmites;

// Player start at (300,0) z=0
static float playerStartX2 = 0.0f;
static float playerStartZ2 = 0.0f;

// Altar at x=300, z=100
static float altarX = 0.0f;
static float altarZ = 100.0f;

// Cave bounding box: x=250 to 350, z=-20 to 120
// We'll just prevent player from leaving this area.

// Small torchlight around the player
// We'll create a light (GL_LIGHT2) that follows the player
// No external dayNight for cave, can ignore or set stable lighting

extern bool isJumping;
extern float hitAngle;

struct CavePos {
    float x, z;
};
static std::vector<CavePos> usedPositions2;
bool detectCaveCollision(float newX, float newZ, float newY) {
    // Player radius (similar to level1)
    float playerRadius = 0.5f;

    // Check caveRocks
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
    for (auto& t : trees) {
        if (!t.visible) continue;
        float dx = abs(newX - t.x);
        float dz = abs(newZ - t.z);
        float dy = abs(newY - t.y);
        float distSq = dx * dx * dx + dz * dz * dz + dy * dy * dy;
        float sumR = playerRadius + t.boundingRadius;
        if (distSq < sumR * sumR * sumR) {
            return true;
        }
    }

    // Check stalagmites
    for (auto& s : stalagmites) {
        if (!s.visible) continue;
        float dx = fabs(newX - s.x);
        float dz = fabs(newZ - s.z);
        float dy = fabs(newY - s.y);

        float distSq = dx * dx * dx + dz * dz * dz + dy * dy * dy;
        float sumR = playerRadius + s.boundingRadius;
        if (distSq < sumR * sumR * sumR) {
            return true;
        }
    }

    return false;
}
bool positionsOverlap2(float x1, float z1, float x2, float z2, float minDist = 3.0f) {
    float dx = x1 - x2;
    float dz = z1 - z2;
    return (dx * dx + dz * dz) < (minDist * minDist);
}

int detectGemInRange(float px, float py, float pz) {
    // Similar to detectCoinInRange
    int index = 0;

    // Gem1 (like gold)
    for (int i = 0; i < (int)gem1Gems.size(); i++) {
        if (!gem1Gems[i].collected && !gem1Gems[i].beingPickedUp) {
            float dx = px - (gem1Gems[i].x -7);
            float dz = pz - gem1Gems[i].z;
            float distSq = dx * dx + dz * dz;
            if (distSq < pickupRange2 * pickupRange2) {
                return i; // gem1 index
            }
        }
    }
    index += (int)gem1Gems.size();

    // Gem2 (like silver)
    for (int i = 0; i < (int)gem2Gems.size(); i++) {
        if (!gem2Gems[i].collected && !gem2Gems[i].beingPickedUp) {
            float dx = px - (gem2Gems[i].x +2);
            float dz = pz - gem2Gems[i].z;
            float distSq = dx * dx + dz * dz;
            if (distSq < pickupRange2 * pickupRange2) {
                return index + i;
            }
        }
    }

    index += (int)gem2Gems.size();

    // Gem3 (like bronze)
    for (int i = 0; i < (int)gem3Gems.size(); i++) {
        if (!gem3Gems[i].collected && !gem3Gems[i].beingPickedUp) {
            float dx = px - (gem3Gems[i].x+4);
            float dz = pz - gem3Gems[i].z;
            float distSq = dx * dx + dz * dz;
            if (distSq < pickupRange2 * pickupRange2) {
                return index + i;
            }
        }
    }

    return -1; // No gem in range
}

void enablePlayerLight() {
    glEnable(GL_LIGHT2);

    // Purple diffuse: full red and blue to make it purple (1.0f, 0.0f, 1.0f)
    // Purple ambient: a low-intensity purple to keep the scene somewhat dark
    GLfloat purpleDiffuse[] = { 1.0f, 0.0f, 1.0f, 1.0f };
    GLfloat purpleAmbient[] = { 0.1f, 0.0f, 0.1f, 1.0f };

    glLightfv(GL_LIGHT2, GL_DIFFUSE, purpleDiffuse);
    glLightfv(GL_LIGHT2, GL_AMBIENT, purpleAmbient);

    // Attenuation stays the same as before
    glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 0.5f);
    glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.3f);
    glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.1f);
}

void disablePlayerLight() {
    glDisable(GL_LIGHT2);
}

float getBoundingRadiusForCaveModel(const char* modelPath) {
    // Similar logic or just return some guess
    std::string m = modelPath;
    if (m.find("Rock") != std::string::npos) return 2.0f;
    if (m.find("rock1") != std::string::npos) return 2.0f;
    if (m.find("Rock2") != std::string::npos) return 2.0f;
    if (m.find("minerales") != std::string::npos) return 0.5f;
    if (m.find("Tree_s") != std::string::npos) return 0.2f;
    if (m.find("Tree_m") != std::string::npos) return 0.7f;
    if (m.find("Tree_l") != std::string::npos) return 0.7f;
    if (m.find("Tree_c") != std::string::npos) return 0.5f;
    if (m.find("Tree_d") != std::string::npos) return 0.7f;
    if (m.find("log") != std::string::npos) return 1.0f;
    if (m.find("Tree1") != std::string::npos) return 5.0f;// stalagmite smaller radius

    return 1.0f;
}

// Instead of canPlaceHere logic from level1, we can reuse a similar logic for cave
bool canPlaceHereCave(float x, float z, float minDist = 3.0f) {
    // Check against used positions
    for (auto& pos : usedPositions2) {
        if (positionsOverlap2(x, z, pos.x, pos.z, minDist)) {
            return false;
        }
    }
    // Check player start
    if (positionsOverlap2(x, z, 0, 0, minDist)) return false;
    // Check chest
    if (positionsOverlap2(x, z, 0, 100, minDist)) return false;

    return true;
}

void addCaveUsedPosition(float x, float z) {
    CavePos p = { x,z };
    usedPositions2.push_back(p);
}

// Place rocks and stalagmites similarly to trees and rocks
void placeCaveObstacles() {
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
        "assets/models/cave_rocks/Rock.3DS"
    };
    int numRockModels = 1;

    const char* stalagModels[] = {
        "assets/models/stalagmites/minerales.3ds"
    };
    int numStalagModels = 1;

    float startX = -40.0f, endX = 40.0f; // inside cave area
    float startZ = -10.0f, endZ = 110.0f;

    int treesToPlace = 10;
    int attempts = 0;
    int placedTrees = 0;
    while (placedTrees < treesToPlace && attempts < 2000) {
        attempts++;
        float x = startX + (rand() / (float)RAND_MAX) * (endX - startX);
        float z = startZ + (rand() / (float)RAND_MAX) * (endZ - startZ);
        if (canPlaceHereCave(x, z, 4.0f)) {
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
            t.boundingRadius = getBoundingRadiusForCaveModel(chosenModel);
            trees.push_back(t);
            addCaveUsedPosition(x, z);
            placedTrees++;
        }
    }

    // Place about 20 rocks
    int rocksToPlace = 20;
    attempts = 0;
    int placedRocks = 0;
    while (placedRocks < rocksToPlace && attempts < 1000) {
        attempts++;
        float x = startX + (rand() / (float)RAND_MAX) * (endX - startX);
        float z = startZ + (rand() / (float)RAND_MAX) * (endZ - startZ);
        if (canPlaceHereCave(x, z, 4.0f)) {
            Object3D r;
            const char* chosenRock = rockModels[rand() % numRockModels];
            r.model.Load((char*)chosenRock);
            r.x = x + 1.52864f ; r.y = 0; r.z = z + 3.31507;
            r.visible = true;
            r.boundingRadius = getBoundingRadiusForCaveModel(chosenRock);
            caveRocks.push_back(r);
            addCaveUsedPosition(x, z);
            placedRocks++;
        }
    }

    // Place about 10 stalagmites


    int stalagToPlace = 10;
    attempts = 0;
    int placedStalag = 0;
    while (placedStalag < stalagToPlace && attempts < 1000) {
        attempts++;
        float x = startX + (rand() / (float)RAND_MAX) * (endX - startX);
        float z = startZ + (rand() / (float)RAND_MAX) * (endZ - startZ);
        if (canPlaceHereCave(x, z, 2.5f)) {
            Object3D s;
            const char* chosenStalag = stalagModels[rand() % numStalagModels];
            s.model.Load((char*)chosenStalag);
            s.x = x; s.y = 0; s.z = z;
            s.visible = true;
            s.boundingRadius = getBoundingRadiusForCaveModel(chosenStalag);
            stalagmites.push_back(s);
            addCaveUsedPosition(x, z);
            placedStalag++;
        }
    }
}
void enableGemLight(float x, float y, float z) {
    glEnable(GL_LIGHT1);

    // Set diffuse color to purplish blue
    GLfloat purplishBlueDiffuse[] = { 0.5f, 0.0f, 1.0f, 1.0f }; // Red: 0.5, Green: 0.0, Blue: 1.0 (purplish blue)

    // Set ambient color to a dim version of purplish blue
    GLfloat purplishBlueAmbient[] = { 0.1f, 0.0f, 0.3f, 1.0f }; // Dimmer ambient light (mostly blue and red)

    // Set light position
    GLfloat pos[] = { x, y, z, 1.0f };

    // Apply light properties
    glLightfv(GL_LIGHT1, GL_DIFFUSE, purplishBlueDiffuse);
    glLightfv(GL_LIGHT1, GL_AMBIENT, purplishBlueAmbient);
    glLightfv(GL_LIGHT1, GL_POSITION, pos);

    // Set attenuation factors
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.1f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.1f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.05f);
}
void disableGemLight() {
    glDisable(GL_LIGHT1);
}

void loadGems() {
    const char* gem1Model = "assets/models/gem/gem1.3ds"; // like gold
    const char* gem2Model = "assets/models/gem/gem2.3ds"; // like silver
    const char* gem3Model = "assets/models/gem/gem3.3ds"; // like bronze

    const char* gemModels[] = { gem1Model, gem2Model, gem3Model };
    int numGemModels = 3;

    float startX = -40.0f, endX = 40.0f;
    float startZ = -10.0f, endZ = 110.0f;

    int gemsToPlace = 10; // fewer gems maybe
    int attempts = 0;
    int placedGems = 0;
    while (placedGems < gemsToPlace && attempts < 1000) {
        attempts++;
        float x = startX + (rand() / (float)RAND_MAX) * (endX - startX);
        float z = startZ + (rand() / (float)RAND_MAX) * (endZ - startZ);
        if (canPlaceHereCave(x, z, 3.0f)) {
            Gem g;
            const char* chosenGem = gemModels[rand() % numGemModels];
            g.model.Load((char*)chosenGem);
            g.collected = false;
            g.beingPickedUp = false;
            g.pickupTimer = 0.0f;
            if (chosenGem == gem1Model) {
                g.x = x +3;
                g.y = 0;
                g.z = z;
            }
            else if (chosenGem == gem2Model) {
                g.x = x -2;
                g.y = 0;
                g.z = z;
            }
            else if (chosenGem == gem3Model) {
                g.x = x -2;
                g.y = 0;
                g.z = z;
            }

            std::string m = chosenGem;
            // Assign type based on chosenGem
            if (m.find("gem1") != std::string::npos) {
                g.type = GEM1; // gem1 like gold
                gem1Gems.push_back(g);
            }
            else if (m.find("gem2") != std::string::npos) {
                g.type = GEM2; // gem2 like silver
                gem2Gems.push_back(g);
            }
            else {
                g.type = GEM3; // gem3 like bronze
                gem3Gems.push_back(g);
            }

            addCaveUsedPosition(x, z);
            placedGems++;
        }
    }
}

void loadCaveTextures() {
    // Load cave ground
    // loadBMP is assumed to return GLuint or handle it as in level1 code
    // If you have a different loading mechanism, adjust accordingly.
    groundTexture2.Load("assets/textures/ground.bmp");
    loaderror(&wallTexture, "assets/textures/cavewall.bmp", true);
    // Similarly adjust if needed.
}

void initLevel2() {
    if (level2Loaded) return;

    srand((unsigned)time(NULL));

    loadCaveTextures();
    // Load player model and altar model
    playerModel2.Load((char*)"assets/models/player/Finished-adventurer.3ds");
    altarModel.Load((char*)"assets/models/altar/altar.3ds");
    torchModel.Load((char*)"assets/models/torch/torch.3ds");

    // Player start
    playerX = 0.0f;
    playerZ = 0.0f;
    playerY = 0.0f;
    playerRotY= 0.0;

    addCaveUsedPosition(playerX, playerZ);
    addCaveUsedPosition(altarX, altarZ);

    placeCaveObstacles();
    loadGems();

    enablePlayerLight(); // The torch around the player

    level2Loaded = true;
}

bool checkAltarCollision(float px, float py, float pz) {
    float dx = px - altarX;
    float dz = pz - altarZ;
    float distSq = dx * dx + dz * dz;
    float radius = 2.0f; // same logic as chest collision
    return distSq < (radius * radius);
}


void renderCaveGround() {
    glEnable(GL_TEXTURE_2D);
    groundTexture2.Use();
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

void renderCaveWalls() {
    // Render cave walls around the play area
    // For simplicity, render 4 walls close in like a cave interior
    // front wall (facing player?), back wall, left and right walls
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, wallTexture);
    glColor3d(1, 1, 0);

    // Left wall at x=250
    glPushMatrix();
    glBegin(GL_QUADS);
    glNormal3f(1, 0, 0);
    glTexCoord2f(0, 0); glVertex3f(-50, 0, -20);
    glTexCoord2f(5, 0); glVertex3f(-50, 0, 120);
    glTexCoord2f(5, 5); glVertex3f(-50, 50, 120);
    glTexCoord2f(0, 5); glVertex3f(-50, 50, -20);
    glEnd();

    // Right wall at x=350
    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glTexCoord2f(0, 0); glVertex3f(50, 0, 120);
    glTexCoord2f(5, 0); glVertex3f(50, 0, -20);
    glTexCoord2f(5, 5); glVertex3f(50, 50, -20);
    glTexCoord2f(0, 5); glVertex3f(50, 50, 120);
    glEnd();

    // Back wall at z=-20
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glTexCoord2f(0, 0); glVertex3f(-50, 0, -20);
    glTexCoord2f(5, 0); glVertex3f(50, 0, -20);
    glTexCoord2f(5, 5); glVertex3f(50, 50, -20);
    glTexCoord2f(0, 5); glVertex3f(-50, 50, -20);
    glEnd();

    // Front wall at z=120
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    glTexCoord2f(0, 0); glVertex3f(-50, 0, 120);
    glTexCoord2f(5, 0); glVertex3f(50, 0, 120);
    glTexCoord2f(5, 5); glVertex3f(50, 50, 120);
    glTexCoord2f(0, 5); glVertex3f(-50, 50, 120);
    glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

void renderGems(Gem& g) {
    if (g.collected) return;
    glPushMatrix();
    glTranslatef(g.x, g.y , g.z);
    glScalef(0.1, 0.1, 0.1);
    g.model.Draw();
    glPopMatrix();
}

void renderCaveObject(Object3D& obj) {
    if (!obj.visible) return;
    glPushMatrix();
    glTranslatef(obj.x, obj.y, obj.z);
    glScalef(0.3, 0.3, 0.3);
    obj.model.Draw();
    glPopMatrix();
}

void renderObject2(Object3D& obj) {
    if (!obj.visible) return;
    glPushMatrix();
    glTranslatef(obj.x, obj.y, obj.z);
    obj.model.Draw();
    glPopMatrix();
}

void renderCaveObject2(Object3D& obj) {
    if (!obj.visible) return;
    glPushMatrix();
    glTranslatef(obj.x, obj.y, obj.z);
    glScalef(0.05, 0.05, 0.05);
    obj.model.Draw();
    glPopMatrix();
}
void displayBoundingSphere2(float x, float y, float z, float radius) {
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

void renderLevel2() {
    if (!level2Loaded) return;

    // Position player light at player location each frame
    GLfloat torchPos[] = { playerX, playerY + 1.8f, playerZ, 1.0f };
    glLightfv(GL_LIGHT2, GL_POSITION, torchPos);


    renderCaveGround();

    renderCaveWalls();

    // Player
    glPushMatrix();
    glTranslatef(playerX, playerY, playerZ);
    glRotatef(playerRotY, 0, 1, 0);
    //glRotatef((hitAngle > 0 ? hitAngle : 0), 1, 0, 0);
    glScalef((hitAngle > 0 ? 0.3 : 1), (hitAngle > 0 ? 0.3 : 1), (hitAngle > 0 ? 0.3 : 1));
    playerModel2.Draw();
    glPushMatrix();
    glScalef(0.05f, 0.05f, 0.05f);
    glTranslatef(15.0f, 30.0f, 0.0f); // position torch in front/right of player
    torchModel.Draw();
    glPopMatrix();
    glPopMatrix();

    // Altar
    glPushMatrix();
    glTranslatef(altarX, 0.0f, altarZ);
    glScalef(50.0f, 50.0f, 50.0f);
    altarModel.Draw();
    glPopMatrix();

    for (auto& g : gem1Gems) {
        renderGems(g);
    }
    for (auto& g : gem2Gems) { renderGems(g);
    }
    for (auto& g : gem3Gems) { renderGems(g);
    }

    for (auto& t : trees) {
        renderObject2(t);
    }

    for (auto& r : caveRocks) renderCaveObject2(r);
   
    for (auto& s : stalagmites) renderCaveObject(s);
}

void updateGemPickup(Gem& g, float deltaTime) {
    if (g.beingPickedUp) {
        g.pickupTimer += deltaTime;
        g.y += deltaTime * 0.5f;
        if (g.pickupTimer > 1.0f) {
            g.collected = true;
            g.beingPickedUp = false;
            g.pickupTimer = 0.0f;
            disableGemLight();
            // Score depending on type
            if (g.type == GEM1) score += 100;
            else if (g.type == GEM2) score += 50;
            else if (g.type == GEM3) score += 25;
        }
    }
}

void updateLevel2(float deltaTime) {
    // No dayNight here or stable?
    // Just update gems pickup
    for (auto& g : gem1Gems) updateGemPickup(g, deltaTime);
    for (auto& g : gem2Gems) updateGemPickup(g, deltaTime);
    for (auto& g : gem3Gems) updateGemPickup(g, deltaTime);
}


void cleanupLevel2() {
    gem1Gems.clear();
    gem2Gems.clear();
    gem3Gems.clear();
    caveRocks.clear();
    stalagmites.clear();
    usedPositions2.clear();

    // Delete textures if needed:
    

    level2Loaded = false;
}
