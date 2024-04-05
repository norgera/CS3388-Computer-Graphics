const char *vertexShader = R"glsl(
#version 330 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
out vec3 normal;
out vec3 eye_direction;
out vec3 light_direction;
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightDir;
void main() {
    gl_Position =  MVP * vec4(vertexPosition,1);
    normal = mat3(V) * vertexNormal;
    eye_direction = vec3(5, 5, 5) - (V * vec4(-vertexPosition, 1)).xyz;
    light_direction = (mat3(V) * lightDir) + eye_direction;
}
)glsl";

const char *fragmentShader = R"glsl(
#version 330 core
in vec3 normal;
in vec3 light_direction;
in vec3 eye_direction;
out vec4 color;
uniform vec4 modelColor;
void main() {
    vec4 diffuseColor = modelColor;
    vec4 ambient = vec4(0.2, 0.2, 0.2, 1) * diffuseColor; // Reduced ambient intensity
    vec4 specular = vec4(1, 1, 1, 1); // Reduced specular intensity
    float alpha = 64;
    vec3 n = normalize(normal);
    vec3 l = normalize(light_direction);
    vec3 e = normalize(eye_direction);
    vec3 r = reflect(-l, n);
    float cosTheta = clamp(dot(n, l), 0, 1);
    float cosAlpha = clamp(dot(e, r), 0, 1);
    color = ambient + (modelColor * cosTheta * 0.8) + (specular * pow(cosAlpha, alpha)); // Reduced diffuse intensity
}
)glsl";