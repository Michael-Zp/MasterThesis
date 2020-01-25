#include "simulationStructs.hlsl"

cbuffer Time : register(b0)
{
    float deltaTime;
    float3 paddingTime;
};

cbuffer Properties : register(b1)
{
    float drag;
    float stiffness;
    float2 paddingProperties;
};

static const int3 numThreads = int3(1, 1, 1);

RWStructuredBuffer<Particle> particles;

[numthreads(numThreads.x, numThreads.y, numThreads.z)]
void Simulation( uint3 DTid : SV_DispatchThreadID )
{
    int idx = DTid.x * numThreads.x + DTid.y * numThreads.y + DTid.z * numThreads.z;
    
    if (particles[idx].StrandIdx > 0)
    {
        float3 force = float3(0, -5, 0);
        float3 dirToDesiredPos = (particles[idx - 1].Position + particles[idx].DesiredRelativePos) - particles[idx].Position;
        
        force += dirToDesiredPos * stiffness;
        
        float3 oldPos = particles[idx].Position;
        
        float3 frameVelocity = force * deltaTime;
        
        particles[idx].Velocity += frameVelocity;
        
        float3 newPos = particles[idx].Position + particles[idx].Velocity * deltaTime;
        
        float3 diff = newPos - particles[idx - 1].Position;
        
        if (dot(diff, diff) > particles[idx].Size)
        {
            float3 correctedPos = particles[idx - 1].Position + particles[idx].Size * normalize(diff);
            //particles[idx].Velocity += length(particles[idx].Velocity) * normalize(correctedPos - newPos);
            particles[idx].Velocity = (correctedPos - oldPos) / deltaTime;
            particles[idx].Position = correctedPos;
        }
        
        particles[idx].Velocity *= drag;
    }
}