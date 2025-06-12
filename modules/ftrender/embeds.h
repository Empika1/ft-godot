const char renderShader[] = R"###(
shader_type canvas_item;

vec4 premult_mix(vec4 color1, vec4 color2, float t) {
    vec3 premul1 = color1.rgb * color1.a;
    vec3 premul2 = color2.rgb * color2.a;

    vec3 rgb = mix(premul1, premul2, t);
    float a = mix(color1.a, color2.a, t);

    rgb = (a > 0.0) ? rgb / a : vec3(0.0);

    return vec4(rgb, a);
}

//from https://www.shadertoy.com/view/Nlc3zf
float sdRoundedRect(vec2 position, vec2 halfSize, float cornerRadius) {
    position = abs(position) - halfSize + cornerRadius;
    return length(max(position, 0.0)) + min(max(position.x, position.y), 0.0) - cornerRadius;
}

float sdCirc(vec2 p, vec2 c, float r) {
    return length(p - c) - r;
}

//from https://iquilezles.org/articles/distfunctions2d/
float sdTriangle( in vec2 p, in vec2 p0, in vec2 p1, in vec2 p2 ) {
    vec2 e0 = p1-p0, e1 = p2-p1, e2 = p0-p2;
    vec2 v0 = p -p0, v1 = p -p1, v2 = p -p2;
    vec2 pq0 = v0 - e0*clamp( dot(v0,e0)/dot(e0,e0), 0.0, 1.0 );
    vec2 pq1 = v1 - e1*clamp( dot(v1,e1)/dot(e1,e1), 0.0, 1.0 );
    vec2 pq2 = v2 - e2*clamp( dot(v2,e2)/dot(e2,e2), 0.0, 1.0 );
    float s = sign( e0.x*e2.y - e0.y*e2.x );
    vec2 d = min(min(vec2(dot(pq0,pq0), s*(v0.x*e0.y-v0.y*e0.x)),
                     vec2(dot(pq1,pq1), s*(v1.x*e1.y-v1.y*e1.x))),
                     vec2(dot(pq2,pq2), s*(v2.x*e2.y-v2.y*e2.x)));
    return -sqrt(d.x)*sign(d.y);
}

//inexact! works for my purposes tho
float sdPoweredDecal(vec2 p) {
    float d1 = sdCirc(p, vec2(0.5, 0.5), 0.35297619047);
    float d2 = sdCirc(p, vec2(0.27344, 0.53), 0.125);
    float d3 = sdCirc(p, vec2(0.575, 0.575), 0.18035714285);
    float d4 = sdTriangle(p, vec2(0.61875, 0.63125), vec2(0.825, 0.83125), vec2(0.6, 0.84375));
    float d5 = sdTriangle(p, vec2(0.63125, 0.55), vec2(0.53125, 1.59375), vec2(-1., 0.55));
    return min(max(min(max(d1, -d5), d2), -d3), d4);
}

//inexact! works for my purposes tho
float sdUnpoweredDecalHalf(vec2 p) {
    float d1 = sdCirc(p, vec2(0.5, 0.5), 0.35297619047);
    float d2 = sdCirc(p, vec2(0.4227, 0.575030012), 0.276);
    float d3 = distance(p, vec2(0.64945, 0.273)) + distance(p, vec2(0.795, 0.455)) - 0.29439;
    float d4 = sdTriangle(p, vec2(0., 0.63), vec2(1., 0.29), vec2(1., 2.));
    return min(max(max(d1, -d2), -d4), d3);
}

//inexact! works for my purposes tho
float sdUnpoweredDecal(vec2 p) {
    return min(sdUnpoweredDecalHalf(p), sdUnpoweredDecalHalf(vec2(1.) - p));
}

varying flat int id;
void vertex() {
    id = INSTANCE_ID;
}

