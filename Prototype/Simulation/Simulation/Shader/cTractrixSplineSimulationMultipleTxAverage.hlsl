#include "tractrixSplineProperties.hlsl"


cbuffer Time : register(b0)
{
    float deltaTime;
    float totalTime;
    float2 paddingTime;
};

cbuffer Properties : register(b1)
{
    float doTractrix;
    float doKnotInsertion;
    float doKnotRemoval;
    float stopIfKnotChanged;
    float4 padding;
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

struct TractrixStepReturn
{
    float3 NewTailPos;
    float3 NewHeadPos;
};

bool floatEqual(float a, float b, float epsilon)
{
    float sVal = a - epsilon;
    float lVal = a + epsilon;
    bool smaller = sVal < b;
    bool larger = lVal > b;
    return smaller && larger;
}

bool floatEqual(float3 a, float3 b, float3 epsilon)
{
    return floatEqual(a.x, b.x, epsilon.x) && floatEqual(a.y, b.y, epsilon.y) && floatEqual(a.z, b.z, epsilon.z);
}

bool floatEqual(float3 a, float3 b, float epsilon)
{
    bool x = floatEqual(a.x, b.x, epsilon);
    bool y = floatEqual(a.y, b.y, epsilon);
    bool z = floatEqual(a.z, b.z, epsilon);
    return x && y && z;
}

bool floatEqual(float2 a, float2 b, float2 epsilon)
{
    return floatEqual(a.x, b.x, epsilon.x) && floatEqual(a.y, b.y, epsilon.y);
}

bool floatEqual(float2 a, float2 b, float epsilon)
{
    return floatEqual(a.x, b.x, epsilon) && floatEqual(a.y, b.y, epsilon);
}

#define fEq(a, b, e) (((a - e) < b) && ((a + e) > b))
#define fNEq(a, b, e) (((a - e) > b) && ((a + e) < b))

TractrixStepReturn TractrixStep(float3 tailPos, float3 headPos, float3 desiredHeadPos)
{
    TractrixStepReturn ret;
    ret.NewHeadPos = headPos;
    ret.NewTailPos = tailPos;
    
    
    float3 S = desiredHeadPos - headPos;
    float lengthS = length(S);
        
    float L = length(headPos - tailPos);
   
    
    float3 T = tailPos - headPos;
        
    
    //TODO See if this still a bug
    //See if they are linear independent, if cross == (0, 0, 0) they are not
    //If they are linear dependent, just add some stuff on T to get a correct coordinate system 
    //Otherwise cross(S, T) = float3(0, 0, 0) and this is not good.
    //If they just pull it straight the tail should follow exactly the same
    float3 crossST = normalize(cross(S, T));
    if (isnan(crossST.x) && isnan(crossST.y) && isnan(crossST.z))
    {
        ret.NewHeadPos = desiredHeadPos;
        ret.NewTailPos = desiredHeadPos - (headPos - tailPos);
        //strands[0].Particles[0].Color = float4(1, 0, 0, 1);
    }
    else
    {
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
    
    
        
        float3 newTailPos = headPos + mul(tempPos, R);
        
        if (length(newTailPos - tailPos) < length(desiredHeadPos - headPos))
        {
            // I don´t have infinite precision and sechInv -> infinity if x -> 0 so yeah... should prevent these ehm irregularities (basically everything just fucks up)
            // Additionaly it is mathematically correct, because in a tractrix the movement of the tail has to be smaller than the movement of the head (thats the whole point of this thing)
            ret.NewTailPos = headPos + mul(tempPos, R);
            ret.NewHeadPos = desiredHeadPos;
            //strands[0].Particles[0].Color = float4(0, 1, 0, 1);
        }
        else
        {
            //If the calculation fails, just set the head to the desiredHeadPos and 
            //pull the tail along the control polygon. This will prevent weird cases when in which
            //the tail will follow the exact same movement as the head (like a z falling straight down instead of flexing)
            //strands[0].Particles[0].Color = float4(0, 0, 1, 1);
            ret.NewHeadPos = desiredHeadPos;
            ret.NewTailPos = tailPos + (headPos - tailPos) * length(desiredHeadPos - headPos);
        }
    }
    
    
    
    return ret;
}


void RecursiveTractrixForward(int idx, int startIndex, float3 Xp, out float3 newParticlePositions[MAX_PARTICLE_COUNT])
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
    
    for (int i = 0; i < MAX_PARTICLE_COUNT; i++)
    {
        newParticlePositions[i] = float3(0, 0, 0);
    }
    
    if ( /*forwards && */startIndex + 1 < strands[idx].ParticlesCount)
    {
        int headParticleIdx = startIndex;
        int tailParticleIdx = headParticleIdx + 1;
        
        float3 Xh = strands[idx].Particles[headParticleIdx].Position;
        float3 Xt = strands[idx].Particles[tailParticleIdx].Position;
        
        headParticleIdx++;
        tailParticleIdx++;
        
        TractrixStepReturn tractrixResult;
        
        for (; tailParticleIdx < strands[idx].ParticlesCount; headParticleIdx++, tailParticleIdx++)
        {
            tractrixResult = TractrixStep(Xt, Xh, Xp);
        
            Xh = strands[idx].Particles[headParticleIdx].Position;
            Xt = strands[idx].Particles[tailParticleIdx].Position;
            Xp = tractrixResult.NewTailPos;
            
            newParticlePositions[tailParticleIdx - 1] = tractrixResult.NewTailPos;
        }
        
        tractrixResult = TractrixStep(Xt, Xh, Xp);
        
        newParticlePositions[tailParticleIdx - 1] = tractrixResult.NewTailPos;
    }
}

