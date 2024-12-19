#include "include/camera.h"
#include <GL/freeglut.h>
#include <cmath>

CameraMode currentCameraMode = CAMERA_FIRST_PERSON;

void initCamera() {
    currentCameraMode = CAMERA_FIRST_PERSON;
}

void cycleCameraMode() {
    if (currentCameraMode == CAMERA_FIRST_PERSON) {
        currentCameraMode = CAMERA_THIRD_PERSON;
    }
    else {
        currentCameraMode = CAMERA_FIRST_PERSON;
    }
}

void updateCamera(float playerX, float playerY, float playerZ, float playerRotY) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 1280.0 / 720.0, 0.1, 5000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (currentCameraMode == CAMERA_FIRST_PERSON) {
        float rad = playerRotY * 3.14159f / 180.0f;
        float cx = playerX + sin(rad);
        float cz = playerZ + cos(rad);
        gluLookAt(playerX, playerY + 1.8f, playerZ,
            cx, playerY + 1.8f, cz,
            0, 1, 0);
    }
    else {
        // Closer third person camera
        float dist = 5.0f; // Reduced from 10.0f
        float height = 3.0f; // Slightly lower height can also bring intimacy
        float rad = playerRotY * 3.14159f / 180.0f;
        float camX = playerX - sin(rad) * dist;
        float camZ = playerZ - cos(rad) * dist;
        gluLookAt(camX, playerY + height, camZ,
            playerX, playerY + 1.0f, playerZ,
            0, 1, 0);
    }
}
