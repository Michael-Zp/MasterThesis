#include "oneDFlexObjProperties.hlsl"

cbuffer Time : register(b0)
{
    float deltaTime;
    float totalTime;
    float2 paddingTime;
};

cbuffer Properties : register(b1)
{
    float4 paddingProperties;
};

static const int3 numThreads = int3(1, 1, 1);
static const float3 GRAVITY = float3(0, -.981, 0);

RWStructuredBuffer<Strand> strands;

float sechInv(float z)
{
    //http://mathworld.wolfram.com/InverseHyperbolicSecant.html
    //return log(sqrt(1 / z - 1) * sqrt(1 / z + 1) + 1 / z);
    //But this seems to be for a complex number
    //thus
    //https://en.wikipedia.org/wiki/Inverse_hyperbolic_functions
    return log((1 + sqrt(1 - (z * z))) / z);
}

float sech(float z)
{
    //http://mathworld.wolfram.com/HyperbolicSecant.html
    //this seems to be identical with https://en.wikipedia.org/wiki/Hyperbolic_function
    return 1 / cosh(z);
}

[numthreads(numThreads.x, numThreads.y, numThreads.z)]
void Simulation(uint3 DTid : SV_DispatchThreadID)
{
    int idx = DTid.x * numThreads.x + DTid.y * numThreads.y + DTid.z * numThreads.z;
    
    
    
    int i = strands[idx].NumberOfParticles - 1;
        
    
    
    float3 X = strands[idx].Particles[i - 1].Position;
    float3 Xh = strands[idx].Particles[i].Position;
    float3 Xp = float3(sin(totalTime), -1.25 - totalTime * 0.5, 0);
    
    float3 rodLength = length(Xh - X);
    float3 movement = Xp - Xh;
    
    if (length(movement) < 0.01)
    {
        return;
    }
    
    float3 velocity = movement / deltaTime; // m = v * dT --> m / dT = v
    float velocityMagnitude = length(velocity);
    
    float3 tailVelocityDirection = normalize(normalize(Xp - X) + normalize(Xh - X));
    float3 desiredTailPos = X + tailVelocityDirection * velocityMagnitude * deltaTime;
    
    
    
    strands[idx].Particles[i].Position = Xp;
    strands[idx].Particles[i - 1].Position = Xp + normalize(desiredTailPos - Xp) * rodLength;
    
}