void RecursiveTractrixBackward(int idx, int startIndex, float3 Xp, out float3 newParticlePositions[MAX_PARTICLE_COUNT])
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
    
    for (int i = 0; i < MAX_PARTICLE_COUNT; i++)
    {
        newParticlePositions[i] = float3(0, 0, 0);
    }

    if ( /*!forwards && */startIndex - 1 >= 0)
    {
        int headParticleIdx = startIndex;
        int tailParticleIdx = headParticleIdx - 1;
        
        float3 Xh = strands[idx].Particles[headParticleIdx].Position;
        float3 Xt = strands[idx].Particles[tailParticleIdx].Position;
        
        headParticleIdx--;
        tailParticleIdx--;
        
        TractrixStepReturn tractrixResult;
        
        //for (; tailParticleIdx >= 0; headParticleIdx--, tailParticleIdx--)
        for (int i = 0; i < MAX_PARTICLE_COUNT; i++)
        {
            tractrixResult = TractrixStep(Xt, Xh, Xp);
        
            Xh = strands[idx].Particles[max(headParticleIdx - i, 0)].Position;
            Xt = strands[idx].Particles[max(tailParticleIdx - i, 0)].Position;
            Xp = tractrixResult.NewTailPos;
            
            float idxAcessable = step(-0.5, tailParticleIdx - i + 1);
            newParticlePositions[max(tailParticleIdx - i + 1, 0)] = tractrixResult.NewTailPos * idxAcessable + newParticlePositions[max(tailParticleIdx - i + 1, 0)] * (1 - idxAcessable);
        }
        
        //tractrixResult = TractrixStep(Xt, Xh, Xp);
        
        //newParticlePositions[tailParticleIdx + 1] = tractrixResult.NewTailPos;
    }
    
    
    
    
    //if (step(0.5, reverse) > 0.5)
    //{
    //    for (int i = tailParticleIdx; i < strands[idx].ParticlesCount - 2; i++)
    //    {
    //        tractrixResult = TractrixStep(Xt, Xh, Xp);
        
    //        Xh = strands[idx].Particles[headParticleIdx + i * dir].Position;
    //        Xt = strands[idx].Particles[tailParticleIdx + i * dir].Position;
    //        Xp = tractrixResult.NewTailPos;
    //        newParticlePositions[tailParticleIdx + i * dir - dir] = tractrixResult.NewTailPos;
    //    }
    //}
    //else
    //{
    //    for (int i = 1; tailParticleIdx + i * dir >= 0; i++)
    //    {
    //        tractrixResult = TractrixStep(Xt, Xh, Xp);
        
    //        Xh = strands[idx].Particles[headParticleIdx + i * dir].Position;
    //        Xt = strands[idx].Particles[tailParticleIdx + i * dir].Position;
    //        Xp = tractrixResult.NewTailPos;
    //        newParticlePositions[tailParticleIdx + i * dir - dir] = tractrixResult.NewTailPos;
    //    }
    //}
}

