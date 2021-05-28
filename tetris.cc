#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

typedef long  uint32_t;
typedef int   uint16_t;
typedef char  uint8_t;
typedef int   boolean;
typedef short byte;

#define true  1
#define false 0

/* arduino style functions */
#define delay(x)  (usleep(x*1000))
#define random(x) (rand() % x)

#define WIDTH  16
#define HEIGHT 16

#define TETRIS_PIECE_WIDTH  4
#define TETRIS_PIECE_HEIGHT 4

#define SCORE_PER_LEVEL     1

#define DEBUG

/*===========================================================================
 * Tetris Board
 *===========================================================================*/

enum TetrisBoardState {
  PREGAME,
  GAME,
  CLEANUP,
  ENDGAME
};

struct TetrisBoard {
  TetrisBoardState state;
  uint8_t          level;
  uint16_t         score;
  int              cell[WIDTH][HEIGHT + 1];

  TetrisBoard() {
    state = PREGAME;
    level = 0;
  }
};
TetrisBoard s_TetrisBoard;

/*===========================================================================
 * Tetris Pieces
 *===========================================================================*/

enum TetrisPieceState {
  UNINITIALIZED,
  MOVING,
  DROPPING
};

typedef struct Coord {
  int x;
  int y;
  int z;
} Coord;

typedef struct TetrisFrame {
  int cell[4][4];
} TetrisFrame;

typedef struct TetrisPiece {
  TetrisPieceState        state;
  TetrisFrame            *frames[4];
  int                     index;

  Coord                   origin;
} TetrisPiece;

TetrisFrame OFrame[1] = {
  {
    {
      {0,0,0,0},
      {0,1,1,0},
      {0,1,1,0},
      {0,0,0,0}
    }
  }
};

TetrisFrame IFrame[2] = {
  {
    {
      {0,0,2,0},
      {0,0,2,0},
      {0,0,2,0},
      {0,0,2,0}
    }
  },
  {
    {
      {0,0,0,0},
      {2,2,2,2},
      {0,0,0,0},
      {0,0,0,0}
    }
  }
};

TetrisFrame SFrame[2] = {
  {
    {
      {0,3,0,0},
      {0,3,3,0},
      {0,0,3,0},
      {0,0,0,0}
    }
  },
  {
    {
      {0,0,0,0},
      {0,3,3,0},
      {3,3,0,0},
      {0,0,0,0}
    }
  }
};

TetrisFrame ZFrame[2] = {
  {
    {
      {0,0,4,0},
      {0,4,4,0},
      {0,4,0,0},
      {0,0,0,0}
    }
  },
  {
    {
      {0,0,0,0},
      {0,4,4,0},
      {0,0,4,4},
      {0,0,0,0}
    }
  }
};

TetrisFrame LFrame[4] = {
  {
    {
      {0,5,0,0},
      {0,5,0,0},
      {0,5,5,0},
      {0,0,0,0}
    }
  },
  {
    {
      {0,0,5,0},
      {5,5,5,0},
      {0,0,0,0},
      {0,0,0,0}
    }
  },
  {
    {
      {5,5,0,0},
      {0,5,0,0},
      {0,5,0,0},
      {0,0,0,0}
    }
  },
  {
    {
      {0,0,0,0},
      {5,5,5,0},
      {5,0,0,0},
      {0,0,0,0}
    }
  }
};

TetrisFrame JFrame[4] = {
  {
    {
      {0,6,0,0},
      {0,6,0,0},
      {6,6,0,0},
      {0,0,0,0}
    }
  },
  {
    {
      {0,0,0,0},
      {6,6,6,0},
      {0,0,6,0},
      {0,0,0,0}
    }
  },
  {
    {
      {0,6,6,0},
      {0,6,0,0},
      {0,6,0,0},
      {0,0,0,0}
    }
  },
  {
    {
      {6,0,0,0},
      {6,6,6,0},
      {0,0,0,0},
      {0,0,0,0}
    }
  }
};

TetrisFrame TFrame[4] = {
  {
    {
      {0,7,0,0},
      {7,7,0,0},
      {0,7,0,0},
      {0,0,0,0}
    }
  },
  {
    {
      {0,0,0,0},
      {7,7,7,0},
      {0,7,0,0},
      {0,0,0,0}
    }
  },
  {
    {
      {0,7,0,0},
      {0,7,7,0},
      {0,7,0,0},
      {0,0,0,0}
    }
  },
  {
    {
      {0,7,0,0},
      {7,7,7,0},
      {0,0,0,0},
      {0,0,0,0}
    }
  }
};

