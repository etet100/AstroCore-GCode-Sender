#version 130

#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform mat4 u_mvp_matrix;
uniform sampler2D u_palette;
uniform int u_scaleWithDistance;  // 1 = scale with distance, 0 = constant screen size
uniform float u_globalScale;      // Global scale multiplier
uniform int u_isOrthographic;     // 1 = orthographic projection, 0 = perspective
uniform float u_aspectRatio;      // Viewport width / height

attribute vec3 a_position;        // Billboard center in world space
attribute vec2 a_billboardSize;   // Size in pixels
attribute vec2 a_corner;          // Corner position: (0,0), (1,0), (1,1), (0,1)
attribute vec2 a_texCoord;        // Texture coordinates from atlas

varying vec2 v_texCoord;
varying vec2 v_corner; // For debugging

void main()
{
    // Pass texture coordinates to fragment shader
    v_texCoord = a_texCoord;
    v_corner = a_corner;

    // PROPER BILLBOARDING: Screen-space aligned
    // Transform billboard center to clip space
    gl_Position = u_mvp_matrix * vec4(a_position, 1.0);

    // Apply offset in screen space (after projection, before perspective divide)
    // This makes the quad always face the camera
    vec2 offset = (a_corner - vec2(0.5, 0.5)) * a_billboardSize;

    // Apply global scale
    offset *= u_globalScale;

    // Compensate for aspect ratio to maintain square billboards
    offset.x /= u_aspectRatio;

    // Different handling based on scaleWithDistance setting and projection mode
    if (u_isOrthographic == 1) {
        // Orthographic projection: w = 1.0, need smaller multiplier
        gl_Position.xy += offset * 0.001;
    } else {
        // Perspective projection
        if (u_scaleWithDistance == 1) {
            // Scale with distance: no w multiplication (farther = smaller after perspective divide)
            gl_Position.xy += offset * 0.3;
        } else {
            // Constant screen size: multiply by w (compensates for perspective divide)
            gl_Position.xy += offset * 0.001 * gl_Position.w;
        }
    }
}
