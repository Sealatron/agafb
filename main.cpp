#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
 * Platform Stuff
 */

//Constants
enum GlobalConstants{
    //Application Constants
    SCREEN_WIDTH    = 640,
    SCREEN_HEIGHT   = 480,
    FRAMERATE = 30, //Hz

    //Main Grid Dimensions
    GRID_WIDTH  = SCREEN_WIDTH/3,
    GRID_HEIGHT = SCREEN_HEIGHT,
    GRID_ROWS   = 20,
    GRID_COLS   = 10,
    GRID_X      = SCREEN_WIDTH/3,
    GRID_Y      = 0,

    //Preview Grid Dimensions
    PREVIEW_WIDTH   = SCREEN_WIDTH/3,
    PREVIEW_HEIGHT  = SCREEN_HEIGHT/3,
    PREVIEW_ROWS    = 4,
    PREVIEW_COLS    = 4,
    PREVIEW_X       = SCREEN_WIDTH*2/3,
    PREVIEW_Y       = 0,

    //Cell Dimensions
    SCREEN_ROWS       = 20,
    SCREEN_COLS       = 30,
    CELL_PADDING = 0,
    CELL_WIDTH  = (SCREEN_WIDTH - SCREEN_COLS*CELL_PADDING)/SCREEN_COLS,
    CELL_HEIGHT = (SCREEN_HEIGHT - SCREEN_ROWS*CELL_PADDING)/SCREEN_ROWS,

    //Game Constants
    FALL_FRAMES = 30
};

//A wrapper for the SDL textures
struct Texture{
    SDL_Texture* Data;
    unsigned int Width;
    unsigned int Height;
};

struct TextureArray{
    bool Initialised;
    Texture* Textures;
    unsigned int Length;
};

struct InputState{
    bool Up;
    bool Down;
    bool Left;
    bool Right;
    bool Space;
    bool Escape;
};

enum Alignment{
    LEFT,
    RIGHT,
    CENTRE
};

struct Rect{
    float X;
    float Y;
    float Width;
    float Height;
};

bool init();
bool loadMedia();
TextureArray LoadFontTextures();
void close();
void DrawRect(int X, int Y, int Width, int Height, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha);
bool loadFromRenderedText(const char* String, SDL_Color TextColor, Texture* Result);
bool LoadGlyphToTexture(char Glyph, SDL_Color TextColor, Texture* Result);

void DrawText(const char* Text, float X, float Y);
void DrawTextToRect(const char* Text, Rect Box, Alignment Align);

Texture GenerateTexture();
void DestroyTexture(Texture TextureToKill);
TextureArray GenerateTextureArray(unsigned int Length);
void DestroyTextureArray(TextureArray ArrayToKill);
void DrawTexture(Texture T, unsigned int X, unsigned int Y, unsigned int Width, unsigned int Height);

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;
TextureArray Glyphs;

/*
 * Game Stuff
 */

enum GameState{
    INITIALISING,
    RUNNING,
    PAUSED,
    GAMEOVER
};

enum TetrominoType{
    I_SHAPE,
    T_SHAPE,
    O_SHAPE,
    Z_SHAPE,
    S_SHAPE,
    L_L_SHAPE,
    L_R_SHAPE,
};

struct Vector2D{
    float X;
    float Y;
};

struct Block{
    unsigned char Occupied;
    unsigned char Red;
    unsigned char Green;
    unsigned char Blue;
    unsigned char Alpha;
};

struct BlockGrid{
    unsigned int Rows;
    unsigned int Cols;
    Block* Blocks;
};

struct Tetromino{
    TetrominoType Type;
    unsigned int GridSize;
    int Row;
    int Col;
    BlockGrid Grid;
};

struct GameData{
    GameState State;

    bool Quit;
    bool MoveLeft;
    bool MoveRight;
    bool Rotate;
    bool MoveDown;
    bool Pause;
    bool Redraw;
    bool RenderScore;
    bool Restart;

    unsigned int Score;
    unsigned int FallingTimer;
    unsigned int StartTick;
    unsigned int EndTick;

    Tetromino FallingTetro;
    Tetromino NextTetro;
    BlockGrid MainGrid;
    Texture ScoreTexture;
};

GameData HandleInputGame(InputState Inputs, GameData Current);
GameData HandleInputPaused(InputState Inputs, GameData Current);
GameData HandleInputGameOver(InputState Inputs, GameData Current);

GameData UpdateGame(GameData Current);
GameData UpdatePaused(GameData Current);
GameData UpdateGameOver(GameData Current);

GameData DrawGame(GameData Current);

Block   GenerateBlock();
Block*  GetBlock(BlockGrid Grid, unsigned int Row, unsigned int Col);
void    EraseBlock(Block* BlockToErase);

