#include "bSplineStructs.hlsl"

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

float3 cubicSplineInterpolationStep(float3 c0, float3 c1, float t_lower, float t_upper, float t)
{
    return (t_upper - t) / (t_upper - t_lower) * c0 + (t - t_lower) / (t_upper - t_lower) * c1;
}


float3 myMultipleCubicSplineInterpolation(float3 c[4], float ts[4], float t)
{
    float3 q01 = cubicSplineInterpolationStep(c[0], c[1], ts[0], ts[1], t);
    float3 q11 = cubicSplineInterpolationStep(c[1], c[2], ts[1], ts[2], t);
    float3 q21 = cubicSplineInterpolationStep(c[2], c[3], ts[2], ts[3], t);
    
    float3 q02 = cubicSplineInterpolationStep(q01, q11, ts[0], ts[2], t);
    float3 q12 = cubicSplineInterpolationStep(q11, q21, ts[1], ts[2], t);
    
    float3 q03 = cubicSplineInterpolationStep(q02, q12, ts[0], ts[3], t);
    
    return q03;
}

float3 cubicSpline(uint vertexId)
{
    int strandIdx = (int) floor(vertexId / vertexCount);
    int cubicPartIdx = (int) floor(clamp(((float) vertexId / vertexCount) * strands[strandIdx].ParticlesCount, 0, strands[strandIdx].ParticlesCount - 1));
    
    float t = (vertexId / vertexCount) * (strands[strandIdx].ParticlesCount - 3);
    
    int d = strands[strandIdx].ParticlesCount - 1;
    
    
    int knotSize = strands[strandIdx].ParticlesCount + 4;
    const static int maxKnotSize = MAX_PARTICLE_COUNT + 4;
    float knot[maxKnotSize];
    float maxKnotValue = strands[strandIdx].ParticlesCount - 3;
    for (int i = 0; i < 4; i++)
    {
        knot[i] = 0;
        knot[knotSize - i - 1] = maxKnotValue;
    }
    
    for (i = 0; i < knotSize - 8; i++)
    {
        knot[i + 4] = i + 1;
    }
    
    int minStartIndex = 3; //Because the knot is always (0, 0, 0, 0, 1, ...) and 3 is the fourth entry in the knot where knot[i] <= t <= knot[i+1] if 0 < t < 1
    int maxStartIndex = minStartIndex + knotSize - 8; //Knot is (0,0,0,0,1,..,n,n,n,n) so in what part t is => 3 + knotSize - 4*0s - 4*ns
    
    int startJ = clamp(minStartIndex + floor(t), minStartIndex, maxStartIndex);
    
    float3 p00 = strands[strandIdx].particles[startJ - 3].Position;
    float3 p10 = strands[strandIdx].particles[startJ - 2].Position;
    float3 p20 = strands[strandIdx].particles[startJ - 1].Position;
    float3 p30 = strands[strandIdx].particles[startJ - 0].Position;
    
    float j = max(startJ - 3, 1);
    float rr = 1;
    float3 p11 = (knot[j + d - rr + 1] - t) / (knot[j + d - rr + 1] - knot[j]) * p00 +
                    (t - knot[j]) / (knot[j + d - rr + 1] - knot[j]) * p10;
    j++;
    float3 p21 = (knot[j + d - rr + 1] - t) / (knot[j + d - rr + 1] - knot[j]) * p10 +
                    (t - knot[j]) / (knot[j + d - rr + 1] - knot[j]) * p20;
    j++;
    float3 p31 = (knot[j + d - rr + 1] - t) / (knot[j + d - rr + 1] - knot[j]) * p20 +
                    (t - knot[j]) / (knot[j + d - rr + 1] - knot[j]) * p30;
    
    rr++;
    j = startJ - 1;
    float3 p22 = (knot[j + d - rr + 1] - t) / (knot[j + d - rr + 1] - knot[j]) * p11 +
                    (t - knot[j]) / (knot[j + d - rr + 1] - knot[j]) * p21;
    j++;
    float3 p32 = (knot[j + d - rr + 1] - t) / (knot[j + d - rr + 1] - knot[j]) * p21 +
                    (t - knot[j]) / (knot[j + d - rr + 1] - knot[j]) * p31;
    
    
    rr++;
    j = startJ;
    float3 p33 = (knot[j + d - rr + 1] - t) / (knot[j + d - rr + 1] - knot[j]) * p22 +
                    (t - knot[j]) / (knot[j + d - rr + 1] - knot[j]) * p32;
    
    return p33;
}

VertexOut HairVS(uint vertexId : SV_VertexID)
{
    VertexOut vout;
    
    float4x4 viewProj = mul(view, proj);
    viewProj = transpose(viewProj);
    
    float3 pos = cubicSpline(vertexId);
    
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