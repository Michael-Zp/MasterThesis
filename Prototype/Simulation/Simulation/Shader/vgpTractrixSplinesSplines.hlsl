#include "tractrixSplineProperties.hlsl"

cbuffer Camera
{
    float4x4 world;
    float4x4 view;
    float4x4 proj;
};

cbuffer CubicSpline
{
    float vertexCount;
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

#define getKnot(i) (strands[strandIdx].Knot[i])

// See: https://www.ibiblio.org/e-notes/Splines/basis.html
float Ni1(int i, float t, int strandIdx)
{
    return step(getKnot(i), t) * step(t + 1e-5, getKnot(i + 1));
    //if (t >= getKnot(i) && t < getKnot(i + 1))
    //    return 1;
    //else
    //    return 0;
}

float3 splines(uint vertexId)
{
    int strandIdx = (int) floor(vertexId / vertexCount);
    
    float maxKnotValue = strands[strandIdx].ParticlesCount - 3;
    float t = (float(vertexId) / vertexCount) * maxKnotValue;
    
    float3 sum = float3(0, 0, 0);
    int k = 0;
    int j = 0;
    int limit = MAX_PARTICLE_COUNT; //Readability in debugger
    static const float NO_DIVISION_BY_ZERO_GUARD = 1e-5;
    for (int i = 0; i < limit; i++)
    {
        
        k = 1;
        j = i;
        float Nik1 = Ni1(j, t, strandIdx);
        j = i + 1;
        float Ni1k1 = Ni1(j, t, strandIdx);
        j = i + 2;
        float Ni2k1 = Ni1(j, t, strandIdx);
        j = i + 3;
        float Ni3k1 = Ni1(j, t, strandIdx);
        
        k = 2;
        j = i;
        float Nik2 = Nik1 * (t - getKnot(j)) / (getKnot(j + k - 1) - getKnot(j) + NO_DIVISION_BY_ZERO_GUARD) +
                Ni1k1 * (getKnot(j + k) - t) / (getKnot(j + k) - getKnot(j + 1) + NO_DIVISION_BY_ZERO_GUARD);
        j = i + 1;
        float Ni1k2 = Ni1k1 * (t - getKnot(j)) / (getKnot(j + k - 1) - getKnot(j) + NO_DIVISION_BY_ZERO_GUARD) +
                Ni2k1 * (getKnot(j + k) - t) / (getKnot(j + k) - getKnot(j + 1) + NO_DIVISION_BY_ZERO_GUARD);
        j = i + 2;
        float Ni2k2 = Ni2k1 * (t - getKnot(j)) / (getKnot(j + k - 1) - getKnot(j) + NO_DIVISION_BY_ZERO_GUARD) +
                Ni3k1 * (getKnot(j + k) - t) / (getKnot(j + k) - getKnot(j + 1) + NO_DIVISION_BY_ZERO_GUARD);
        
        k = 3;
        j = i;
        float Nik3 = Nik2 * (t - getKnot(j)) / (getKnot(j + k - 1) - getKnot(j) + NO_DIVISION_BY_ZERO_GUARD) +
                Ni1k2 * (getKnot(j + k) - t) / (getKnot(j + k) - getKnot(j + 1) + NO_DIVISION_BY_ZERO_GUARD);
        j = i + 1;
        float Ni1k3 = Ni1k2 * (t - getKnot(j)) / (getKnot(j + k - 1) - getKnot(j) + NO_DIVISION_BY_ZERO_GUARD) +
                Ni2k2 * (getKnot(j + k) - t) / (getKnot(j + k) - getKnot(j + 1) + NO_DIVISION_BY_ZERO_GUARD);
        
        k = 4;
        j = i;
        float Nik4 = Nik3 * (t - getKnot(j)) / (getKnot(j + k - 1) - getKnot(j) + NO_DIVISION_BY_ZERO_GUARD) +
                Ni1k3 * (getKnot(j + k) - t) / (getKnot(j + k) - getKnot(j + 1) + NO_DIVISION_BY_ZERO_GUARD);
        
        sum += Nik4 * strands[strandIdx].Particles[i].Position;
    }
    
    return sum;
}

VertexOut
    HairVS(
    uint vertexId : SV_VertexID)
{
    VertexOut vout;
    
    float4x4 viewProj = mul(view, proj);
    viewProj = transpose(viewProj);
    
    float3 pos = splines(vertexId);
    
    vout.position = mul(float4(pos, 1.0f), world);
    vout.position = mul(viewProj, vout.position);
    
    vout.color = float4(1, 0, 0, 1);

    return vout;
}


[maxvertexcount(4)]
void HairGS(line VertexOut vin[2], inout TriangleStream<GeoOut> gout)
{
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