#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/texture.h>

// Function declaration for loading a texture
GLuint loadTextures(const char* filepath);

#endif // TEXTURE_H
