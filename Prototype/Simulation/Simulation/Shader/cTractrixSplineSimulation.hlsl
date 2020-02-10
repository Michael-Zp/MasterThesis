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
    
    int i = strands[idx].NumberOfParticles - 1;
        
    
    
    float3 X = strands[idx].Particles[i - 1].Position;
    float3 Xh = strands[idx].Particles[i].Position;
    float3 Xp = strands[idx].DesiredHeadPosition;
    
    
    //Xp = float3(sin(totalTime * 0.5), -1.25 - totalTime, 0);
    //Xp = float3(sin(totalTime), -1.25 - totalTime, 0);
    //Xp = float3(sin(deltaTime), -1.25 - deltaTime, 0);
    //Xp = float3(0, -1.25, sin(totalTime));
    //Xp = float3(sin(totalTime), -1.25, 0);
    //Xp = float3(-1, -1.25 - totalTime % (3.1415 * 2), 0);
    //Xp = float3(0, -1.25 - 0.1 * totalTime, sin(totalTime));
    Xp = float3(cos(totalTime * 0.5), -1.25 - totalTime * 0.5, sin(totalTime * 0.5));
    
    
    float3 S = Xp - Xh;
    float lengthS = length(S);
        
    if (lengthS < 0.01)
    {
        return;
    }
    
    
    float L = length(Xh - X);
    
    float3 T = X - Xh;
        
    float3 Xr = normalize(S);
        
    float3 Zr = normalize(cross(S, T));
        
    float3 Yr = cross(Zr, Xr);
       
    
    float3x3 R = float3x3(Xr.x, Xr.y, Xr.z, Yr.x, Yr.y, Yr.z, Zr.x, Zr.y, Zr.z);
        
    float y = dot(Yr, T);

    
    float p_p = L * sechInv(y / L) + lengthS;
    float p_n = L * sechInv(y / L) - lengthS;
    
    p_p = clamp(p_p, -100, 100);
    
    float xr_p = +lengthS - L * tanh(p_p / L);
    float xr_n = -lengthS - L * tanh(p_p / L);
    
    float xr_np_p = +lengthS - L * tanh(p_n / L);
    float xr_np_n = -lengthS - L * tanh(p_n / L);
    
    float yr_p = L * sech(p_p / L);
    float yr_n = L * sech(p_n / L);
        
    
    float3 tempPos = float3(xr_p, yr_p, 0);
    
    

    if (length(X - Xp) > length(X - Xh))  // Replacable by 'dot(normalize(T), normalize(S)) < 0' [for performance, because length of S and T might be used elsewhere too]
    {
        tempPos = float3(xr_p, yr_p, 0);
    }
    else
    {
        // Adjustment by me. Looks more realistic, if the motion of the head is in the direction of X
        strands[idx].Particles[i - 1].Color = float3(0, 1, 0);
        tempPos = float3(-xr_np_n, yr_n, 0);
    }
    
    
    float3 newPos0 = Xh + mul(tempPos, R);

    
    strands[idx].Particles[i - 1].Position = newPos0;
    strands[idx].Particles[i].Position = Xp;
}