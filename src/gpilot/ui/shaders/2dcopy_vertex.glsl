#version 130

#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

attribute vec2 a_position;
attribute vec2 a_texcoord;

varying vec3 v_position;
varying vec2 v_texcoord;

void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
    v_texcoord = a_texcoord;
}
