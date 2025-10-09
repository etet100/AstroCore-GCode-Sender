#version 130

#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision mediump float;
#endif

uniform sampler2D u_texture;
uniform int u_mode;
// == texture size
uniform vec2 u_resolution;
uniform float u_offset;

in vec2 v_texcoord;
out vec4 fragColor;

// FXAA algorithm from NVIDIA, C# implementation by Jasper Flick, GLSL port by Dave Hoskins
// http://developer.download.nvidia.com/assets/gamedev/files/sdk/11/FXAA_WhitePaper.pdf
// https://catlikecoding.com/unity/tutorials/advanced-rendering/fxaa/

#define EDGE_STEP_COUNT 6
#define EDGE_GUESS 8.0
#define EDGE_STEPS 1.0, 1.5, 2.0, 2.0, 2.0, 4.0
const float edgeSteps[EDGE_STEP_COUNT] = float[EDGE_STEP_COUNT]( EDGE_STEPS );

float _ContrastThreshold = 0.0312;
float _RelativeThreshold = 0.063;
float _SubpixelBlending = 1.0;

vec4 Sample(sampler2D tex2D, vec2 uv) {
    return texture(tex2D, uv);
}

float SampleLuminance(sampler2D tex2D, vec2 uv) {
    return dot(Sample(tex2D, uv).rgb, vec3(0.3, 0.59, 0.11));
}

float SampleLuminance( sampler2D tex2D, vec2 texSize, vec2 uv, float uOffset, float vOffset ) {
    uv += texSize * vec2(uOffset, vOffset);
    return SampleLuminance(tex2D, uv);
}

struct LuminanceData {
    float m, n, e, s, w;
    float ne, nw, se, sw;
    float highest, lowest, contrast;
};

LuminanceData SampleLuminanceNeighborhood( sampler2D tex2D, vec2 texSize, vec2 uv ) {
    LuminanceData l;
    l.m = SampleLuminance( tex2D, uv );
    l.n = SampleLuminance( tex2D, texSize, uv,  0.0,  1.0 );
    l.e = SampleLuminance( tex2D, texSize, uv,  1.0,  0.0 );
    l.s = SampleLuminance( tex2D, texSize, uv,  0.0, -1.0 );
    l.w = SampleLuminance( tex2D, texSize, uv, -1.0,  0.0 );

    l.ne = SampleLuminance( tex2D, texSize, uv,  1.0,  1.0 );
    l.nw = SampleLuminance( tex2D, texSize, uv, -1.0,  1.0 );
    l.se = SampleLuminance( tex2D, texSize, uv,  1.0, -1.0 );
    l.sw = SampleLuminance( tex2D, texSize, uv, -1.0, -1.0 );

    l.highest = max( max( max( max( l.n, l.e ), l.s ), l.w ), l.m );
    l.lowest = min( min( min( min( l.n, l.e ), l.s ), l.w ), l.m );
    l.contrast = l.highest - l.lowest;
    return l;
}

bool ShouldSkipPixel( LuminanceData l ) {
    float threshold = max( _ContrastThreshold, _RelativeThreshold * l.highest );
    return l.contrast < threshold;
}

float DeterminePixelBlendFactor( LuminanceData l ) {
    float f = 2.0 * (l.n + l.e + l.s + l.w);
    f += l.ne + l.nw + l.se + l.sw;
    f *= 1.0 / 12.0;
    f = abs(f - l.m);
    f = clamp(f / l.contrast, 0.0, 1.0);

    float blendFactor = smoothstep(0.0, 1.0, f);
    return blendFactor * blendFactor * _SubpixelBlending;
}

struct EdgeData {
    bool isHorizontal;
    float pixelStep;
    float oppositeLuminance, gradient;
};

EdgeData DetermineEdge( vec2 texSize, LuminanceData l ) {
    EdgeData e;
    float horizontal =
        abs( l.n + l.s - 2.0 * l.m ) * 2.0 +
        abs( l.ne + l.se - 2.0 * l.e ) +
        abs( l.nw + l.sw - 2.0 * l.w );
    float vertical =
        abs( l.e + l.w - 2.0 * l.m ) * 2.0 +
        abs( l.ne + l.nw - 2.0 * l.n ) +
        abs( l.se + l.sw - 2.0 * l.s );
    e.isHorizontal = horizontal >= vertical;

    float pLuminance = e.isHorizontal ? l.n : l.e;
    float nLuminance = e.isHorizontal ? l.s : l.w;
    float pGradient = abs( pLuminance - l.m );
    float nGradient = abs( nLuminance - l.m );

    e.pixelStep = e.isHorizontal ? texSize.y : texSize.x;

    if (pGradient < nGradient) {
        e.pixelStep = -e.pixelStep;
        e.oppositeLuminance = nLuminance;
        e.gradient = nGradient;
    } else {
        e.oppositeLuminance = pLuminance;
        e.gradient = pGradient;
    }

    return e;
}