BlockGrid   GenerateGrid(unsigned int Rows, unsigned int Cols);
void        DestroyGrid(BlockGrid Grid);

Tetromino       GenerateTetromino();
void            DestroyTetromino(Tetromino Tetro);
void            StoreTetromino(BlockGrid Grid, Tetromino Tetro);
Tetromino       RotateTetroClockwise(Tetromino Tetro);
Tetromino       RotateTetroAntiClockwise(Tetromino Tetro);

unsigned int    CheckCollisions(BlockGrid Grid, Tetromino Tetro);
unsigned int    RemoveGridLines(BlockGrid Grid);
Vector2D CalculateGridCentreOfMass(BlockGrid Grid);

void DrawGrid(BlockGrid Grid, unsigned int X, unsigned int Y, unsigned Width, unsigned Height);

int main( int argc, char* args[] )
{
    //Start up SDL and create window
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        if( !loadMedia() )
        {
            printf( "Failed to load media!\n" );
        }
        else
        {
            //Main loop flag
            GameData CurrentGameData;

            CurrentGameData.Quit = false;
            CurrentGameData.State = INITIALISING;
            CurrentGameData.StartTick = 0;
            CurrentGameData.EndTick = 0;

            //Input Struct
            InputState Inputs;

            Inputs.Up = false;
            Inputs.Down = false;
            Inputs.Left = false;
            Inputs.Right = false;
            Inputs.Space = false;

            //Event handler
            SDL_Event e;

            unsigned int FrameDuration = 0;

            float CellWidth = (float)(SCREEN_WIDTH - SCREEN_COLS*CELL_PADDING)/SCREEN_COLS;
            float CellHeight = (float)(SCREEN_HEIGHT - SCREEN_ROWS*CELL_PADDING)/SCREEN_ROWS;

            //While application is running
            while( !CurrentGameData.Quit )
            {
                CurrentGameData.StartTick = SDL_GetTicks();

                //Handle events on queue
                while( SDL_PollEvent( &e ) != 0 )
                {
                    //User requests quit
                    if( e.type == SDL_QUIT )
                    {
                        CurrentGameData.Quit = true;
                    }
                    else if (e.type == SDL_KEYDOWN)
                    {
                        switch( e.key.keysym.sym )
                        {
                            case SDLK_UP:
                                Inputs.Up = true;
                                break;
                            case SDLK_DOWN:
                                Inputs.Down = true;
                                break;
                            case SDLK_LEFT:
                                Inputs.Left = true;
                                break;
                            case SDLK_RIGHT:
                                Inputs.Right = true;
                                break;
                            case SDLK_SPACE:
                                Inputs.Space = true;
                                break;
                            case SDLK_ESCAPE:
                                Inputs.Escape = true;
                                break;
                            default:
                                break;
                        }
                    }
                }

                switch(CurrentGameData.State)
                {
                    case INITIALISING:
                        {
                            //Initialise all game parameters
                            CurrentGameData.FallingTimer = FALL_FRAMES;
                            CurrentGameData.MoveLeft = 0;
                            CurrentGameData.MoveRight = 0;
                            CurrentGameData.MoveDown = 0;
                            CurrentGameData.Rotate = 0;
                            CurrentGameData.Score = 0;
                            CurrentGameData.Redraw = 1;
                            CurrentGameData.RenderScore = 1;

                            //Create Game Grid
                            //Create first Tetro and next Tetro
                            CurrentGameData.MainGrid      = GenerateGrid(GRID_ROWS, GRID_COLS);
                            CurrentGameData.FallingTetro = GenerateTetromino();
                            CurrentGameData.NextTetro = GenerateTetromino();
                            CurrentGameData.NextTetro.Col = 1;
                            CurrentGameData.NextTetro.Row = 1;

                            //Transition to Running
                            CurrentGameData.State = RUNNING;
                        }
                        break;
                    case RUNNING:
                        {
                            if(CurrentGameData.Redraw)
                            {
                                CurrentGameData = DrawGame(CurrentGameData);
                                CurrentGameData.Redraw = 0;
                            }

                            CurrentGameData = HandleInputGame(Inputs, CurrentGameData);
                            CurrentGameData = UpdateGame(CurrentGameData);
                        }
                        break;
                    case PAUSED:
                        {
                            if(CurrentGameData.Redraw)
                            {
                                CurrentGameData = DrawGame(CurrentGameData);
                                DrawRect(GRID_X,GRID_Y,GRID_WIDTH,GRID_HEIGHT,0,0,0,128);
                                Rect TextBox = {GRID_X, GRID_Y, GRID_WIDTH, GRID_HEIGHT};
                                DrawTextToRect("Paused!", TextBox, CENTRE);
                                SDL_RenderPresent( gRenderer );
                                CurrentGameData.Redraw = 0;
                            }

                            CurrentGameData = HandleInputPaused(Inputs, CurrentGameData);
                            CurrentGameData = UpdatePaused(CurrentGameData);

                        }
                        break;
                     case GAMEOVER:
                        {
                            if(CurrentGameData.Redraw)
                            {
                                CurrentGameData = DrawGame(CurrentGameData);
                                DrawRect(GRID_X,GRID_Y,GRID_WIDTH,GRID_HEIGHT,0,0,0,128);
                                Rect TextBox= {GRID_X, GRID_Y, GRID_WIDTH, GRID_HEIGHT};
                                DrawTextToRect("Game Over!\nPress [Esc] to Quit or [Space] to Try again!", TextBox, CENTRE);
                                SDL_RenderPresent( gRenderer );
                                CurrentGameData.Redraw = 0;
                            }

                            CurrentGameData = HandleInputGameOver(Inputs, CurrentGameData);
                            CurrentGameData = UpdateGameOver(CurrentGameData);
                        }
                }
                
                //Reset Inputs
                Inputs.Up = false;
                Inputs.Down = false;
                Inputs.Left = false;
                Inputs.Right = false;
                Inputs.Space = false;
                Inputs.Escape = false;

                //Delay till the end of the maximum frame duration (1/FRAMERATE seconds)
                CurrentGameData.EndTick = SDL_GetTicks();
                FrameDuration = CurrentGameData.EndTick - CurrentGameData.StartTick;

                SDL_Delay((1000/FRAMERATE)-FrameDuration);
            }

            //Clean up game
            DestroyTetromino(CurrentGameData.FallingTetro);
            DestroyGrid(CurrentGameData.MainGrid);

            //Free textures
            DestroyTextureArray(Glyphs);
        }
    }

    //Free resources and close SDL
    close();

    return 0;
}

