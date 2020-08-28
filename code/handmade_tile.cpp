#if !defined(HANDMADE_TILE_H)

inline tile_chunk *
GetTileChunk(tile_map *TileMap, u32 TileChunkX, u32 TileChunkY, u32 TileChunkZ)
{
    tile_chunk *TileChunk = 0;
    
    if((TileChunkX >= 0) && (TileChunkX < TileMap->TileChunkCountX) &&
       (TileChunkY >= 0) && (TileChunkY < TileMap->TileChunkCountY) &&
       (TileChunkZ >= 0) && (TileChunkZ < TileMap->TileChunkCountZ))
    {
        TileChunk = &TileMap->TileChunks[
                                         TileChunkZ*TileMap->TileChunkCountY*TileMap->TileChunkCountX +
                                         TileChunkY*TileMap->TileChunkCountX +
                                         TileChunkX];
    }
    
    return(TileChunk);
}

inline u32
GetTileValueUnchecked(tile_map *TileMap, tile_chunk *TileChunk, u32 TileX, u32 TileY)
{
    Assert(TileChunk);
    Assert(TileX < TileMap->ChunkDim);
    Assert(TileY < TileMap->ChunkDim);
    
    u32 TileChunkValue = TileChunk->Tiles[TileY*TileMap->ChunkDim + TileX];
    return(TileChunkValue);
}

inline void
SetTileValueUnchecked(tile_map *TileMap, tile_chunk *TileChunk, u32 TileX, u32 TileY,
                      u32 TileValue)
{
    Assert(TileChunk);
    Assert(TileX < TileMap->ChunkDim);
    Assert(TileY < TileMap->ChunkDim);
    
    TileChunk->Tiles[TileY*TileMap->ChunkDim + TileX] = TileValue;
}

inline u32
GetTileValue(tile_map *TileMap, tile_chunk *TileChunk, u32 TestTileX, u32 TestTileY)
{
    u32 TileChunkValue = 0;
    
    if(TileChunk && TileChunk->Tiles)
    {
        TileChunkValue = GetTileValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY);
    }
    
    return(TileChunkValue);
}

inline void
SetTileValue(tile_map *TileMap, tile_chunk *TileChunk,
             u32 TestTileX, u32 TestTileY, u32 TileValue)
{
    if(TileChunk && TileChunk->Tiles)
    {
        SetTileValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY, TileValue);
    }
}

inline tile_chunk_position
GetChunkPositionFor(tile_map *TileMap, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    tile_chunk_position Result;
    
    Result.TileChunkX = AbsTileX >> TileMap->ChunkShift;
    Result.TileChunkY = AbsTileY >> TileMap->ChunkShift;
    Result.TileChunkZ = AbsTileZ;
    Result.RelTileX = AbsTileX & TileMap->ChunkMask;
    Result.RelTileY = AbsTileY & TileMap->ChunkMask;
    
    return(Result);
}

internal u32
GetTileValue(tile_map *TileMap, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY, AbsTileZ);
    tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);
    u32 TileChunkValue = GetTileValue(TileMap, TileChunk, ChunkPos.RelTileX, ChunkPos.RelTileY);
    
    return(TileChunkValue);
}



internal u32
GetTileValue(tile_map *TileMap, tile_map_position Pos)
{
    u32 TileChunkValue = GetTileValue(TileMap, Pos.AbsTileX, Pos.AbsTileY, Pos.AbsTileZ);
    
    return(TileChunkValue);
}

internal bool
IsTileValueEmpty(u32 Value)
{
    bool32 Empty = ((Value == 1) ||
                    (Value == 3) ||
                    (Value == 4));
    
    return(Empty);
}

internal bool32
IsTileMapPointEmpty(tile_map *TileMap, tile_map_position Pos)
{
    u32 TileChunkValue = GetTileValue(TileMap, Pos);
    bool32 Empty = IsTileValueEmpty(TileChunkValue);
    
    return(Empty);
}

internal void
SetTileValue(memory_arena *Arena, tile_map *TileMap,
             u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ,
             u32 TileValue)
{
    tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY, AbsTileZ);
    tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);
    
    Assert(TileChunk);
    if(!TileChunk->Tiles)
    {
        u32 TileCount = TileMap->ChunkDim*TileMap->ChunkDim;
        TileChunk->Tiles = PushArray(Arena, TileCount, u32);
        for(u32 TileIndex = 0;
            TileIndex < TileCount;
            ++TileIndex)
        {
            TileChunk->Tiles[TileIndex] = 1;
        }
    }
    
    SetTileValue(TileMap, TileChunk, ChunkPos.RelTileX, ChunkPos.RelTileY, TileValue);
}

//
// TODO(casey): Do these really belong in more of a "positioning" or "geometry" file?
//

inline void
RecanonicalizeCoord(tile_map *TileMap, u32 *Tile, f32 *TileRel)
{
    // TODO(casey): Need to do something that doesn't use the divide/multiply method
    // for recanonicalizing because this can end up rounding back on to the tile
    // you just came from.
    
    // NOTE(casey): TileMap is assumed to be toroidal topology, if you
    // step off one end you come back on the other!
    s32 Offset = RoundReal32ToInt32(*TileRel / TileMap->TileSideInMeters);
    *Tile += Offset;
    *TileRel -= Offset*TileMap->TileSideInMeters;
    
    // TODO(casey): Fix floating point math so this can be < ?
    Assert(*TileRel >= -0.5f*TileMap->TileSideInMeters);
    Assert(*TileRel <= 0.5f*TileMap->TileSideInMeters);
}

inline tile_map_position
RecanonicalizePosition(tile_map *TileMap, tile_map_position Pos)
{
    tile_map_position Result = Pos;
    
    RecanonicalizeCoord(TileMap, &Result.AbsTileX, &Result.Offset.X);
    RecanonicalizeCoord(TileMap, &Result.AbsTileY, &Result.Offset.Y);
    
    return(Result);
}

inline bool32
AreOnSameTile(tile_map_position *A, tile_map_position *B)
{
    bool32 Result = ((A->AbsTileX == B->AbsTileX) &&
                     (A->AbsTileY == B->AbsTileY) &&
                     (A->AbsTileZ == B->AbsTileZ));
    
    return(Result);
}

inline tile_map_difference
Subtract(tile_map *TileMap, tile_map_position *A, tile_map_position *B)
{
    tile_map_difference Result;
    
    v2 dTileXY = {(f32)A->AbsTileX - (f32)B->AbsTileX,
        (f32)A->AbsTileY - (f32)B->AbsTileY};
    f32 dTileZ = (f32)A->AbsTileZ - (f32)B->AbsTileZ;
    
    Result.dXY = TileMap->TileSideInMeters*dTileXY + (A->Offset - B->Offset);
    
    // TODO(casey): Think about what we want to do about Z
    Result.dZ = TileMap->TileSideInMeters*dTileZ;
    
    return(Result);
}

inline tile_map_position
GetCenteredTileMapPosition(s32 AbsTileX, s32 AbsTileY, s32 AbsTileZ)
{
    tile_map_position Result = {};
    
    Result.AbsTileX = AbsTileX;
    Result.AbsTileY = AbsTileY;
    Result.AbsTileZ = AbsTileZ;
    
    return Result;
}


#define HANDMADE_TILE_H
#endif