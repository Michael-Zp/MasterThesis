struct Particle
{
    float3 Position;
};

static const int PARTICLES_IN_STRAND = 16;

struct Strand
{
    int NumberOfParticles;
    int StrandIdx;
    float3 DesiredHeadPosition;
    Particle Particles[16];
};
