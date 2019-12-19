#include "simulationStructs.hlsl"

RWStructuredBuffer<Particle> positions;

[numthreads(2, 1, 1)]
void Simulation( uint3 DTid : SV_DispatchThreadID )
{
    if (DTid.x == 0 && DTid.y == 0 && DTid.z == 0)
    {
        positions[0].Parameter += 0.01;
        positions[0].Position = float3(sin(positions[0].Parameter), positions[0].Position.y, cos(positions[0].Parameter));
    }
}