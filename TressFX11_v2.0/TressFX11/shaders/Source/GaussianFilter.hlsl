//--------------------------------------------------------------------------------------
// File: GaussianFilter.hlsl
//
// Implements a classic Gaussian filter.
// 
// Author: Jon Story
// 
// Copyright © AMD Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "..\\..\\..\\AMD_SDK\\Shaders\\SeparableFilter\\FilterCommon.hlsl"

// Defines
#define PI                      ( 3.1415927f )
#define GAUSSIAN_DEVIATION      ( KERNEL_RADIUS * 0.5f )

// The input texture
Texture2D g_txInput : register( t0 ); 

// The output UAV used by the CS 
RWTexture2D<float4> g_uavOutput : register( u0 );

// CS output structure
struct CS_Output
{
    float4 f4Color[PIXELS_PER_THREAD]; 
};

// PS output structure
struct PS_Output
{
    float4 f4Color[1]; 
};

// Uncompressed data as sampled from inputs
struct RAWDataItem
{
    float3 f3Color;
};

// Data stored for a kernel
struct KernelData
{
    float fWeight;
    float fWeightSum;
};

//--------------------------------------------------------------------------------------
// Get a Gaussian weight
// The weights get compiled to constants, so the cost for this macro disappears 
//--------------------------------------------------------------------------------------
#define GAUSSIAN_WEIGHT( _fX, _fDeviation, _fWeight ) \
    _fWeight = 1.0f / sqrt( 2.0f * PI * _fDeviation * _fDeviation ); \
    _fWeight *= exp( -( _fX * _fX ) / ( 2.0f * _fDeviation * _fDeviation ) );


//--------------------------------------------------------------------------------------
// Sample from chosen input(s)
//--------------------------------------------------------------------------------------
#define SAMPLE_FROM_INPUT( _Sampler, _f2SamplePosition, _RAWDataItem ) \
    _RAWDataItem.f3Color = g_txInput.SampleLevel( _Sampler, _f2SamplePosition, 0 ).xyz;


//--------------------------------------------------------------------------------------
// Compute what happens at the kernels center 
//--------------------------------------------------------------------------------------
#define KERNEL_CENTER( _KernelData, _iPixel, _iNumPixels, _O, _RAWDataItem ) \
    [unroll] for( _iPixel = 0; _iPixel < _iNumPixels; ++_iPixel ) { \
        GAUSSIAN_WEIGHT( 0, GAUSSIAN_DEVIATION, _KernelData[_iPixel].fWeight ) \
        _KernelData[_iPixel].fWeightSum = _KernelData[_iPixel].fWeight; \
        _O.f4Color[_iPixel].xyz = _RAWDataItem[_iPixel].f3Color * _KernelData[_iPixel].fWeight; }     


//--------------------------------------------------------------------------------------
// Compute what happens for each iteration of the kernel 
//--------------------------------------------------------------------------------------
#define KERNEL_ITERATION( _iIteration, _KernelData, _iPixel, _iNumPixels, _O, _RAWDataItem ) \
    [unroll] for( _iPixel = 0; _iPixel < _iNumPixels; ++_iPixel ) { \
        GAUSSIAN_WEIGHT( ( _iIteration - KERNEL_RADIUS + ( 1.0f - 1.0f / float( STEP_SIZE ) ) ), GAUSSIAN_DEVIATION, _KernelData[_iPixel].fWeight ) \
        _KernelData[_iPixel].fWeightSum += _KernelData[_iPixel].fWeight; \
        _O.f4Color[_iPixel].xyz += _RAWDataItem[_iPixel].f3Color * _KernelData[_iPixel].fWeight; }


//--------------------------------------------------------------------------------------
// Perform final weighting operation 
//--------------------------------------------------------------------------------------
#define KERNEL_FINAL_WEIGHT( _KernelData, _iPixel, _iNumPixels, _O ) \
    [unroll] for( _iPixel = 0; _iPixel < _iNumPixels; ++_iPixel ) { \
        _O.f4Color[_iPixel].xyz /= _KernelData[_iPixel].fWeightSum; \
        _O.f4Color[_iPixel].w = 1.0f; }
        

//--------------------------------------------------------------------------------------
// Output to chosen UAV 
//--------------------------------------------------------------------------------------
#define KERNEL_OUTPUT( _i2Center, _i2Inc, _iPixel, _iNumPixels, _O, _KernelData ) \
    [unroll] for( _iPixel = 0; _iPixel < _iNumPixels; ++_iPixel ) \
        g_uavOutput[_i2Center + _iPixel * _i2Inc] = _O.f4Color[_iPixel];


//--------------------------------------------------------------------------------------
// Include the filter kernel logic that uses the above macros
//--------------------------------------------------------------------------------------
#include "..\\..\\..\\AMD_SDK\\Shaders\\SeparableFilter\\FilterKernel.hlsl"
#include "..\\..\\..\\AMD_SDK\\Shaders\\SeparableFilter\\HorizontalFilter.hlsl"
#include "..\\..\\..\\AMD_SDK\\Shaders\\SeparableFilter\\VerticalFilter.hlsl"


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------
