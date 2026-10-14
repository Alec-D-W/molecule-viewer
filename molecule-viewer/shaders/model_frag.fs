#version 330 core

// Fragment Shader (atom.frag)

in vec3 vColor;
in vec3 vNormal;
in vec3 vWorldPosition;
out vec4 fragColor;

uniform vec3 ambientLightColor;
uniform vec3 directionalLightColor;
uniform vec3 directionalLightDirection;
uniform vec3 specularColor;
uniform vec3 cameraPos;
uniform float specularStrength;
uniform float shininess;

void main() {
    vec3 normal = normalize(vNormal);
    vec3 lightDirection = normalize(-directionalLightDirection);
    vec3 viewDirection = normalize(cameraPos - vWorldPosition);
    vec3 halfwayDirection = normalize(lightDirection + viewDirection);

    float diffuse = max(dot(normal, lightDirection), 0.0);
    float specular = diffuse > 0.0
        ? pow(max(dot(normal, halfwayDirection), 0.0), shininess) * specularStrength
        : 0.0;

    vec3 diffuseLighting = ambientLightColor + directionalLightColor * diffuse;
    vec3 highlight = specularColor * directionalLightColor * specular;
    fragColor = vec4(vColor * diffuseLighting + highlight, 1.0);
}
