struct Particle
{
    float3 Position;
    float4 Color;
};

static const int MAX_PARTICLE_COUNT = 16;
static const int MAX_KNOT_SIZE = MAX_PARTICLE_COUNT + 4;
static const float KNOT_SUBDIVISION_THRESHOLD = 0.8;
static const float KNOT_REMOVAL_THRESHOLD = 0.2;

struct Strand
{
    int ParticlesCount;
    int StrandIdx;
    float3 DesiredHeadPosition;
    Particle Particles[MAX_PARTICLE_COUNT];
    float Knot[MAX_KNOT_SIZE];
};