void InsertValueIntoKnot(int idx, float newKnotValue)
{
    //bool foundSomething = false;
    //for (int u = MAX_KNOT_SIZE - 2; u >= 0; u--)
    //{
    //    if (strands[idx].Knot[i] > 0)
    //    {
    //        foundSomething = true;
    //    }
        
    //    if(foundSomething)
    //    {
    //        if (newKnotValue < strands[idx].Knot[i])
    //        {
    //            strands[idx].Knot[i + 1] = strands[idx].Knot[i];
    //        }
    //        else
    //        {
    //            strands[idx].Knot[i + 1] = newKnotValue;
    //            break;
    //        }
    //    }
    //}
    float foundSomething = 0.0;
    float theOlSwitcheroooHappened = 0.0;
    for (int u = MAX_KNOT_SIZE - 2; u >= 0; u--)
    {
        foundSomething = step(0.5, foundSomething + step(0.5, strands[idx].Knot[u]));
        float isBigger = step(newKnotValue, strands[idx].Knot[u]);
        
        //if (foundSomething)
        //{
        //    if (!theOlSwitcheroooHappened)
        //    {
        //        if (isBigger)
        //        {
        //            strands[idx].Knot[u + 1] = strands[idx].Knot[u];
        //        }
        //        else
        //        {
        //            strands[idx].Knot[u + 1] = newKnotValue;
        //        }
        //    }
        //    else
        //    {
        //        strands[idx].Knot[u + 1] = strands[idx].Knot[u + 1];
        //    }
        //}
        //else
        //{
        //    strands[idx].Knot[u + 1] = strands[idx].Knot[u + 1];
        //}
        
        strands[idx].Knot[u + 1] = (1 - foundSomething) * strands[idx].Knot[u + 1] +
                                        foundSomething * (1 - theOlSwitcheroooHappened) * isBigger * strands[idx].Knot[u] +
                                        foundSomething * (1 - theOlSwitcheroooHappened) * (1 - isBigger) * newKnotValue +
                                        foundSomething * theOlSwitcheroooHappened * strands[idx].Knot[u + 1];
        
        
        theOlSwitcheroooHappened = step(0.5, theOlSwitcheroooHappened + foundSomething * (1 - isBigger));
    }
    
    //Same thing as above just with KnotValues instead of Knot. Yes they will differ at some point
    foundSomething = 0.0;
    theOlSwitcheroooHappened = 0.0;
    for (u = MAX_KNOT_SIZE - 2; u >= 0; u--)
    {
        foundSomething = step(0.5, foundSomething + step(0.5, strands[idx].KnotValues[u]));
        float isBigger = step(newKnotValue, strands[idx].KnotValues[u]);
        
        strands[idx].KnotValues[u + 1] = (1 - foundSomething) * strands[idx].KnotValues[u + 1] +
                                        foundSomething * (1 - theOlSwitcheroooHappened) * isBigger * strands[idx].KnotValues[u] +
                                        foundSomething * (1 - theOlSwitcheroooHappened) * (1 - isBigger) * newKnotValue +
                                        foundSomething * theOlSwitcheroooHappened * strands[idx].KnotValues[u + 1];
        
        
        theOlSwitcheroooHappened = step(0.5, theOlSwitcheroooHappened + foundSomething * (1 - isBigger));
    }
}