/*
 * Platform Operations
 */

bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        srand(time(NULL));

        //Create window
        gWindow = SDL_CreateWindow( "A Game About Falling Blocks", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Create renderer for window
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
            if( gRenderer == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                //Initialize renderer color
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

                if(TTF_Init() == -1)
                {
                    success = false;
                }
            }
        }
    }

    return success;
}

bool loadMedia()
{
    bool success = true;

    gFont = TTF_OpenFont("/usr/share/fonts/TTF/FiraMono-Regular.ttf", 18);

    if(gFont == NULL)
    {
        success = false;
    }

    Glyphs = LoadFontTextures();

    if(!Glyphs.Initialised)
    {
        success = false;
    }

    return success;
}

TextureArray LoadFontTextures()
{
    unsigned int StartGlyph = 0x20;
    unsigned int EndGlyph = 0x7E;
    unsigned int GlyphRange = EndGlyph - StartGlyph;


    TextureArray Result = GenerateTextureArray(GlyphRange+1);
    Result.Initialised = true;
    SDL_Color TextColor = {255,255,255};


    //Load ASCII A-Z
    for(unsigned int Char = StartGlyph; Char <= EndGlyph; ++Char)
    {
        unsigned int Index = Char - StartGlyph;
        Result.Textures[Index] = GenerateTexture();

        bool result = LoadGlyphToTexture((char)Char,TextColor,&Result.Textures[Index]);

        if(!result)
        {
            printf( "Failed to load texture for '%c'!\n", Char );
            Result.Initialised = false;
        }
    }

    return Result;
}

Texture GetGlyph(char Character)
{
    Texture Result;

    if((Character >= 0x20) || (Character <= 0x7E))
    {
        Result = Glyphs.Textures[Character - 0x20];
    }
    else
    {
        Result = Glyphs.Textures['X' - 0x20];
    }
    return Result;
}

void close()
{
    //Free global text
    TTF_CloseFont(gFont);
    gFont=NULL;

    //Destroy window	
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;
    gRenderer = NULL;

    //Quit SDL subsystems
    SDL_Quit();
    TTF_Quit();
}

