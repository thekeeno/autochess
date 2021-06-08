/***************************************************************************/
/* Automatic Chessboard System
/* A. Orr 2021                                                              
/* Adapted heavily from rom3's implementation of micro-Max                 */
/* Ported chess program to Arduino UNO by rom3                             */
/* Based on:         micro-Max,       version 4.8                          */
/* A chess program smaller than 2KB (of non-blank source), by H.G. Muller  */
/* Chessuino by Diego Cueva                                                */ 
/* Atmel ATMega644 and AVR GCC, by Andre Adrian                            */
/* May 12 2019        Germany                                              */
/***************************************************************************/


#include <AccelStepper.h>

#include "ChessAxis.h"


#include "ramps.h"

// Define motor interface type
// In this case, 1 refers to a standard step/dir pin arrangement.


//AccelStepper XStepper(motorInterfaceType, X_STEP_PIN, X_DIR_PIN);
//AccelStepper YStepper(motorInterfaceType, Y_STEP_PIN, Y_DIR_PIN);


#define grid_size_steps 90



ChessAxis XAxis(X_STEP_PIN, X_DIR_PIN, X_ENABLE_PIN,X_MAX_PIN,X_MIN_PIN, false);
//ChessAxis YAxis(Y_STEP_PIN, Y_DIR_PIN, Y_ENABLE_PIN,Y_MAX_PIN,Y_MIN_PIN, true);







#define M 0x88
#define S 128
#define I 8000

#define STRENGTH 0x10


#define MYRAND_MAX 65535     /* 16bit pseudo random generator */
long  N, T;                  /* N=evaluated positions+S, T=recursion limit */
short Q, O, K, R, k=16;      /* k=moving side */
char *p, c[5], Z;            /* p=pointer to c, c=user input, computer output, Z=recursion counter */

char enPassantRow = 0;
char enPassantCol = 0;

char L,
w[]={0,2,2,7,-1,8,12,23},                             /* relative piece values    */
o[]={-16,-15,-17,0,1,16,0,1,16,15,17,0,14,18,31,33,0, /* step-vector lists */
     7,-1,11,6,8,3,6,                                 /* 1st dir. in o[] per piece*/
     6,3,5,7,4,5,3,6};                                /* initial piece setup      */
/* board is left part, center-pts table is right part, and dummy */  

char board[16*8+1];

const char boardInit[16*8+1]={     
  22, 19, 21, 23, 20, 21, 19, 22, 28, 21, 16, 13, 12, 13, 16, 21,
  18, 18, 18, 18, 18, 18, 18, 18, 22, 15, 10,  7,  6,  7, 10, 15,
   0,  0,  0,  0,  0,  0,  0,  0, 18, 11,  6,  3,  2,  3,  6, 11,
   0,  0,  0,  0,  0,  0,  0,  0, 16,  9,  4,  1,  0,  1,  4,  9,
   0,  0,  0,  0,  0,  0,  0,  0, 16,  9,  4,  1,  0,  1,  4,  9,
   0,  0,  0,  0,  0,  0,  0,  0, 18, 11,  6,  3,  2,  3,  6, 11,
   9,  9,  9,  9,  9,  9,  9,  9, 22, 15, 10,  7,  6,  7, 10, 15,
  14, 11, 13, 15, 12, 13, 11, 14, 28, 21, 16, 13, 12, 13, 16, 21, 0
};


unsigned int seed=0;
uint32_t  byteBoard[8];

char sym[17] = {".?pnkbrq?P?NKBRQ"};
int moveNumber;
int plyNumber;
char lastWhite[5], lastBlack[5];
unsigned short ledv=1;

String inputString = "";
bool stringComplete = false;  // whether the string is complete

bool whiteAI = false;
bool blackAI = false;

void setup() {
  Serial.begin(115200);
  //mainSetup();
  //XAxis.init();

  
  XAxis.init();
 
  XAxis.motor.moveTo(5000);


}

void mainSetup(){
  Serial.println("*** MicroMaxChess ***");
  
  Serial.println("White AI or Human? y/n:");
  getSerialLine();
  if(inputString.charAt(0) == 'y'){
    Serial.println("White AI.");
    whiteAI = true;
  }
  else{
    Serial.println("White Human.");
  }

  Serial.println("Black AI or Human? y/n:");
  getSerialLine();
  if(inputString.charAt(0) == 'y'){
    Serial.println("Black AI.");
    blackAI = true;
  }
  else{
    Serial.println("Black Human.");
  }


  
  resetGame();
  serialBoard();
}

