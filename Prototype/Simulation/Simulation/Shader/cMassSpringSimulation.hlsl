#include "massSpringProperties.hlsl"

cbuffer Time : register(b0)
{
    float deltaTime;
    float3 paddingTime;
};

cbuffer Properties : register(b1)
{
    float4 paddingProperties;
};

static const int3 numThreads = int3(1, 1, 1);
static const float3 GRAVITY = float3(0, -.981, 0);

RWStructuredBuffer<Strand> strands;

float3x3 quaternionToRotMatrix(float4 quat);
float4 hamiltonProduct(float4 p1, float4 p2);
float4 quaternionRotation(float3 pnt, float4 quat);

[numthreads(numThreads.x, numThreads.y, numThreads.z)]
void Simulation(uint3 DTid : SV_DispatchThreadID)
{
    int idx = DTid.x * numThreads.x + DTid.y * numThreads.y + DTid.z * numThreads.z;
    int i = 0;
    float3 externalForces = GRAVITY;
    for (; i < strands[idx].NumberOfParticles - 1; i++)
    {        
        /*
        # Add external forces together
        Force += GRAVITY;
        Force += WIND;
        */
        
        float3 forceRoot = float3(0, 0, 0);
        float3 forceLeaf = float3(0, 0, 0);
        
        forceRoot += externalForces;
        forceLeaf += externalForces;
        
        
        
        /*
        # Add spring forces for length springs
        //LenghtForce = SPRING_FORCE * length(posi, posi-1) - sizePosiToPosi-1 * direction; //Try to keep the length of a segment constant
        //Force[i] += 0.5 * LengthForce //Every action has a counteraction
        //Force[i-1] -= 0.5 * LengthForce
        //Now with actual formulas
        // ### Force = -k * x //k => Spring constant, x => distance to desired pos/length
        Force[i] -= 0.5 * k * (length(p0, p1) - size) * ((p0 - p1) / length(p0, p1)); //Make sure p0 =/= p1
        */
        
        float3 p0 = strands[idx].Particles[i].Position;
        float3 p1 = strands[idx].Particles[i + 1].Position;
        float3 lengthDirection = normalize(p1 - p0);
        float desiredLength = strands[idx].Particles[i].DesiredLength;
        
        /*
        # Clamp the stretching
        //Choose a high k, because hair does not stretch well
        //If dT is to big, system will diverge
        //--> Limit the extension of springs clamp(length(p0, p1) - size, -t, t)
            // ==> Could also limit velocity in general (whould actually make sense with hair, because it is really light in my opinion)
        */
        
        float lengthDiff = clamp((desiredLength - length(p1 - p0)), -MAXIMUM_STRECHING, MAXIMUM_STRECHING);
        
        float forceAmplitude = -SPRING_CONSTANT * lengthDiff;
        forceRoot += 0.5 * lengthDirection * forceAmplitude;
        forceLeaf -= 0.5 * lengthDirection * forceAmplitude;
        
        
        
        
        
        /*
        # Add angular springs
        //At particle p1 the angle between segment p01 and p12 has a desired value
        //If the segments are not unit length k has to be adjusted for it to work properly (bigger segment, lower k)
        AngularForce[i] = 0.5 * k * angle(p01, p12) - desiredAngle * direction;
        */
        
        if (i > 0)
        {
            //float3 segment0 = strands[idx].Particles[i].Position - strands[idx].Particles[i - 1].Position;
            //float3 segment1 = strands[idx].Particles[i + 1].Position - strands[idx].Particles[i].Position;
            
            
            ////See http://lolengine.net/blog/2013/09/18/beautiful-maths-quaternion-from-vectors for explanation
            //float3 w = cross(segment0, segment1);
            //float4 quaternion = float4(1.f + dot(segment0, segment1), w.x, w.y, w.z);
            //quaternion.w += length(quaternion);
            //quaternion = normalize(quaternion);
            
            //float4 quaternionDif = strands[idx].Particles[i].DesiredRotation - quaternion;

            //float3x3 rotMatrix = quaternionToRotMatrix(quaternionDif);
            
            //float3 rootPos = segment1;
            //float3 desiredPos = mul(rootPos, rotMatrix);
            
            //float3 angularDirection = normalize(desiredPos - rootPos);
            
            //strands[idx].Particles[i].Position += desiredPos;
            //strands[idx].Particles[i].Position -= desiredPos;
            
            float3 currentRelativPosition = strands[idx].Particles[i].Position - strands[idx].Particles[i - 1].Position;
            float3 vecToDesiredPos = strands[idx].Particles[i].DesiredRelativPos - currentRelativPosition;
            
            //See MathTest - AngularSpringsWithQuaternions for explanation
            float3 angularDirToDesiredPos = vecToDesiredPos + (1 - dot(strands[idx].Particles[i].DesiredRelativPos, currentRelativPosition)) * currentRelativPosition;
            angularDirToDesiredPos = normalize(angularDirToDesiredPos);
            
            
            float boundSkewLength = clamp(length(vecToDesiredPos), -MAXIMUM_SKEWING, MAXIMUM_SKEWING);
            
            //Normally length(vecToDesiredPos) * normlize(vecToDesiredPos) but this is rather useless
            forceRoot += 0.5 * ANGULAR_SPRING_CONSTANT * angularDirToDesiredPos * boundSkewLength;
            forceLeaf -= 0.5 * ANGULAR_SPRING_CONSTANT * angularDirToDesiredPos * boundSkewLength;
            
            
            //forceRoot = 0.5 * ANGULAR_SPRING_CONSTANT * length(quaternion - strands[idx].Particles[i].DesiredRotation) * angularDirection;
        }
        
        /*
        
        Force += SPRING_FORCES; (For example DesiredRelativePos)
        
        
        */
        
        /*
        # Move particle
        accel = Force / mass;
        velocity += accel * dTime;
        
        offset += velocity * dTime;
        postion = basePos + offset;
        */
        
        float3 accelRoot = forceRoot / strands[idx].Particles[i].Mass;
        float3 accelLeaf = forceLeaf / strands[idx].Particles[i + 1].Mass;
        
        strands[idx].Particles[i].Velocity += accelRoot * deltaTime;
        strands[idx].Particles[i + 1].Velocity += accelLeaf * deltaTime;
        
        float3 frameMovementRoot = strands[idx].Particles[i].Velocity * deltaTime;
        float3 frameMovementLeaf = strands[idx].Particles[i + 1].Velocity * deltaTime;
        
        if (i > 0)
        {
            strands[idx].Particles[i].Position += frameMovementRoot;
        }
        strands[idx].Particles[i + 1].Position += frameMovementLeaf;
        
        float3 direction = normalize(strands[idx].Particles[i + 1].Position - strands[idx].Particles[i].Position);
        float stretchLength = length(strands[idx].Particles[i + 1].Position - strands[idx].Particles[i].Position);
        float stretchFactor = clamp(stretchLength / desiredLength, -MAXIMUM_STRECHING + 1, MAXIMUM_STRECHING + 1);
        
        strands[idx].Particles[i + 1].Position = strands[idx].Particles[i].Position + direction * stretchFactor * desiredLength;
        
        /*
        # Move by angular velocity and force
        angularAccel = AngularForce / mass;
        angularVelocity += angularAccel * dTime;
        
        angleOffset += angularVelocity * dTime;
        */
        //### PROBABLY OBSOLETE
        //if (i > 0)
        //{
        //    float4 accelAngular = angularForce / strands[idx].Particles[i].Mass;
            
        //    strands[idx].Particles[i].AngularVelocity += accelAngular * deltaTime;
        
        //    float3x3 rotMatrix = quaternionToRotMatrix(strands[idx].Particles[i].AngularVelocity) * length(strands[idx].Particles[i].AngularVelocity);
                        
        //    float3x3 frameAngularMovement = rotMatrix;
        
        //    float3 centeredPosition = strands[idx].Particles[i].Position - strands[idx].Particles[i - 1].Position;
            
        //    centeredPosition = mul(centeredPosition, frameAngularMovement);
            
        //    strands[idx].Particles[i].Position = strands[idx].Particles[i - 1].Position + centeredPosition;
        //}
        
        //float4(1, 0, 0, 0);
        //float4(0.71, 0, -0.71, 0);
        
        //float4(1.71, 0, -0.71, 0);
        
        //float4(0.93, 0, -0.37, 0); Target
        
        
        /*
        
        //Add offset and angleOffset on particle i+1, except if tip of strand
        basePos = particle[i+1].position + offset

        */
    }
    
    strands[idx].Particles[i].Velocity += externalForces / strands[idx].Particles[i].Mass * deltaTime;
    strands[idx].Particles[i].Position += strands[idx].Particles[i].Velocity * deltaTime;

}



