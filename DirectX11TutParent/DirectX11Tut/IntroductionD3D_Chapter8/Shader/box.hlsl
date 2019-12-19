cbuffer Camera
{
    float4x4 world;
};

cbuffer Camera2
{
    float4x4 view;
    float4x4 proj;
};

struct VertexIn
{
    float3 position : POSITION;
    float2 tex : TEXTURE;
};

struct VertexOut
{
    float4 position : SV_POSITION;
    float2 tex : TEXTURE;
};

Texture2D boxTex;

SamplerState sam
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 4;
};

VertexOut BoxVertex(VertexIn vin)
{
    VertexOut vout;
    vout.position = mul(float4(vin.position, 1.0f), world);
    vout.position = mul(vout.position, view);
    vout.position = mul(vout.position, proj);
    vout.tex = vin.tex;

    return vout;
}

float4 BoxPixel(VertexOut pin) : SV_Target
{
    return boxTex.Sample(sam, pin.tex);
}