void DrawRect(int X, int Y, int Width, int Height, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha)
{
    SDL_SetRenderDrawColor( gRenderer, Red, Green, Blue, Alpha);
    SDL_Rect Rect = {X, Y, Width, Height};
    SDL_SetRenderDrawBlendMode( gRenderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect( gRenderer, &Rect );
}

bool LoadGlyphToTexture(char Glyph, SDL_Color TextColor, Texture* Result)
{
    if(!Result)
    {
        printf("Null pointer!\n");
        return false;
    }

    SDL_Surface* textSurface = TTF_RenderGlyph_Blended( gFont, Glyph, TextColor);

    if(!textSurface)
    {
        printf("Unable to render text surface!\n");
    }
    else
    {
        Result->Data = SDL_CreateTextureFromSurface(gRenderer, textSurface);

        if(Result->Data == NULL)
        {
            printf("Unable to create texture from rendered surface!\n");
        }
        else
        {
            Result->Width = textSurface->w;
            Result->Height = textSurface->h;
        }        

        SDL_FreeSurface( textSurface);
    }

    return Result->Data != NULL;
}

bool loadFromRenderedText(const char* String, SDL_Color TextColor, Texture* Result)
{
    if(!Result)
    {
        printf("Null pointer!\n");
        return false;
    }

    SDL_Surface* textSurface = TTF_RenderText_Blended( gFont, String, TextColor);

    if(!textSurface)
    {
        printf("Unable to render text surface!\n");
    }
    else
    {
        Result->Data = SDL_CreateTextureFromSurface(gRenderer, textSurface);

        if(Result->Data == NULL)
        {
            printf("Unable to create texture from rendered surface!\n");
        }
        else
        {
            Result->Width = textSurface->w;
            Result->Height = textSurface->h;
        }        

        SDL_FreeSurface( textSurface);
    }

    return Result->Data != NULL;
}

Texture GenerateTexture()
{
    Texture Result;

    Result.Data = NULL;
    Result.Width = 0;
    Result.Height = 0;

    return Result;
}
void DestroyTexture(Texture TextureToKill)
{
    //free(TextureToKill.Data);
    SDL_DestroyTexture(TextureToKill.Data);
}

TextureArray GenerateTextureArray(unsigned int Length)
{
    TextureArray Result;

    Result.Length = Length;
    Result.Initialised = false;

    Result.Textures = (Texture*)(malloc(sizeof(Texture)*Length));

    unsigned int Count = 0;
    Texture* CurrentTexture = Result.Textures;

    while(Count < Result.Length)
    {
        *CurrentTexture = GenerateTexture();

        ++Count;
        ++CurrentTexture;
    }

    return Result;
}

void DestroyTextureArray(TextureArray ArrayToKill)
{
    for(unsigned int Index = 0; Index < ArrayToKill.Length; ++Index)
    {
        DestroyTexture(ArrayToKill.Textures[Index]);
    }

    free(ArrayToKill.Textures);
}

GameData HandleInputGame(InputState Inputs, GameData Current)
{
    GameData Result = Current;

    Result.MoveLeft = Inputs.Left;
    Result.MoveRight = Inputs.Right;
    Result.MoveDown = Inputs.Down;
    Result.Rotate = Inputs.Up;
    Result.Pause = Inputs.Space;

    return Result;
}

GameData HandleInputPaused(InputState Inputs, GameData Current)
{
    GameData Result = Current;

    Result.Pause = Inputs.Space;

    return Result;
}

GameData HandleInputGameOver(InputState Inputs, GameData Current)
{
    GameData Result = Current;

    Result.Restart = Inputs.Space;
    Result.Quit = Inputs.Escape;

    return Result;
}

GameData UpdateGame(GameData Current)
{
    GameData Result = Current;

    //Drop Tetro if necessary
    if(Result.FallingTimer == 0)
    {
        Result.MoveDown = 1;
        Result.FallingTimer = FALL_FRAMES;
    }
    else
    {
        Result.FallingTimer--;
    }

    //Move Tetro if necessary
    if(Result.MoveLeft)
    {
        //Duplicate Tetromino
        Tetromino NewTetro = Result.FallingTetro;

        NewTetro.Col--;

        if(!CheckCollisions(Result.MainGrid, NewTetro))
        {
            Result.FallingTetro = NewTetro;
            Result.Redraw = 1;
        }

        Result.MoveLeft = 0;
    }

    if(Result.MoveRight)
    {
        //Duplicate Tetromino
        Tetromino NewTetro = Result.FallingTetro;

        NewTetro.Col++;

        if(!CheckCollisions(Result.MainGrid, NewTetro))
        {
            Result.FallingTetro = NewTetro;
            Result.Redraw = 1;
        }

        Result.MoveRight = 0;
    }

    if(Result.MoveDown)
    {
        //Duplicate Tetromino
        Tetromino NewTetro = Result.FallingTetro;

        NewTetro.Row++;
        //Check for collisions
        if(CheckCollisions(Result.MainGrid, NewTetro))
        {
            StoreTetromino(Result.MainGrid, Result.FallingTetro);
            DestroyTetromino(Result.FallingTetro);
            Result.FallingTetro = Result.NextTetro;

            Result.NextTetro    = GenerateTetromino();
            Result.NextTetro.Col = 1;
            Result.NextTetro.Row = 1;


            Result.FallingTimer = FALL_FRAMES;
        }
        else
        {
            Result.FallingTetro = NewTetro;
        }

        Result.Redraw = 1;
        Result.MoveDown = 0;
    }

    if(Result.Rotate)
    {
        if(Result.FallingTetro.Type != O_SHAPE)
        {
            //Duplicate Tetromino
            Tetromino NewTetro = RotateTetroClockwise(Result.FallingTetro);

            if(!CheckCollisions(Result.MainGrid, NewTetro))
            {
                DestroyTetromino(Result.FallingTetro);
                Result.FallingTetro = NewTetro;
                Result.Redraw = 1;
            }
            else
            {
                DestroyTetromino(NewTetro);
            }

        }
        Result.Rotate = 0;
    }

    // Final check for collisons - quit game if any are found
    if(CheckCollisions(Result.MainGrid, Result.FallingTetro))
    {
        Result.State = GAMEOVER;
        Result.Redraw = 1;
    }

    //Remove any lines in the grid
    unsigned int LinesRemoved = RemoveGridLines(Result.MainGrid);

    if(LinesRemoved)
    {
        Result.RenderScore = 1;
    }

    switch(LinesRemoved)
    {
        case 1:
            Result.Score+=1;
            break;
        case 2:
            Result.Score+=4;
            break;
        case 3:
            Result.Score+=8;
            break;
        case 4:
            Result.Score+=16;
            break;
        default:
            break;
    }

    if(Result.Pause)
    {
        Result.State = PAUSED;
        Result.Pause = 0;
        Result.Redraw = 1;
    }

    return Result;
}

GameData UpdatePaused(GameData Current)
{
    GameData Result = Current;

    if(Result.Pause)
    {
        Result.State = RUNNING;
        Result.Pause = 0;
        Result.Redraw = 0;
    }

    return Result;
}

GameData UpdateGameOver(GameData Current)
{
    GameData Result = Current;

    if(Result.Restart)
    {
        //Clean up game
        DestroyTetromino(Result.FallingTetro);
        DestroyGrid(Result.MainGrid);

        Result.State = INITIALISING;
        Result.Restart = 0;
        Result.Redraw = 0;
    }

    return Result;
}

GameData DrawGame(GameData Current)
{
    GameData Result = Current;

    //Clear screen
    SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0 );
    SDL_RenderClear( gRenderer );

    float BlockWidth = (float)GRID_WIDTH/(float)Result.MainGrid.Cols;
    float BlockHeight = (float)GRID_HEIGHT/(float)Result.MainGrid.Rows;

    // Draw Grid Background
    DrawRect(GRID_X, GRID_Y, GRID_WIDTH, GRID_HEIGHT, 0x55, 0x55, 0x55, 0xFF);

    //Draw Grid
    DrawGrid(Result.MainGrid, GRID_X, GRID_Y, GRID_WIDTH, GRID_HEIGHT);

    //Draw Tetromino
    DrawGrid(Result.FallingTetro.Grid,
            GRID_X+Result.FallingTetro.Col*BlockWidth,
            GRID_Y+Result.FallingTetro.Row*BlockHeight,
            Result.FallingTetro.GridSize*BlockWidth,
            Result.FallingTetro.GridSize*BlockHeight);

    //Draw Preview Grid Background

    unsigned int PreviewWidth = 6.0*BlockWidth;
    unsigned int PreviewHeight = 6.0*BlockHeight;

    DrawRect(PREVIEW_X,
            PREVIEW_Y,
            PreviewWidth,
            PreviewHeight,
            0x77,
            0x77,
            0x77,
            0xFF);

    //Draw Next Tetrimino

    //Calculate centre of mass of discrete grid
    Vector2D TetroCentre = CalculateGridCentreOfMass(Result.NextTetro.Grid);

    unsigned int PreviewCentreX = PreviewWidth*0.5;
    unsigned int PreviewCentreY = PreviewHeight*0.5;

    DrawGrid(Result.NextTetro.Grid,
            PREVIEW_X + PreviewCentreX - TetroCentre.X*BlockWidth,
            PREVIEW_Y + PreviewCentreY - TetroCentre.Y*BlockHeight,
            Result.NextTetro.GridSize*BlockWidth,
            Result.NextTetro.GridSize*BlockHeight);

    //Draw Score
    SDL_Color TextColor = {255,255,255};
    char ScoreText[100] = "\0";

    sprintf(ScoreText, "Score: %04d", Result.Score);
    DrawText(ScoreText, PREVIEW_X, PREVIEW_Y + PreviewHeight+ Result.ScoreTexture.Height);

    //Update screen
    SDL_RenderPresent( gRenderer );
    return Result;
}

