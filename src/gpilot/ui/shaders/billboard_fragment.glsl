#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform sampler2D u_billboardTexture;

varying vec4 v_color;
varying vec2 v_texCoord;
varying vec2 v_corner;

void main()
{
    // Sample the text texture
    vec4 texColor = texture2D(u_billboardTexture, v_texCoord);

    // Discard fully transparent pixels
    if (texColor.a < 0.01) {
        discard;
    }

    // Combine texture with color from palette
    gl_FragColor = texColor * v_color;
}
