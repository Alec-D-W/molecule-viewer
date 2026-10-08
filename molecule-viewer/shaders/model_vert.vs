// Vertex Shader (atom.vert)

layout(location = 0) in vec3 aPos;         // Kugel-Vertex
layout(location = 1) in vec3 iPosition;    // Instanz: Atomposition
layout(location = 2) in float iRadius;     // Instanz: Radius
layout(location = 3) in vec3 iColor;       // Instanz: Farbe

uniform mat4 uMVP;
out vec3 vColor;

void main() {
    vec3 scaled = aPos * iRadius;
    gl_Position = uMVP * vec4(iPosition + scaled, 1.0);
    vColor = iColor;
}