void DrawTexture(Texture T, unsigned int X, unsigned int Y, unsigned int Width, unsigned int Height)
{
    SDL_Rect Rect = {X, Y, Width, Height};
    SDL_RenderCopy( gRenderer, T.Data, NULL, &Rect );
}

void DrawText(const char* Text, float X, float Y)
{
    unsigned int StringLength = strlen(Text);

    float StringWidth = 0;
    for(unsigned int Index = 0; Index < StringLength; ++Index)
    {
        Texture Char = GetGlyph(Text[Index]);
        DrawTexture(Char, X+StringWidth, Y, Char.Width, Char.Height);

        StringWidth += Char.Width;
    }
}

void DrawGrid(BlockGrid Grid, unsigned int X, unsigned int Y, unsigned Width, unsigned Height)
{
    float BlockWidth = (float)Width/(float)Grid.Cols;
    float BlockHeight = (float)Height/(float)Grid.Rows;

    for(unsigned int Row = 0; Row < Grid.Rows; ++Row)
    {
        for(unsigned int Col = 0; Col < Grid.Cols; ++Col)
        {
            Block* CurrentBlock = GetBlock(Grid, Row, Col);

            if(CurrentBlock->Occupied)
            {
                unsigned int GapFillerHori = 0;
                unsigned int GapFillerVert = 0;

                if((unsigned int)(BlockWidth*(Col+1)) > ((unsigned int)(BlockWidth)*(Col+1)))
                {
                    GapFillerHori = 1;
                }

                if((unsigned int)(BlockHeight*(Row+1)) > ((unsigned int)(BlockHeight)*(Row+1)))
                {
                    GapFillerVert = 1;
                }

                DrawRect(X + Col*BlockWidth,
                        Y + Row*BlockHeight,
                        BlockWidth + GapFillerHori,
                        BlockHeight + GapFillerVert,
                        CurrentBlock->Red,
                        CurrentBlock->Green,
                        CurrentBlock->Blue,
                        CurrentBlock->Alpha );
            }
        }
    }
}

