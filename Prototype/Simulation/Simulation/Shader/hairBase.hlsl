cbuffer Camera
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
};

struct VertexIn
{
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VertexOut
{
    float4 position : SV_Position;
    float4 color : COLOR;
};


VertexOut HairBaseVS(VertexIn vin)
{
    VertexOut vout;
    
    float4x4 viewProj = mul(view, proj);
    viewProj = transpose(viewProj);
    
    vout.position = mul(float4(vin.position, 1.0f), world);
    vout.position = mul(viewProj, vout.position);
    
    vout.color = vin.color;

    return vout;
}


float4 HairBasePS(VertexOut pin) : SV_Target
{
    return pin.color;
}