#include <SDL.h>
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
    CELL_PADDING = 1,
    CELL_WIDTH  = (SCREEN_WIDTH - SCREEN_COLS*CELL_PADDING)/SCREEN_COLS,
    CELL_HEIGHT = (SCREEN_HEIGHT - SCREEN_ROWS*CELL_PADDING)/SCREEN_ROWS,

    //Game Constants
    FALL_TICKS  = 10000
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
    unsigned int X;
    unsigned int Y;
    Block* Blocks;
};

struct Entity{
    unsigned int X;
    unsigned int Y;
};

struct Tetromino{
    TetrominoType Type;
    unsigned int GridSize;
    unsigned int X;
    unsigned int Y;
    BlockGrid Grid;
};

Block   GenerateBlock();
Block*  GetBlock(BlockGrid Grid, unsigned int Row, unsigned int Col);
void    EraseBlock(Block* BlockToErase);

BlockGrid   GenerateGrid(unsigned int X, unsigned int Y, unsigned int Rows, unsigned int Cols);
void        DestroyGrid(BlockGrid Grid);

Tetromino       GenerateTetromino();
void            DestroyTetromino(Tetromino Tetro);
void            StoreTetromino(BlockGrid Grid, Tetromino Tetro);
Tetromino       RotateTetroClockwise(Tetromino Tetro);
Tetromino       RotateTetroAntiClockwise(Tetromino Tetro);

unsigned int    CheckCollisions(BlockGrid Grid, Tetromino Tetro);
unsigned int    RemoveGridLines(BlockGrid Grid);

void DrawMainCanvas(unsigned int X, unsigned int Y, unsigned int Width, unsigned int Height, BlockGrid Grid, Tetromino Tetro, float CellWidth, float CellHeight);
void DrawPreviewCanvas(unsigned int X, unsigned int Y, unsigned int Width, unsigned int Height, Tetromino Tetro, float CellWidth, float CellHeight);
void DrawTetromino(Tetromino Tetro, unsigned int X, unsigned int Y, float CellWidth, float CellHeight);

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

        BlockGrid MainGrid      = GenerateGrid(GRID_X, GRID_Y, GRID_ROWS, GRID_COLS);

        Tetromino FallingTetro = GenerateTetromino();
        Tetromino NextTetro = GenerateTetromino();

        unsigned int FallingTimer = FALL_TICKS;
        unsigned int MoveLeft = 0;
        unsigned int MoveRight = 0;
        unsigned int MoveDown = 0;
        unsigned int Rotate = 0;
        unsigned int Score = 0;

        float CellWidth = (float)(SCREEN_WIDTH - SCREEN_COLS*CELL_PADDING)/SCREEN_COLS;
        float CellHeight = (float)(SCREEN_HEIGHT - SCREEN_ROWS*CELL_PADDING)/SCREEN_ROWS;

        //While application is running
        while( !quit )
        {
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
                FallingTimer = FALL_TICKS;
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

                NewTetro.X--;

                if(!CheckCollisions(MainGrid, NewTetro))
                {
                    FallingTetro = NewTetro;
                }

                MoveLeft = 0;
            }

            if(MoveRight)
            {
                //Duplicate Tetromino
                Tetromino NewTetro = FallingTetro;

                NewTetro.X++;

                if(!CheckCollisions(MainGrid, NewTetro))
                {
                    FallingTetro = NewTetro;
                }

                MoveRight = 0;
            }

            if(MoveDown)
            {
                //Duplicate Tetromino
                Tetromino NewTetro = FallingTetro;

                NewTetro.Y++;
                //Check for collisions
                if(CheckCollisions(MainGrid, NewTetro))
                {
                    StoreTetromino(MainGrid, FallingTetro);
                    DestroyTetromino(FallingTetro);
                    FallingTetro = NextTetro;
                    NextTetro    = GenerateTetromino();
                    FallingTimer = FALL_TICKS;
                }
                else
                {
                    FallingTetro = NewTetro;
                }

                MoveDown = 0;
            }

            if(Rotate)
            {
                //Duplicate Tetromino
                Tetromino NewTetro = RotateTetroClockwise(FallingTetro);

                if(!CheckCollisions(MainGrid, NewTetro))
                {
                    FallingTetro = NewTetro;
                }

                Rotate = 0;
            }

            // Final check for collisons - quit game if any are found
            if(CheckCollisions(MainGrid, FallingTetro))
            {
                //quit = true;
            }

            //Remove any lines in the grid
            Score += RemoveGridLines(MainGrid);

            printf("Score: %d\n", Score);

            /*
             * Drawing Stuff
             */

            //Clear screen
            SDL_SetRenderDrawColor( gRenderer, 0, 0, 0, 0 );
            SDL_RenderClear( gRenderer );

            DrawMainCanvas(GRID_X, GRID_Y, GRID_WIDTH, GRID_HEIGHT, MainGrid, FallingTetro, CellWidth, CellHeight);

            //Draw Tetromino
            DrawTetromino(FallingTetro, GRID_X, GRID_Y, CellWidth, CellHeight);

            DrawPreviewCanvas(PREVIEW_X, PREVIEW_Y, PREVIEW_WIDTH, PREVIEW_HEIGHT, NextTetro, CellWidth, CellHeight);
            //Draw Preview Tetromino
            DrawTetromino(NextTetro, PREVIEW_X, PREVIEW_Y, CellWidth, CellHeight);

            //Update screen
            SDL_RenderPresent( gRenderer );
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