void KnotRemoval(int idx, int i, float dotProdL0L1, float dotProdL1L2)
{
    i = 2;
    
    float3 l0 = strands[idx].Particles[i].Position - strands[idx].Particles[i - 1].Position;
    float3 l1 = strands[idx].Particles[i + 1].Position - strands[idx].Particles[i].Position;
    float3 l2 = strands[idx].Particles[i + 2].Position - strands[idx].Particles[i + 1].Position;
    
    float3 p1 = strands[idx].Particles[i - 1].Position;
    float3 p2 = strands[idx].Particles[i].Position;
    float3 p3 = strands[idx].Particles[i + 1].Position;
    float3 p4 = strands[idx].Particles[i + 2].Position;
    
    
    
    //See menon2016 p.17
    float3 p1s = p2 + l1 * 0.5;
    //For p2s:
    /*
        All references (like equations) are from menon2016 p.17
        If given equation 24 and we substitute:
        p4 = [-L3; 0]
        p3 = [0; 0]
        p2 = [L3 * cos(sig2); L3 * sin(sig2)]
        p1 = [L3 * cos(sig2) - L2 * cos(sig2 + sig1); L3 * sin(sig2) - L3 * sin(sig2 + sig1)]
        
        It should be the same, but just using p3 as origin instead of p2, which makes the equation for p2s much simpler.
    
        p2s = [1 / 2 * L3; 0]
    
        which is very similar to the equation for p1s in eq. 26
    */
    float3 p2s = p3 - l2 * 0.5;
    
    
    float3 newPoint = float3(0, 0, 0);
   
    
    float originalLenght = length(l0) + length(l1) + length(l2);
    //Replace with dot if it works as I should not need squared length. Should do the same (dont forget to change originalLength!!)
    float3 p1ToP1s = p1 - p1s;
    float3 p3ToP1s = p3 - p1s;
    float3 p1ToP2s = p1 - p2s;
    float3 p3ToP2s = p3 - p2s;
    float p1sLength = dot(p1ToP1s, p1ToP1s) + dot(p3ToP1s, p3ToP1s);
    float p2sLength = dot(p1ToP2s, p1ToP2s) + dot(p3ToP2s, p3ToP2s);
    
    float factorWhetherPointAtIorIPlus1 = 0.0;
    
    //TODO This check is shit, find a better heuristic (maybe squared distance from original points?)
    if (p1sLength > p2sLength)
    {
        newPoint = p1s;
        factorWhetherPointAtIorIPlus1 = 1.0;
    }
    else
    {
        newPoint = p2s;
        factorWhetherPointAtIorIPlus1 = 2.0;
    }
    
    //TODO Rewrite to support loop unrolling
    for (int j = i + 1; j < strands[idx].ParticlesCount - 1; j++)
    {
        strands[idx].Particles[j] = strands[idx].Particles[j + 1];
    }
    
    strands[idx].ParticlesCount--;
    strands[idx].Particles[i].Position = newPoint;
    strands[idx].Particles[i].Color = float4(1, 1, 0, 1);
    
    
    float myKnotValue = strands[idx].KnotValues[i];
    float otherKnotValue = strands[idx].KnotValues[i + 1];
    
       
    
    float epsilon = 1e-3;
    
    
    // TODO Make without ifs should be easy with steps
    // Start at k = 4 because the first 4 zeros should never be removed
    // The MaxKnotValue (0 0 0 0 1 2 2 2 2 => 2) should also not be removed, but the KnotValue should never be 2
    // because it needs 3 polygons with 4 points and the second knotValue is always the value of third point, thus
    // never the last point in the spline thus never MaxKnotValue
    for (int k = 4, nk = k; nk < MAX_KNOT_SIZE - 3; k++, nk++)
    {
        if (floatEqual(myKnotValue, strands[idx].Knot[k], epsilon))
        {
            nk++;
        }
        
        if (floatEqual(otherKnotValue, strands[idx].Knot[k], epsilon))
        {
            nk++;
        }
        
        strands[idx].Knot[k] = strands[idx].Knot[nk];
    }
    
    
    //// Strat from zero because loop unrolling
    for (int u = 0; u < MAX_KNOT_SIZE - 2; u++)
    {
        if (u >= i)
        {
            strands[idx].KnotValues[u] = strands[idx].KnotValues[u + 2];
        }
        else
        {
            strands[idx].KnotValues[u] = strands[idx].KnotValues[u];
        }
    }
    
    
    float newKnotValue = myKnotValue + factorWhetherPointAtIorIPlus1 * 0.34 * (otherKnotValue - myKnotValue);
    
    InsertValueIntoKnot(idx, newKnotValue);
}

void KnotInsertion(int idx, int i)
{
    float3 cp0 = strands[idx].Particles[i].Position - strands[idx].Particles[i - 1].Position;
    float3 cp1 = strands[idx].Particles[i].Position - strands[idx].Particles[i + 1].Position;
    
    float3 p0 = strands[idx].Particles[i - 1].Position;
    float3 p1 = float3(0, 0, 0);
    float3 p2 = float3(0, 0, 0);
    float3 p3 = strands[idx].Particles[i + 1].Position;
            
    float lcp0 = length(cp0);
    float lcp1 = length(cp1);
            
            
    static const float c = 0.3;
    static const float d = 0.3;
            
    
    //If the knot is inserted at t = 0.66 decide whether to insert the new knot between 0.66 and 1 -> after middle or 0.33 and 0.66 -> before middle
    float getKnotAfterMiddle = 0.0;
    //Add particle
    if (lcp0 < lcp1)
    {
        p1 = p0 + cp0 * c;
                
        //p2 = p3 + (normalize(cp1) * (lcp1 - (lcp0 * (1 - c))));
        p2 = p3 + (normalize(cp1) * (lcp1 - (lcp0 * d)));

    }
    else
    {
        p2 = p3 + cp1 * c;
                
        //p1 = p0 + (normalize(cp0) * (lcp0 - (lcp1 * (1 - c))));
        p1 = p0 + (normalize(cp0) * (lcp0 - (lcp1 * d)));
        getKnotAfterMiddle = 1.0;
    }
            
            
    for (int j = strands[idx].ParticlesCount; j > i; j--)
    {
        strands[idx].Particles[j] = strands[idx].Particles[j - 1];
    }
    strands[idx].ParticlesCount++;
            
    strands[idx].Particles[i].Position = p1;
    strands[idx].Particles[i + 1].Position = p2;
    
    
    float myKnotValue = strands[idx].KnotValues[i];
    
    
    //if (getKnotAfterMiddle)
    //{
    //    otherKnotValue = strands[idx].KnotValues[i + 1];
    //}
    //else
    //{
    //    otherKnotValue = strands[idx].KnotValues[i - 1];
    //}
    float otherKnotValue = strands[idx].KnotValues[i - 1 + step(0.5, getKnotAfterMiddle) * 2];
    float newKnotValue = (myKnotValue + otherKnotValue) / 2;
    
    InsertValueIntoKnot(idx, newKnotValue);
}

