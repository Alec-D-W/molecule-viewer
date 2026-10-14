#version 330 core

// Vertex Shader (atom.vert)

layout(location = 0) in vec3 aPos;         // Kugel-Vertex
layout(location = 1) in vec3 iPosition;    // Instanz: Atomposition
layout(location = 2) in float iRadius;     // Instanz: Radius
layout(location = 3) in vec3 iColor;       // Instanz: Farbe

uniform mat4 uMVP;
out vec3 vColor;
out vec3 vNormal;
out vec3 vWorldPosition;

void main() {
    vec3 scaled = aPos * iRadius;
    vec3 worldPosition = iPosition + scaled;
    gl_Position = uMVP * vec4(worldPosition, 1.0);
    vColor = iColor;
    vNormal = normalize(aPos);
    vWorldPosition = worldPosition;
}
