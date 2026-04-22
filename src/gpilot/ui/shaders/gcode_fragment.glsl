#version 130

#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

in vec4 v_color;
in vec3 v_normal;
in vec3 v_light_direction;
in vec3 v_eye;
in float v_log_depth;
noperspective in float v_cumSegPosition;
in float v_viewDepth;

out vec4 fragColor;

// Dash pattern in screen-space pixels
float dashPixels = 10.0;
float gapPixels = 6.0;

void main()
{
    if (v_cumSegPosition >= 0.0) {
        float worldPerPixel = fwidth(v_cumSegPosition);
        if (worldPerPixel > 0.0) {
            float dashWorld = dashPixels * worldPerPixel;
            float gapWorld = gapPixels * worldPerPixel;
            float totalPattern = dashWorld + gapWorld;
            float patternPos = mod(v_cumSegPosition, totalPattern);
            if (patternPos > dashWorld) {
                discard;
            }
        }
    }

    vec3 viewDir = normalize(-v_eye);

    // normal vector pointing perpendicular to view direction and line direction
    vec3 normal = normalize(cross(viewDir, v_normal));

    // calc diffuse light
    float diff = max(dot(v_normal, v_light_direction), 0.0);

    // calc specular light
    vec3 reflectDir = reflect(-v_light_direction, v_normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    // ambient light
    float ambient = 0.6;

    // calc fragment color
    vec3 color = v_color.rgb * (diff + ambient) + spec;

    // depth-based brightness: brighten near lines, darken far lines
    float fogDensity = 0.01;
    float nearBoost = 3.5;
    float farAttenuation = 0.55;
    float fog = 1.0 - exp(-v_viewDepth * fogDensity);
    color *= mix(nearBoost, farAttenuation, clamp(fog, 0.0, 1.0));

    fragColor = vec4(color, v_color.a);
}