float3x3 quaternionToRotMatrix(float4 q)
{
    //See https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation#Quaternion-derived_rotation_matrix
    
    float s = length(q);
    
    float m00 = 1 - 2 * s * (q.z * q.z + q.w * q.w);
    float m01 = 2 * s * (q.y * q.z - q.w * q.x);
    float m02 = 2 * s * (q.y * q.w + q.z * q.x);
    float m10 = 2 * s * (q.y * q.z + q.w * q.x);
    float m11 = 1 - 2 * s * (q.y * q.y + q.w * q.w);
    float m12 = 2 * s * (q.z * q.w - q.y * q.x);
    float m20 = 2 * s * (q.y * q.w - q.z * q.x);
    float m21 = 2 * s * (q.z * q.w + q.y * q.x);
    float m22 = 1 - 2 * s * (q.y * q.y + q.z * q.z);
    float3x3 ret = float3x3(
        m00, m01, m02,
        m10, m11, m12,
        m20, m21, m22
    );
    return ret;
}


// See https://en.wikipedia.org/wiki/Quaternion#Hamilton_product
float4 hamiltonProduct(float4 p1, float4 p2)
{
    return float4(p1.x * p2.x - p1.y * p2.y - p1.z * p2.z - p1.w * p2.w,
		p1.x * p2.y + p1.y * p2.x + p1.z * p2.w - p1.w * p2.z,
		p1.x * p2.z - p1.y * p2.w + p1.z * p2.x + p1.w * p2.y,
		p1.x * p2.w + p1.y * p2.z - p1.z * p2.y + p1.w * p2.x);
}


// See https://math.stackexchange.com/questions/40164/how-do-you-rotate-a-vector-by-a-unit-quaternion
float4 quaternionRotation(float3 pnt, float4 quat)
{
    float4 p = float4(0, pnt.x, pnt.y, pnt.z);
    float4 q0 = quat;
    float4 q1 = float4(quat.x, -quat.y, -quat.z, -quat.w);

    return hamiltonProduct(hamiltonProduct(q0, p), q1);
}