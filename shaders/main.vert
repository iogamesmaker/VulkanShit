struct VSOutput {
    float4 Pos   : SV_POSITION;
    float2 UV    : TEXCOORD0;
};

void main(in uint VertId : SV_VertexID, out VSOutput VSOut) {
    float4 Pos[6] = {
        float4(-1.0, -1.0, 0.0, 1.0), float4(+1.0, +1.0, 0.0, 1.0), float4(+1.0, -1.0, 0.0, 1.0),
        float4(-1.0, -1.0, 0.0, 1.0), float4(-1.0, +1.0, 0.0, 1.0), float4(+1.0, +1.0, 0.0, 1.0),
    };

    float2 UVs[6] = {
        float2(0.0, 1.0), float2(1.0, 0.0), float2(1.0, 1.0),
        float2(0.0, 1.0), float2(0.0, 0.0), float2(1.0, 0.0)
    };

    VSOut.Pos = Pos[VertId];
    VSOut.UV  = UVs[VertId];
}
