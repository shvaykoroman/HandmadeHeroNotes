#include "handmade.h"
#include "handmade_tile.cpp"
#include "handmade_random.h"

internal void
GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    s16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;
    
    s16 *SampleOut = SoundBuffer->Samples;
    for(int SampleIndex = 0;
        SampleIndex < SoundBuffer->SampleCount;
        ++SampleIndex)
    {
        // TODO(casey): Draw this out for people
#if 0
        f32 SineValue = sinf(GameState->tSine);
        s16 SampleValue = (s16)(SineValue * ToneVolume);
#else
        s16 SampleValue = 0;
#endif
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
        
#if 0
        GameState->tSine += 2.0f*Pi32*1.0f/(f32)WavePeriod;
        if(GameState->tSine > 2.0f*Pi32)
        {
            GameState->tSine -= 2.0f*Pi32;
        }
#endif
    }
}

internal void
DrawRectangle(game_offscreen_buffer *Buffer, v2 vMin, v2 vMax, f32 R, f32 G, f32 B)
{    
    s32 MinX = RoundReal32ToInt32(vMin.X);
    s32 MinY = RoundReal32ToInt32(vMin.Y);
    s32 MaxX = RoundReal32ToInt32(vMax.X);
    s32 MaxY = RoundReal32ToInt32(vMax.Y);
    
    if(MinX < 0)
    {
        MinX = 0;
    }
    
    if(MinY < 0)
    {
        MinY = 0;
    }
    
    if(MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }
    
    if(MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }
    
    u32 Color = ((RoundReal32ToUInt32(R * 255.0f) << 16) |
                 (RoundReal32ToUInt32(G * 255.0f) << 8) |
                 (RoundReal32ToUInt32(B * 255.0f) << 0));
    
    u8 *Row = ((u8 *)Buffer->Memory +
               MinX*Buffer->BytesPerPixel +
               MinY*Buffer->Pitch);
    for(int Y = MinY;
        Y < MaxY;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(int X = MinX;
            X < MaxX;
            ++X)
        {            
            *Pixel++ = Color;
        }
        
        Row += Buffer->Pitch;
    }
}

internal void
DrawBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *Bitmap,
           f32 RealX, f32 RealY,
           s32 AlignX = 0, s32 AlignY = 0)
{
    RealX -= (f32)AlignX;
    RealY -= (f32)AlignY;
    
    s32 MinX = RoundReal32ToInt32(RealX);
    s32 MinY = RoundReal32ToInt32(RealY);
    s32 MaxX = RoundReal32ToInt32(RealX + (f32)Bitmap->Width);
    s32 MaxY = RoundReal32ToInt32(RealY + (f32)Bitmap->Height);
    
    s32 SourceOffsetX = 0;
    if(MinX < 0)
    {
        SourceOffsetX = -MinX;
        MinX = 0;
    }
    
    s32 SourceOffsetY = 0;
    if(MinY < 0)
    {
        SourceOffsetY = -MinY;
        MinY = 0;
    }
    
    if(MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }
    
    if(MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }
    
    // TODO(casey): SourceRow needs to be changed based on clipping.
    u32 *SourceRow = Bitmap->Pixels + Bitmap->Width*(Bitmap->Height - 1);
    SourceRow += -SourceOffsetY*Bitmap->Width + SourceOffsetX;
    u8 *DestRow = ((u8 *)Buffer->Memory +
                   MinX*Buffer->BytesPerPixel +
                   MinY*Buffer->Pitch);
    for(int Y = MinY;
        Y < MaxY;
        ++Y)
    {
        u32 *Dest = (u32 *)DestRow;
        u32 *Source = SourceRow;
        for(int X = MinX;
            X < MaxX;
            ++X)
        {
            f32 A = (f32)((*Source >> 24) & 0xFF) / 255.0f;
            f32 SR = (f32)((*Source >> 16) & 0xFF);
            f32 SG = (f32)((*Source >> 8) & 0xFF);
            f32 SB = (f32)((*Source >> 0) & 0xFF);
            
            f32 DR = (f32)((*Dest >> 16) & 0xFF);
            f32 DG = (f32)((*Dest >> 8) & 0xFF);
            f32 DB = (f32)((*Dest >> 0) & 0xFF);
            
            // TODO(casey): Someday, we need to talk about premultiplied alpha!
            // (this is not premultiplied alpha)
            f32 R = (1.0f-A)*DR + A*SR;
            f32 G = (1.0f-A)*DG + A*SG;
            f32 B = (1.0f-A)*DB + A*SB;
            
            *Dest = (((u32)(R + 0.5f) << 16) |
                     ((u32)(G + 0.5f) << 8) |
                     ((u32)(B + 0.5f) << 0));
            
            ++Dest;
            ++Source;
        }
        
        DestRow += Buffer->Pitch;
        SourceRow -= Bitmap->Width;
    }
}