const int TypeNames_STATIC_RECT_BORDER = 0, TypeNames_STATIC_RECT_INSIDE = 1;
const int TypeNames_STATIC_CIRC_BORDER = 2, TypeNames_STATIC_CIRC_INSIDE = 3;
const int TypeNames_DYNAMIC_RECT_BORDER = 4, TypeNames_DYNAMIC_RECT_INSIDE = 5;
const int TypeNames_DYNAMIC_CIRC_BORDER = 6, TypeNames_DYNAMIC_CIRC_INSIDE = 7;
const int TypeNames_GP_RECT_BORDER = 8, TypeNames_GP_RECT_INSIDE = 9;
const int TypeNames_GP_CIRC_BORDER = 10, TypeNames_GP_CIRC_INSIDE = 11;
const int TypeNames_WOOD_BORDER = 12, TypeNames_WOOD_INSIDE = 13;
const int TypeNames_WATER_BORDER = 14, TypeNames_WATER_INSIDE = 15;
const int TypeNames_CW_BORDER = 16, TypeNames_CW_INSIDE = 17, TypeNames_CW_DECAL = 18;
const int TypeNames_CCW_BORDER = 19, TypeNames_CCW_INSIDE = 20, TypeNames_CCW_DECAL = 21;
const int TypeNames_UPW_BORDER = 22, TypeNames_UPW_INSIDE = 23, TypeNames_UPW_DECAL = 24;
const int TypeNames_BUILD_BORDER = 25, TypeNames_BUILD_INSIDE = 26;
const int TypeNames_GOAL_BORDER = 27, TypeNames_GOAL_INSIDE = 28;
const int TypeNames_JOINT_NORMAL = 29, TypeNames_JOINT_WHEEL_CENTER = 30;
const int TypeNames_SIZE = 31;

uniform vec4[TypeNames_SIZE] colorsGlobal;
uniform float[TypeNames_SIZE] cornerRadiiGlobal;
uniform float[TypeNames_SIZE] borderThicknessesGlobal; //TODO: figure out if this is needed

instance uniform float scale;
instance uniform float aaWidth;
const int layers = 3;
instance uniform int layerID;

uniform sampler2DArray data;

