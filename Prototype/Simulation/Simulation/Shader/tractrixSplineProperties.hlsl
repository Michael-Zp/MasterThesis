struct Particle
{
    float3 Position;
    float4 Color;
};

static const int MAX_PARTICLE_COUNT = 16;
static const int MIN_PARTICLE_COUNT = 4;
static const int MAX_KNOT_SIZE = MAX_PARTICLE_COUNT * 2;
static const float KNOT_INSERTION_THRESHOLD = -0.8;
static const float KNOT_REMOVAL_THRESHOLD = 0.86; //As suggested in menon2016 160Åã (or in this case 20Åã because I measure the opposite angle)

struct Strand
{
    int ParticlesCount;
    int StrandIdx;
    float3 HairRoot;
    float3 DesiredHeadMovement;
    float3 OriginalHeadPosition;
    Particle Particles[MAX_PARTICLE_COUNT];
    float Knot[MAX_KNOT_SIZE];
    float KnotValues[MAX_KNOT_SIZE];
    float MaxKnotValue;
    float KnotHasChangedOnce;
};