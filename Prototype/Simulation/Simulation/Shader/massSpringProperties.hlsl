struct Particle
{
    float DesiredLength;
    //float4 DesiredRotation;
    float3 DesiredRelativPos;
    float3 Position;
    float Mass;
    float3 Velocity;
    int ParticleIdx;
};

static const float SPRING_CONSTANT = 10;
static const float ANGULAR_SPRING_CONSTANT = 10;
static const int PARTICLES_IN_STRAND = 16;
static const float MAXIMUM_STRECHING = 0.1;
static const float MAXIMUM_SKEWING = 0.3;

struct Strand
{
    int NumberOfParticles;
    int StrandIdx;
    Particle Particles[16];
};