void KnotInsertionAndRemoval(int idx)
{
    float dotProds[MAX_PARTICLE_COUNT - 2];
    for (int j = 1; j < strands[idx].ParticlesCount - 1; j++)
    {
        float3 cp0 = strands[idx].Particles[j].Position - strands[idx].Particles[j - 1].Position;
        float3 cp1 = strands[idx].Particles[j + 1].Position - strands[idx].Particles[j].Position;
        
        static const float minLength = 0.2;
        if (length(cp0) < minLength || length(cp1) < minLength)
        {
            dotProds[j - 1] = 0;
        }
        
        dotProds[j - 1] = dot(normalize(cp0), normalize(cp1));
    }
    
    for (int i = 0; i < strands[idx].ParticlesCount - 2 && strands[idx].ParticlesCount < MAX_PARTICLE_COUNT; i++)
    {
        if (dotProds[i] < KNOT_INSERTION_THRESHOLD)
        {
            if (doKnotInsertion)
            {
                KnotInsertion(idx, i + 1);
            }
            strands[idx].KnotHasChangedOnce = 1.0;
        }
    }
    
    //Start from k = 2 because the first three knots values are intrinsic (0 0 0 0 1 2 2 2 2) you cant remove one of the zeroes thus I just ignore these cases
    for (int k = 1; k < strands[idx].ParticlesCount - 3 && strands[idx].ParticlesCount > MIN_PARTICLE_COUNT; k++)
    {
        if (dotProds[k] > KNOT_REMOVAL_THRESHOLD && dotProds[k + 1] > KNOT_REMOVAL_THRESHOLD)
        {
            if (doKnotRemoval)
            {
                KnotRemoval(idx, k + 1, dotProds[k], dotProds[k + 1]);
            }
            strands[idx].KnotHasChangedOnce = 1.0;
        }
    }
}

