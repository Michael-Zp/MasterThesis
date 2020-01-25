struct Particle
{
    float3 Position;
    float3 Color;
};

static const int PARTICLES_IN_STRAND = 16;

struct Strand
{
    int NumberOfParticles;
    int StrandIdx;
    float SimulatedTime;
    float3 DesiredHeadPosition;
    Particle Particles[16];
};
