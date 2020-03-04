#include "tractrixSplineProperties.hlsl"

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
void SimpleSimulation(uint3 DTid : SV_DispatchThreadID)
{
    int idx = DTid.x * numThreads.x + DTid.y * numThreads.y + DTid.z * numThreads.z;
    
}

struct TractrixStepReturn
{
    float3 NewTailPos;
    float3 NewHeadPos;
};

TractrixStepReturn TractrixStep(float3 tailPos, float3 headPos, float3 desiredHeadPos)
{
    TractrixStepReturn ret;
    ret.NewHeadPos = headPos;
    ret.NewTailPos = tailPos;
    
    
    float3 S = desiredHeadPos - headPos;
    float lengthS = length(S);
        
    float L = length(headPos - tailPos);
    
    float3 T = tailPos - headPos;
        
    float3 Xr = normalize(S);
        
    float3 Zr = normalize(cross(S, T));
        
    float3 Yr = cross(Zr, Xr);
       
    
    float3x3 R = float3x3(Xr.x, Xr.y, Xr.z, Yr.x, Yr.y, Yr.z, Zr.x, Zr.y, Zr.z);
        
    float y = dot(Yr, T);

    
    float p_p = L * sechInv(y / L) + lengthS;
    float p_n = L * sechInv(y / L) - lengthS;
    
    //Prevent stuff from jumping to harsh, because sechInv -> infinity if x -> 0
    p_p = clamp(p_p, -100, 100);
    
    float xr_p = +lengthS - L * tanh(p_p / L);
    float xr_n = -lengthS - L * tanh(p_p / L);
    
    float xr_np_p = +lengthS - L * tanh(p_n / L);
    float xr_np_n = -lengthS - L * tanh(p_n / L);
    
    float yr_p = L * sech(p_p / L);
    float yr_n = L * sech(p_n / L);
        
    
    float3 tempPos = float3(xr_p, yr_p, 0);
    
    

    if (length(tailPos - desiredHeadPos) > length(tailPos - headPos))  // Replacable by 'dot(normalize(T), normalize(S)) < 0' [for performance, because length of S and T might be used elsewhere too]
    {
        tempPos = float3(xr_p, yr_p, 0);
    }
    else
    {
        // Adjustment by me. Looks more realistic, if the motion of the head is in the direction of X
        tempPos = float3(-xr_np_n, yr_n, 0);
    }
    
    
    ret.NewHeadPos = desiredHeadPos;
        
    float3 newTailPos = headPos + mul(tempPos, R);
        
    if (length(newTailPos - tailPos) < length(desiredHeadPos - headPos))
    {
        // I donLt have infinite precision and sechInv -> infinity if x -> 0 so yeah... should prevent these ehm irregularities (basically everything just fucks up)
        // Additionaly it is mathematically correct, because in a tractrix the movement of the tail has to be smaller than the movement of the head (thats the whole point of this thing)
        ret.NewTailPos = headPos + mul(tempPos, R);
    }
    
    
    return ret;
}


