#pragma once

#include <nlohmann/json.hpp>
#include <glm/glm/glm.hpp>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

struct MoleculeConfig {
    std::string atoms = "models/2RT4.pdb";
    std::string volume = "models/volmap_7.dx";
};

struct WindowConfig {
    int width = 1600;
    int height = 900;
    bool vsync = true;
};

struct CameraConfig {
    float fovDegrees = 45.0f;
    glm::vec3 positionOffset{ 0.0f, 0.0f, 100.0f };
};

struct RenderingConfig {
    bool showAtoms = true;
    bool showVolume = false;
    float volumeStepSize = 0.2f;
    int volumeMaxSteps = 512;
    unsigned int sphereSegments = 12;
    unsigned int sphereRings = 12;
    float sphereRadius = 1.0f;
};

struct GraphicsConfig {
    glm::vec3 backgroundColor{ 0.1f, 0.1f, 0.1f };
    glm::vec3 ambientLightColor{ 0.2f, 0.2f, 0.2f };
    glm::vec3 directionalLightColor{ 1.0f, 1.0f, 1.0f };
    glm::vec3 directionalLightDirection{ 1.0f, -1.0f, -1.0f };
    glm::vec3 specularColor{ 1.0f, 1.0f, 1.0f };
    float specularStrength = 0.35f;
    float shininess = 32.0f;
    int msaaSamples = 4;
};

struct AppConfig {
    int version = 1;
    MoleculeConfig molecule;
    WindowConfig window;
    CameraConfig camera;
    RenderingConfig rendering;
    std::unordered_map<std::string, glm::vec3> atomStyles;
    GraphicsConfig graphics;
};

namespace config_detail {

using Json = nlohmann::json;

inline const Json& optionalObject(const Json& parent, const char* key) {
    static const Json emptyObject = Json::object();
    const auto it = parent.find(key);
    if (it == parent.end()) {
        return emptyObject;
    }
    if (!it->is_object()) {
        throw std::runtime_error(std::string("Config section '") + key + "' must be an object");
    }
    return *it;
}

template <typename T>
inline void readIfPresent(const Json& object, const char* key, T& value) {
    const auto it = object.find(key);
    if (it != object.end()) {
        it->get_to(value);
    }
}

inline glm::vec3 readVec3(const Json& value, const std::string& name) {
    if (!value.is_array() || value.size() != 3) {
        throw std::runtime_error(name + " must be an array of three numbers");
    }

    return glm::vec3(
        value[0].get<float>(),
        value[1].get<float>(),
        value[2].get<float>());
}

inline void readVec3IfPresent(const Json& object, const char* key, glm::vec3& value) {
    const auto it = object.find(key);
    if (it != object.end()) {
        value = readVec3(*it, key);
    }
}

inline void validateColor(const glm::vec3& color, const std::string& name) {
    if (color.x < 0.0f || color.x > 1.0f ||
        color.y < 0.0f || color.y > 1.0f ||
        color.z < 0.0f || color.z > 1.0f) {
        throw std::runtime_error(name + " components must be between 0 and 1");
    }
}

} // namespace config_detail

