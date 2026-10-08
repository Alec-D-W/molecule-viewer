// Korrigierter Vertex-Shader
#version 330 core

layout (location = 0) in vec3 aPos; // Vertex-Position im Model Space (-0.5 bis +0.5)

uniform mat4 mvp; // Model-View-Projection Matrix aus dem C++ Code

void main()
{
    // Transformiere den Vertex in den Clip Space
    gl_Position = mvp * vec4(aPos, 1.0);
}