void DrawTetromino(Tetromino Tetro, unsigned int X, unsigned int Y, float CellWidth, float CellHeight)
{
    for(unsigned int TRow = 0; TRow < 4; ++TRow)
    {
        for(unsigned int TCol = 0; TCol < 4; ++TCol)
        {
            Block* CurrentBlock = GetBlock(Tetro.Grid,TRow,TCol);

            if(CurrentBlock->Occupied)
            {
                unsigned int Col = Tetro.X + TCol;
                unsigned int Row = Tetro.Y + TRow;

                DrawRect(X + float(Col)*CellWidth + CELL_PADDING,
                         Y + float(Row)*CellHeight + CELL_PADDING,
                         CellWidth,
                         CellHeight, 
                         CurrentBlock->Red,
                         CurrentBlock->Green,
                         CurrentBlock->Blue,
                         CurrentBlock->Alpha );
            }
        }
    }
}

void DrawMainCanvas(unsigned int X, unsigned int Y, unsigned int Width, unsigned int Height, BlockGrid Grid, Tetromino Tetro, float CellWidth, float CellHeight)
{
    // Draw Grid Background
    DrawRect(X, Y, Width, Height, 0x55, 0x55, 0x55, 0xFF);

    //Draw Grid
    for(unsigned int Row = 0; Row < Grid.Rows; ++Row)
    {
        for(unsigned int Col = 0; Col < Grid.Cols; ++Col)
        {
            Block* CurrentBlock = GetBlock(Grid, Row, Col);

            if(CurrentBlock->Occupied)
            {
                // Draw grid blocks
                DrawRect(X + Col*CELL_WIDTH,
                         Y + Row*CELL_HEIGHT,
                         CELL_WIDTH,
                         CELL_HEIGHT,
                         CurrentBlock->Red,
                         CurrentBlock->Green,
                         CurrentBlock->Blue,
                         CurrentBlock->Alpha );
            }
        }
    }
}

void DrawPreviewCanvas(unsigned int X, unsigned int Y, unsigned int Width, unsigned int Height, Tetromino Tetro, float CellWidth, float CellHeight)
{
    // Draw Preview Background
    DrawRect(X,
             Y,
             4*CELL_WIDTH,
             4*CELL_HEIGHT,
             0x77,
             0x77,
             0x77,
             0xFF);
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

BlockGrid GenerateGrid(unsigned int X, unsigned int Y, unsigned int Rows, unsigned int Cols)
{
    BlockGrid Result;

    Result.X = X;
    Result.Y = Y;
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
                unsigned int GridCol = Tetro.X + Col;
                unsigned int GridRow = Tetro.Y + Row;

                Block* GridBlock = GetBlock(Grid, GridRow, GridCol);

                // Copy across block
                *GridBlock = *CurrentBlock;
            } 
        }
    }

}

unsigned int CheckCollisions(BlockGrid Grid, Tetromino Tetro)
{
    if( (Tetro.Grid.Blocks == NULL) || (Grid.Blocks == NULL) )
    {
        return 0;
    }

    for(unsigned int Row = 0; Row < Tetro.Grid.Rows; ++Row)
    {
        for(unsigned int Col = 0; Col < Tetro.Grid.Cols; ++Col)
        {
            Block* CurrentBlock = GetBlock(Tetro.Grid, Row, Col);

            if(CurrentBlock->Occupied)
            {
                unsigned int GridCol = Tetro.X + Col;
                unsigned int GridRow = Tetro.Y + Row;

                //If out of bounds, count as collision
                if( (GridCol >= Grid.Cols) || (GridRow >= Grid.Rows) )
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

    Result.X = 0;
    Result.Y = 0;
    Result.Grid = GenerateGrid(0,0,4,4);

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

    for(unsigned int Index = 0; Index < 4; ++Index)
    {
        Block* TBlock = Result.Grid.Blocks + Coords[Index];

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

    BlockGrid TransposeGrid = GenerateGrid(0,0,4,4);

    for(unsigned int Row = 0; Row < 4; ++Row)
    {
        for(unsigned int Col = 0; Col < 4; ++Col)
        {
            Block* Block1 = GetBlock(Result.Grid,   Row, Col);
            Block* Block2 = GetBlock(TransposeGrid, Col, Row);

            *Block2 = *Block1;
        }
    }

    DestroyGrid(Result.Grid);
    Result.Grid = TransposeGrid;

    // Swap Cols
    BlockGrid SwappedGrid = GenerateGrid(0,0,4,4);

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

Tetromino RotateTetroAntiClockwise(Tetromino Tetro)
{
    Tetromino Result = Tetro;

    if(Result.Type == O_SHAPE)
    {
        return Result;
    }

    BlockGrid TransposeGrid = GenerateGrid(0,0,4,4);

    for(unsigned int Row = 0; Row < 4; ++Row)
    {
        for(unsigned int Col = 0; Col < 4; ++Col)
        {
            Block* Block1 = GetBlock(Result.Grid,    Row, Col);
            Block* Block2 = GetBlock(TransposeGrid, Col, Row);

            *Block2 = *Block1;
        }
    }

    DestroyGrid(Result.Grid);
    Result.Grid = TransposeGrid;

    // Swap Rows
    BlockGrid SwappedGrid = GenerateGrid(0,0,4,4);

    Block* EndBlock = NULL;
    Block* StartBlock = NULL;

    for(unsigned int Row = 0; Row < Result.GridSize; ++Row)
    {
        for(unsigned int Col = 0; Col < Result.GridSize; ++Col)
        {
            unsigned int SwapRow = (Result.GridSize - 1) - Row;

            EndBlock    = GetBlock(Result.Grid,         Row,        Col);
            StartBlock  = GetBlock(SwappedGrid,    SwapRow,    Col);

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
