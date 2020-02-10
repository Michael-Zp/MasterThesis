struct Particle
{
    float3 Position;
    float4 Color;
};

static const int MAX_PARTICLE_COUNT = 16;

struct Strand
{
    float ParticlesCount;
    Particle particles[MAX_PARTICLE_COUNT];
};