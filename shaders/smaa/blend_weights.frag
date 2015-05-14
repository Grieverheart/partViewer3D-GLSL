#version 330 core

#define SMAA_PRESET_MEDIUM

#if defined(SMAA_PRESET_LOW)
#define SMAA_MAX_SEARCH_STEPS_DIAG 0
#define SMAA_CORNER_ROUNDING 100
#elif defined(SMAA_PRESET_MEDIUM)
#define SMAA_MAX_SEARCH_STEPS_DIAG 0
#define SMAA_CORNER_ROUNDING 100
#elif defined(SMAA_PRESET_HIGH)
#define SMAA_MAX_SEARCH_STEPS_DIAG 8
#define SMAA_CORNER_ROUNDING 25
#elif defined(SMAA_PRESET_ULTRA)
#define SMAA_MAX_SEARCH_STEPS_DIAG 16
#define SMAA_CORNER_ROUNDING 25
#endif

#define SMAA_AREATEX_MAX_DISTANCE 16
#define SMAA_AREATEX_MAX_DISTANCE_DIAG 20
#define SMAA_AREATEX_PIXEL_SIZE (1.0 / vec2(160.0, 560.0))
#define SMAA_AREATEX_SUBTEX_SIZE (1.0 / 7.0)

#define SMAA_PIXEL_SIZE vec2(1.0 / 600.0, 1.0 / 600.0)
#define SMAAMad(a, b, c) (a * b + c)
#define SMAASaturate(a) clamp(a, 0.0, 1.0)

uniform sampler2D edgesTex;
uniform sampler2D areaTex;
uniform sampler2D searchTex;

noperspective in vec2 texcoord;
noperspective in vec2 pixcoord;
noperspective in vec4 offset[3];

layout(location = 0) out vec4 outColor;

#if (SMAA_MAX_SEARCH_STEPS_DIAG > 0) || defined(SMAA_FORCE_DIAGONAL_DETECTION)

/**
 * These functions allows to perform diagonal pattern searches.
 */
float SMAASearchDiag1(vec2 texcoord, vec2 dir, float c) {
    texcoord += dir * SMAA_PIXEL_SIZE;
    vec2 e = vec2(0.0, 0.0);
    float i;
    for (i = 0.0; i < float(SMAA_MAX_SEARCH_STEPS_DIAG); i++) {
        e.rg = texture(edgesTex, texcoord).rg;
        if (dot(e, vec2(1.0, 1.0)) < 1.9) break;
        texcoord += dir * SMAA_PIXEL_SIZE;
    }
    return i + float(e.g > 0.9) * c;
}

float SMAASearchDiag2(vec2 texcoord, vec2 dir, float c) {
    texcoord += dir * SMAA_PIXEL_SIZE;
    vec2 e = vec2(0.0, 0.0);
    float i;
    for (i = 0.0; i < float(SMAA_MAX_SEARCH_STEPS_DIAG); i++) {
        e.g = texture(edgesTex, texcoord).g;
        e.r = textureOffset(edgesTex, texcoord, ivec2(1, 0)).r;
        if (dot(e, vec2(1.0, 1.0)) < 1.9) break;
        texcoord += dir * SMAA_PIXEL_SIZE;
    }
    return i + float(e.g > 0.9) * c;
}

/** 
 * Similar to SMAAArea, this calculates the area corresponding to a certain
 * diagonal distance and crossing edges 'e'.
 */
vec2 SMAAAreaDiag(vec2 dist, vec2 e, float offset) {
    vec2 texcoord = float(SMAA_AREATEX_MAX_DISTANCE_DIAG) * e + dist;

    // We do a scale and bias for mapping to texel space:
    texcoord = SMAA_AREATEX_PIXEL_SIZE * texcoord + (0.5 * SMAA_AREATEX_PIXEL_SIZE);

    // Diagonal areas are on the second half of the texture:
    texcoord.x += 0.5;

    // Move to proper place, according to the subpixel offset:
    texcoord.y += SMAA_AREATEX_SUBTEX_SIZE * offset;

    return texture(areaTex, texcoord).rg;
}

/**
 * This searches for diagonal patterns and returns the corresponding weights.
 */