#pragma pack(push, 1)
struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    s32 Width;
    s32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    s32 HorzResolution;
    s32 VertResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
    
    u32 RedMask;
    u32 GreenMask;
    u32 BlueMask;
};
#pragma pack(pop)

internal loaded_bitmap
DEBUGLoadBMP(thread_context *Thread, debug_platform_read_entire_file *ReadEntireFile, char *FileName)
{
    loaded_bitmap Result = {};
    
    debug_read_file_result ReadResult = ReadEntireFile(Thread, FileName);    
    if(ReadResult.ContentsSize != 0)
    {
        bitmap_header *Header = (bitmap_header *)ReadResult.Contents;
        u32 *Pixels = (u32 *)((u8 *)ReadResult.Contents + Header->BitmapOffset);
        Result.Pixels = Pixels;
        Result.Width = Header->Width;
        Result.Height = Header->Height;
        
        Assert(Header->Compression == 3);
        
        // NOTE(casey): If you are using this generically for some reason,
        // please remember that BMP files CAN GO IN EITHER DIRECTION and
        // the height will be negative for top-down.
        // (Also, there can be compression, etc., etc... DON'T think this
        // is complete BMP loading code because it isn't!!)
        
        // NOTE(casey): Byte order in memory is determined by the Header itself,
        // so we have to read out the masks and convert the pixels ourselves.
        u32 RedMask = Header->RedMask;
        u32 GreenMask = Header->GreenMask;
        u32 BlueMask = Header->BlueMask;
        u32 AlphaMask = ~(RedMask | GreenMask | BlueMask);        
        
        bit_scan_result RedScan = FindLeastSignificantSetBit(RedMask);
        bit_scan_result GreenScan = FindLeastSignificantSetBit(GreenMask);
        bit_scan_result BlueScan = FindLeastSignificantSetBit(BlueMask);
        bit_scan_result AlphaScan = FindLeastSignificantSetBit(AlphaMask);
        
        Assert(RedScan.Found);
        Assert(GreenScan.Found);
        Assert(BlueScan.Found);
        Assert(AlphaScan.Found);
        
        s32 RedShift = 16 - (s32)RedScan.Index;
        s32 GreenShift = 8 - (s32)GreenScan.Index;
        s32 BlueShift = 0 - (s32)BlueScan.Index;
        s32 AlphaShift = 24 - (s32)AlphaScan.Index;
        
        u32 *SourceDest = Pixels;
        for(s32 Y = 0;
            Y < Header->Height;
            ++Y)
        {
            for(s32 X = 0;
                X < Header->Width;
                ++X)
            {
                u32 C = *SourceDest;
                
                *SourceDest++ = (RotateLeft(C & RedMask, RedShift) |
                                 RotateLeft(C & GreenMask, GreenShift) |
                                 RotateLeft(C & BlueMask, BlueShift) |
                                 RotateLeft(C & AlphaMask, AlphaShift));
            }
        }
    }
    
    return(Result);
}

inline entity *
GetEntity(game_state *GameState, u32 Index)
{
    entity *Entity = 0;
    
    if((Index > 0) && (Index < ArrayCount(GameState->Entities)))
    {
        Entity = &GameState->Entities[Index];
    }
    
    return(Entity);
}