float DetermineEdgeBlendFactor( sampler2D  tex2D, vec2 texSize, LuminanceData l, EdgeData e, vec2 uv ) {
    vec2 uvEdge = uv;
    vec2 edgeStep;
    if (e.isHorizontal) {
        uvEdge.y += e.pixelStep * 0.5;
        edgeStep = vec2(texSize.x, 0.0);
    } else {
        uvEdge.x += e.pixelStep * 0.5;
        edgeStep = vec2(0.0, texSize.y);
    }

    float edgeLuminance = (l.m + e.oppositeLuminance) * 0.5;
    float gradientThreshold = e.gradient * 0.25;

    vec2 puv = uvEdge + edgeStep * edgeSteps[0];
    float pLuminanceDelta = SampleLuminance( tex2D, puv) - edgeLuminance;
    bool pAtEnd = abs(pLuminanceDelta) >= gradientThreshold;

    for (int i = 1; i < EDGE_STEP_COUNT && !pAtEnd; i++) {
        puv += edgeStep * edgeSteps[i];
        pLuminanceDelta = SampleLuminance(tex2D, puv) - edgeLuminance;
        pAtEnd = abs(pLuminanceDelta) >= gradientThreshold;
    }

    if (!pAtEnd) {
        puv += edgeStep * EDGE_GUESS;
    }

    vec2 nuv = uvEdge - edgeStep * edgeSteps[0];
    float nLuminanceDelta = SampleLuminance(tex2D, nuv) - edgeLuminance;
    bool nAtEnd = abs(nLuminanceDelta) >= gradientThreshold;

    for (int i = 1; i < EDGE_STEP_COUNT && !nAtEnd; i++) {
        nuv -= edgeStep * edgeSteps[i];
        nLuminanceDelta = SampleLuminance(tex2D, nuv) - edgeLuminance;
        nAtEnd = abs(nLuminanceDelta) >= gradientThreshold;
    }

    if ( !nAtEnd ) {
        nuv -= edgeStep * EDGE_GUESS;
    }

    float pDistance, nDistance;
    if ( e.isHorizontal ) {
        pDistance = puv.x - uv.x;
        nDistance = uv.x - nuv.x;
    } else {
        pDistance = puv.y - uv.y;
        nDistance = uv.y - nuv.y;
    }

    float shortestDistance;
    bool deltaSign;
    if ( pDistance <= nDistance ) {
        shortestDistance = pDistance;
        deltaSign = pLuminanceDelta >= 0.0;
    } else {
        shortestDistance = nDistance;
        deltaSign = nLuminanceDelta >= 0.0;
    }

    if (deltaSign == (l.m - edgeLuminance >= 0.0)) {
        return 0.0;
    }

    return 0.5 - shortestDistance / (pDistance + nDistance);
}

vec4 ApplyFXAA(sampler2D tex2D, vec2 resolution, vec2 uv) {
    vec2 texSize = 1.0 / resolution;

    LuminanceData luminance = SampleLuminanceNeighborhood(tex2D, texSize, uv);
    if ( ShouldSkipPixel( luminance ) ) {

        return Sample( tex2D, uv );

    }

    float pixelBlend = DeterminePixelBlendFactor(luminance );
    EdgeData edge = DetermineEdge(texSize, luminance);
    float edgeBlend = DetermineEdgeBlendFactor(tex2D, texSize, luminance, edge, uv);
    float finalBlend = max(pixelBlend, edgeBlend);

    if (edge.isHorizontal) {
        uv.y += edge.pixelStep * finalBlend;
    } else {
        uv.x += edge.pixelStep * finalBlend;
    }

    return Sample(tex2D, uv);
}

// hermite

float hermiteWeight(float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    return (t3 * (t * (-6.0) + 15.0) - 10.0 * t2 + 1.0);
}