/*
 * Block/Grid Operations
 */

Block GenerateBlock()
{
    Block Result = {0, 0, 0, 0, 0};
    return Result;
}

void DestroyGrid(BlockGrid Grid)
{
    free(Grid.Blocks);
}

Block* GetBlock(BlockGrid Grid, unsigned int Row, unsigned int Col)
{
    return Grid.Blocks + Grid.Cols*Row + Col;
}

void EraseBlock(Block* BlockToErase)
{
    BlockToErase->Occupied = 0;
    BlockToErase->Red = 0;
    BlockToErase->Green = 0;
    BlockToErase->Blue = 0;
    BlockToErase->Alpha = 0;
}

BlockGrid GenerateGrid(unsigned int Rows, unsigned int Cols)
{
    BlockGrid Result;

    Result.Rows = Rows;
    Result.Cols = Cols;

    Result.Blocks = (Block*)(malloc(sizeof(Block)*Rows*Cols));

    unsigned int BlockTotal = (Result.Rows)*(Result.Cols);
    unsigned int Count = 0;
    Block* CurrentBlock = Result.Blocks;

    while(Count < BlockTotal)
    {
        *CurrentBlock = GenerateBlock();

        ++Count;
        ++CurrentBlock;
    }

    return Result;
}

void StoreTetromino(BlockGrid Grid, Tetromino Tetro)
{
    if( (Tetro.Grid.Blocks == NULL) || (Grid.Blocks == NULL) )
    {
        return;
    }

    for(unsigned int Row = 0; Row < Tetro.Grid.Rows; ++Row)
    {
        for(unsigned int Col = 0; Col < Tetro.Grid.Cols; ++Col)
        {
            Block* CurrentBlock = GetBlock(Tetro.Grid, Row, Col);

            if(CurrentBlock->Occupied)
            {
                unsigned int GridCol = Tetro.Col + Col;
                unsigned int GridRow = Tetro.Row + Row;

                Block* GridBlock = GetBlock(Grid, GridRow, GridCol);

                // Copy across block
                *GridBlock = *CurrentBlock;
            } 
        }
    }

}

