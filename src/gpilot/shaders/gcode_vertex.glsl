#version 130

#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 u_mvp_matrix;
uniform mat4 u_mv_matrix;
uniform vec3 u_light_position;
uniform vec3 u_eye;
uniform sampler2D u_palette;

in vec3 a_position;
in uint a_color;
in vec3 a_start;
in float a_cumSegPosition;

out vec4 v_color;
out vec2 v_start;
out vec3 v_normal;
out vec3 v_light_direction;
out vec3 v_eye;
out float v_cumSegPosition;

void main()
{
    vec4 vertex_position = vec4(a_position, 1.0);
    v_normal = a_start;

    vec3 light_position_ = vec4(u_light_position, 1.0).xyz;
    v_light_direction = normalize(light_position_ - vertex_position.xyz);

    gl_Position = u_mvp_matrix * vertex_position;

    v_color = texture2D(u_palette, vec2(float(a_color) * (1.0 / 25.0) + (1.0 / 50.0), 0.0));
    v_eye = (vec4(u_eye, 1.0) * u_mvp_matrix).xyz;

    v_cumSegPosition = a_cumSegPosition;
}
