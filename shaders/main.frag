cbuffer Constants {
    float2 resolution;
    float2 pad;
    float4x4 viewmat;
    float3 campos;
    float4 padin[10];
};

struct PSInput {
    float4 Pos   : SV_POSITION;
    float2 UV    : TEXCOORD0;
};
struct PSOutput {
    float4 Color : SV_TARGET;
};

// Mandelbulb SDF
float map(float3 p, out float4 resColor) {
    float3 w = p;
    float m = dot(w, w);
    float4 trap = float4(abs(w), m);
    float dz = 1.0;

    for (int i = 0; i < 6; i++) {
        dz = 8.0 * pow(m, 3.5) * dz + 1.0;
        float r = length(w);
        float b = 8.0 * acos(w.y / r);
        float a = 8.0 * atan2(w.x, w.z);
        w = p + pow(r, 8.0) * float3(sin(b) * sin(a), cos(b), sin(b) * cos(a));
        trap = min(trap, float4(abs(w), m));
        m = dot(w, w);
        if (m > 256.0) break;
    }
    resColor = float4(m, trap.yzw);
    return 0.25 * log(m) * sqrt(m) / dz;
}

void main(in PSInput PSIn, out PSOutput PSOut)
{
    float2 uv = (PSIn.UV * 2.0 - 1.0);
    uv.x *= resolution.x / resolution.y;

    float3 viewDir = normalize(float3(uv, 1.5)); // camera matrix shit magic adfjadklfajsk
    float3 worldDir = normalize(mul((float3x3)viewmat, viewDir));
    float3 rayPos = campos;

    float4 trap;
    float t = 0.0;
    float maxDist = 20.0;
    bool hit = false;
    int i;

    for (i = 0; i < 512; i++) {
        float h = map(rayPos + worldDir * t, trap);
        if (h < 0.0001 || t > maxDist) break;
        t += h;
    }

    float3 color = float3(0.0, 0.0, 0.0);
    if (t < maxDist) {
        color = float3(0.1, 0.2, 0.3) * trap.y;
        color += float3(0.3, 0.1, 0.0) * trap.w;
    }
//     color *= (256.0f - i) / 256.0f;
    color.rgb += i / 1024.0f;

    PSOut.Color = float4(color, 1.0);
}
