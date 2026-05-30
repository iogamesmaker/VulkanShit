cbuffer Constants {
    float2 resolution;
    float2 pad;
    float4x4 viewmat;
    float3 campos;
    float4 padin[10];
};

#define NOHIT 0
#define WATERHIT 1
#define LANDHIT 2
#define CAPSULEHIT 3
#define BULBHIT 4
#define EPSILON 0.0001f


struct PSInput {
    float4 Pos   : SV_POSITION;
    float2 UV    : TEXCOORD0;
};
struct PSOutput {
    float4 Color : SV_TARGET;
};

float sdVerticalCapsule( float3 p, float h, float r )
{
    p.y -= clamp( p.y, 0.0, h );
    return length( p ) - r;
}

float sphereSDF (float3 p, float r, float3 spherePos) {
    return length(p - spherePos) - r;
}

float mandelbulbSDF(float3 p, out float4 trap) {
    float3 w = p;
    float m = dot(w, w);
    trap = float4(abs(w), m);
    float dz = 1.0;

    for (int i = 0; i < 6; i++) {
        dz = 8.0 * pow(m, 3.5) * dz + 1.0;
        float r = length(w);
        float b = 8.0 * acos(w.y / r);
        float a = 8.0 * atan2(w.x, w.z);
        w = p + (r*r*r*r*r*r*r*r) * float3(sin(b) * sin(a), cos(b), sin(b) * cos(a));
        trap = min(trap, float4(abs(w), m));
        m = dot(w, w);
        if (m > 256.0) break;
    }
    return 0.25 * log(m) * sqrt(m) / dz;
}

float map(float3 p, out int hittype, out float4 trap) {
    float SDF = 10000.0f;

    hittype = NOHIT;

    float landSDF = sphereSDF(p, 50.0f, float3(0.0f, -50.0f, 0.0f));
    SDF = min(SDF, landSDF);
    float capsuleSDF = sdVerticalCapsule(p - float3(0.0, 1.0f, 0.0f), 5.0f, 1.0f);
    SDF = min(SDF, capsuleSDF);
    float circleSDF = sphereSDF(p, 1.0f, float3(-1.0f,2.0f,-4.0f));
    SDF = min(SDF, circleSDF);

    float mbSDF = mandelbulbSDF((p * 0.25f) - float3(1.5, 0.9, 0.0), trap);
    SDF = min(SDF, mbSDF);


    if(SDF <= EPSILON) {
        if(mbSDF <= EPSILON) hittype = BULBHIT;
        if(landSDF <= EPSILON) hittype = WATERHIT;
        if(capsuleSDF <= EPSILON) hittype = CAPSULEHIT;
        if(circleSDF <= EPSILON) hittype = CAPSULEHIT;
    }

    return SDF;
}
float3 calcNormal(float3 p) {
    int d; float4 d2;
    float2 e = float2(1.0, -1.0) * 0.001;
    return normalize(e.xyy * map(p + e.xyy, d, d2) +
                     e.yyx * map(p + e.yyx, d, d2) +
                     e.yxy * map(p + e.yxy, d, d2) +
                     e.xxx * map(p + e.xxx, d, d2));
}
float softshadow( in float3 ro, in float3 rd, float mint, float maxt, float w )
{
    int dummy;
    float4 dummy2;
    float res = 1.0;
    float ph = 1e20;
    float t = mint;
    for( int i=0; i<256 && t<maxt; i++ )
    {
        float h = map(ro + rd*t, dummy, dummy2);
        if( h<0.001 )
            return 0.0;
        float y = h*h/(2.0*ph);
        float d = sqrt(h*h-y*y);
        res = min( res, d/(w*max(0.0,t-y)) );
        ph = h;
        t += h;
    }
    return res;
}


float3 raymarch(float3 pos, float3 dir, out float reflection = 0.0f, out float3 hitPos = float3(0.0f)) {
    float t = 0.0;
    float maxDist = 200.0;
    bool hit = false;
    int i;
    int hittype;
    reflection = 0.0f;
    hitPos = float3(0.0f);
    float4 trap;

    float3 color = float3(0.0);

    for (i = 0; i < 512; i++) {
        float h = map(pos + dir * t, hittype, trap);
        if (h < EPSILON || t > maxDist) {break;}
        t += h;
    }

    if(hittype != NOHIT) {
        hitPos = pos + dir * t;

        float3 lightDir = normalize(float3(0.1, 0.6, 0.7));
        float3 normal = calcNormal(hitPos);

        float shadow = softshadow(hitPos + normal * 0.01f, lightDir, EPSILON * 2.0f, 100.0f, 0.5f);

        float lighting = 0.1 + (0.9 * shadow);

        if(hittype == BULBHIT) {
            color = float3(trap.xyx) * lighting;
        } else if(hittype == WATERHIT) {
            color = float3(0.0, 0.3, 1.0) * lighting;
            reflection = 0.6f;
        } else if(hittype == CAPSULEHIT) {
            color = float3(1.0) * lighting;
            reflection = 0.4f;
        }
    }

    return color;
}

void main(in PSInput PSIn, out PSOutput PSOut)
{
    float2 uv = (PSIn.UV * 2.0 - 1.0);
    uv.x *= resolution.x / resolution.y;
    if(uv.x % 2 == 0) discard;

    float3 viewDir = normalize(float3(uv, 1.5)); // camera matrix shit magic adfjadklfajsk
    float3 rayDir = normalize(mul((float3x3)viewmat, viewDir)); // thank you netanyahu for chat gpt
    float3 rayPos = campos;

    float reflectionStrength = 0.0f;

    float3 hitPos;
    float3 color = raymarch(rayPos, rayDir, reflectionStrength, hitPos);

    if(reflectionStrength != 0.0f) {
        float3 normal = calcNormal(hitPos);
        float3 reflectDir = reflect(rayDir, normal);

        float dummy;
        float3 dummy2;

        color = lerp(color, raymarch(hitPos - rayDir * EPSILON * 4.0f, reflectDir, dummy, dummy2), reflectionStrength);
    }

    PSOut.Color = float4(color, 1.0);
}