TetrisPiece TetrisPieces[7] = {
  /* One-frame pieces */
  {
    UNINITIALIZED,
    {&(OFrame[0]), &(OFrame[0]), &(OFrame[0]), &(OFrame[0])},
    0,
    {0,0,0}
  },

  /* Two-frame pieces */
  {
    UNINITIALIZED,
    {&(IFrame[0]), &(IFrame[1]), &(IFrame[0]), &(IFrame[1])},
    0,
    {0,0,0}
  },
  {
    UNINITIALIZED,
    {&(SFrame[0]), &(SFrame[1]), &(SFrame[0]), &(SFrame[1])},
    0,
    {0,0,0}
  },
  {
    UNINITIALIZED,
    {&(ZFrame[0]), &(ZFrame[1]), &(ZFrame[0]), &(ZFrame[1])},
    0,
    {0,0,0}
  },

  /* Four-frame pieces */
  {
    UNINITIALIZED,
    {&(LFrame[0]), &(LFrame[1]), &(LFrame[2]), &(LFrame[3])},
    0,
    {0,0,0}
  },
  {
    UNINITIALIZED,
    {&(JFrame[0]), &(JFrame[1]), &(JFrame[2]), &(JFrame[3])},
    0,
    {0,0,0}
  },
  {
    UNINITIALIZED,
    {&(TFrame[0]), &(TFrame[1]), &(TFrame[2]), &(TFrame[3])},
    0,
    {0,0,0}
  },
};

/*******************************************************************************
 * Debug Functions
 ******************************************************************************/
void debugPrintCell(int c)
{
  switch (c) {
    /* empty space */
    case 0:
      printf("[ ]");
      break;

    /* moving bricks */
    case 1:
      printf("[o]");
      break;
    case 2:
      printf("[i]");
      break;
    case 3:
      printf("[s]");
      break;
    case 4:
      printf("[z]");
      break;
    case 5:
      printf("[l]");
      break;
    case 6:
      printf("[j]");
      break;
    case 7:
      printf("[t]");
      break;

    /* frozen bricks */
    case 11:
      printf("[O]");
      break;
    case 12:
      printf("[I]");
      break;
    case 13:
      printf("[S]");
      break;
    case 14:
      printf("[Z]");
      break;
    case 15:
      printf("[L]");
      break;
    case 16:
      printf("[J]");
      break;
    case 17:
      printf("[T]");
      break;

    /* artificial bottom */
    case 100:
      printf("[-]");
      break;

    /* death blinkies */
    case 200:
      printf("[X]");
      break;
    case 201:
      printf("[.]");
      break;

    /* filled lines */
    case 300:
      printf("[=]");
      break;

    /* unknown value */
    default:
      printf("[%d]", c);
      break;
  }
}

void debugClearScreen()
{
  system("clear");
}

