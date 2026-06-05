cbuffer Constants {
    float2 resolution;
    float2 pad;
    float4x4 viewmat;
    float3 campos;
    float time;
    float _Speed;
    int _Steps;
    float _Size;
    float acreationSize;
    float3 objectPos;
    float4 padington[8];
};

#define EPSILON 0.0001f
#define NOHIT 0
#define ACREATIONHIT 1
#define HOLEHIT 2
#define OBJECTHIT 3

struct PSInput {
    float4 Pos   : SV_POSITION;
    float2 UV    : TEXCOORD0;
};

struct PSOutput {
    float4 Color : SV_TARGET;
};

// This fragment shader (except for the stars) was almost fully ported from https://www.shadertoy.com/view/sdjcWm, by A_Toatser, which was an improved version https://www.shadertoy.com/view/tsBXW3 made by set111, toaster didn't give credit to that guy ):

float hash1(float x) { return frac(sin(x) * 15254.742); }
float hash2(float2 x) { return hash1(x.x + hash1(x.y)); }
float hash3(float3 p) {p = frac(p * 0.3183099 + .1) * 17; return frac(p.x * p.y * p.z * (p.x + p.y + p.z));}
float value(float2 p, float f)
{
    float bl = hash2(floor(p * f + float2(0.0, 0.0)));
    float br = hash2(floor(p * f + float2(1.0, 0.0)));
    float tl = hash2(floor(p * f + float2(0.0, 1.0)));
    float tr = hash2(floor(p * f + float2(1.0, 1.0)));

    float2 fr = frac(p * f);
    fr = (3.0 - 2.0 * fr) * fr * fr;
    float b = lerp(bl, br, fr.x);
    float t = lerp(tl, tr, fr.x);
    return lerp(b, t, fr.y);
}

float3 generateStars(float3 rayDir) {
    float3 p = 1000.0 + rayDir * 300.0 + rayDir * 100.0 + sin(time*0.01);
    float h = hash3(floor(p));
    return float3(smoothstep(0.995, 1.0, h));
}


float map(float3 p, out int hittype) {
    hittype = NOHIT;

    float distToHole = length(p);
    if (distToHole < 2.0) {
        hittype = HOLEHIT;
        return distToHole - 2.0;
    }

    if (abs(p.y) < (0.5 / _Steps) && length(p) < acreationSize) {
        hittype = ACREATIONHIT;
    }

    return length(p) - 2.0;
}

float3 calcNormal(float3 p) {
    int d;
    float2 e = float2(1.0, -1.0) * 0.001;
    return normalize(e.xyy * map(p + e.xyy, d) +
    e.yyx * map(p + e.yyx, d) +
    e.yxy * map(p + e.yxy, d) +
    e.xxx * map(p + e.xxx, d));
}

float3 raytrace(float3 ro, float3 rd) {
    float3 p = ro;
    float3 v = rd;
    float3 col = float3(0, 0, 0);
    int hittype = NOHIT;

    for (int i = 0; i < _Steps; i++) {
        float d = map(p, hittype);

        // Handle Hit
        if (hittype == HOLEHIT) return float3(0, 0, 0); // Event horizon

        if (hittype == ACREATIONHIT) {
            float brightness = 50.0 + 3.75 * (8.0 - pow(8.0 - d, 2.0) / 1.7);
            col += float3(brightness / 31.875, (0.5 * brightness) / 31.875 / 1.8, 0) * 0.1;
        }

        // Gravity update (The 3 lines from your code)
        float distSq = dot(p, p);
        float3 gravity = -p / (max(distSq, 0.1) * _Steps) * _Speed;
        v = normalize(v + gravity);

        // March
        p += v * (1.0 / _Steps);

        // Optimization: Exit if too far
        if (length(p) > 200.0) break;
    }

    // Add stars if we didn't hit anything
    col += generateStars(rd);

    return col;
}

void main(in PSInput PSIn, out PSOutput PSOut)
{
    float2 uv = (PSIn.UV * 2.0 - 1.0);
    uv.x *= resolution.x / resolution.y;

    float3 viewDir = normalize(float3(uv, 1.5));
    float3 rayDir = normalize(mul((float3x3)viewmat, viewDir));
    float3 rayPos = campos;

    float3 color = raytrace(rayPos, rayDir);

    PSOut.Color = float4(color, 1.0);
}