[numthreads(numThreads.x, numThreads.y, numThreads.z)]
void Simulation(uint3 DTid : SV_DispatchThreadID)
{
    int idx = DTid.x * numThreads.x + DTid.y * numThreads.y + DTid.z * numThreads.z;
    
    float3 Xp = strands[idx].OriginalHeadPosition;
    float timeDialation = 1;
   
    float3 originalPosition[MAX_PARTICLE_COUNT];
    for (int op_i = 0; op_i < MAX_PARTICLE_COUNT; op_i++)
    {
        originalPosition[op_i] = strands[idx].Particles[op_i].Position;
    }
    
    //Get the old segment length of each segment, to maintain the strand structure
    float oldSegmentLength[MAX_PARTICLE_COUNT - 1];
    for (int u = 0; u < MAX_PARTICLE_COUNT - 1; u++)
    {
        oldSegmentLength[u] = length(strands[idx].Particles[u].Position - strands[idx].Particles[u + 1].Position);
    }
    
    
    float3 desiredPosition[MAX_PARTICLE_COUNT];
    for (int dp_i = 0; dp_i < MAX_PARTICLE_COUNT; dp_i++)
    {
        desiredPosition[dp_i] = strands[idx].Particles[dp_i].Position;
    }
    
    desiredPosition[0] = strands[idx].HairRoot;
    
    //Save desired position of every particle if they would move independently (except the root)
    //The root does not have a desiredPosition, because it does not move on its own and if the head moves, than the movement is covered by the backpull
    for (int i = 1; i < MAX_PARTICLE_COUNT; i++)
    {
        strands[idx].Particles[i].Velocity += float3(0, -9.81, 0) * strands[idx].Particles[i].Mass * deltaTime * timeDialation;
        strands[idx].Particles[i].Velocity *= 0.995; // Simple drag
        
        
        //Drag force by air resistance
        float airDensity = 1.2; //Density of air see: https://en.wikipedia.org/wiki/Density
        float3 velocitySquare = pow(strands[idx].Particles[i].Velocity, float3(2, 2, 2));
        float dragCoefficient = 0.47; //Hair should be roughtly a sphere see https://en.wikipedia.org/wiki/Drag_coefficient
        float crossSection = (0.1 / 100) * oldSegmentLength[i - 1]; //Diameter of hair (ranges from 0.017mm to 0.18mm see https://en.wikipedia.org/wiki/Hair thus ~0.1mm thus 0.01cm)[right now I am in cm/s^2 for gravity]
        float3 dragForce = 0.5 * airDensity * velocitySquare * dragCoefficient * crossSection;
        
        strands[idx].Particles[i].Velocity -= sign(strands[idx].Particles[i].Velocity) * abs(dragForce);
        
        desiredPosition[i] = strands[idx].Particles[i].Position + strands[idx].Particles[i].Velocity * deltaTime;
        
        float3 desiredPositionByHairStyle = strands[idx].Particles[i - 1].Position + normalize(strands[idx].DesiredSegmentDirections[i - 1]) * oldSegmentLength[i - 1];
        float hairStyleFactor = 0.1;
        strands[idx].Particles[i].Velocity += pow((desiredPositionByHairStyle - strands[idx].Particles[i].Position), 2) * 0.5;
    }
    
    //TODO Maybe use less space with using one 1D array, and calculating like:
    //Calc pos -> Multiply by factor -> Add to allPossPartPos[i] -> Start again
    //After all are summed up -> take average
    float3 forwardPossibleParticlePositions[MAX_PARTICLE_COUNT][MAX_PARTICLE_COUNT];
    float3 backwardPossibleParticlePositions[MAX_PARTICLE_COUNT][MAX_PARTICLE_COUNT];
    for (int appp_i = 0; appp_i < MAX_PARTICLE_COUNT; appp_i++)
    {
        RecursiveTractrixForward(idx, appp_i, desiredPosition[appp_i], forwardPossibleParticlePositions[appp_i]);
        forwardPossibleParticlePositions[appp_i][appp_i] = desiredPosition[appp_i] / 2;
        RecursiveTractrixBackward(idx, appp_i, desiredPosition[appp_i], backwardPossibleParticlePositions[appp_i]);
        backwardPossibleParticlePositions[appp_i][appp_i] = desiredPosition[appp_i] / 2;
    }
    
    const static int numberOfParticlesAveraged = 4;
    
    
    //Make copy of all temporary strands and pull them back
    //This backpull is used for altering the velocity of the particle for which the temporary strand was generated
    //This backpull will prevent velocities from going out of hand as this will service as the counter force by the strand and the root
    for (int backpullForce_i = 1; backpullForce_i < numberOfParticlesAveraged; backpullForce_i++)
    {
        for (int backpullForce_u = 0; backpullForce_u < numberOfParticlesAveraged; backpullForce_u++)
        {
            strands[idx].Particles[backpullForce_u].Position = forwardPossibleParticlePositions[backpullForce_i][backpullForce_u] + backwardPossibleParticlePositions[backpullForce_i][backpullForce_u];
        }
        
        float3 backpullForcePositions[MAX_PARTICLE_COUNT];
        for (int backpullForceZero_i = 0; backpullForceZero_i < MAX_PARTICLE_COUNT; backpullForceZero_i++)
        {
            backpullForcePositions[backpullForceZero_i] = float3(0, 0, 0);
        }
        
        RecursiveTractrixForward(idx, 0, strands[idx].HairRoot, backpullForcePositions);
        
        float3 backMovement = (backpullForcePositions[backpullForce_i] - strands[idx].Particles[backpullForce_i].Position) / deltaTime;
        strands[idx].Particles[backpullForce_i].Velocity += backMovement;
    }
    
    //RecursiveTractrix(idx, strands[idx].ParticlesCount - 1, desiredPosition[strands[idx].ParticlesCount - 1], false, allPossibleParticlePositions[0]);
        
        
    float3 currentParticlePosition[numberOfParticlesAveraged];
    for (int setPos_i = 0; setPos_i < numberOfParticlesAveraged; setPos_i++)
    {
        
        currentParticlePosition[setPos_i] = float3(0, 0, 0);
        for (int ppp_i = 0; ppp_i < numberOfParticlesAveraged; ppp_i++)
        {
            currentParticlePosition[setPos_i] += forwardPossibleParticlePositions[ppp_i][setPos_i] + backwardPossibleParticlePositions[ppp_i][setPos_i];
        }
        currentParticlePosition[setPos_i] /= numberOfParticlesAveraged;
        
        //currentParticlePosition[setPos_i] = float3(0, 0, 0);
        
        //currentParticlePosition += forwardPossibleParticlePositions[0][setPos_i] + backwardPossibleParticlePositions[0][setPos_i];
        //currentParticlePosition += forwardPossibleParticlePositions[1][setPos_i] + backwardPossibleParticlePositions[1][setPos_i];
        //currentParticlePosition += forwardPossibleParticlePositions[2][setPos_i] + backwardPossibleParticlePositions[2][setPos_i];
        //currentParticlePosition[setPos_i] += forwardPossibleParticlePositions[3][setPos_i] + backwardPossibleParticlePositions[3][setPos_i];
        //currentParticlePosition /= 1;
        //strands[idx].Particles[setPos_i].Position = currentParticlePosition[setPos_i];
    }
    
    //The average positions can change the length of the segments.
    //Thus just use these positions as pointers for the segment direction.
    float3 averageDirs[numberOfParticlesAveraged - 1];
    for (int getDirs_i = 0; getDirs_i < numberOfParticlesAveraged - 1; getDirs_i++)
    {
        averageDirs[getDirs_i] = currentParticlePosition[getDirs_i + 1] - currentParticlePosition[getDirs_i];
    }
    
    //Add the desired strand direction to the average directions to maintain a certain hair style
    //for (int addDesiredStrandDirection_i = 0; addDesiredStrandDirection_i < numberOfParticlesAveraged - 1; addDesiredStrandDirection_i++)
    //{
    //    float hairStyleFactor = 0.05;
    //    averageDirs[addDesiredStrandDirection_i] = (1 - hairStyleFactor) * averageDirs[addDesiredStrandDirection_i];
    //    averageDirs[addDesiredStrandDirection_i] += hairStyleFactor * strands[idx].DesiredSegmentDirections[addDesiredStrandDirection_i];
    //}
   
    
    //Set the particle positions, by setting the root position as the average position of every simulation
    //The next particle positions will be determined by the last set position + the average direction of the strand * the segment length
    float3 currentPos = currentParticlePosition[0];
    //float3 currentPos = strands[idx].HairRoot;
    for (int setFinalPos_i = 0; setFinalPos_i < numberOfParticlesAveraged; setFinalPos_i++)
    {
        strands[idx].Particles[setFinalPos_i].Position = currentPos;
        //Use the max to prevent the index from going out of bounds
        currentPos += oldSegmentLength[min(setFinalPos_i, numberOfParticlesAveraged - 2)] * normalize(averageDirs[min(setFinalPos_i, numberOfParticlesAveraged - 2)]);
    }
    
    //float3 allPossibleParticlePositions[MAX_PARTICLE_COUNT];
    //for (int setPos_i = 0; setPos_i < 1; setPos_i++)
    //{
    //    allPossibleParticlePositions[setPos_i] = float3(0, 0, 0);
    //    for (int setPos_k = 0; setPos_k < MAX_PARTICLE_COUNT; setPos_k++)
    //    {
    //        float3 possibleParticlePosition = forwardPossibleParticlePositions[setPos_k][setPos_i] + backwardPossibleParticlePositions[setPos_k][setPos_i];
    //        allPossibleParticlePositions[setPos_i] += possibleParticlePosition;
    //    }
        
    //    //allPossibleParticlePositions[setPos_i] /= strands[idx].ParticlesCount;
    //    strands[idx].Particles[setPos_i].Position = allPossibleParticlePositions[setPos_i];
    //}
    
    
    if (doTractrix && !(stopIfKnotChanged && strands[idx].KnotHasChangedOnce))
    {
        float3 positionsAfterBackpull[MAX_PARTICLE_COUNT];
        float3 velocityToAdd[MAX_PARTICLE_COUNT];
        for (int u = 0; u < MAX_PARTICLE_COUNT; u++)
        {
            positionsAfterBackpull[u] = float3(0, 0, 0);
            velocityToAdd[u] = float3(0, 0, 0);
        }
       
        
        
        for (int addPerpendicularForce_i = 0; addPerpendicularForce_i < numberOfParticlesAveraged - 1; addPerpendicularForce_i++)
        {
            //A strand can move either anywhere if it is moved by its own velocity or it is swung in a circular motion with the connection to the previous particle as fix point.
            //All excessive force should be directed in this perpendicular/circular motion
            float3 strandDir = normalize(averageDirs[addPerpendicularForce_i]);
            float3 velocityDir = normalize(strands[idx].Particles[addPerpendicularForce_i + 1].Velocity);
            float3 velocityDirOfPrevious = normalize(strands[idx].Particles[addPerpendicularForce_i + 1 - 1].Velocity);
            float3 movementOfPrevious = normalize(strands[idx].Particles[addPerpendicularForce_i + 1 - 1].Position - originalPosition[addPerpendicularForce_i + 1 - 1]);
            
            //Get a vector, that is perpendicular to the strand direction, but this would specify a 360° space, thus orient it in the direction of the velocity.
            //float3 perpendicularDir = normalize(velocityDir - dot(strandDir, velocityDir) * strandDir); //Project velocity on a plane perpendicular to the strand dir
            float3 perpendicularDir = normalize(velocityDirOfPrevious - dot(strandDir, velocityDirOfPrevious) * strandDir); //Project velocity of the previous particle on a plane perpendicular to the strand dir
            
            if (all(!isnan(perpendicularDir)))
            {
                //strands[idx].Particles[addPerpendicularForce_i + 1].Velocity += perpendicularDir * (length(strands[idx].HairRoot - strands[idx].Particles[0].Position) / 2);
                //velocityToAdd[addPerpendicularForce_i + 1] = perpendicularDir * (length(strands[idx].HairRoot - strands[idx].Particles[0].Position)) / 2;
                //velocityToAdd[addPerpendicularForce_i + 1] = perpendicularDir * (length(movementOfPrevious));
            }
        }
        
        
        RecursiveTractrixForward(idx, 0, strands[idx].HairRoot, positionsAfterBackpull);
        
        //for (int subtractForce_i = 0; subtractForce_i < MAX_PARTICLE_COUNT; subtractForce_i++)
        //{
        //    float3 particleMovementByBackpull = strands[idx].Particles[subtractForce_i].Position - positionsAfterBackpull[subtractForce_i];
        //    float velLength = length(strands[idx].Particles[subtractForce_i].Velocity);
            
        //    float3 normalizedVelocityToAdd = normalize(velocityToAdd[subtractForce_i]);
            
        //    if (all(!isnan(normalizedVelocityToAdd)))
        //    {
        //        velocityToAdd[subtractForce_i] = normalizedVelocityToAdd * length(particleMovementByBackpull);
        //    }
            
        //}
        
        for (int i = 0; i < MAX_PARTICLE_COUNT; i++)
        {
            strands[idx].Particles[i].Position = positionsAfterBackpull[i];
        }
        
        strands[idx].Particles[0].Position = strands[idx].HairRoot;
        
        
        for (int addForce_i = 0; addForce_i < MAX_PARTICLE_COUNT; addForce_i++)
        {
            //float3 tractrixParticleFromMovement = (strands[idx].Particles[addForce_i].Position - originalPosition[addForce_i]);
            //float3 vel = strands[idx].Particles[addForce_i].Velocity;
            //vel += float3(tractrixParticleFromMovement.x, tractrixParticleFromMovement.y, tractrixParticleFromMovement.z) * float3(1, 1, 1);
            //strands[idx].Particles[addForce_i].Velocity = vel;
            //strands[idx].Particles[addForce_i].Velocity = ((strands[idx].Particles[addForce_i].Position - originalPosition[addForce_i]) / deltaTime) * 1.1;
            
            //float3 actualMovement = originalPosition[addForce_i] - strands[idx].Particles[addForce_i].Position;
            //float3 desiredMovement = originalPosition[addForce_i] - desiredPosition[addForce_i];
            //float3 notMovedMovment = normalize(desiredMovement) * (length(desiredMovement) - length(actualMovement));
            //strands[idx].Particles[addForce_i].Velocity = normalize(strands[idx].Particles[addForce_i].Velocity) * (length(desiredMovement) - length(notMovedMovment)) / deltaTime;
            
            //strands[idx].Particles[addForce_i].Velocity = (strands[idx].Particles[addForce_i].Position - originalPosition[addForce_i]) / deltaTime;
            float3 vel = strands[idx].Particles[addForce_i].Velocity;
            vel += float3(1, 1, 1);
            vel -= float3(1, 1, 1);
            strands[idx].Particles[addForce_i].Velocity = vel + velocityToAdd[addForce_i];

        }
    }
    
    for (int k = 0; k < MAX_PARTICLE_COUNT - 1; k++)
    {
        float L = length(strands[idx].Particles[k].Position - strands[idx].Particles[k + 1].Position);
        
        if (L > 2.0)
        {
            strands[idx].Particles[0].Color = float4(1, 1, 1, 1);
            break;
        }
        else
        {
            strands[idx].Particles[0].Color = float4(1, 0, 0, 1);
        }
    }
    

    //KnotInsertionAndRemoval(idx);
}