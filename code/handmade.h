#if !defined(HANDMADE_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include "handmade_platform.h"

//
//
//

struct memory_arena
{
    memory_index Size;
    u8 *Base;
    memory_index Used;
};

internal void
InitializeArena(memory_arena *Arena, memory_index Size, u8 *Base)
{
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
void *
PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    
    return(Result);
}

#include "handmade_math.h"
#include "handmade_intrinsics.h"
#include "handmade_tile.h"

struct world
{    
    tile_map *TileMap;
};

struct loaded_bitmap
{
    s32 Width;
    s32 Height;
    u32*Pixels;
};

struct hero_bitmaps
{
    s32 AlignX;
    s32 AlignY;
    loaded_bitmap Head;
    loaded_bitmap Cape;
    loaded_bitmap Torso;
};

struct entity
{
    bool Exists;
    tile_map_position P;
    v2 dP;
    f32 Width;
    f32 Height;
    
    u32 FacingDirection;
};

struct game_state
{
    memory_arena WorldArena;
    world *World;
    
    tile_map_position CameraP;
    
    u32 CameraFollowingEntityIndex;
    
    s32 PlayerIndexForController[5];
    entity Entities[256];
    u32 EntityCount;
    
    loaded_bitmap Backdrop;
    hero_bitmaps HeroBitmaps[4];
};

#define HANDMADE_H
#endif

#if !defined(HANDMADE_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include "handmade_platform.h"

//
//
//

struct memory_arena
{
    memory_index Size;
    u8 *Base;
    memory_index Used;
};

internal void
InitializeArena(memory_arena *Arena, memory_index Size, u8 *Base)
{
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
void *
PushSize_(memory_arena *Arena, memory_index Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    
    return(Result);
}

#include "handmade_math.h"
#include "handmade_intrinsics.h"
#include "handmade_tile.h"

struct world
{    
    tile_map *TileMap;
};

struct loaded_bitmap
{
    s32 Width;
    s32 Height;
    u32*Pixels;
};

struct hero_bitmaps
{
    s32 AlignX;
    s32 AlignY;
    loaded_bitmap Head;
    loaded_bitmap Cape;
    loaded_bitmap Torso;
};

struct entity
{
    bool Exists;
    tile_map_position P;
    v2 dP;
    f32 Width;
    f32 Height;
    
    u32 FacingDirection;
};

struct game_state
{
    memory_arena WorldArena;
    world *World;
    
    tile_map_position CameraP;
    
    u32 CameraFollowingEntityIndex;
    
    s32 PlayerIndexForController[5];
    entity Entities[256];
    u32 EntityCount;
    
    loaded_bitmap Backdrop;
    hero_bitmaps HeroBitmaps[4];
};

#define HANDMADE_H
#endif

