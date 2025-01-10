
const vertexShaderSource1 = `#version 300 es

#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 mvp_matrix;
uniform mat4 mv_matrix;
uniform vec3 u_light_position;
uniform bool u_shadow;
uniform vec3 u_eye;
uniform sampler2D u_palette;

in vec3 a_position;
in vec3 a_color;
in vec3 a_normal;
in float a_alpha;

out vec4 v_color;
//out vec3 v_position;
out vec2 v_start;
out vec3 v_normal;
out vec2 v_texture;
out vec3 v_light_direction;
out vec3 v_eye;

// bool isNan(float val)
// {
//     return (val > 65535.0);
// }

void main()
{
    vec4 vertex_position = vec4(a_position, 1.0);
    v_normal = a_normal;
    if (u_shadow) {
        //vertex_position += vec4(a_normal, 0.0) * 0.01;
    }

    //vec3 light_position_ = (mv_matrix * vec4(u_light_position, 1.0)).xyz;
    vec3 light_position_ = vec4(u_light_position, 1.0).xyz;
    v_light_direction = normalize(light_position_ - vertex_position.xyz);

    gl_Position = mvp_matrix * vertex_position;
    if (u_shadow) {
        // gl_Position += vec4(
        //     // (2.0 / 800.0) * a_normal,
        //     // (2.0 / 600.0) * 5.0,
        //     0.001,
        //     0.0
        // );
    }

    //v_position = vertex_position.xyz;

    v_color = vec4(a_color, 0.2);
    //v_color = texture(u_palette, vec2(a_color / 255.0, 0.0)); // Przeskaluj indeks
    v_eye = (vec4(u_eye, 1.0) * mvp_matrix).xyz;
}

    `;

const fragmentShaderSource1 = `#version 300 es

#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

//Dash grid (px) = factor * pi;
// const float factor = 2.0;

in vec4 v_color;
//in vec3 v_position;
in vec3 v_normal;
// varying vec2 v_texture;
in vec3 v_light_direction;
in vec3 v_eye;

uniform sampler2D texture;
uniform mat4 mvp_matrix;
uniform mat4 mv_matrix;
uniform bool u_shadow;

out vec4 fragColor;

// bool isNan(float val)
// {
//     return (val > 65535.0);
// }

void main()
{
    if (u_shadow) {
        //fragColor = vec4(0.2, 0.2, 0.2, 0.5);
        fragColor = vec4(1.0);
        return;
    }

    vec3 lightColor = vec3(0.5, 0.0, 0.5);

    // Draw dash lines
    // if (!isNan(v_start.x)) {
    //     vec2 sub = v_position - v_start;
    //     float coord = length(sub.x) > length(sub.y) ? gl_FragCoord.x : gl_FragCoord.y;
    //     if (cos(coord / factor) > 0.0) discard;
    // }

    vec3 viewDir = normalize(-v_eye);

    // Wektor normalny (iloczyn wektorowy wektora widzenia i kierunku linii)
    vec3 normal = normalize(cross(viewDir, v_normal));

    // calc diffuse light
    float diff = max(dot(v_normal, v_light_direction), 0.0);

    // calc specular light
    vec3 reflectDir = reflect(-v_light_direction, v_normal);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    // calc fragment color
    fragColor = vec4(v_color.rgb * (diff + 0.3) + spec, v_color.a);
}
    `;