const float inf = 1. / 0.;
void fragment() {
    vec4 idDataF = texelFetch(data, ivec3(id % 128, id / 128, layerID), 0);
    uvec4 idDataU = uvec4(floatBitsToUint(idDataF[0]), floatBitsToUint(idDataF[1]), floatBitsToUint(idDataF[2]), floatBitsToUint(idDataF[3]));

    const uint mask = 0xFFFFFFu;
    uvec2 sizeU = uvec2(idDataU[0] >> 8u, (idDataU[0] << 16u | idDataU[1] >> 16u) & mask);
    uvec2 centerU = uvec2((idDataU[1] << 8u | idDataU[2] >> 24u) & mask, idDataU[2] & mask);
    vec2 size = vec2(float(sizeU.x) / 16., float(sizeU.y) / 16.);
    vec2 center = vec2(float(centerU.x) / 16., float(centerU.y) / 16.);
    int type = int(idDataU[3]);

    const vec4 outsideColor = vec4(0.);
    vec4 borderColor;
    vec4 insideColor;
    float cornerRadius;
    float borderThickness;

    vec2 sizeWithAA = size + vec2(aaWidth) * 2.;
    vec2 pixelPos = UV * sizeWithAA - vec2(aaWidth) - center;
    float d = inf;

    switch(type) {
        case TypeNames_STATIC_RECT_BORDER: //rect borders
        case TypeNames_DYNAMIC_RECT_BORDER:
        case TypeNames_GP_RECT_BORDER:
        case TypeNames_WOOD_BORDER:
        case TypeNames_WATER_BORDER:
        case TypeNames_BUILD_BORDER:
        case TypeNames_GOAL_BORDER:
            borderColor = colorsGlobal[type];
            insideColor = borderColor;
            cornerRadius = cornerRadiiGlobal[type] * scale;
            borderThickness = 0.;
            break;
        case TypeNames_STATIC_RECT_INSIDE: //rect insides
        case TypeNames_DYNAMIC_RECT_INSIDE:
        case TypeNames_GP_RECT_INSIDE:
        case TypeNames_WOOD_INSIDE:
        case TypeNames_WATER_INSIDE:
        case TypeNames_BUILD_INSIDE:
        case TypeNames_GOAL_INSIDE:
            borderColor = colorsGlobal[type];
            insideColor = borderColor;
            cornerRadius = 0.;
            borderThickness = 0.;
            break;
        case TypeNames_STATIC_CIRC_BORDER: //filled circles
        case TypeNames_STATIC_CIRC_INSIDE:
        case TypeNames_DYNAMIC_CIRC_BORDER:
        case TypeNames_DYNAMIC_CIRC_INSIDE:
        case TypeNames_GP_CIRC_BORDER:
        case TypeNames_GP_CIRC_INSIDE:
        case TypeNames_CW_BORDER:
        case TypeNames_CW_INSIDE:
        case TypeNames_CCW_BORDER:
        case TypeNames_CCW_INSIDE:
        case TypeNames_UPW_BORDER:
        case TypeNames_UPW_INSIDE:
            borderColor = colorsGlobal[type];
            insideColor = borderColor;
            cornerRadius = size.x / 2.; //should be equal to size.y because this is a circle
            borderThickness = 0.;
            break;
        case TypeNames_JOINT_NORMAL: //hollow circles
        case TypeNames_JOINT_WHEEL_CENTER:
            borderColor = colorsGlobal[type];
            insideColor = vec4(0.);
            cornerRadius = size.x / 2.; //should be equal to size.y because this is a circle
            borderThickness = borderThicknessesGlobal[type] * scale;
            break;
        case TypeNames_CW_DECAL: //wheel decals (all handled individually) TODO: figure out why decals make an ugly square at very high AA value
            borderColor = colorsGlobal[type];
            insideColor = colorsGlobal[type];
            borderThickness = 0.;
            d = sdPoweredDecal(vec2(UV.x, 1. - UV.y)) * sizeWithAA.x; //decal SDF assumed fixed size
            break;
        case TypeNames_CCW_DECAL:
            borderColor = colorsGlobal[type];
            insideColor = colorsGlobal[type];
            borderThickness = 0.;
            d = sdPoweredDecal(vec2(1. - UV.x, 1. - UV.y)) * sizeWithAA.x; //flip horizontally for CCW decal
            break;
        case TypeNames_UPW_DECAL:
            borderColor = colorsGlobal[type];
            insideColor = colorsGlobal[type];
            borderThickness = 0.;
            d = sdUnpoweredDecal(vec2(1. - UV.x, UV.y)) * sizeWithAA.x; //flip horizontally for CCW decal
            break;
    }

    if(d == inf) { //decals set d separately
        d = sdRoundedRect(pixelPos, size / 2., cornerRadius);
    }

    float outsideA = clamp((d + aaWidth) / (2. * aaWidth),  0.0, 1.0);
    float insideA  = clamp(1.0 - (d + borderThickness + aaWidth) / (2. * aaWidth), 0.0, 1.0);
    float borderA  = clamp(1.0 - outsideA - insideA, 0.0, 1.0);

    vec4 outsideColorPremult = outsideColor;
    vec4 borderColorPremult = borderColor;
    vec4 insideColorPremult  = insideColor;
    outsideColorPremult.rgb *= outsideColorPremult.a;
    borderColorPremult.rgb *= borderColorPremult.a;
    insideColorPremult.rgb  *= insideColorPremult.a;

    vec4 colPremult = outsideColorPremult * outsideA + borderColorPremult * borderA + insideColorPremult * insideA;

    COLOR = vec4(colPremult.a == 0. ? vec3(0.) : colPremult.rgb / colPremult.a, colPremult.a);
}
)###";

// const char renderShader[] = R"(
// shader_type canvas_item;

// void fragment() {
//     COLOR = vec4(1.0, 0.0, 0.0, 1.0); // red
// }
// )";