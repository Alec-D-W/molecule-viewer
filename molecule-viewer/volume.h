#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <glad/glad.h>

struct VolumetricData {
    int nx, ny, nz;
    float origin[3] = { 0.0f,0.0f,0.0f };
    float delta[3] = { 1.0f,1.0f,1.0f }; // assuming cubic grid
    std::vector<float> values;         // scalar field
};

float sizeX;
float sizeY;
float sizeZ;

float center[3];

// Eckpunkte der Bounding Box
glm::vec3 minCorner;
glm::vec3 maxCorner;
VolumetricData volume;

VolumetricData loadDx(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) throw std::runtime_error("Unable to open .dx file");

    std::string line;
    int i = 0;

    while (std::getline(file, line)) {
        if (line.find("gridpositions counts") != std::string::npos) {
            std::istringstream iss(line);
            std::string dummy;
            iss >> dummy >> dummy >> dummy >> dummy >> dummy >> volume.nx >> volume.ny >> volume.nz;
        }
        else if (line.find("origin") != std::string::npos) {
            std::istringstream iss(line);
            std::string dummy;
            iss >> dummy >> volume.origin[0] >> volume.origin[1] >> volume.origin[2];
        }
        else if (line.find("delta") != std::string::npos) {
            std::istringstream iss(line);
            std::string dummy;
            float dx, dy, dz;
            iss >> dummy >> dx >> dy >> dz;
            if (dx != 0) volume.delta[0] = dx;
            if (dy != 0) volume.delta[1] = dy;
            if (dz != 0) volume.delta[2] = dz;
        }
        else if (line.find("data follows") != std::string::npos) {
            break;
        }
    }

    int total = volume.nx * volume.ny * volume.nz;
    volume.values.resize(total);
    float val;
    while (i < total && file >> val) {
        int x = i / (volume.ny * volume.nz);
        int y = (i / volume.nz) % volume.ny;
        int z = i % volume.nz;
        int openGLIndex = x + volume.nx * (y + volume.ny * z);
        volume.values[openGLIndex] = val;
        i++;
    }
    if (i != total)
        throw std::runtime_error("Datenmenge stimmt nicht mit Gridgröße überein");
    std::cout << i << std::endl;
    // physikalische Größe
    sizeX = (volume.nx - 1) * volume.delta[0];
    sizeY = (volume.ny - 1) * volume.delta[1];
    sizeZ = (volume.nz - 1) * volume.delta[2];

    // Mittelpunkt des Gitters (bleibt gleich)
    center[0] = volume.origin[0] + sizeX * 0.5f;
    center[1] = volume.origin[1] + sizeY * 0.5f;
    center[2] = volume.origin[2] + sizeZ * 0.5f;

    // ----- HIER IST DIE KORREKTUR -----

    // Diese globalen Variablen sind für die GEOMETRIE (den VBO)
    // Sie müssen die ÄUSSEREN KANTEN umschließen.

    glm::vec3 deltaVec(volume.delta[0], volume.delta[1], volume.delta[2]);

    // Finde das Zentrum des ersten Voxels
    glm::vec3 minCenter(volume.origin[0], volume.origin[1], volume.origin[2]);
    // Finde das Zentrum des letzten Voxels
    glm::vec3 maxCenter = minCenter + glm::vec3(sizeX, sizeY, sizeZ);

    // Erweitere die Box um ein halbes Voxel in jede Richtung
    // Diese globalen Variablen werden von BoundingBox() verwendet.
    minCorner = minCenter - 0.5f * deltaVec;
    maxCorner = maxCenter + 0.5f * deltaVec;

    std::cout << "Center: " << center[0] << " " << center[1] << " " << center[2] << std::endl;
    std::cout << "Size:   " << sizeX << " x " << sizeY << " x " << sizeZ << std::endl;
    std::cout << "Grid: " << volume.nx << " × " << volume.ny << " × " << volume.nz << std::endl;
    std::cout << "Delta: " << volume.delta[0] << " " << volume.delta[1] << " " << volume.delta[2] << std::endl;
    std::cout << "Origin: " << volume.origin[0] << " " << volume.origin[1] << " " << volume.origin[2] << std::endl;
    std::cout << "--- Bounding Box Eckpunkte zur Überprüfung ---" << std::endl;
    std::cout << "Min Corner (minCorner): (" << minCorner.x << ", " << minCorner.y << ", " << minCorner.z << ")" << std::endl;
    std::cout << "Max Corner (maxCorner): (" << maxCorner.x << ", " << maxCorner.y << ", " << maxCorner.z << ")" << std::endl;

    // Berechnung und Ausgabe der Abmessungen der Bounding Box
    glm::vec3 boxSize = maxCorner - minCorner;
    std::cout << "Bounding Box Größe: " << boxSize.x << " x " << boxSize.y << " x " << boxSize.z << std::endl;
    std::cout << "---------------------------------------------" << std::endl;

    return volume;
}

unsigned int Gen3DTex(VolumetricData volume) {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_3D, textureID);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, volume.nx, volume.ny, volume.nz,
        0, GL_RED, GL_FLOAT, volume.values.data());

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    return textureID;
}

// Cube auf Basis der Bounding Box
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void BoundingBox() {
    float cubeVertices[] = {
        // minCorner -> maxCorner
        minCorner.x, minCorner.y, minCorner.z,
        maxCorner.x, minCorner.y, minCorner.z,
        maxCorner.x, maxCorner.y, minCorner.z,
        minCorner.x, maxCorner.y, minCorner.z,
        minCorner.x, minCorner.y, maxCorner.z,
        maxCorner.x, minCorner.y, maxCorner.z,
        maxCorner.x, maxCorner.y, maxCorner.z,
        minCorner.x, maxCorner.y, maxCorner.z,
    };

    unsigned int cubeIndices[] = {
        0,2,1, 2,0,3,  4,5,6, 6,7,4,
        0,1,5, 5,4,0,  2,3,7, 7,6,2,
        0,7,3, 7,0,4,  1,2,6, 6,5,1,
    };

    unsigned int cubeEBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}