void getSerialLine(){
  inputString = "";
  while (stringComplete == false) {
    getserialchar();
  }
  stringComplete = false;
}

void loop()
{
    // If at the end of travel go to the other end
    if (XAxis.motor.distanceToGo() == 0)
      XAxis.motor.moveTo(-XAxis.motor.currentPosition());

    

    XAxis.run();
}

void updateSubCharacters(){
  c[0] = inputString.charAt(0);
  c[1] = inputString.charAt(1);
  c[2] = inputString.charAt(2);
  c[3] = inputString.charAt(3);
  c[4] = 0;
}

void setupAIMove(){
  if (k==16){Serial.println("White AI move:");}
  else{Serial.println("Black AI move: ");}
      K = I; //AI to move.
      L = 0; //maybe not needed.
      N = 0; //maybe not needed.
      T = STRENGTH;

}

void checkForGameOver(){
  int r = D(-I, I, Q, O, 1, 3); 
  if ( !(r > -I + 1) ) {
    Serial.println("Game Over.");
  }
}

bool setupHumanMove(){
  if (k==16){Serial.println("White to move:");}
  else{Serial.println("Black to move: ");}
      
      getSerialLine();
      Serial.print(moveNumber);
      Serial.print(". ");
      Serial.print(inputString.substring(0,4));
      updateSubCharacters();
      if (c[0] == "r"){
        resetGame;
        return false;
      }
      // clear the string:

      Serial.print(" Thinking... ");                       

      K = *c - 16 * c[1] + 799;
      L = c[2] - 16 * c[3] + 799; /* parse entered move */
      N = 0;
      return true;
}

void runSinglePly(){
  int r;
  bool waitingForValidMove;
  bool AI;
  int lastk;

  lastk = k;
  AI = (k == 16 ? whiteAI : blackAI);
  waitingForValidMove = true;  
  while(waitingForValidMove){
    if (AI){
      setupAIMove();
    }
    else{setupHumanMove();}

    r = D(-I, I, Q, O, 1, 3);      //attempts to move

    if ( !(r > -I + 1) ) {
      Serial.println("Lose ");
      crash();
    }
    if (k == lastk) {                          
      Serial.println("Move not valid.");
    }
    else{
      waitingForValidMove = false;
      Serial.println("Move valid.");
    }
  }
  serialBoard();
  if(k == 16){strcpy(lastWhite, c);}
  else{strcpy(lastBlack, c);}
  checkForGameOver();
  plyNumber++;
  if(k == 8){moveNumber++; }

}


/* User interface routines */

void getserialchar() {
  while (Serial.available() > 0) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}
