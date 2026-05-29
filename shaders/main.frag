struct PSInput {
    float4 Pos : SV_POSITION;
    float2 UV  : TEXCOORD0;
};

struct PSOutput {
    float4 Color : SV_TARGET;
};

float SDF(float3 pos) {
    float sphere = length(pos) - 1.0;
    float plane = pos.y + 1.0;

    return min(sphere, plane);
}

void main(in PSInput In, out PSOutput Out) {
    float2 uv = In.UV * 2.0 - 1.0;

    float3 rayPos = float3(0.0, 1.5, -4.0);
    float3 rayDir = normalize(float3(uv, 1.5));

    float4 color = float4(0.15, 0.3, 1.0, 1.0);

    for(int i = 0; i < 100; i++) {
        float dist = SDF(rayPos);

        if(dist < 0.001) {
            color = float4(1.0, 1.0, 1.0, 1.0);
            break;
        }

        if(rayPos.z > 10.0) {
            break;
        }

        rayPos += rayDir * dist;
    }

    Out.Color = color;
}
