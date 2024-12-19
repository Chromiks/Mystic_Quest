#ifndef LEVEL1_H
#define LEVEL1_H
#include "include/game.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>



enum CoinType { COIN_GOLD, COIN_SILVER, COIN_BRONZE };


struct Coin {
    Model_3DS model;
    float x, y, z;
    bool collected;
    bool beingPickedUp;
    float pickupTimer;
    CoinType type; // Store the type of the coin
};

extern bool level1Loaded;

// Coin vectors
extern std::vector<Coin> goldCoins;
extern std::vector<Coin> silverCoins;
extern std::vector<Coin> bronzeCoins;

// Initialization and cleanup
void initLevel1();
void cleanupLevel1();

// Rendering and updating
void renderLevel1();
void updateLevel1(float deltaTime);

// Pickup and collision related functions
int detectCoinInRange(float px, float py, float pz);
void enableCoinLight(float x, float y, float z);
void disableCoinLight();
bool checkChestCollision(float px, float py, float pz);



#endif
