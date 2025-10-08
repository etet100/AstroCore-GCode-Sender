#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 u_mvp_matrix;
uniform mat4 u_c_matrix;

attribute vec3 a_position;
attribute uint a_color;
attribute vec3 a_texcoord;

varying vec4 v_color;
varying vec2 v_texcoord;
varying float v_dashed;

void main()
{
    gl_Position = u_mvp_matrix * u_c_matrix * vec4(a_position, 1.0);
    //1.0 / 9.0
    v_texcoord = vec2(a_texcoord.x, a_texcoord.y * (1.0 / 9.0) + float(a_color) * (1.0 / 9.0));
    //v_color = texture2D(u_palette, vec2(a_color * (1.0 / 25.0) + (1.0 / 50.0), 0.0));
}
