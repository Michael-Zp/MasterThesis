#include "tractrixSplineProperties.hlsl"

cbuffer Camera
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
};

StructuredBuffer<Strand> strands;

struct VertexOut
{
    float4 position : POSITION;
    float4 color : COLOR;
};

struct GeoOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};


VertexOut HairVS(uint vertexId : SV_VertexID)
{
    VertexOut vout;
    
    float4x4 viewProj = mul(view, proj);
    viewProj = transpose(viewProj);
   
    
    int strandIdx = (int) floor(vertexId / MAX_PARTICLE_COUNT);
    
    vertexId = min(vertexId % MAX_PARTICLE_COUNT, strands[strandIdx].ParticlesCount - 1);
    
    float3 pos = strands[strandIdx].Particles[vertexId].Position;
    
    vout.position = mul(float4(pos, 1.0f), world);
    vout.position = mul(viewProj, vout.position);
    
    vout.color = float4(0, 0, 1, 1);

    return vout;
}


[maxvertexcount(4)]
void HairGS(line VertexOut vin[2], inout TriangleStream<GeoOut> gout)
{
    float width = 0.03;
    
    float2 forward = float2(vin[1].position.xy - vin[0].position.xy);
    float4 side = normalize(float4(-forward.y, forward.x, 0, 0));
    
    GeoOut topLeft;
    topLeft.position = float4(vin[0].position.x, vin[0].position.y, vin[0].position.z, vin[0].position.w) - side * width;
    topLeft.color = vin[0].color;
    
    GeoOut topRight;
    topRight.position = float4(vin[0].position.x, vin[0].position.y, vin[0].position.z, vin[0].position.w) + side * width;
    topRight.color = vin[0].color;
    
    GeoOut bottomLeft;
    bottomLeft.position = float4(vin[1].position.x, vin[1].position.y, vin[1].position.z, vin[1].position.w) - side * width;
    bottomLeft.color = vin[1].color;
    
    GeoOut bottomRight;
    bottomRight.position = float4(vin[1].position.x, vin[1].position.y, vin[1].position.z, vin[1].position.w) + side * width;
    bottomRight.color = vin[1].color;

    gout.Append(topRight);
    gout.Append(bottomRight);
    gout.Append(topLeft);
    gout.Append(bottomLeft);

}



float4 HairPS(GeoOut pin) : SV_Target
{
    return pin.color;
}