unsigned int CheckCollisions(BlockGrid Grid, Tetromino Tetro)
{
    //Check for null pointers
    if( (Tetro.Grid.Blocks == NULL) || (Grid.Blocks == NULL) )
    {
        return 1;
    }

    for(unsigned int Row = 0; Row < Tetro.Grid.Rows; ++Row)
    {
        for(unsigned int Col = 0; Col < Tetro.Grid.Cols; ++Col)
        {
            Block* CurrentBlock = GetBlock(Tetro.Grid, Row, Col);

            if(CurrentBlock->Occupied)
            {
                unsigned int GridCol = Tetro.Col + Col;
                unsigned int GridRow = Tetro.Row + Row;

                //If out of bounds, count as collision
                if( (GridCol < 0) || (GridRow < 0) || (GridCol >= Grid.Cols) || (GridRow >= Grid.Rows) )
                {
                    return 1;
                }

                Block* GridBlock = GetBlock(Grid, GridRow, GridCol);

                // Check for Collision
                if(GridBlock->Occupied)
                {
                    return 1;
                }
            } 
        }
    }

    return 0;
}

/*
 * Tetromino Operations
 */
Tetromino GenerateTetromino()
{
    Tetromino Result;

    Result.Col = 0;
    Result.Row = 0;

    unsigned int Red   = rand()%0xFF;
    unsigned int Green = rand()%0xFF;
    unsigned int Blue  = rand()%0xFF;

    unsigned int Rand = rand();
    Result.Type = (TetrominoType)(Rand%7);

    unsigned int Coords[4] = {0};

    /*
     *| 0  | 1  | 2  | 3  |
     *| 4  | 5  | 6  | 7  |
     *| 8  | 9  | 10 | 11 |
     *| 12 | 13 | 14 | 15 |
     */
    switch(Result.Type)
    {
        case I_SHAPE:
            Result.GridSize = 4;
            Coords[0] = 0;
            Coords[1] = 4;
            Coords[2] = 8;
            Coords[3] = 12;
            break;
        case T_SHAPE:
            Result.GridSize = 3;
            Coords[0] = 0;
            Coords[1] = 1;
            Coords[2] = 2;
            Coords[3] = 5;
            break;
        case O_SHAPE:
            Result.GridSize = 2;
            Coords[0] = 0;
            Coords[1] = 1;
            Coords[2] = 4;
            Coords[3] = 5;
            break;
        case Z_SHAPE:
            Result.GridSize = 3;
            Coords[0] = 0;
            Coords[1] = 1;
            Coords[2] = 5;
            Coords[3] = 6;
            break;
        case S_SHAPE:
            Result.GridSize = 3;
            Coords[0] = 1;
            Coords[1] = 2;
            Coords[2] = 4;
            Coords[3] = 5;
            break;
        case L_L_SHAPE:
            Result.GridSize = 3;
            Coords[0] = 0;
            Coords[1] = 1;
            Coords[2] = 2;
            Coords[3] = 6;
            break;
        case L_R_SHAPE:
            Result.GridSize = 3;
            Coords[0] = 0;
            Coords[1] = 1;
            Coords[2] = 2;
            Coords[3] = 4;
            break;
        default:
            Result.GridSize = 4;
            Coords[0] = 0;
            Coords[1] = 3;
            Coords[2] = 12;
            Coords[3] = 15;
            break;
    }

    Result.Grid = GenerateGrid(Result.GridSize,Result.GridSize);

    for(unsigned int Index = 0; Index < 4; ++Index)
    {
        Block* TBlock = GetBlock(Result.Grid, Coords[Index]/4, Coords[Index]%4);

        TBlock->Occupied = 1;
        TBlock->Red   = Red;
        TBlock->Green = Green;
        TBlock->Blue  = Blue;
        TBlock->Alpha = 0xFF;
    }

    return Result;
}

void DestroyTetromino(Tetromino Tetro)
{
    DestroyGrid(Tetro.Grid);
}

Tetromino RotateTetroClockwise(Tetromino Tetro)
{
    Tetromino Result = Tetro;

    if(Result.Type == O_SHAPE)
    {
        return Result;
    }

    BlockGrid TransposeGrid = GenerateGrid(Result.GridSize,Result.GridSize);

    for(unsigned int Row = 0; Row < Result.GridSize; ++Row)
    {
        for(unsigned int Col = 0; Col < Result.GridSize; ++Col)
        {
            Block* Block1 = GetBlock(Result.Grid,   Row, Col);
            Block* Block2 = GetBlock(TransposeGrid, Col, Row);

            *Block2 = *Block1;
        }
    }

    Result.Grid = TransposeGrid;

    // Swap Cols
    BlockGrid SwappedGrid = GenerateGrid(Result.GridSize,Result.GridSize);

    Block* EndBlock = NULL;
    Block* StartBlock = NULL;

    for(unsigned int Row = 0; Row < Result.GridSize; ++Row)
    {
        for(unsigned int Col = 0; Col < Result.GridSize; ++Col)
        {
            unsigned int SwapCol = (Result.GridSize - 1) - Col;

            EndBlock    = GetBlock(Result.Grid,     Row,    Col);
            StartBlock  = GetBlock(SwappedGrid,     Row,    SwapCol);

            *StartBlock = *EndBlock;
        }
    }

    DestroyGrid(Result.Grid);
    Result.Grid = SwappedGrid;

    return Result;
}

