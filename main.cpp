#include <SDL2/SDL.h>
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

bool init();
void close();
void DrawRect(int X, int Y, int Width, int Height, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha);

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

/*
 * Game Stuff
 */

enum TetrominoType{
    I_SHAPE,
    T_SHAPE,
    O_SHAPE,
    Z_SHAPE,
    S_SHAPE,
    L_L_SHAPE,
    L_R_SHAPE,
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
        //Main loop flag
        bool quit = false;

        //Event handler
        SDL_Event e;

        BlockGrid MainGrid      = GenerateGrid(GRID_ROWS, GRID_COLS);

        Tetromino FallingTetro = GenerateTetromino();
        Tetromino NextTetro = GenerateTetromino();
        NextTetro.Col = 1;
        NextTetro.Row = 1;

        unsigned int StartTick = 0;
        unsigned int EndTick = 0;
        unsigned int FrameDuration = 0;

        unsigned int FallingTimer = FALL_FRAMES;
        unsigned int MoveLeft = 0;
        unsigned int MoveRight = 0;
        unsigned int MoveDown = 0;
        unsigned int Rotate = 0;
        unsigned int Score = 0;
        unsigned int Redraw = 1;

        float CellWidth = (float)(SCREEN_WIDTH - SCREEN_COLS*CELL_PADDING)/SCREEN_COLS;
        float CellHeight = (float)(SCREEN_HEIGHT - SCREEN_ROWS*CELL_PADDING)/SCREEN_ROWS;

        //While application is running
        while( !quit )
        {
            StartTick = SDL_GetTicks();
            /*
             * Input Stuff
             */

            //Handle events on queue
            while( SDL_PollEvent( &e ) != 0 )
            {
                //User requests quit
                if( e.type == SDL_QUIT )
                {
                    quit = true;
                }
                else if (e.type == SDL_KEYDOWN)
                {
                    switch( e.key.keysym.sym )
                    {
                        case SDLK_UP:
                            Rotate = 1;
                            break;
                        case SDLK_DOWN:
                            MoveDown = 1;
                            break;
                        case SDLK_LEFT:
                            MoveLeft = 1;
                            break;
                        case SDLK_RIGHT:
                            MoveRight = 1;
                            break;
                        default:
                            break;
                    }
                }
            }

            /*
             * Game Stuff
             */

            //Drop Tetro if necessary
            if(FallingTimer == 0)
            {
                MoveDown = 1;
                FallingTimer = FALL_FRAMES;
            }
            else
            {
                FallingTimer--;
            }

            //Move Tetro if necessary
            if(MoveLeft)
            {
                //Duplicate Tetromino
                Tetromino NewTetro = FallingTetro;

                NewTetro.Col--;

                if(!CheckCollisions(MainGrid, NewTetro))
                {
                    FallingTetro = NewTetro;
                    Redraw = 1;
                }

                MoveLeft = 0;
            }

            if(MoveRight)
            {
                //Duplicate Tetromino
                Tetromino NewTetro = FallingTetro;

                NewTetro.Col++;

                if(!CheckCollisions(MainGrid, NewTetro))
                {
                    FallingTetro = NewTetro;
                    Redraw = 1;
                }

                MoveRight = 0;
            }

            if(MoveDown)
            {
                //Duplicate Tetromino
                Tetromino NewTetro = FallingTetro;

                NewTetro.Row++;
                //Check for collisions
                if(CheckCollisions(MainGrid, NewTetro))
                {
                    StoreTetromino(MainGrid, FallingTetro);
                    DestroyTetromino(FallingTetro);
                    FallingTetro = NextTetro;

                    NextTetro    = GenerateTetromino();
                    NextTetro.Col = 1;
                    NextTetro.Row = 1;


                    FallingTimer = FALL_FRAMES;
                }
                else
                {
                    FallingTetro = NewTetro;
                }

                Redraw = 1;
                MoveDown = 0;
            }

            if(Rotate)
            {
                if(FallingTetro.Type != O_SHAPE)
                {
                    //Duplicate Tetromino
                    Tetromino NewTetro = RotateTetroClockwise(FallingTetro);

                    if(!CheckCollisions(MainGrid, NewTetro))
                    {
                        DestroyTetromino(FallingTetro);
                        FallingTetro = NewTetro;
                        Redraw = 1;
                    }
                    else
                    {
                        DestroyTetromino(NewTetro);
                    }

                }
                Rotate = 0;
            }

            // Final check for collisons - quit game if any are found
            if(CheckCollisions(MainGrid, FallingTetro))
            {
                quit = true;
            }

            //Remove any lines in the grid
            Score += RemoveGridLines(MainGrid);

            /*
             * Drawing Stuff
             */

            //Clear screen
            if(Redraw)
            {
                SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0 );
                SDL_RenderClear( gRenderer );

                float BlockWidth = (float)GRID_WIDTH/(float)MainGrid.Cols;
                float BlockHeight = (float)GRID_HEIGHT/(float)MainGrid.Rows;

                // Draw Grid Background
                DrawRect(GRID_X, GRID_Y, GRID_WIDTH, GRID_HEIGHT, 0x55, 0x55, 0x55, 0xFF);

                //Draw Grid
                DrawGrid(MainGrid, GRID_X, GRID_Y, GRID_WIDTH, GRID_HEIGHT);

                //Draw Tetromino
                DrawGrid(FallingTetro.Grid,
                         GRID_X+FallingTetro.Col*BlockWidth,
                         GRID_Y+FallingTetro.Row*BlockHeight,
                         FallingTetro.GridSize*BlockWidth,
                         FallingTetro.GridSize*BlockHeight);

                //Draw Preview Grid Background

                unsigned int PreviewWidth = 6.0*BlockWidth;
                unsigned int PreviewHeight = 6.0*BlockHeight;

                unsigned int TetroWidth = (NextTetro.GridSize*BlockWidth);
                unsigned int TetroHeight = (NextTetro.GridSize*BlockHeight);

                DrawRect(PREVIEW_X,
                         PREVIEW_Y,
                         PreviewWidth,
                         PreviewHeight,
                         0x77,
                         0x77,
                         0x77,
                         0xFF);

                //Draw Next Tetrimino
                DrawGrid(NextTetro.Grid,
                         PREVIEW_X + ((PreviewWidth-TetroWidth)/2),
                         PREVIEW_Y + ((PreviewHeight-TetroHeight)/2),
                         TetroWidth,
                         TetroHeight);

                //Update screen
                SDL_RenderPresent( gRenderer );
                Redraw = 0;
            }

            //Delay till the end of the maximum frame duration (1/FRAMERATE seconds)
            EndTick = SDL_GetTicks();
            FrameDuration = EndTick - StartTick;

            SDL_Delay((1000/FRAMERATE)-FrameDuration);
        }

        DestroyTetromino(FallingTetro);
        DestroyGrid(MainGrid);
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
			}
		}
	}

	return success;
}

void close()
{
	//Destroy window	
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	SDL_Quit();
}

void DrawRect(int X, int Y, int Width, int Height, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha)
{
    SDL_SetRenderDrawColor( gRenderer, Red, Green, Blue, Alpha);
    SDL_Rect Rect = {X, Y, Width, Height};
    SDL_SetRenderDrawBlendMode( gRenderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect( gRenderer, &Rect );
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
            Coords[0] = 1;
            Coords[1] = 5;
            Coords[2] = 9;
            Coords[3] = 13;
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
