struct Particle
{
    float3 Position;
    float4 Color;
};

static const int MAX_PARTICLE_COUNT = 16;

struct Strand
{
    int ParticlesCount;
    int StrandIdx;
    float3 DesiredHeadPosition;
    Particle Particles[MAX_PARTICLE_COUNT];
};