void RecursiveTractrix(int idx)
{
    
    
    //See sreenivasan2010.pdf
    //https://doi.org/10.1016/j.mechmachtheory.2009.10.005
    //Chapter 3.1 and 4
        
        
    /*
    // S x T --> Crossproduct; |S| --> Length of S; ^T --> Transpose
    (0) //Added from eq. 12// L^2 = (x - xe)^2 + (y - ye)^2 + (z - ze)^2
    (1) Define the vector S = Xp - Xh where Xh is the current location of the head and Xp is the destination point of the head.
    (2) Define the vector T = X - Xh where X = (x; y; z)^T is the tail of the link lying on the tractrix.
    (3) Define the new reference coordinate system frg with the X-axis along S. Hence ^Xr = S / |S|.
    (4) Define the Z-axis as ^Zr = S x T / |S x T|.
    (5) Define rotation matrix [R] = [ ^Xr; ^Zr x ^Xr; ^Zr ].
    (6) The Y-coordinate of the tail (lying on the tractrix) is given by y = dot(^Yr, T) and the parameter p can be obtained as
    p = L * sech^-1(y / L) +- |S|.
    (7) From p, we can obtain the X and Y-coordinate of the point on the tractrix in the reference coordinate system as
    xr = +- |S| - L * tanh(p / L).
    yr = L * sech(p / l).
    (8) Once xr and yr are known, the point on the tractrix (x; y; z)^T in the global fixed coordinate system {0} is given by
    (x; y; z)^T = Xh + [R]*(xr; yr; 0)^T
    */
    
    int headParticleIdx = strands[idx].ParticlesCount - 1;
    int tailParticleIdx = headParticleIdx - 1;
    
    
    float3 X = strands[idx].Particles[tailParticleIdx].Position;
    float3 Xh = strands[idx].Particles[headParticleIdx].Position;
    float3 Xp = strands[idx].DesiredHeadPosition;
    
    Xp = float3(-1, -4.5, 0) + float3(5 * sin(totalTime * 0.5), 0, 0);
    
    for (int i = 1; i <= headParticleIdx; i++)
    {
        TractrixStepReturn tractrixResult = TractrixStep(X, Xh, Xp);

        X = strands[idx].Particles[tailParticleIdx - i].Position;
        Xh = strands[idx].Particles[headParticleIdx - i].Position;
        Xp = tractrixResult.NewTailPos;
        
        strands[idx].Particles[tailParticleIdx - i + 1].Position = tractrixResult.NewTailPos;
        strands[idx].Particles[headParticleIdx - i + 1].Position = tractrixResult.NewHeadPos;
    }
}

void KnotSubdivisionAndRemoval(int idx)
{
    for (int i = 1; i < strands[idx].ParticlesCount - 1; i++)
    {
        float3 cp0 = strands[idx].Particles[i].Position - strands[idx].Particles[i - 1].Position;
        float3 cp1 = strands[idx].Particles[i].Position - strands[idx].Particles[i + 1].Position;
        
        static const float minLength = 0.2;
        if (length(cp0) < minLength || length(cp1) < minLength)
        {
            continue;
        }
        
        float dotProd = dot(normalize(cp0), normalize(cp1));
        
        if (dotProd < KNOT_REMOVAL_THRESHOLD)
        {
            //Remove particle
        }
        else if (dotProd > KNOT_SUBDIVISION_THRESHOLD && strands[idx].ParticlesCount < MAX_PARTICLE_COUNT)
        {
            float3 p0 = strands[idx].Particles[i - 1].Position;
            float3 p1 = float3(0, 0, 0);
            float3 p2 = float3(0, 0, 0);
            float3 p3 = strands[idx].Particles[i + 1].Position;
            
            float lcp0 = length(cp0);
            float lcp1 = length(cp1);
            
            
            static const float c = 0.25;
            
            //Add particle
            if (lcp0 < lcp1)
            {
                p1 = p0 + cp0 * c;
                
                p2 = p3 + (normalize(cp1) * (lcp1 - (lcp0 * (1 - c))));
            }
            else
            {
                p2 = p3 + cp1 * c;
                
                p1 = p0 + (normalize(cp0) * (lcp0 - (lcp1 * (1 - c))));
            }
            
            
            for (int j = strands[idx].ParticlesCount; j > i; j--)
            {
                strands[idx].Particles[j] = strands[idx].Particles[j - 1];
            }
            strands[idx].ParticlesCount++;
            
            strands[idx].Particles[i].Position = p1;
            strands[idx].Particles[i].Color = float4(0, 1, 0, 1);
            strands[idx].Particles[i + 1].Position = p2;
            strands[idx].Particles[i + 1].Color = float4(0, 1, 0, 1);
        }
        

    }

}


[numthreads(numThreads.x, numThreads.y, numThreads.z)]
void Simulation(uint3 DTid : SV_DispatchThreadID)
{
    int idx = DTid.x * numThreads.x + DTid.y * numThreads.y + DTid.z * numThreads.z;
    
    RecursiveTractrix(idx);

    KnotSubdivisionAndRemoval(idx);
}