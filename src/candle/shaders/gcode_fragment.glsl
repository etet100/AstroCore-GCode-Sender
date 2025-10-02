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
in float v_cumSegPosition;

uniform sampler2D u_texture;

out vec4 fragColor;

float dashLength = 0.5;
float gapLength = 0.3;
float totalPatternLength = dashLength + gapLength;

void main()
{
    float patternPos = mod(v_cumSegPosition, totalPatternLength);
    if (v_cumSegPosition >= 0 && patternPos > dashLength) {
        discard;
    }

    vec3 viewDir = normalize(-v_eye);

    // Wektor normalny (iloczyn wektorowy wektora widzenia i kierunku linii)
    vec3 normal = normalize(cross(viewDir, v_normal));

    // calc diffuse light
    float diff = max(dot(v_normal, v_light_direction), 0.0);

    // calc specular light
    vec3 reflectDir = reflect(-v_light_direction, v_normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    // ambient light
    float ambient = 0.7;

    // calc fragment color
    fragColor = vec4(v_color.rgb * (diff + ambient) + spec, v_color.a);
}