inline AppConfig loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Unable to open config file: " + filename);
    }

    config_detail::Json json;
    try {
        file >> json;
    }
    catch (const config_detail::Json::exception& error) {
        throw std::runtime_error("Unable to parse config file '" + filename + "': " + error.what());
    }

    if (!json.is_object()) {
        throw std::runtime_error("Config root must be a JSON object");
    }

    AppConfig config;
    config_detail::readIfPresent(json, "version", config.version);
    if (config.version != 1) {
        throw std::runtime_error("Unsupported config version: " + std::to_string(config.version));
    }

    const auto& molecule = config_detail::optionalObject(json, "molecule");
    config_detail::readIfPresent(molecule, "atoms", config.molecule.atoms);
    config_detail::readIfPresent(molecule, "volume", config.molecule.volume);

    const auto& window = config_detail::optionalObject(json, "window");
    config_detail::readIfPresent(window, "width", config.window.width);
    config_detail::readIfPresent(window, "height", config.window.height);
    config_detail::readIfPresent(window, "vsync", config.window.vsync);
    if (config.window.width <= 0 || config.window.height <= 0) {
        throw std::runtime_error("Window width and height must be positive");
    }

    const auto& camera = config_detail::optionalObject(json, "camera");
    config_detail::readIfPresent(camera, "fovDegrees", config.camera.fovDegrees);
    config_detail::readVec3IfPresent(camera, "positionOffset", config.camera.positionOffset);
    if (camera.find("positionOffset") == camera.end()) {
        config_detail::readVec3IfPresent(camera, "position", config.camera.positionOffset);
    }
    if (config.camera.fovDegrees <= 0.0f || config.camera.fovDegrees >= 180.0f) {
        throw std::runtime_error("Camera fovDegrees must be between 0 and 180");
    }
    if (glm::dot(config.camera.positionOffset, config.camera.positionOffset) == 0.0f) {
        throw std::runtime_error("Camera positionOffset must not be zero");
    }

    const auto& rendering = config_detail::optionalObject(json, "rendering");
    config_detail::readIfPresent(rendering, "showAtoms", config.rendering.showAtoms);
    config_detail::readIfPresent(rendering, "showVolume", config.rendering.showVolume);
    config_detail::readIfPresent(rendering, "volumeStepSize", config.rendering.volumeStepSize);
    config_detail::readIfPresent(rendering, "volumeMaxSteps", config.rendering.volumeMaxSteps);
    config_detail::readIfPresent(rendering, "sphereSegments", config.rendering.sphereSegments);
    config_detail::readIfPresent(rendering, "sphereRings", config.rendering.sphereRings);
    config_detail::readIfPresent(rendering, "sphereRadius", config.rendering.sphereRadius);
    if (config.rendering.volumeStepSize <= 0.0f || config.rendering.volumeMaxSteps <= 0) {
        throw std::runtime_error("Volume step size and maximum steps must be positive");
    }
    if (config.rendering.sphereSegments < 3 || config.rendering.sphereSegments > 128 ||
        config.rendering.sphereRings < 2 || config.rendering.sphereRings > 128) {
        throw std::runtime_error("Sphere segments must be 3..128 and rings must be 2..128");
    }
    if (config.rendering.sphereRadius <= 0.0f || config.rendering.sphereRadius > 100.0f) {
        throw std::runtime_error("Sphere radius must be greater than 0 and at most 100");
    }

    const auto atomStyles = json.find("atomStyles");
    if (atomStyles != json.end()) {
        if (!atomStyles->is_object()) {
            throw std::runtime_error("Config section 'atomStyles' must be an object");
        }
        for (auto it = atomStyles->begin(); it != atomStyles->end(); ++it) {
            if (!it.value().is_object() || !it.value().contains("color")) {
                throw std::runtime_error("Each atom style must define a color");
            }
            const glm::vec3 color = config_detail::readVec3(
                it.value().at("color"), "atomStyles." + it.key() + ".color");
            config_detail::validateColor(color, "atomStyles." + it.key() + ".color");
            config.atomStyles.emplace(it.key(), color);
        }
    }

    const auto& graphics = config_detail::optionalObject(json, "graphics");
    config_detail::readVec3IfPresent(graphics, "backgroundColor", config.graphics.backgroundColor);
    config_detail::readVec3IfPresent(graphics, "ambientLightColor", config.graphics.ambientLightColor);
    config_detail::readVec3IfPresent(graphics, "directionalLightColor", config.graphics.directionalLightColor);
    config_detail::readVec3IfPresent(graphics, "directionalLightDirection", config.graphics.directionalLightDirection);
    config_detail::readVec3IfPresent(graphics, "specularColor", config.graphics.specularColor);
    config_detail::readIfPresent(graphics, "specularStrength", config.graphics.specularStrength);
    config_detail::readIfPresent(graphics, "shininess", config.graphics.shininess);
    config_detail::readIfPresent(graphics, "msaaSamples", config.graphics.msaaSamples);
    config_detail::validateColor(config.graphics.backgroundColor, "graphics.backgroundColor");
    config_detail::validateColor(config.graphics.ambientLightColor, "graphics.ambientLightColor");
    config_detail::validateColor(config.graphics.directionalLightColor, "graphics.directionalLightColor");
    config_detail::validateColor(config.graphics.specularColor, "graphics.specularColor");
    if (config.graphics.specularStrength < 0.0f || config.graphics.specularStrength > 1.0f) {
        throw std::runtime_error("graphics.specularStrength must be between 0 and 1");
    }
    if (config.graphics.shininess <= 0.0f || config.graphics.shininess > 256.0f) {
        throw std::runtime_error("graphics.shininess must be greater than 0 and at most 256");
    }
    if (config.graphics.msaaSamples < 0 || config.graphics.msaaSamples > 16) {
        throw std::runtime_error("graphics.msaaSamples must be between 0 and 16");
    }
    if (glm::dot(config.graphics.directionalLightDirection,
                 config.graphics.directionalLightDirection) == 0.0f) {
        throw std::runtime_error("graphics.directionalLightDirection must not be zero");
    }

    return config;
}