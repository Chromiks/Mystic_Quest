#ifndef LEVEL2_H
#define LEVEL2_H

#include "include/game.h"
#include <vector>



// Gem types analogous to coin types
enum GemType {
    GEM1, // like gold
    GEM2, // like silver
    GEM3  // like bronze
};

// Gem structure analogous to Coin
struct Gem {
    Model_3DS model;
    float x, y, z;
    bool collected;
    bool beingPickedUp;
    float pickupTimer;
    GemType type;
};



// Global variables
extern bool level2Loaded;

// Gem vectors
extern std::vector<Gem> gem1Gems;
extern std::vector<Gem> gem2Gems;
extern std::vector<Gem> gem3Gems;

// Cave obstacles
extern std::vector<Object3D> caveRocks;
extern std::vector<Object3D> stalagmites;

// Functions for Level 2 initialization, update, render, cleanup
void initLevel2();
void updateLevel2(float deltaTime);
void renderLevel2();
void cleanupLevel2();

// Collision checks
bool checkAltarCollision(float px, float py, float pz);
int detectGemInRange(float px, float py, float pz);
bool detectCaveCollision(float newX, float newZ, float newY); // Similar to detectCollision but for cave

// Lighting functions for player torch and gems
void enablePlayerLight();
void disablePlayerLight();
void enableGemLight(float x, float y, float z);
void disableGemLight();

// (If needed) A function to render the game win screen, declared here if it's called in game.cpp
// You must implement it somewhere else.
// void renderGamewin();

#endif
