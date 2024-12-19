#ifndef CAMERA_H
#define CAMERA_H

enum CameraMode {
    CAMERA_FIRST_PERSON,
    CAMERA_THIRD_PERSON
};

extern CameraMode currentCameraMode;

// The camera will follow or be at the player's position
void initCamera();
void updateCamera(float playerX, float playerY, float playerZ, float playerRotY);
void cycleCameraMode();

#endif
