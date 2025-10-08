#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 u_mvp_matrix;
uniform mat4 u_mv_matrix;
uniform sampler2D u_palette;

attribute vec3 a_position;
attribute float a_color;
attribute vec3 a_start;
attribute float a_alpha;

varying vec4 v_color;
varying vec2 v_position;
varying vec2 v_start;
varying vec2 v_texture;

bool isNan(float val)
{
    return (val > 65535.0);
}

void main()
{
    // Calculate interpolated vertex position & line start point
    v_position = (u_mv_matrix * vec4(a_position, 1.0)).xy;

    v_start = a_start.xy;

    // if (!isNan(a_start.x) && !isNan(a_start.y)) {
    //     v_start = (u_mv_matrix * a_start).xy;
    //     v_texture = vec2(65536.0, 0);
    // } else {
    //     // v_start.x should be Nan to draw solid lines
    //     v_start = a_start.xy;

    //     // set texture coord
    //     v_texture = a_start.yz;

    //     // set point size
    //     if (isNan(a_start.y) && !isNan(a_start.z)) gl_PointSize = a_start.z;
    // }

    // Calculate vertex position in screen space
    gl_Position = u_mvp_matrix * vec4(a_position, 1.0);

    v_color = texture2D(u_palette, vec2(a_color * (1.0 / 25.0) + (1.0 / 50.0), 0.0));
}