void debugPrintFrame(TetrisFrame *f)
{
  int i,j;
  for (i=0; i<4; i++) {
    for (j=0; j<4; j++) {
      debugPrintCell(f->cell[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

void debugPrintPiece(TetrisPiece *p) 
{
  int i,j;
  TetrisFrame *f = p->frames[p->index];

  for (i=0; i<4; i++) {
    for (j=0; j<4; j++) {
      debugPrintCell(f->cell[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

void debugPrintBoard(TetrisBoard *b)
{
  int x1=0, y1=0;
  for (y1=0; y1<(HEIGHT + 1); y1++) {
    for (x1=0; x1<WIDTH; x1++) {
      debugPrintCell(b->cell[x1][y1]);
    }
    printf("\n");
  }
}

void debugPrintState(TetrisBoard *b, TetrisPiece *p) 
{
  int x1=0, y1=0, x=p->origin.x, y=p->origin.y;
  int boardValue = 0, pieceValue = 0;

  if (b->state == PREGAME) return;

  /* score and whatnot */
  printf("SCORE: %04d  LEVEL: %04d\n", b->score, b->level);

  for (y1=0; y1<(HEIGHT + 1); y1++) {
    for (x1=0; x1<WIDTH; x1++) {

      boardValue = s_TetrisBoard.cell[x1][y1];
      pieceValue = 0;

      if (p->state == UNINITIALIZED) goto DisplayCell;

      /* if the vertical alignment says we are in a piece... */
      if ( (y1 >= p->origin.y) && (y1 < p->origin.y + 4) ) {
        /* then maybe that piece is wrapping around... or maybe not...
         * but hey, unknown means we gotta do the check... so oh well */

        /* ex: (15 + 0) % 16 == 15 */
        if (x1 == ((x + 0) % WIDTH)) {
          pieceValue = (p->frames[p->index]->cell[0][y1 - y]);
        }
        /* ex: (15 + 1) % 16 == 0 */
        else
        if (x1 == ((x + 1) % WIDTH)) {
          pieceValue = (p->frames[p->index]->cell[1][y1 - y]);
        }
        /* ex: (15 + 2) % 16 == 1 */
        else
        if (x1 == ((x + 2) % WIDTH)) {
          pieceValue = (p->frames[p->index]->cell[2][y1 - y]);
        }
        /* ex: (15 + 3) % 16 == 2 */
        else
        if (x1 == ((x + 3) % WIDTH)) {
          pieceValue = (p->frames[p->index]->cell[3][y1 - y]);
        }
        else {
          if ( (x1 >= x) && (x1 < x + 4) ) {
            pieceValue = (p->frames[p->index]->cell[x1 - x][y1 - y]);
          }
        }
      }

DisplayCell:
      debugPrintCell(boardValue + pieceValue);
    }
    printf("\n");
  }
}

/*******************************************************************************
 * Tetris Functions
 ******************************************************************************/
TetrisPiece aPiece;

void initializeBoardIfNeeded(TetrisBoard *b) {
  int x1, y1;

  if (b->state != PREGAME) return;

  for (y1=0; y1<(HEIGHT + 1); y1++) {
    for (x1=0; x1<WIDTH; x1++) {
      if (y1 == HEIGHT) b->cell[x1][y1] = 100;
      else b->cell[x1][y1] = 0;
    }
  }

#ifdef DEBUG_PREFILL
  /* set up some fake obsticals */
  for (int i=0; i<WIDTH; i++) {
    if ((i==7) || (i==8)) continue;
    b->cell[i][15] = 11;
  }
#endif

  b->state = GAME;
}

bool isValidNextPosition(TetrisPiece *p, int x, int y)
{
  int x1=0, y1=0;
  int boardValue = 0, pieceValue = 0;

  for (y1=0; y1<(HEIGHT + 1); y1++) {
    for (x1=0; x1<WIDTH; x1++) {
      boardValue = s_TetrisBoard.cell[x1][y1];
      pieceValue = 0;

      /* if the vertical alignment says we are in a piece... */
      if ( (y1 >= y) && (y1 < (y + 4)) ) {
        /* then maybe that piece is wrapping around... or maybe not...
         * but hey, unknown means we gotta do the check... so oh well */

        /* ex: (15 + 0) % 16 == 15 */
        if (x1 == ((x + 0) % WIDTH)) {
          pieceValue = (p->frames[p->index]->cell[0][y1 - y]);
        }
        /* ex: (15 + 1) % 16 == 0 */
        else
        if (x1 == ((x + 1) % WIDTH)) {
          pieceValue = (p->frames[p->index]->cell[1][y1 - y]);
        }
        /* ex: (15 + 2) % 16 == 1 */
        else
        if (x1 == ((x + 2) % WIDTH)) {
          pieceValue = (p->frames[p->index]->cell[2][y1 - y]);
        }
        /* ex: (15 + 3) % 16 == 2 */
        else
        if (x1 == ((x + 3) % WIDTH)) {
          pieceValue = (p->frames[p->index]->cell[3][y1 - y]);
        }
        else {
          /* XXX: Without this if statement, the adjacent memory of
           *      the piece is written to the screen... LOL */
          if ( (x1 >= x) && (x1 < x + 4) ) {
            pieceValue = (p->frames[p->index]->cell[x1 - x][y1 - y]);
          }
        }
      }

      if ((pieceValue > 0) && (boardValue + pieceValue > 10)) /* hit */
        goto BadPosition;

    }
  }
  return true;

BadPosition:
  return false;
}

int initializePieceIfNeeded(TetrisPiece *p) {

  TetrisPiece *src;
  int x=0;

  if (p->state != UNINITIALIZED) return 0;

  /* Change piece? */
  src = &(TetrisPieces[rand() % 7]);
#ifdef DEBUG
  src = &(TetrisPieces[0]);
#endif

  /* get frames from new piece */
  p->frames[0] = src->frames[0];
  p->frames[1] = src->frames[1];
  p->frames[2] = src->frames[2];
  p->frames[3] = src->frames[3];

  /* randomize index (starting rotation of piece) */
  p->index = rand() % 4;

  /* Where does the piece start? */
  p->origin.y = 0;
#ifdef DEBUG
  p->origin.x = p->origin.x + 2; /* debug */
  if (p->origin.x >= WIDTH)
    p->origin.x = (p->origin.x % WIDTH);
#else
  p->origin.x = rand() % WIDTH;
#endif

  /* And Change the state */
  p->state = MOVING;

  /* test to see if this piece is even valid... */
  if (isValidNextPosition(p, p->origin.x, p->origin.y)) return 0;

#ifndef DEBUG
  /* if invalid, lets try moving position a few times */
  for (x=0; x<WIDTH; x++) {
    if (isValidNextPosition(p, ((p->origin.x + x)%WIDTH), p->origin.y)) {
      p->origin.x = ((p->origin.x + x)%WIDTH);
      return 0;
    }
  }
#endif

  /* getting here means we were unable to put a piece on the board...*/
  p->state = UNINITIALIZED;
  s_TetrisBoard.state = ENDGAME;
  return 1;
}

bool rotatePieceLeft(TetrisPiece *p) {
  p->index -= 1;
  if( p->index < 0 ) p->index = 3;

  /* test to see if that rotation would have hit something... */
  if (isValidNextPosition(p, p->origin.x, p->origin.y)) return true;

  /* undo the rotation */
  p->index += 1;
  if (p->index >= 4) p->index = 0;
  return false;
}

bool rotatePieceRight(TetrisPiece *p) {
  p->index += 1;
  if( p->index >= 4 ) p->index = 0;

  /* test to see if that rotation would have hit something... */
  if (isValidNextPosition(p, p->origin.x, p->origin.y)) return true;

  /* undo the rotation */
  p->index -= 1;
  if (p->index < 0) p->index = 3;
  return false;
}

bool movePieceLeft(TetrisPiece *p) {
  int x1=0, y1=0, x=0, y=0;
  int boardValue = 0, pieceValue = 0;

  /* dont test where we ARE, test where we are going... */
  x = p->origin.x - 1;
  if (x < 0) x = (WIDTH - 1);
  y = p->origin.y;

  if (! isValidNextPosition(p, x, y)) goto NoChange;

  /* do the move left */
  p->origin.x = x;
  return true;

NoChange:
  return false;
}

bool movePieceRight(TetrisPiece *p) {
  int x1=0, y1=0, x=0, y=0;
  int boardValue = 0, pieceValue = 0;

  /* dont test where we ARE, test where we are going... */
  x = p->origin.x + 1;
  if (x >= WIDTH) x = 0;
  y = p->origin.y;

  if (! isValidNextPosition(p, x, y)) goto NoChange;

  /* do the move right */
  p->origin.x = x;
  return true;

NoChange:
  return false;
}

bool movePieceDown(TetrisPiece *p) {
  int x1=0, y1=0, x=0, y=0;
  int boardValue = 0, pieceValue = 0;

  /* dont test where we ARE, test where we are going... */
  x = p->origin.x;
  y = p->origin.y + 1;

  if (! isValidNextPosition(p, x, y)) goto FreezePiece;

  /* do the move */
  p->origin.y = y;
  return true;

FreezePiece:

  x = p->origin.x;
  y = p->origin.y;
  
  for (y1=0; y1<HEIGHT; y1++) {
    for (x1=0; x1<WIDTH; x1++) {
      boardValue = s_TetrisBoard.cell[x1][y1];
      pieceValue = 0;

      /* if the vertical alignment says we are in a piece... */
      if ( (y1 >= y) && (y1 < (y + 4)) ) {
        /* then maybe that piece is wrapping around... or maybe not...
         * but hey, unknown means we gotta do the check... so oh well */


        /* ex: (15 + 0) % 16 == 15 */
        if (x1 == ((x + 0) % WIDTH)) {
          pieceValue = (p->frames[p->index]->cell[0][y1 - y]);
        }
        /* ex: (15 + 1) % 16 == 0 */
        else
        if (x1 == ((x + 1) % WIDTH)) {
          pieceValue = (p->frames[p->index]->cell[1][y1 - y]);
        }
        /* ex: (15 + 2) % 16 == 1 */
        else
        if (x1 == ((x + 2) % WIDTH)) {
          pieceValue = (p->frames[p->index]->cell[2][y1 - y]);
        }
        /* ex: (15 + 3) % 16 == 2 */
        else
        if (x1 == ((x + 3) % WIDTH)) {
          pieceValue = (p->frames[p->index]->cell[3][y1 - y]);
        }
        else {
          /* XXX: Without this if statement, the adjacent memory of
           *      the piece is written to the screen... LOL */
          if ( (x1 >= x) && (x1 < x + 4) ) {
            pieceValue = (p->frames[p->index]->cell[x1 - x][y1 - y]);
          }
        }
      }

      /* modify the "piece" to be "frozen" */
      if (pieceValue > 0) {
        pieceValue += 10;

        /* write it down to the baord... */
        s_TetrisBoard.cell[x1][y1] = (boardValue + pieceValue);
      }

    }
  }

  /* show everybody that the piece froze */
  p->state = UNINITIALIZED;
  return false;
}

bool linesNeedToBeRemoved(TetrisBoard *b)
{
  int x1=0,y1=0,c=0;
  for (y1=0; y1<HEIGHT; y1++) { /* never git rid of the artificial floor */

    /* check that the entire row is filled */
    c=0;
    for (x1=0; x1<WIDTH; x1++) {
      if (b->cell[x1][y1] <= 0) break;
      if (b->cell[x1][y1] == 100) break; /* shouldnt happen */
      c++;
    }

    /* if filled, set to be removed... */
    if (c == WIDTH) {
      for (x1=0; x1<WIDTH; x1++) {
        b->cell[x1][y1] = 300; /* all removables turn white? */
      }

      /* let somebody know they should clean this line up */
      b->state = CLEANUP;
    }
  }
}

void removeLinesIfNeeded(TetrisBoard *b)
{
  int x1=0,y1=0,y=0;
  bool found = false;

  if (b->state != CLEANUP) return;
  b->state = GAME;

  do {
    found = false;
    for (y1=0; y1<HEIGHT; y1++) {
      for (x1=0; x1<WIDTH; x1++) {
        if (b->cell[x1][y1] == 300) {
          found = true;
          goto RemoveOneLine;
        }
      }
    }

    return;
RemoveOneLine:

    /* we just scored... btw */
    b->score++;
    if ((b->score % SCORE_PER_LEVEL) == 0) b->level++;

    for (y=y1; y>0; y--) {
      for (x1=0; x1<WIDTH; x1++) {
        /* temporarily duplicate the row above us INTO this row... */
        b->cell[x1][y] = b->cell[x1][y - 1];
      }
    }
    for (x1=0; x1<WIDTH; x1++) {
      /* just in case we are stacked to the top... */
      b->cell[x1][0] = 0;
    }

  } while (found);

  /* if here, no lines are removable... */
}

int deathBlinkCount;
void dieIfNecessary(TetrisBoard *b)
{
  int x1=0,y1=0;
  
  if (b->state != ENDGAME) return;

  if (deathBlinkCount > 6) {
    deathBlinkCount = 0;
    b->state = PREGAME;
    return;
  }

  deathBlinkCount++;
  if (deathBlinkCount % 2 == 0) {
    for (y1=0; y1<HEIGHT; y1++) {
      for (x1=0; x1<WIDTH; x1++) {
        b->cell[x1][y1] = 200;
      }
    }
  }
  else {
    for (y1=0; y1<HEIGHT; y1++) {
      for (x1=0; x1<WIDTH; x1++) {
        b->cell[x1][y1] = 201;
      }
    }
  }

  
}

/*******************************************************************************
 * Fake loop()
 ******************************************************************************/
int main()
{
  int delayTime = 100;
  aPiece.state = UNINITIALIZED;
  srand (time(NULL));

  while (1) {
    /* When the user "drops" a piece, it just moves down faster... */
    if (aPiece.state == DROPPING) delay(1);
    else {
      /* we dont get any faster after level 10 */
      if (s_TetrisBoard.level <= 10)
        delayTime = (330 - (s_TetrisBoard.level * 30));

      delay(delayTime);
    }

    /* debug console output (fake display) */
    debugClearScreen();
    debugPrintState( &s_TetrisBoard, &aPiece );

    /* show the death sequence if needed */
    dieIfNecessary(&s_TetrisBoard);

    /* it only initializes pre-game board... */
    initializeBoardIfNeeded(&s_TetrisBoard);

    /* it only initializes un-initialized pieces... */
    if (initializePieceIfNeeded(&aPiece) != 0) continue;

    /* artificial motion */
    movePieceDown(&aPiece);

    /* test for line removal */
    if (s_TetrisBoard.state == GAME) {
      if (linesNeedToBeRemoved(&s_TetrisBoard)) continue;
      /* if pieces need to be removed, state will == CLEANUP */
    }

    /* remove lines if necessary */
    removeLinesIfNeeded(&s_TetrisBoard);
  }
  return 0;
}

