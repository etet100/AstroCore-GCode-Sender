#version 130

#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform mat4 u_mvp_matrix;
uniform sampler2D u_palette;

attribute vec3 a_position;        // Billboard center in world space
attribute vec2 a_billboardSize;   // Size in pixels
attribute vec2 a_corner;          // Corner position: (0,0), (1,0), (1,1), (0,1)
attribute vec2 a_texCoord;        // Texture coordinates from atlas
attribute float a_color;          // Color index

varying vec4 v_color;
varying vec2 v_texCoord;
varying vec2 v_corner; // For debugging

void main()
{
    // Get color from palette
    v_color = texture2D(u_palette, vec2(a_color * (1.0 / 100.0) + (1.0 / 200.0), 0.0));

    // Pass texture coordinates to fragment shader
    v_texCoord = a_texCoord;
    v_corner = a_corner;

    // PROPER BILLBOARDING: Screen-space aligned
    // Transform billboard center to clip space
    gl_Position = u_mvp_matrix * vec4(a_position, 1.0);

    // Apply offset in screen space (after projection, before perspective divide)
    // This makes the quad always face the camera
    vec2 offset = (a_corner - vec2(0.5, 0.5)) * a_billboardSize;

    // Scale by w for perspective-correct size
    gl_Position.xy += offset * 0.003 * gl_Position.w;
}