unsigned short myrand(void) {
  unsigned short r = (unsigned short)(seed % MYRAND_MAX);
  return r = ((r << 11) + (r << 7) + r) >> 1;
}
/* recursive minimax search */
/* (q,l)=window, e=current eval. score, */
/* E=e.p. sqr.z=prev.dest, n=depth; return score */
short D(short q, short l, short e, unsigned char E, unsigned char z, unsigned char n) {
  short m, v, i, P, V, s;
  unsigned char t, p, u, x, y, X, Y, H, B, j, d, h, F, G, C;
  signed char r;
  if (++Z > 30) {                                   /* stack underrun check */
    --Z; return e;
  }
  q--;                                          /* adj. window: delay bonus */
  k ^= 24;                                      /* change sides             */
  d = Y = 0;                                    /* start iter. from scratch */
  X = myrand() & ~M;                            /* start at random field    */
  while(d++ < n || d < 3 ||                         /* iterative deepening loop */
    z & K == I && (N < T & d < 98 ||            /* root: deepen upto time   */
                   (K = X, L = Y & ~M, d = 3)))                /* time's up: go do best    */
  { x = B = X;                                   /* start scan at prev. best */
    h = Y & S;                                   /* request try noncastl. 1st*/
    P = d < 3 ? I : D(-l, 1 - l, -e, S, 0, d - 3); /* Search null move         */
    m = -P < l | R > 35 ? d > 2 ? -I : e : -P;   /* Prune or stand-pat       */
    ++N;                                         /* node count (for timing)  */
    do {
      u = board[x];                                   /* scan board looking for   */
      if (u & k) {                                /*  own piece (inefficient!)*/
        r = p = u & 7;                             /* p = piece type (set r>0) */
        j = o[p + 16];                             /* first step vector f.piece*/
        while(r = p > 2 & r < 0 ? -r : -o[++j])        /* loop over directions o[] */
        { A:                                        /* resume normal after best */
          y = x; F = G = S;                         /* (x,y)=move, (F,G)=castl.R*/
          do {                                      /* y traverses ray, or:     */
            H = y = h ? Y ^ h : y + r;               /* sneak in prev. best move */
            if (y & M)break;                         /* board edge hit           */
            m = E - S & board[E] && y - E < 2 & E - y < 2 ? I : m; /* bad castling             */
            if (p < 3 & y == E)H ^= 16;              /* shift capt.sqr. H if e.p.*/
            t = board[H]; if (t & k | p < 3 & !(y - x & 7) - !t)break; /* capt. own, bad pawn mode */
            i = 37 * w[t & 7] + (t & 192);           /* value of capt. piece t   */
            m = i < 0 ? I : m;                       /* K capture                */
            if (m >= l & d > 1)goto C;               /* abort on fail high       */
            v = d - 1 ? e : i - p;                   /* MVV/LVA scoring          */
            if (d - !t > 1)                          /* remaining depth          */
            { v = p < 6 ? board[x + 8] - board[y + 8] : 0;    /* center positional pts.   */
              board[G] = board[H] = board[x] = 0; board[y] = u | 32;  /* do move, set non-virgin  */
              if (!(G & M))board[F] = k + 6, v += 50;     /* castling: put R & score  */
              v -= p - 4 | R > 29 ? 0 : 20;           /* penalize mid-game K move */
              if (p < 3)                              /* pawns:                   */
              { v -= 9 * ((x - 2 & M || board[x - 2] - u) + /* structure, undefended    */
                          (x + 2 & M || board[x + 2] - u) - 1  /*        squares plus bias */
                          + (board[x ^ 16] == k + 36))          /* kling to non-virgin King */
                     - (R >> 2);                       /* end-game Pawn-push bonus */
                V = y + r + 1 & S ? 647 - p : 2 * (u & y + 16 & 32); /* promotion or 6/7th bonus */
                board[y] += V; i += V;                     /* change piece, add score  */
              }
              v += e + i; V = m > q ? m : q;          /* new eval and alpha       */
              C = d - 1 - (d > 5 & p > 2 & !t & !h);
              C = R > 29 | d < 3 | P - I ? C : d;     /* extend 1 ply if in check */
              do
                s = C > 2 | v > V ? -D(-l, -V, -v,     /* recursive eval. of reply */
                                       F, 0, C) : v;    /* or fail low if futile    */
              while(s > q&++C < d); v = s;
              if (z && K - I && v + I && x == K & y == L) /* move pending & in root:  */
              { Q = -e - i; O = F;                     /*   exit if legal & found  */
                R += i >> 7; --Z; return l;            /* captured non-P material  */
              }
              board[G] = k + 6; board[F] = board[y] = 0; board[x] = u; board[H] = t; /* undo move,G can be dummy */
            }
            if (v > m)                               /* new best, update max,best*/
              m = v, X = x, Y = y | S & F;            /* mark double move with S  */
            if (h) {
              h = 0;  /* redo after doing old best*/
              goto A;
            }
            if (x + r - y | u & 32 |                 /* not 1st step,moved before*/
                p > 2 & (p - 4 | j - 7 ||             /* no P & no lateral K move,*/
                         board[G = x + 3 ^ r >> 1 & 7] - k - 6     /* no virgin R in corner G, */
                         || board[G ^ 1] | board[G ^ 2])               /* no 2 empty sq. next to R */
               )t += p < 5;                           /* fake capt. for nonsliding*/
            else F = y;                              /* enable e.p.              */
          } while(!t);                                  /* if not capt. continue ray*/
        }
      }
    } while((x = x + 9 & ~M) - B);                 /* next sqr. of board, wrap */
C: if (m > I - M | m < M - I)d = 98;           /* mate holds to any depth  */
    m = m + I | P == I ? m : 0;                  /* best loses K: (stale)mate*/
    if (z && d > 2)
    { *c = 'a' + (X & 7); c[1] = '8' - (X >> 4); c[2] = 'a' + (Y & 7); c[3] = '8' - (Y >> 4 & 7); c[4] = 0;
      char buff[150];
    }
  }                                             /*    encoded in X S,8 bits */
  k ^= 24;                                      /* change sides back        */
  --Z; 
  enPassantRow = 'a' + (E & 7);
  enPassantCol = '8' - (E >> 4 ); //store enpassant data
  return m += m < e;                       /* delayed-loss bonus       */
}