internal void
InitializePlayer(game_state *GameState, u32 EntityIndex)
{
    entity *Entity = GetEntity(GameState, EntityIndex);
    
    Entity->Exists = true;
    Entity->P.AbsTileX = 1;
    Entity->P.AbsTileY = 3;
    Entity->P.Offset.X = 5.0f;
    Entity->P.Offset.Y = 5.0f;
    Entity->Height = 1.4f;
    Entity->Width = 0.75f*Entity->Height;
    
    if(!GetEntity(GameState, GameState->CameraFollowingEntityIndex))
    {
        GameState->CameraFollowingEntityIndex = EntityIndex;
    }
}

internal u32
AddEntity(game_state *GameState)
{
    u32 EntityIndex = GameState->EntityCount++;
    
    Assert(GameState->EntityCount < ArrayCount(GameState->Entities));
    entity *Entity = &GameState->Entities[EntityIndex];
    *Entity = {};
    
    return(EntityIndex);
}

internal void
TestWall(f32 WallX, f32 RelX,f32 RelY,f32 PlayerDeltaX, f32 PlayerDeltaY,
         f32 *tClosest, f32 MinY, f32 MaxY)
{
    f32 epsilon = 0.001f;
    if(PlayerDeltaX != 0.0f)
    {
        f32 tResult = (WallX-RelX) / PlayerDeltaX;
        f32 Y = RelY + tResult*PlayerDeltaY;
        
        if((tResult >= 0.0f) &&(*tClosest > tResult))
        {
            
            if((Y >= MinY) && (Y <= MaxY))
            {
                *tClosest = Max(0, tResult - epsilon);
            }
        }
    }
}

