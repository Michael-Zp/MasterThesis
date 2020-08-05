//Properties
static const int MAX_PARTICLE_COUNT = 32;
static const int MIN_PARTICLE_COUNT = 4;
static const int MAX_KNOT_SIZE = MAX_PARTICLE_COUNT * 2;
static const float KNOT_INSERTION_THRESHOLD = -0.8;
static const float KNOT_REMOVAL_THRESHOLD = 0.86; //As suggested in menon2016 160Åã (or in this case 20Åã because I measure the opposite angle)

struct Particle
{
    float3 Position;
    float3 Velocity;
};


struct Strand
{
    int ParticlesCount;
    int StrandIdx;
    float3 HairRoot;
    float3 OriginalHeadPosition;
    float3 DesiredSegmentDirections[MAX_PARTICLE_COUNT - 1];
    Particle Particles[MAX_PARTICLE_COUNT];
    float4 Color;
    float Knot[MAX_KNOT_SIZE];
    float KnotValues[MAX_KNOT_SIZE];
    float MaxKnotValue;
    float KnotHasChangedOnce;
};