vec2 SMAACalculateDiagWeights(vec2 texcoord, vec2 e) {
    vec2 weights = vec2(0.0, 0.0);

    vec2 d;
    d.x = (e.r > 0.0)? SMAASearchDiag1(texcoord, vec2(-1.0,  1.0), 1.0) : 0.0;
    d.y = SMAASearchDiag1(texcoord, vec2(1.0, -1.0), 0.0);

    if(d.r + d.g > 2.0){ // d.r + d.g + 1 > 3
        vec4 coords = SMAAMad(vec4(-d.r, d.r, d.g, -d.g), SMAA_PIXEL_SIZE.xyxy, texcoord.xyxy);

        vec4 c;
        c.x = textureOffset(edgesTex, coords.xy, ivec2(-1,  0)).g;
        c.y = textureOffset(edgesTex, coords.xy, ivec2( 0,  0)).r;
        c.z = textureOffset(edgesTex, coords.zw, ivec2( 1,  0)).g;
        c.w = textureOffset(edgesTex, coords.zw, ivec2( 1, -1)).r;
        vec2 e = 2.0 * c.xz + c.yw;
        float t = float(SMAA_MAX_SEARCH_STEPS_DIAG) - 1.0;
        e *= step(d.rg, vec2(t, t));

        weights += SMAAAreaDiag(d, e, 0.0);
    }

    d.x = SMAASearchDiag2(texcoord, vec2(-1.0, -1.0), 0.0);
    float right = textureOffset(edgesTex, texcoord, ivec2(1, 0)).r;
    d.y = right > 0.0? SMAASearchDiag2(texcoord, vec2(1.0, 1.0), 1.0) : 0.0;

    if (d.r + d.g > 2.0) { // d.r + d.g + 1 > 3
        vec4 coords = SMAAMad(vec4(-d.r, -d.r, d.g, d.g), SMAA_PIXEL_SIZE.xyxy, texcoord.xyxy);

        vec4 c;
        c.x  = textureOffset(edgesTex, coords.xy, ivec2(-1,  0)).g;
        c.y  = textureOffset(edgesTex, coords.xy, ivec2( 0, -1)).r;
        c.zw = textureOffset(edgesTex, coords.zw, ivec2( 1,  0)).gr;
        vec2 e = 2.0 * c.xz + c.yw;
        float t = float(SMAA_MAX_SEARCH_STEPS_DIAG) - 1.0;
        e *= step(d.rg, vec2(t, t));

        weights += SMAAAreaDiag(d, e, 0.0).gr;
    }

    return weights;
}
#endif

/**
 * Ok, we have the distance and both crossing edges. So, what are the areas
 * at each side of current edge?
 */
vec2 SMAAArea(vec2 dist, float e1, float e2, float offset) {
    // Rounding prevents precision errors of bilinear filtering:
    vec2 texcoord = float(SMAA_AREATEX_MAX_DISTANCE) * round(4.0 * vec2(e1, e2)) + dist;

    // We do a scale and bias for mapping to texel space:
    texcoord = SMAA_AREATEX_PIXEL_SIZE * texcoord + (0.5 * SMAA_AREATEX_PIXEL_SIZE);

    // Move to proper place, according to the subpixel offset:
    texcoord.y += SMAA_AREATEX_SUBTEX_SIZE * offset;

    // Do it!
    return texture(areaTex, texcoord).rg;
}

/**
 * This allows to determine how much length should we add in the last step
 * of the searches. It takes the bilinearly interpolated edge (see
 * @PSEUDO_GATHER4), and adds 0, 1 or 2, depending on which edges and
 * crossing edges are active.
 */
float SMAASearchLength(vec2 e, float bias, float scale) {
    // Not required if searchTex accesses are set to point:
    // vec2 SEARCH_TEX_PIXEL_SIZE = 1.0 / vec2(66.0, 33.0);
    // e = vec2(bias, 0.0) + 0.5 * SEARCH_TEX_PIXEL_SIZE +
    //     e * vec2(scale, 1.0) * vec2(64.0, 32.0) * SEARCH_TEX_PIXEL_SIZE;
    e.r = bias + e.r * scale;
	e.g = -e.g;
    return 255.0 * texture(searchTex, e).r;
}

/**
 * Horizontal/vertical search functions for the 2nd pass.
 */
float SMAASearchXLeft(vec2 texcoord, float end) {
    /**
     * @PSEUDO_GATHER4
     * This texcoord has been offset by (-0.25, -0.125) in the vertex shader to
     * sample between edge, thus fetching four edges in a row.
     * Sampling with different offsets in each direction allows to disambiguate
     * which edges are active from the four fetched ones.
     */
    vec2 e = vec2(0.0, 1.0);
    while (texcoord.x > end &&
           e.g > 0.8281 && // Is there some edge not activated?
           e.r == 0.0) { // Or is there a crossing edge that breaks the line?
        e = texture(edgesTex, texcoord).rg;
        texcoord -= vec2(2.0, 0.0) * SMAA_PIXEL_SIZE;
    }

    // We correct the previous (-0.25, -0.125) offset we applied:
    texcoord.x += 0.25 * SMAA_PIXEL_SIZE.x;

    // The searches are bias by 1, so adjust the coords accordingly:
    texcoord.x += SMAA_PIXEL_SIZE.x;

    // Disambiguate the length added by the last step:
    texcoord.x += 2.0 * SMAA_PIXEL_SIZE.x; // Undo last step
    texcoord.x -= SMAA_PIXEL_SIZE.x * SMAASearchLength(e, 0.0, 0.5);

    return texcoord.x;
}

