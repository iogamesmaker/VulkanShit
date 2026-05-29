struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD;
};

VSOutput main(uint VertexID : SV_VertexID)
{
    VSOutput o;
    o.uv = float2((VertexID << 1) & 2, VertexID & 2);
    o.pos = float4(o.uv * 2.0f - 1.0f, 0.0f, 1.0f);
    return o;
}
