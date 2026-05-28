struct PSInput {
    float4 Pos   : SV_POSITION;
    float3 Color : COLOR;
};
void main(in uint VertId : SV_VertexID, out PSInput PSIn) {
    float4 Pos[3] = { float4(-0.5, -0.5, 0.0, 1.0), float4(0.0, +0.5, 0.0, 1.0), float4(+0.5, -0.5, 0.0, 1.0) };
    float3 Col[3] = { float3(1.0, 0.0, 0.0), float3(0.0, 1.0, 0.0), float3(0.0, 0.0, 1.0) };
    PSIn.Pos   = Pos[VertId];
    PSIn.Color = Col[VertId];
}
