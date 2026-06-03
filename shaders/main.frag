cbuffer Constants {
    float2 resolution;
    float2 pad;
    float4x4 viewmat;
    float3 campos;
    float time;
    float _Speed;
    int _Steps;
    float _Size;
    float4 padington[9];
};

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

float4 raymarchDisk(float3 ray, float3 zeroPos)
{
    float3 position = zeroPos;
    float lengthPos = length(position.xz);
    float dist = min(1.0, lengthPos * (1.0 / _Size) * 0.5) * _Size * 0.4 * (1.0 / _Steps) / (abs(ray.y));

    position += dist * _Steps * ray * 0.5;

    float2 deltaPos;
    deltaPos.x = -zeroPos.z * 0.01 + zeroPos.x;
    deltaPos.y = zeroPos.x * 0.01 + zeroPos.z;
    deltaPos = normalize(deltaPos - zeroPos.xz);

    float parallel = dot(ray.xz, deltaPos);
    parallel /= sqrt(lengthPos);
    parallel *= 0.5;
    float redShift = parallel + 0.3;
    redShift *= redShift;

    redShift = clamp(redShift, 0.0, 1.0);

    float disMix = clamp((lengthPos - _Size * 2.0) * (1.0 / _Size) * 0.24, 0.0, 1.0);
    float3 insideCol = lerp(float3(1.0, 0.8, 0.0), float3(0.5, 0.13, 0.02) * 0.2, disMix);

    insideCol *= lerp(float3(0.4, 0.2, 0.1), float3(1.6, 2.4, 4.0), redShift);
    insideCol *= 1.25;
    redShift += 0.12;
    redShift *= redShift;

    float4 o = float4(0.0, 0.0, 0.0, 0.0);

    for(float i = 0.0 ; i < _Steps; i++)
    {
        position -= dist * ray;

        float intensity = clamp(1.0 - abs((i - 0.8) * (1.0 / _Steps) * 2.0), 0.0, 1.0);
        float lenSq = length(position.xz);
        float distMult = 1.0;

        distMult *= clamp((lenSq - _Size * 0.75) * (1.0 / _Size) * 1.5, 0.0, 1.0);
        distMult *= clamp((_Size * 10.0 - lenSq) * (1.0 / _Size) * 0.20, 0.0, 1.0);
        distMult *= distMult;

        float u = lenSq + time * _Size * 0.3 + intensity * _Size * 0.2;

        float2 xy;
        float rot = fmod(time * _Speed, 8192.0);
        xy.x = -position.z * sin(rot) + position.x * cos(rot);
        xy.y = position.x * sin(rot) + position.z * cos(rot);

        float x = abs(xy.x / xy.y);
        float angle = 0.02 * atan(x);

        const float f = 70.0;
        float noise = value(float2(angle, u * (1.0 / _Size) * 0.05), f);
        noise = noise * 0.66 + 0.33 * value(float2(angle, u * (1.0 / _Size) * 0.05), f * 2.0);

        float extraWidth = noise * 1.0 * (1.0 - clamp(i * (1.0 / _Steps) * 2.0 - 1.0, 0.0, 1.0));
        float alpha = clamp(noise * (intensity + extraWidth) * ((1.0 / _Size) * 10.0 + 0.01) * dist * distMult, 0.0, 1.0);

        float3 col = 2.0 * lerp(float3(0.3, 0.2, 0.15) * insideCol, insideCol, min(1.0, intensity * 2.0));
        o = clamp(float4(col * alpha + o.rgb * (1.0 - alpha), o.a * (1.0 - alpha) + alpha), float4(0.0, 0.0, 0.0, 0.0), float4(1.0, 1.0, 1.0, 1.0));

        lenSq *= (1.0 / _Size);

        o.rgb += redShift * (intensity * 1.0 + 0.5) * (1.0 / _Steps) * 100.0 * distMult / (lenSq * lenSq);
    }

    o.rgb = clamp(o.rgb - 0.005, 0.0, 1.0);
    return o;
}

float3 raytrace(float3 ro, float3 rd) {
    float3 pos = ro;
    float3 ray = rd;

    float4 col = float4(0.0, 0.0, 0.0, 0.0);
    float4 glow = float4(0.0, 0.0, 0.0, 0.0);
    float4 outCol = float4(100.0, 100.0, 100.0, 100.0);

    for(int disks = 0; disks < 256; disks++)
    {
        for (int h = 0; h < 6; h++)
        {
            float dotpos = dot(pos, pos);
            float invDist = rsqrt(dotpos);
            float centDist = dotpos * invDist;

            float stepDist = min(dotpos * 0.2, max(length(pos) - _Size * 9.0, abs(pos.y / ray.y) * 0.9));
            float farLimit = centDist * 0.5;
            float closeLimit = centDist * 0.1 + 0.05 * centDist * centDist * (1.0 / _Size);
            stepDist = min(stepDist, min(farLimit, closeLimit));

            float invDistSqr = invDist * invDist;
            float bendForce = stepDist * invDistSqr * _Size * 0.625;

            float3 pos_cross_pole_axis = cross(float3(0.0, 1.0, 0.0), pos);

            float sin2_colatitude = length(pos_cross_pole_axis) / max(length(pos), 0.0001);
            sin2_colatitude = sin2_colatitude * sin2_colatitude;

            float3 frameDragForce = normalize(pos_cross_pole_axis) * invDistSqr * sin2_colatitude * 0.012;

            ray = normalize(ray - (bendForce * invDist) * pos);
            pos += stepDist * (ray - frameDragForce);

            glow += float4(1.2, 1.1, 1.0, 1.0) * (0.01 * stepDist * invDistSqr * invDistSqr * clamp(centDist * 2.0 - 1.2, 0.0, 1.0));
        }

        float dist2 = length(pos);

        if(dist2 < _Size * 0.5)
        {
            outCol = float4(col.rgb * col.a + glow.rgb * (1.0 - col.a), 1.0);
            break;
        }
        else if(dist2 > _Size * 1000.0)
        {
            float3 bg = generateStars(ray);
            outCol = float4(col.rgb * col.a + bg * (1.0 - col.a) + glow.rgb * (1.0 - col.a), 1.0);
            break;
        }
        else if (abs(pos.y) <= _Size * 0.0005 && dist2 < _Size * 10.0)
        {
            float4 diskCol = raymarchDisk(ray, pos);
            pos += abs(_Size * 0.001 / ray.y) * ray;
            col = float4(diskCol.rgb * (1.0 - col.a) + col.rgb, col.a + diskCol.a * (1.0 - col.a));
        }
    }

    if(outCol.r == 100.0)
        outCol = float4(col.rgb + glow.rgb * (col.a + glow.a), 1.0);

    return pow(max(outCol.rgb, 0.0), float3(0.6, 0.6, 0.6));
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
