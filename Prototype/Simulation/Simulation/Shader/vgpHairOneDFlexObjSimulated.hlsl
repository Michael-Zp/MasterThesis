#include "oneDFlexObjProperties.hlsl"

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
    nointerpolation int strandIdx : STRAND_IDX;
    nointerpolation int particleIdx : PARTICLE_IDX;
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
    
    int strandIdx = int(floor(vertexId / PARTICLES_IN_STRAND));
    int particleIdx = vertexId % PARTICLES_IN_STRAND;
    
    vout.position = mul(float4(strands[strandIdx].Particles[particleIdx].Position, 1.0f), world);
    vout.position = mul(viewProj, vout.position);
    vout.strandIdx = strandIdx;
    vout.particleIdx = particleIdx;
    
    vout.color = float4(strands[strandIdx].Particles[particleIdx].Color, 1.0f);

    return vout;
}


[maxvertexcount(4)]
void HairGS(line VertexOut vin[2], inout TriangleStream<GeoOut> gout)
{
    
    if (vin[1].particleIdx >= strands[vin[1].strandIdx].NumberOfParticles)
    {
        return;
    }
    
    float width = 0.1;
    
    GeoOut topLeft;
    topLeft.position = float4(vin[0].position.x - width, vin[0].position.y, vin[0].position.z, vin[0].position.w);
    topLeft.color = vin[0].color;
    
    GeoOut topRight;
    topRight.position = float4(vin[0].position.x + width, vin[0].position.y, vin[0].position.z, vin[0].position.w);
    topRight.color = vin[0].color;
    
    GeoOut bottomLeft;
    bottomLeft.position = float4(vin[1].position.x - width, vin[1].position.y, vin[1].position.z, vin[1].position.w);
    bottomLeft.color = vin[1].color;
    
    GeoOut bottomRight;
    bottomRight.position = float4(vin[1].position.x + width, vin[1].position.y, vin[1].position.z, vin[1].position.w);
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