#if !defined(HANDMADE_INTRINSICS_H)

#include "math.h"

inline s32
RotateLeft(u32 Value, s32 Amount)
{
    u32 Result = _rotl(Value, Amount);
    
    return Result;
}

inline f32
AbsoluteValue(f32 value)
{
    f32 Result = 0;
    
    Result = (f32)fabs(value);
    
    return Result;
}


inline s32
RoundReal32ToInt32(f32 Real32)
{
    s32 Result = (s32)roundf(Real32);
    return(Result);
}

inline u32
RoundReal32ToUInt32(f32 Real32)
{
    u32 Result = (u32)roundf(Real32);
    return(Result);
}

inline s32 
FloorReal32ToInt32(f32 Real32)
{
    s32 Result = (s32)floorf(Real32);
    return(Result);
}

inline s32
TruncateReal32ToInt32(f32 Real32)
{
    s32 Result = (s32)Real32;
    return(Result);
}

inline f32
Sin(f32 Angle)
{
    f32 Result = sinf(Angle);
    return(Result);
}

inline f32
Cos(f32 Angle)
{
    f32 Result = cosf(Angle);
    return(Result);
}

inline f32
ATan2(f32 Y, f32 X)
{
    f32 Result = atan2f(Y, X);
    return(Result);
}

struct bit_scan_result
{
    bool32 Found;
    u32 Index;
};
inline bit_scan_result
FindLeastSignificantSetBit(u32 Value)
{
    bit_scan_result Result = {};
    
#if COMPILER_MSVC
    Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
    for(u32 Test = 0;
        Test < 32;
        ++Test)
    {
        if(Value & (1 << Test))
        {
            Result.Index = Test;
            Result.Found = true;
            break;
        }
    }
#endif
    
    return(Result);
}

#define HANDMADE_INTRINSICS_H
#endif

#if !defined(HANDMADE_INTRINSICS_H)

#include "math.h"

inline s32
RotateLeft(u32 Value, s32 Amount)
{
    u32 Result = _rotl(Value, Amount);
    
    return Result;
}

inline f32
AbsoluteValue(f32 value)
{
    f32 Result = 0;
    
    Result = (f32)fabs(value);
    
    return Result;
}


inline s32
RoundReal32ToInt32(f32 Real32)
{
    s32 Result = (s32)roundf(Real32);
    return(Result);
}

inline u32
RoundReal32ToUInt32(f32 Real32)
{
    u32 Result = (u32)roundf(Real32);
    return(Result);
}

inline s32 
FloorReal32ToInt32(f32 Real32)
{
    s32 Result = (s32)floorf(Real32);
    return(Result);
}

inline s32
TruncateReal32ToInt32(f32 Real32)
{
    s32 Result = (s32)Real32;
    return(Result);
}

inline f32
Sin(f32 Angle)
{
    f32 Result = sinf(Angle);
    return(Result);
}

inline f32
Cos(f32 Angle)
{
    f32 Result = cosf(Angle);
    return(Result);
}

inline f32
ATan2(f32 Y, f32 X)
{
    f32 Result = atan2f(Y, X);
    return(Result);
}

struct bit_scan_result
{
    bool32 Found;
    u32 Index;
};
inline bit_scan_result
FindLeastSignificantSetBit(u32 Value)
{
    bit_scan_result Result = {};
    
#if COMPILER_MSVC
    Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
    for(u32 Test = 0;
        Test < 32;
        ++Test)
    {
        if(Value & (1 << Test))
        {
            Result.Index = Test;
            Result.Found = true;
            break;
        }
    }
#endif
    
    return(Result);
}

#define HANDMADE_INTRINSICS_H
#endif