internal void
MovePlayer(game_state *GameState, entity *Entity, f32 dt, v2 ddP)
{
    tile_map *TileMap = GameState->World->TileMap;
    
    if((ddP.X != 0.0f) && (ddP.Y != 0.0f))
    {
        ddP *= 0.707106781187f;
    }
    
    f32 PlayerSpeed = 50.0f; // m/s^2
    ddP *= PlayerSpeed;
    
    // TODO(casey): ODE here!
    ddP += -8.0f*Entity->dP;
    
    tile_map_position OldPlayerP = Entity->P;
    tile_map_position NewPlayerP = OldPlayerP;
    v2 PlayerDelta = (0.5f*ddP*Square(dt) +
                      Entity->dP*dt);
    NewPlayerP.Offset += PlayerDelta;
    Entity->dP = ddP*dt + Entity->dP;
    NewPlayerP = RecanonicalizePosition(TileMap, NewPlayerP);
    // TODO(casey): Delta function that auto-recanonicalizes
    
#if 0
    tile_map_position PlayerLeft = NewPlayerP;
    PlayerLeft.Offset.X -= 0.5f*Entity->Width;
    PlayerLeft = RecanonicalizePosition(TileMap, PlayerLeft);
    
    tile_map_position PlayerRight = NewPlayerP;
    PlayerRight.Offset.X += 0.5f*Entity->Width;
    PlayerRight = RecanonicalizePosition(TileMap, PlayerRight);
    
    bool32 Collided = false;
    tile_map_position ColP = {};
    if(!IsTileMapPointEmpty(TileMap, NewPlayerP))
    {
        ColP = NewPlayerP;
        Collided = true;
    }
    if(!IsTileMapPointEmpty(TileMap, PlayerLeft))
    {
        ColP = PlayerLeft;
        Collided = true;
    }
    if(!IsTileMapPointEmpty(TileMap, PlayerRight))
    {
        ColP = PlayerRight;
        Collided = true;
    }
    
    if(Collided)
    {
        v2 r = {0,0};
        if(ColP.AbsTileX < Entity->P.AbsTileX)
        {
            r = v2{1, 0};
        }
        if(ColP.AbsTileX > Entity->P.AbsTileX)
        {
            r = v2{-1, 0};
        }
        if(ColP.AbsTileY < Entity->P.AbsTileY)
        {
            r = v2{0, 1};
        }
        if(ColP.AbsTileY > Entity->P.AbsTileY)
        {
            r = v2{0, -1};
        }
        
        Entity->dP = Entity->dP - 1*Inner(Entity->dP, r)*r;
    }
    else
    {
        Entity->P = NewPlayerP;
    }
#else
    u32 MinTileX = Min(OldPlayerP.AbsTileX,NewPlayerP.AbsTileX);
    u32 MinTileY = Min( OldPlayerP.AbsTileY,NewPlayerP.AbsTileY);
    u32 OnePastMaxTileX = Max(OldPlayerP.AbsTileX,NewPlayerP.AbsTileX) + 1;
    u32 OnePastMaxTileY = Max(OldPlayerP.AbsTileY,NewPlayerP.AbsTileY) + 1;
    u32 AbsTileZ = Entity->P.AbsTileZ;
    // NOTE(shvayko): 1.0f full path
    f32 tClosest = 1.0f;
    for(u32 AbsTileY = MinTileY;
        AbsTileY != OnePastMaxTileY;
        ++AbsTileY)
    {
        for(u32 AbsTileX = MinTileX;
            AbsTileX != OnePastMaxTileX;
            ++AbsTileX)
        {
            tile_map_position TestTileP = GetCenteredTileMapPosition(AbsTileX, AbsTileY, AbsTileZ);
            u32 TileValue = GetTileValue(TileMap, TestTileP);
            if(!IsTileValueEmpty(TileValue))
            {
                v2 MinCorner = -0.5f*v2{TileMap->TileSideInMeters, TileMap->TileSideInMeters};
                v2 MaxCorner = 0.5f*v2{TileMap->TileSideInMeters, TileMap->TileSideInMeters};
                
                tile_map_difference RelOldPlayerP = Subtract(TileMap, &OldPlayerP, &TestTileP);
                
                v2 Rel = RelOldPlayerP.dXY;
                TestWall(MinCorner.X, Rel.X, Rel.Y, PlayerDelta.X,  PlayerDelta.Y,
                         &tClosest, MinCorner.Y, MaxCorner.Y);
                TestWall(MaxCorner.X, Rel.X, Rel.Y, PlayerDelta.X,  PlayerDelta.Y,
                         &tClosest, MinCorner.Y, MaxCorner.Y);
                TestWall(MinCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y,  PlayerDelta.X,
                         &tClosest, MinCorner.X, MaxCorner.X);
                TestWall(MaxCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y,  PlayerDelta.X,
                         &tClosest, MinCorner.X, MaxCorner.X);
            }
        }
    }
    
    NewPlayerP = OldPlayerP;
    NewPlayerP.Offset += tClosest*PlayerDelta;
    Entity->P = NewPlayerP;
    
    
#endif
    
    //
    // NOTE(casey): Update camera/player Z based on last movement.
    //
    if(!AreOnSameTile(&OldPlayerP, &Entity->P))
    {
        u32 NewTileValue = GetTileValue(TileMap, Entity->P);
        
        if(NewTileValue == 3)
        {
            ++Entity->P.AbsTileZ;
        }
        else if(NewTileValue == 4)
        {
            --Entity->P.AbsTileZ;
        }    
    }
    
    if((Entity->dP.X == 0.0f) && (Entity->dP.Y == 0.0f))
    {
        // NOTE(casey): Leave FacingDirection whatever it was
    }
    else if(AbsoluteValue(Entity->dP.X) > AbsoluteValue(Entity->dP.Y))
    {
        if(Entity->dP.X > 0)
        {
            Entity->FacingDirection = 0;
        }
        else
        {
            Entity->FacingDirection = 2;
        }
    }
    else
    {
        if(Entity->dP.Y > 0)
        {
            Entity->FacingDirection = 1;
        }
        else
        {
            Entity->FacingDirection = 3;
        }
    }
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if(!Memory->IsInitialized)
    {
        // NOTE(casey): Reserve entity slot 0 for the null entity
        AddEntity(GameState);
        
        GameState->Backdrop =
            DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_background.bmp");
        
        hero_bitmaps *Bitmap;
        
        Bitmap = GameState->HeroBitmaps;
        Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_right_head.bmp");
        Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_right_cape.bmp");
        Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_right_torso.bmp");
        Bitmap->AlignX = 72;
        Bitmap->AlignY = 182;
        ++Bitmap;
        
        Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_back_head.bmp");
        Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_back_cape.bmp");
        Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_back_torso.bmp");
        Bitmap->AlignX = 72;
        Bitmap->AlignY = 182;
        ++Bitmap;
        
        Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_left_head.bmp");
        Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_left_cape.bmp");
        Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_left_torso.bmp");
        Bitmap->AlignX = 72;
        Bitmap->AlignY = 182;
        ++Bitmap;
        
        Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_front_head.bmp");
        Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_front_cape.bmp");
        Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test/test_hero_front_torso.bmp");
        Bitmap->AlignX = 72;
        Bitmap->AlignY = 182;
        ++Bitmap;
        
        GameState->CameraP.AbsTileX = 17/2;
        GameState->CameraP.AbsTileY = 9/2;
        
        InitializeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state),
                        (u8 *)Memory->PermanentStorage + sizeof(game_state));
        
        GameState->World = PushStruct(&GameState->WorldArena, world);
        world *World = GameState->World;
        World->TileMap = PushStruct(&GameState->WorldArena, tile_map);
        
        tile_map *TileMap = World->TileMap;
        
        TileMap->ChunkShift = 4;
        TileMap->ChunkMask = (1 << TileMap->ChunkShift) - 1;
        TileMap->ChunkDim = (1 << TileMap->ChunkShift);
        
        TileMap->TileChunkCountX = 128;
        TileMap->TileChunkCountY = 128;
        TileMap->TileChunkCountZ = 2;
        TileMap->TileChunks = PushArray(&GameState->WorldArena,
                                        TileMap->TileChunkCountX*
                                        TileMap->TileChunkCountY*
                                        TileMap->TileChunkCountZ,
                                        tile_chunk);
        
        TileMap->TileSideInMeters = 1.4f;
        
        u32 RandomNumberIndex = 0;
        u32 TilesPerWidth = 17;
        u32 TilesPerHeight = 9;
        u32 ScreenX = 0;
        u32 ScreenY = 0;
        u32 AbsTileZ = 0;
        
        // TODO(casey): Replace all this with real world generation!
        bool32 DoorLeft = false;
        bool32 DoorRight = false;
        bool32 DoorTop = false;
        bool32 DoorBottom = false;
        bool32 DoorUp = false;
        bool32 DoorDown = false;
        for(u32 ScreenIndex = 0;
            ScreenIndex < 100;
            ++ScreenIndex)
        {
            // TODO(casey): Random number generator!
            Assert(RandomNumberIndex < ArrayCount(RandomNumberTable));
            
            u32 RandomChoice;
            if(DoorUp || DoorDown)
            {
                RandomChoice = RandomNumberTable[RandomNumberIndex++] % 2;
            }
            else
            {
                RandomChoice = RandomNumberTable[RandomNumberIndex++] % 3;
            }
            
            bool32 CreatedZDoor = false;
            if(RandomChoice == 2)
            {
                CreatedZDoor = true;
                if(AbsTileZ == 0)
                {
                    DoorUp = true;
                }
                else
                {
                    DoorDown = true;
                }
            }
            else if(RandomChoice == 1)
            {
                DoorRight = true;
            }
            else
            {
                DoorTop = true;
            }
            
            for(u32 TileY = 0;
                TileY < TilesPerHeight;
                ++TileY)
            {
                for(u32 TileX = 0;
                    TileX < TilesPerWidth;
                    ++TileX)
                {
                    u32 AbsTileX = ScreenX*TilesPerWidth + TileX;
                    u32 AbsTileY = ScreenY*TilesPerHeight + TileY;
                    
                    u32 TileValue = 1;
                    if((TileX == 0) && (!DoorLeft || (TileY != (TilesPerHeight/2))))
                    {
                        TileValue = 2;
                    }
                    
                    if((TileX == (TilesPerWidth - 1)) && (!DoorRight || (TileY != (TilesPerHeight/2))))
                    {
                        TileValue = 2;
                    }
                    
                    if((TileY == 0) && (!DoorBottom || (TileX != (TilesPerWidth/2))))
                    {
                        TileValue = 2;
                    }
                    
                    if((TileY == (TilesPerHeight - 1)) && (!DoorTop || (TileX != (TilesPerWidth/2))))
                    {
                        TileValue = 2;
                    }
                    
                    if((TileX == 10) && (TileY == 6))
                    {
                        if(DoorUp)
                        {
                            TileValue = 3;
                        }
                        
                        if(DoorDown)
                        {
                            TileValue = 4;
                        }
                    }
                    
                    SetTileValue(&GameState->WorldArena, World->TileMap, AbsTileX, AbsTileY, AbsTileZ,
                                 TileValue);
                }
            }
            
            DoorLeft = DoorRight;
            DoorBottom = DoorTop;
            
            if(CreatedZDoor)
            {
                DoorDown = !DoorDown;
                DoorUp = !DoorUp;
            }
            else
            {
                DoorUp = false;
                DoorDown = false;
            }
            
            DoorRight = false;
            DoorTop = false;
            
            if(RandomChoice == 2)
            {
                if(AbsTileZ == 0)
                {
                    AbsTileZ = 1;
                }
                else
                {
                    AbsTileZ = 0;
                }                
            }
            else if(RandomChoice == 1)
            {
                ScreenX += 1;
            }
            else
            {
                ScreenY += 1;
            }
        }
        
        Memory->IsInitialized = true;
    }
    
    world *World = GameState->World;        
    tile_map *TileMap = World->TileMap;
    
    s32 TileSideInPixels = 60;
    f32 MetersToPixels = (f32)TileSideInPixels / (f32)TileMap->TileSideInMeters;
    
    f32 LowerLeftX = -(f32)TileSideInPixels/2;
    f32 LowerLeftY = (f32)Buffer->Height;
    
    //
    // NOTE(casey): 
    //
    for(int ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        entity *ControllingEntity = GetEntity(GameState,
                                              GameState->PlayerIndexForController[ControllerIndex]);
        if(ControllingEntity)
        {
            v2 ddP = {};
            
            if(Controller->IsAnalog)
            {
                // NOTE(casey): Use analog movement tuning
                ddP = v2{Controller->StickAverageX, Controller->StickAverageY};
            }
            else
            {
                // NOTE(casey): Use digital movement tuning
                if(Controller->MoveUp.EndedDown)
                {
                    ddP.Y = 1.0f;
                }
                if(Controller->MoveDown.EndedDown)
                {
                    ddP.Y = -1.0f;
                }
                if(Controller->MoveLeft.EndedDown)
                {
                    ddP.X = -1.0f;
                }
                if(Controller->MoveRight.EndedDown)
                {
                    ddP.X = 1.0f;
                }
            }
            
            MovePlayer(GameState, ControllingEntity, Input->dtForFrame, ddP);
        }
        else
        {
            if(Controller->Start.EndedDown)
            {
                u32 EntityIndex = AddEntity(GameState);
                InitializePlayer(GameState, EntityIndex);
                GameState->PlayerIndexForController[ControllerIndex] = EntityIndex;
            }
        }
    }
    
    entity *CameraFollowingEntity = GetEntity(GameState, GameState->CameraFollowingEntityIndex);
    if(CameraFollowingEntity)
    {
        GameState->CameraP.AbsTileZ = CameraFollowingEntity->P.AbsTileZ;
        
        tile_map_difference Diff = Subtract(TileMap, &CameraFollowingEntity->P, &GameState->CameraP);
        if(Diff.dXY.X > (9.0f*TileMap->TileSideInMeters))
        {
            GameState->CameraP.AbsTileX += 17;
        }
        if(Diff.dXY.X < -(9.0f*TileMap->TileSideInMeters))
        {
            GameState->CameraP.AbsTileX -= 17;
        }
        if(Diff.dXY.Y > (5.0f*TileMap->TileSideInMeters))
        {
            GameState->CameraP.AbsTileY += 9;
        }
        if(Diff.dXY.Y < -(5.0f*TileMap->TileSideInMeters))
        {
            GameState->CameraP.AbsTileY -= 9;
        }
    }
    
    //
    // NOTE(casey): Render
    //
    DrawBitmap(Buffer, &GameState->Backdrop, 0, 0);
    
    f32 ScreenCenterX = 0.5f*(f32)Buffer->Width;
    f32 ScreenCenterY = 0.5f*(f32)Buffer->Height;
    
    for(s32 RelRow = -10;
        RelRow < 10;
        ++RelRow)
    {
        for(s32 RelColumn = -20;
            RelColumn < 20;
            ++RelColumn)
        {
            u32 Column = GameState->CameraP.AbsTileX + RelColumn;
            u32 Row = GameState->CameraP.AbsTileY + RelRow;
            u32 TileID = GetTileValue(TileMap, Column, Row, GameState->CameraP.AbsTileZ);
            
            if(TileID > 1)
            {
                f32 Gray = 0.5f;
                if(TileID == 2)
                {
                    Gray = 1.0f;
                }
                
                if(TileID > 2)
                {
                    Gray = 0.25f;
                }
                
                if((Column == GameState->CameraP.AbsTileX) &&
                   (Row == GameState->CameraP.AbsTileY))
                {
                    Gray = 0.0f;
                }
                
                v2 TileSide = {0.5f*TileSideInPixels, 0.5f*TileSideInPixels};
                v2 Cen = {ScreenCenterX - MetersToPixels*GameState->CameraP.Offset.X + ((f32)RelColumn)*TileSideInPixels,
                    ScreenCenterY + MetersToPixels*GameState->CameraP.Offset.Y - ((f32)RelRow)*TileSideInPixels};
                v2 Min = Cen - TileSide;
                v2 Max = Cen + TileSide;
                DrawRectangle(Buffer, Min, Max, Gray, Gray, Gray);
            }
        }
    }
    
    entity *Entity = GameState->Entities;
    for(u32 EntityIndex = 0;
        EntityIndex < GameState->EntityCount;
        ++EntityIndex, ++Entity)
    {
        // TODO(casey): Culling of entities based on Z / camera view
        if(Entity->Exists)
        {
            tile_map_difference Diff = Subtract(TileMap, &Entity->P, &GameState->CameraP);    
            
            f32 PlayerR = 1.0f;
            f32 PlayerG = 1.0f;
            f32 PlayerB = 0.0f;
            f32 PlayerGroundPointX = ScreenCenterX + MetersToPixels*Diff.dXY.X;
            f32 PlayerGroundPointY = ScreenCenterY - MetersToPixels*Diff.dXY.Y; 
            v2 PlayerLeftTop = {PlayerGroundPointX - 0.5f*MetersToPixels*Entity->Width,
                PlayerGroundPointY - MetersToPixels*Entity->Height};
            v2 EntityWidthHeight = {Entity->Width, Entity->Height};
            DrawRectangle(Buffer,
                          PlayerLeftTop,
                          PlayerLeftTop + MetersToPixels*EntityWidthHeight,
                          PlayerR, PlayerG, PlayerB);
            
            hero_bitmaps *HeroBitmaps = &GameState->HeroBitmaps[Entity->FacingDirection];
            DrawBitmap(Buffer, &HeroBitmaps->Torso, PlayerGroundPointX, PlayerGroundPointY, HeroBitmaps->AlignX, HeroBitmaps->AlignY);
            DrawBitmap(Buffer, &HeroBitmaps->Cape, PlayerGroundPointX, PlayerGroundPointY, HeroBitmaps->AlignX, HeroBitmaps->AlignY);
            DrawBitmap(Buffer, &HeroBitmaps->Head, PlayerGroundPointX, PlayerGroundPointY, HeroBitmaps->AlignX, HeroBitmaps->AlignY);
        }
    }
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    GameOutputSound(GameState, SoundBuffer, 400);
}

/*
internal void
RenderWeirdGradient(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
    // TODO(casey): Let's see what the optimizer does

    u8 *Row = (u8 *)Buffer->Memory;    
    for(int Y = 0;
        Y < Buffer->Height;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(int X = 0;
            X < Buffer->Width;
            ++X)
        {
            u8 Blue = (u8)(X + BlueOffset);
            u8 Green = (u8)(Y + GreenOffset);

            *Pixel++ = ((Green << 16) | Blue);
        }
        
        Row += Buffer->Pitch;
    }
}
*/