float SMAASearchXRight(vec2 texcoord, float end) {
    vec2 e = vec2(0.0, 1.0);
    while (texcoord.x < end &&
           e.g > 0.8281 && // Is there some edge not activated?
           e.r == 0.0) { // Or is there a crossing edge that breaks the line?
        e = texture(edgesTex, texcoord).rg;
        texcoord += vec2(2.0, 0.0) * SMAA_PIXEL_SIZE;
    }

    texcoord.x -= 0.25 * SMAA_PIXEL_SIZE.x;
    texcoord.x -= SMAA_PIXEL_SIZE.x;
    texcoord.x -= 2.0 * SMAA_PIXEL_SIZE.x;
    texcoord.x += SMAA_PIXEL_SIZE.x * SMAASearchLength(e, 0.5, 0.5);
    return texcoord.x;
}

float SMAASearchYUp(vec2 texcoord, float end) {
    vec2 e = vec2(1.0, 0.0);
    while (texcoord.y > end &&
           e.r > 0.8281 && // Is there some edge not activated?
           e.g == 0.0) { // Or is there a crossing edge that breaks the line?
        e = texture(edgesTex, texcoord).rg;
        texcoord -= vec2(0.0, 2.0) * SMAA_PIXEL_SIZE;
    }

    texcoord.y += 0.25 * SMAA_PIXEL_SIZE.y;
    texcoord.y += SMAA_PIXEL_SIZE.y;
    texcoord.y += 2.0 * SMAA_PIXEL_SIZE.y;
    texcoord.y -= SMAA_PIXEL_SIZE.y * SMAASearchLength(e.gr, 0.0, 0.5);
    return texcoord.y;
}

float SMAASearchYDown(vec2 texcoord, float end) {
    vec2 e = vec2(1.0, 0.0);
    while (texcoord.y < end &&
           e.r > 0.8281 && // Is there some edge not activated?
           e.g == 0.0) { // Or is there a crossing edge that breaks the line?
        e = texture(edgesTex, texcoord).rg;
        texcoord += vec2(0.0, 2.0) * SMAA_PIXEL_SIZE;
    }

    texcoord.y -= 0.25 * SMAA_PIXEL_SIZE.y;
    texcoord.y -= SMAA_PIXEL_SIZE.y;
    texcoord.y -= 2.0 * SMAA_PIXEL_SIZE.y;
    texcoord.y += SMAA_PIXEL_SIZE.y * SMAASearchLength(e.gr, 0.5, 0.5);
    return texcoord.y;
}

//-----------------------------------------------------------------------------
// Corner Detection Functions

void SMAADetectHorizontalCornerPattern(inout vec2 weights, vec2 texcoord, vec2 d) {
    #if (SMAA_CORNER_ROUNDING < 100) || defined(SMAA_FORCE_CORNER_DETECTION)
    vec4 coords = SMAAMad(vec4(d.x, 0.0, d.y, 0.0), SMAA_PIXEL_SIZE.xyxy, texcoord.xyxy);
    vec2 e;
    e.r = textureOffset(edgesTex, coords.xy, ivec2(0.0,  1.0)).r;
    bool left = abs(d.x) < abs(d.y);
    e.g = textureOffset(edgesTex, coords.xy, ivec2(0.0, -2.0)).r;
    if (left) weights *= SMAASaturate(float(SMAA_CORNER_ROUNDING) / 100.0 + 1.0 - e);

    e.r = textureOffset(edgesTex, coords.zw, ivec2(1.0,  1.0)).r;
    e.g = textureOffset(edgesTex, coords.zw, ivec2(1.0, -2.0)).r;
    if (!left) weights *= SMAASaturate(float(SMAA_CORNER_ROUNDING) / 100.0 + 1.0 - e);
    #endif
}