unsigned int RemoveGridLines(BlockGrid Grid)
{
    unsigned int FullRowCount    = 0;
    unsigned int ShiftRowsDownBy = 0;

    for(unsigned int Row = (Grid.Rows-1); Row < Grid.Rows; --Row)
    {
        unsigned int IsRowFull = 1;

        for(unsigned int Col = 0; Col < Grid.Cols; ++Col)
        {
            Block* CurrentBlock = GetBlock(Grid, Row, Col);

            if(!CurrentBlock->Occupied)
            {
                IsRowFull = 0;
            }

            if(ShiftRowsDownBy)
            {
                Block* SwapBlock = GetBlock(Grid, Row+ShiftRowsDownBy, Col);
                *SwapBlock = *CurrentBlock;
            }
        }

        if(IsRowFull)
        {
            FullRowCount++;
            ShiftRowsDownBy++;
        }
    }

    //Cleanup the remaining rows
    for(unsigned int Row = 0; Row < ShiftRowsDownBy; ++Row)
    {
        for(unsigned int Col = 0; Col < Grid.Cols; ++Col)
        {
            Block* CurrentBlock = GetBlock(Grid, Row, Col);

            EraseBlock(CurrentBlock);
        }
    }

    return FullRowCount;
}

Vector2D CalculateGridCentreOfMass(BlockGrid Grid)
{
    Vector2D Result = {0,0};

    float TotalX = 0;
    float TotalY = 0;
    float TotalMass = 0;

    for(float Row = 0; Row < Grid.Rows; ++Row)
    {
        for(float Col = 0; Col < Grid.Cols; ++Col)
        {
            Block* CurrentBlock = GetBlock(Grid, Row, Col);
            if(CurrentBlock->Occupied)
            {
                TotalX += (Col+1)-0.5;
                TotalY += (Row+1)-0.5;
                ++TotalMass;
            }
        }
    }

    Result.X = TotalX/TotalMass;
    Result.Y = TotalY/TotalMass;

    return Result;
}

void DrawTextToRect(const char* Text, Rect Box, Alignment Align)
{
    unsigned int StringLength = strlen(Text);

    float MaxWidth = 0;
    float MaxHeight = 0;
    unsigned int MaxLines = 1; //Assume at least one line of text (even if blank)
    unsigned int* LineWidths = (unsigned int*)malloc(sizeof(unsigned int)*MaxLines);

    //Work out MaxLines and the Width of each Line
    for(unsigned int Line = 0; Line < MaxLines; ++Line)
    {
        for(unsigned int Index = 0; Index < StringLength; ++Index)
        {
            if(Text[Index] == '\n')
            {
                ++MaxLines;
                realloc((void*)LineWidths, sizeof(unsigned int)*MaxLines);
            }
            else
            {
                Texture Char = GetGlyph(Text[Index]);
                LineWidths[Line] += Char.Width;

                if(Char.Height > MaxHeight)
                {
                    MaxHeight = Char.Height;
                }
            }
        }
    }

    printf("There are %d lines in \"%s\".\n");

    for(unsigned int Line = 0; Line < MaxLines; ++Line)
    {
        float DrawWidth = LineWidths[Line];
        float DrawHeight = MaxHeight;

        if(Box.Width < LineWidths[Line])
        {
            //Shrink text to fit box
            DrawWidth = Box.Width;
        }

        if(Box.Height < (MaxHeight*MaxLines))
        {
            DrawHeight = Box.Height;
        }

        float PositionX = 0;
        float PositionY = 0;
        float Width = 0;
        float Height = 0;

        switch(Align)
        {
            case LEFT:
                PositionX = 0;
                PositionY = 0;
                break;
            case RIGHT:
                PositionX = Box.Width - LineWidths[Line];
                PositionY = 0;
                break;
            case CENTRE:
                PositionX = (Box.Width - LineWidths[Line])/2;
                PositionY = (Box.Height - MaxHeight*MaxLines)/2 + MaxHeight*Line;
                break;
            default:
                break;
        }

        for(unsigned int Index = 0; Index < StringLength; ++Index)
        {
            if(Text[Index] == '\n')
            {
                break;
            }
            else
            {
                Texture Char = GetGlyph(Text[Index]);

                float Width = DrawWidth*(float)Char.Width/LineWidths[Line];
                float Height = DrawHeight*(float)Char.Height/MaxHeight;

                DrawTexture(Char, Box.X + PositionX, Box.Y + PositionY, Width, Height);
                PositionX += Width;
            }
        }
    }

    free(LineWidths);
}
