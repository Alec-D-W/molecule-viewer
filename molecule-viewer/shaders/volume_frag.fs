#version 330 core
out vec4 FragColor;

uniform sampler3D volumeTex;
uniform mat4 invMVP;       // inverse(proj * view * model)
uniform vec2 screenSize;   // in pixels

uniform vec3 minCorner;    // min Eckpunkt des Volumens
uniform vec3 maxCorner;    // max Eckpunkt des Volumens
uniform vec3 cameraPos;

const int MAX_STEPS = 512;
const float STEP_SIZE = 0.2;

vec4 transferFunction(float value) {
    float v = smoothstep(0.0, 1.0, value);
    vec4 color;
    color.rgb = mix(vec3(0.0, 0.1, 0.8), vec3(0.2, 1.0, 0.3), v);
    color.rgb = mix(color.rgb, vec3(0.9, 0.9, 0.2), v * v);
    color.a = pow(v, 2.5) * 0.5;
    return color;
}

bool intersectBox(vec3 rayOrigin, vec3 rayDir, out float tmin, out float tmax)
{
    vec3 invDir = 1.0 / rayDir;
    vec3 t0 = (minCorner - rayOrigin) * invDir;
    vec3 t1 = (maxCorner - rayOrigin) * invDir;

    vec3 tsmaller = min(t0, t1);
    vec3 tbigger  = max(t0, t1);

    tmin = max(max(tsmaller.x, tsmaller.y), tsmaller.z);
    tmax = min(min(tbigger.x, tbigger.y), tbigger.z);

    return tmax >= max(tmin, 0.0);
}

void main()
{
    // 1. NDC-Koordinaten aus gl_FragCoord berechnen
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 ndcXY = (fragCoord / screenSize) * 2.0 - 1.0;

    // 2. Einen Punkt auf der fernen Ebene (Far Plane) in Weltkoordinaten finden
    // Wir brauchen diesen Punkt, um die Strahlrichtung zu bestimmen
    vec4 ndcFar  = vec4(ndcXY,  1.0, 1.0);
    vec4 pFar  = invMVP * ndcFar;  pFar  /= pFar.w;

    // 3. Strahl von der KAMERA-POSITION starten, nicht von der Near-Plane
    vec3 rayOrigin = cameraPos;
    vec3 rayDir = normalize(pFar.xyz - cameraPos);

    // 4. Schnittpunkt des Strahls (von der Kamera aus) mit der Bounding Box finden
    float tmin, tmax;
    if (!intersectBox(rayOrigin, rayDir, tmin, tmax)) discard;

    // 5. Korrekten Startpunkt f�r das Ray-Marching bestimmen
    // tmin = Distanz von Kamera zum Box-Eintritt
    // tmax = Distanz von Kamera zum Box-Austritt
    //
    // - Wenn Kamera AUSSEN: tmin > 0. Wir starten bei tmin.
    // - Wenn Kamera INNEN:  tmin < 0. Wir starten bei t = 0 (also bei der Kamera).
    float t = max(tmin, 0.0);

    // 6. Startposition in Weltkoordinaten berechnen
    vec3 pos = rayOrigin + t * rayDir;

    vec4 color = vec4(0.0);

    // 7. Ray-Marching-Schleife (Rest ist identisch)
    for (int i = 0; i < MAX_STEPS && t < tmax; ++i) {
        
        // Weltkoordinaten -> Texturkoordinaten [0,1]
        vec3 texPos = (pos - minCorner) / (maxCorner - minCorner);
        texPos = clamp(texPos, 0.0, 1.0); // Sicherheitshalber clippen

        // Sampling
        float density = texture(volumeTex, texPos).r;
        vec4 sample = transferFunction(density);

        // Compositing (Front-to-Back)
        color.rgb += (1.0 - color.a) * sample.a * sample.rgb;
        color.a += (1.0 - color.a) * sample.a;

        if (color.a >= 0.99) break; // Early exit

        // N�chster Schritt
        t += STEP_SIZE;
        pos += STEP_SIZE * rayDir;
    }

    FragColor = color;
}
