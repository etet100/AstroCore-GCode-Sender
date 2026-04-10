#version 130

#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform mat4 u_mvp_matrix;
uniform mat4 u_mv_matrix;
uniform mat4 u_model_matrix;
uniform sampler2D u_palette;
uniform vec3 u_light_position;
uniform int u_light;
uniform float u_point_size;
uniform bool u_flat_shading;

attribute vec3 a_position;
attribute uint a_color;
attribute vec3 a_normal;
attribute float a_alpha;

varying vec4 v_color;
varying vec2 v_position;
varying vec2 v_texture;

bool isNan(float val)
{
    return (val > 65535.0);
}

void main()
{
    gl_PointSize = u_point_size;

    // Calculate interpolated vertex position & line start point
    v_position = (u_mv_matrix * u_model_matrix * vec4(a_position, 1.0)).xy;

    // Calculate vertex position in screen space
    gl_Position = u_mvp_matrix * u_model_matrix * vec4(a_position, 1.0);

    vec4 baseColor = texture2D(u_palette, vec2(a_color * (1.0 / 100.0) + (1.0 / 200.0), 0.0));

    float diffuse;
    if (u_light != 0) {
        // Transform the normal to camera space
        vec3 transformedNormal = normalize(mat3(u_mv_matrix * u_model_matrix) * a_normal);
        // Transform the vertex position to camera space
        vec3 vertexPos = vec3(u_mv_matrix * u_model_matrix * vec4(a_position, 1.0));
        // Light direction in camera space
        vec3 lightDir = normalize(u_light_position - vertexPos);

        // Swap normal if facing away from light
        if (dot(transformedNormal, lightDir) < 0.0) {
            transformedNormal = -transformedNormal;
        }

        // Lambertian diffuse shading
        diffuse = max(dot(transformedNormal, lightDir), 0.0);
    } else if (!u_flat_shading) {
        // Simple camera-based shading when light is disabled
        vec3 transformedNormal = normalize(mat3(u_mv_matrix * u_model_matrix) * a_normal);
        // Use normal Z component (facing camera) for shading
        diffuse = abs(transformedNormal.z) * 0.5 + 0.5;
    } else {
        diffuse = 1.0;
    }

    // Ambient + diffuse
    v_color = vec4((baseColor * (0.3 + 0.7 * diffuse)).rgb, baseColor.a);
}