vec4 BicubicHermiteTextureSample(sampler2D tex2D, vec2 resolution, vec2 P) {
    vec2 texelSize = 1.0 / resolution;
    vec2 texCoord = P * resolution - 0.5;

    vec2 f = fract(texCoord);
    texCoord = (texCoord - f + 0.5) * texelSize;

    // Używamy jednowymiarowej tablicy do przechowywania próbek
    vec4 samples[16]; // 4x4 grid
    int index = 0;

    for (int i = -1; i <= 2; ++i) {
        for (int j = -1; j <= 2; ++j) {
            vec2 sampleCoord = texCoord + vec2(i, j) * texelSize;
            samples[index] = Sample(tex2D, sampleCoord);
            index++;
        }
    }

    vec4 result = vec4(0.0);
    float sumWeights = 0.0;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            float wx = hermiteWeight(f.x - float(i - 1));
            float wy = hermiteWeight(f.y - float(j - 1));
            sumWeights += wx * wy;
            result += samples[i * 4 + j] * wx * wy;
        }
    }
    result /= sumWeights; // Normalizacja

    return result;
}

// bicubic catmull

float catmullRomWeight(float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    return 0.5 * (2.0 * t2 - 3.0 * t + t3 + 1.0);
}

// Interpolacja 1D Catmull-Rom
vec4 bicubicInterpolate(vec4 c0, vec4 c1, vec4 c2, vec4 c3, float t) {
    float w0 = catmullRomWeight(t + 1.0);
    float w1 = catmullRomWeight(t);
    float w2 = catmullRomWeight(t - 1.0);
    float w3 = catmullRomWeight(t - 2.0);

    float sumW = w0 + w1 + w2 + w3;
    return (c0 * w0 + c1 * w1 + c2 * w2 + c3 * w3) / sumW;
}

vec4 BicubicCatmullRomTextureSample(sampler2D tex, vec2 resolution, vec2 P) {
    vec2 texelSize = vec2(1.0 / resolution.x, 1.0 / resolution.y);
    vec2 texCoord = P * resolution - 0.5;

    //vec2 f = fract(texCoord);  // Części ułamkowe dla interpolacji
    vec2 f = texCoord - floor(texCoord);
    texCoord = (floor(texCoord) + 0.5) * texelSize;  // Przesunięcie do siatki pikseli

    vec4 col[4];
    for (int i = -1; i <= 2; i++) {
        col[i + 1] = bicubicInterpolate(
            Sample(tex, texCoord + vec2(-1.0, i) * texelSize),
            Sample(tex, texCoord + vec2( 0.0, i) * texelSize),
            Sample(tex, texCoord + vec2( 1.0, i) * texelSize),
            Sample(tex, texCoord + vec2( 2.0, i) * texelSize),
            f.x
        );
    }

    return mix(Sample(tex, P), bicubicInterpolate(col[0], col[1], col[2], col[3], f.y), 0.9);
    //return bicubicInterpolate(col[0], col[1], col[2], col[3], f.y);
}

// bilinear

vec4 BilinearTextureSample(sampler2D tex, vec2 resolution, vec2 P) {
    vec2 texelSize = 1.0 / resolution;
    vec2 texCoord = P * resolution - 0.5;

    vec2 f = fract(texCoord); // Frakcyjna część współrzędnych tekstury
    texCoord = (floor(texCoord) + 0.5) * texelSize; // Przesunięcie na środek pikseli

    // Pobieranie 4 próbek sąsiadujących pikseli
    vec4 c00 = Sample(tex, texCoord);
    vec4 c10 = Sample(tex, texCoord + vec2(texelSize.x, 0.0));
    vec4 c01 = Sample(tex, texCoord + vec2(0.0, texelSize.y));
    vec4 c11 = Sample(tex, texCoord + texelSize);

    // Interpolacja liniowa w poziomie
    vec4 c0 = mix(c00, c10, f.x);
    vec4 c1 = mix(c01, c11, f.x);

    // Interpolacja liniowa w pionie
    return mix(c0, c1, f.y);
}

void main() {
    if (u_mode == 0) {
        fragColor = Sample(u_texture, v_texcoord);
    } else if (u_mode == 1) {
        fragColor = ApplyFXAA(u_texture, u_resolution, v_texcoord);
    } else if (u_mode == 2) {
        fragColor = BicubicCatmullRomTextureSample(u_texture, u_resolution, v_texcoord);
    } else if (u_mode == 3) {
        fragColor = BilinearTextureSample(u_texture, u_resolution, v_texcoord);
    }
}