void SMAADetectVerticalCornerPattern(inout vec2 weights, vec2 texcoord, vec2 d) {
    #if (SMAA_CORNER_ROUNDING < 100) || defined(SMAA_FORCE_CORNER_DETECTION)
    vec4 coords = SMAAMad(vec4(0.0, d.x, 0.0, d.y), SMAA_PIXEL_SIZE.xyxy, texcoord.xyxy);
    vec2 e;
    e.r = textureOffset(edgesTex, coords.xy, ivec2( 1.0, 0.0)).g;
    bool left = abs(d.x) < abs(d.y);
    e.g = textureOffset(edgesTex, coords.xy, ivec2(-2.0, 0.0)).g;
    if (left) weights *= SMAASaturate(float(SMAA_CORNER_ROUNDING) / 100.0 + 1.0 - e);

    e.r = textureOffset(edgesTex, coords.zw, ivec2( 1.0, 1.0)).g;
    e.g = textureOffset(edgesTex, coords.zw, ivec2(-2.0, 1.0)).g;
    if (!left) weights *= SMAASaturate(float(SMAA_CORNER_ROUNDING) / 100.0 + 1.0 - e);
    #endif
}

void main(void){
    vec4 weights = vec4(0.0, 0.0, 0.0, 0.0);

    vec2 e = texture(edgesTex, texcoord).rg;

    if (e.g > 0.0) { // Edge at north
        #if (SMAA_MAX_SEARCH_STEPS_DIAG > 0) || defined(SMAA_FORCE_DIAGONAL_DETECTION)
        // Diagonals have both north and west edges, so searching for them in
        // one of the boundaries is enough.
        weights.rg = SMAACalculateDiagWeights(texcoord, e);

        // We give priority to diagonals, so if we find a diagonal we skip
        // horizontal/vertical processing.
        if (dot(weights.rg, vec2(1.0, 1.0)) == 0.0) {
        #endif

        vec2 d;

        // Find the distance to the left:
        vec2 coords;
        coords.x = SMAASearchXLeft(offset[0].xy, offset[2].x);
        coords.y = offset[1].y; // offset[1].y = texcoord.y - 0.25 * SMAA_PIXEL_SIZE.y (@CROSSING_OFFSET)
        d.x = coords.x;

        // Now fetch the left crossing edges, two at a time using bilinear
        // filtering. Sampling at -0.25 (see @CROSSING_OFFSET) enables to
        // discern what value each edge has:
        float e1 = texture(edgesTex, coords).r;

        // Find the distance to the right:
        coords.x = SMAASearchXRight(offset[0].zw, offset[2].y);
        d.y = coords.x;

        // We want the distances to be in pixel units (doing this here allow to
        // better interleave arithmetic and memory accesses):
        d = d / SMAA_PIXEL_SIZE.x - pixcoord.x;

        // SMAAArea below needs a sqrt, as the areas texture is compressed
        // quadratically:
        vec2 sqrt_d = sqrt(abs(d));

        // Fetch the right crossing edges:
        float e2 = textureOffset(edgesTex, coords, ivec2(1, 0)).r;

        // Ok, we know how this pattern looks like, now it is time for getting
        // the actual area:
        weights.rg = SMAAArea(sqrt_d, e1, e2, 0.0);

        // Fix corners:
        SMAADetectHorizontalCornerPattern(weights.rg, texcoord, d);

        #if (SMAA_MAX_SEARCH_STEPS_DIAG > 0) || defined(SMAA_FORCE_DIAGONAL_DETECTION)
        } else
            e.r = 0.0; // Skip vertical processing.
        #endif
    }

    if (e.r > 0.0) { // Edge at west
        vec2 d;

        // Find the distance to the top:
        vec2 coords;
        coords.y = SMAASearchYUp(offset[1].xy, offset[2].z);
        coords.x = offset[0].x; // offset[1].x = texcoord.x - 0.25 * SMAA_PIXEL_SIZE.x;
        d.x = coords.y;

        // Fetch the top crossing edges:
        float e1 = texture(edgesTex, coords).g;

        // Find the distance to the bottom:
        coords.y = SMAASearchYDown(offset[1].zw, offset[2].w);
        d.y = coords.y;

        // We want the distances to be in pixel units:
        d = d / SMAA_PIXEL_SIZE.y - pixcoord.y;

        // SMAAArea below needs a sqrt, as the areas texture is compressed
        // quadratically:
        vec2 sqrt_d = sqrt(abs(d));

        // Fetch the bottom crossing edges:
        float e2 = textureOffset(edgesTex, coords, ivec2(0, 1)).g;

        // Get the area for this direction:
        weights.ba = SMAAArea(sqrt_d, e1, e2, 0.0);

        // Fix corners:
        SMAADetectVerticalCornerPattern(weights.ba, texcoord, d);
    }

    outColor = weights;
}
