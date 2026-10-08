#include <glm/glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>

struct SphereMesh {
    GLuint vao;
    GLuint vbo;
    GLsizei vertexCount;
};

struct Atom {
    glm::vec3 position;
    std::string element;
    float radius;
    glm::vec3 color;
};

//Hilfsstruct
struct AtomProperties {
    float radius;        // Van-der-Waals-Radius (Å)
    glm::vec3 color;     // RGB (0–1)
};

const std::unordered_map<std::string, AtomProperties> atomLookup = {
    { "H",  { 1.20f, glm::vec3(1.0f, 1.0f, 1.0f) } },        // White
    { "C",  { 1.70f, glm::vec3(0.5f, 0.5f, 0.5f) } },        // Grey
    { "N",  { 1.55f, glm::vec3(0.0f, 0.0f, 1.0f) } },        // Blue
    { "O",  { 1.52f, glm::vec3(1.0f, 0.0f, 0.0f) } },        // Red
    { "F",  { 1.47f, glm::vec3(0.0f, 1.0f, 0.0f) } },        // Green
    { "P",  { 1.80f, glm::vec3(1.0f, 0.5f, 0.0f) } },        // Orange
    { "S",  { 1.80f, glm::vec3(1.0f, 1.0f, 0.0f) } },        // Yellow
};


std::vector<Atom> loadPDB(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<Atom> atoms;

    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 4) == "ATOM" || line.substr(0, 6) == "HETATM") {
            float x = std::stof(line.substr(30, 8));
            float y = std::stof(line.substr(38, 8));
            float z = std::stof(line.substr(46, 8));
            std::string element = line.substr(77, 1);
            //std::cout << element << std::endl;
            auto pointer = atomLookup.find(element);
            atoms.push_back({ glm::vec3(x, y, z), element, pointer->second.radius*0.1f, pointer->second.color });
        }
    }
    return atoms;
}

struct AtomInstance {
    glm::vec3 position;
    float radius;
    glm::vec3 color;
    float _pad = 0.0f; // Padding für Alignment (16-Byte Alignment)
};


GLuint instanceVBO = 0;

void setupMolecule(const std::vector<Atom>& atoms, GLuint sphereVAO) {
    std::vector<AtomInstance> instances;
    for (const auto& atom : atoms) {
        instances.push_back({ atom.position, atom.radius, atom.color });
    }

    glGenBuffers(1, &instanceVBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(AtomInstance), instances.data(), GL_STATIC_DRAW);

    // Position (vec3)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(AtomInstance), (void*)0);
    glVertexAttribDivisor(1, 1);

    // Radius (float)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(AtomInstance), (void*)(offsetof(AtomInstance, radius)));
    glVertexAttribDivisor(2, 1);

    // Color (vec3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(AtomInstance), (void*)(offsetof(AtomInstance, color)));
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);
}

void drawMolecule(GLuint shaderProgram, GLuint sphereVAO, GLsizei sphereVertexCount, size_t atomCount, const glm::mat4& mvp){
    glUseProgram(shaderProgram);

    GLint mvpLoc = glGetUniformLocation(shaderProgram, "uMVP");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, &mvp[0][0]);

    glBindVertexArray(sphereVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, sphereVertexCount, atomCount);
    glBindVertexArray(0);
}

SphereMesh createSphereVAO(unsigned int segments = 12, unsigned int rings = 12, float radius = 1.0f) {
    std::vector<glm::vec3> vertices;

    for (unsigned int y = 0; y <= rings; ++y) {
        float v = (float)y / rings;
        float phi = v * glm::pi<float>();

        for (unsigned int x = 0; x <= segments; ++x) {
            float u = (float)x / segments;
            float theta = u * glm::two_pi<float>();

            float xPos = radius * sinf(phi) * cosf(theta);
            float yPos = radius * cosf(phi);
            float zPos = radius * sinf(phi) * sinf(theta);

            vertices.emplace_back(xPos, yPos, zPos);
        }
    }

    std::vector<glm::vec3> finalVerts;
    for (unsigned int y = 0; y < rings; ++y) {
        for (unsigned int x = 0; x < segments; ++x) {
            int i0 = y * (segments + 1) + x;
            int i1 = i0 + 1;
            int i2 = i0 + (segments + 1);
            int i3 = i2 + 1;

            // Zwei Dreiecke pro Quad
            finalVerts.push_back(vertices[i0]);
            finalVerts.push_back(vertices[i2]);
            finalVerts.push_back(vertices[i1]);

            finalVerts.push_back(vertices[i1]);
            finalVerts.push_back(vertices[i2]);
            finalVerts.push_back(vertices[i3]);
        }
    }

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, finalVerts.size() * sizeof(glm::vec3), finalVerts.data(), GL_STATIC_DRAW);

    // position = location 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);

    SphereMesh mesh;
    mesh.vao = vao;
    mesh.vbo = vbo;
    mesh.vertexCount = static_cast<GLsizei>(finalVerts.size());
    return mesh;
}