void crash() {
  for (;;);
}

void serialBoard(){
  Serial.println("  +-----------------+");
  for(int i=0; i<8; i++){
        Serial.print(' ');
        Serial.print(8-i);
        Serial.print("| ");
        for(int j=0; j<8; j++){
            char c = sym[board[16*i+j]&15];
            Serial.print(c);
            Serial.print(' ');
        }
        Serial.println('|');
  }
  Serial.println("  +-----------------+");
  Serial.println("    a b c d e f g h");
  boardToFen();
  updateBitBoard();
}

void resetBoard() {
  for (int i = 0; i < 16 * 8 + 1; i++) {
    board[i] = boardInit[i];
  }
}

void resetGame(){
  resetBoard();
  lastWhite[0] = 0; //last white movement.
  lastBlack[0] = 0; //last white movement.
  moveNumber = 1;
  plyNumber = 1;
}


void boardToFen(){
  int emptySpaceCounter = 0;
  char buf[150] = {'\0'};
  strcat(buf, "FEN: ");
  


  


  for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            char c = sym[board[16*i+j]&15];
            if(c == '.'){
              emptySpaceCounter++;
            }
            else{
              if(emptySpaceCounter){
                sprintf(buf, "%s%i", buf, emptySpaceCounter);
                emptySpaceCounter = 0;
              }
              sprintf(buf, "%s%c", buf, c);
            }
            
        }
        if(emptySpaceCounter){
          sprintf(buf, "%s%i", buf, emptySpaceCounter);
          emptySpaceCounter = 0;
        }
        if (i < 7){sprintf(buf, "%s%c", buf, '/');}
  }

  strcat(buf, " ");
  char toMove = (k == 16 ? 'w' : 'b');
  sprintf(buf, "%s%c", buf, toMove);

  sprintf(buf, "%s%c", buf, ' ');

//castling

  bool noCastleFlag = true;

  

  bool bqrk = (board[16*0+0] == 0b00010110);
  bool bkrk = (board[16*0+7] == 0b00010110);
  bool bk = (board[16*0+4] ==   0b00010100);


  bool wqrk = (board[16*7+0] == 0b00001110);
  bool wkrk = (board[16*7+7] == 0b00001110);
  bool wk = (board[16*7+4] ==   0b00001100);


  if(wk && wkrk){sprintf(buf, "%s%c", buf, 'K');noCastleFlag = false;}
  if(wk && wqrk){sprintf(buf, "%s%c", buf, 'Q');noCastleFlag = false;}
  if(bk && bkrk){sprintf(buf, "%s%c", buf, 'k');noCastleFlag = false;}
  if(bk && bqrk){sprintf(buf, "%s%c", buf, 'q');noCastleFlag = false;}

  if(noCastleFlag){
    sprintf(buf, "%s%c", buf, '-');
  }


//still needs en passant and clocks

  Serial.println(buf);
  Serial.println(enPassantRow);
  Serial.println(enPassantCol);  //en passant still does not work
}

void lastMoveToCoordinates(char move[5], bool after, bool axis){
  int i = ~axis + 2*after;
  //Serial.print(char[i]);
    
}



//todo 
//finish FEN implementation
//convert to algebra notation
//bitboard pointer function
//last move to coords function to get stepper motor to right place.
//proper resetting.


void updateBitBoard(){
  uint64_t bitBoard = 0;
  bool occupied;
  for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
          int index = 16*i+j;
          if(board[index]){}
            
            bitBoard |= (1 << index);
          }
          Serial.print(occupied);

        }
        Serial.println("");

  
  

  }