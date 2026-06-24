#include "ConnectFour.h"
#include <iostream>

ConnectFour::ConnectFour()
{
    _grid = new Grid(7, 6);
}

ConnectFour::~ConnectFour()
{
    delete _grid;
}

void ConnectFour::setUpBoard()
{
    _gameOptions.rowX = 7;
    _gameOptions.rowY = 6;

    // Initialize all squares
    _grid->initializeSquares(80, "boardsquare.png");
    
    // Load the opening book
    loadOpeningBook();
    
    startGame();
}

Bit* ConnectFour::createPiece(int pieceType)
{
    Bit* bit = new Bit();

    bool isRed = (pieceType == RED_PIECE);
    bit->LoadTextureFromFile(isRed ? "red.png" : "yellow.png");
    bit->setOwner(getPlayerAt(isRed ? RED_PLAYER : YELLOW_PLAYER));
    bit->setGameTag(pieceType);
    return bit;
}

bool ConnectFour::actionForEmptyHolder(BitHolder& holder)
{
    // check if game is finishe
    if (checkForWinner() != nullptr || checkForDraw()) {
        return false;
    }

    ChessSquare* clickedSquare = static_cast<ChessSquare*>(&holder);
    // we get the column because we can only place at the bottom
    int col = clickedSquare->getColumn();

    ChessSquare* lowestSquare = getLowestEmptySquareInColumn(col);

    // column is full return false
    if (!lowestSquare) {
        return false;
    }

    // check to see whose turn it is
    int pieceType = (getCurrentPlayer()->playerNumber() == RED_PLAYER) ? RED_PIECE : YELLOW_PIECE;
    //create the piece at the top of the column
    Bit* piece = createPiece(pieceType);

    
    ChessSquare* topSquare = _grid->getSquare(col, 0);
    piece->setPosition(topSquare->getPosition());
    topSquare->setBit(piece);
    
    // this does the animation
    bitMovedFromTo(*piece, *topSquare, *lowestSquare);

    return true;
}

ChessSquare* ConnectFour::getLowestEmptySquareInColumn(int column)
{
    // starting from bottom we check if square is available
    for (int y = 5; y >= 0; y--) {
        ChessSquare* sq = _grid->getSquare(column, y);
        if (sq && !sq->bit()) {
            return sq;
        }
    }
    return nullptr;
}

bool ConnectFour::canBitMoveFrom(Bit& bit, BitHolder& src) 
{ 
    return false; 
}

bool ConnectFour::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) 
{ 
    return false; 
}

void ConnectFour::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{

    if (gameHasAI()) {
        _deferAIMove = true;
    }
    bit.moveTo(dst.getPosition());  
    
    dst.setBit(&bit);

    endTurn();
}


void ConnectFour::stopGame()
{
    // cdestroy all pieces
    _grid->forEachSquare([](ChessSquare* square, int x, int y) {
        square->destroyBit();
    });
}

// Bitboard helpers for fast win detection
inline uint64_t stateStringToBitboard(const std::string& state, char player) {
    uint64_t bb = 0;
    // loop each piece in the state string
    for (int i = 0; i < 42; i++) {
        // we only check for one player
        if (state[i] == player) {

            int col = i % 7;
            int row = i / 7;
            // magic calculation
            bb |= (1ULL << (col * 7 + row));
        }
    }
    return bb;
}

inline bool check_win(uint64_t bb) {
    uint64_t t;

    //verticla (stride = 1)
    t = bb & (bb >> 1);
    if (t & (t >> 2)) return true;

    //horizontal (stride = 7)
    t = bb & (bb >> 7);
    if (t & (t >> 14)) return true;

    //diagonal down-right (stride = 8)
    t = bb & (bb >> 8);
    if (t & (t >> 16)) return true;

    //diagonal down-left (stride = 6)
    t = bb & (bb >> 6);
    if (t & (t >> 12)) return true;

    return false;
}

Player* ConnectFour::checkForWinner()
{
    std::string state = stateString();

    uint64_t redBits = stateStringToBitboard(state, '1');
    if (check_win(redBits)) {
        return getPlayerAt(RED_PLAYER);
    }

    uint64_t yellowBits = stateStringToBitboard(state, '2');
    if (check_win(yellowBits)) {
        return getPlayerAt(YELLOW_PLAYER);
    }

    return nullptr;
}


bool ConnectFour::checkForDraw()
{
    // if there is an empty square, it's not a draw
    bool full = true;
    _grid->forEachSquare([&](ChessSquare* square, int x, int y) {
        if (!square->bit()) full = false;
    });
    return full;
}


std::string ConnectFour::initialStateString()
{
    //set all squares to 0
    return std::string(42, '0');
}

std::string ConnectFour::stateString()
{
    std::string s = "";
    //iterate through each piece of the grid
    for (int y = 0; y < 6; y++) {
        for (int x = 0; x < 7; x++) {
            Bit* bit = _grid->getSquare(x, y)->bit();
            if (bit) {
                // I am using gameTag to store the piece type (0 for empty, 1 for red, 2 for yellow)
                s += std::to_string(bit->gameTag()); 
            } else {
                s += "0";
            }
        }
    }
    return s;
}

void ConnectFour::setStateString(const std::string &s)
{
    for (int i = 0; i < (int)s.length(); i++) {
        int val = s[i] - '0';
        if (val != EMPTY) {
            //convert the correct x and y from the index
            int x = i % 7;
            int y = i / 7;
            
            Bit* bit = createPiece(val);
            _grid->getSquare(x, y)->setBit(bit);
        }
    }
}

void ConnectFour::updateAI(){

    std::string currentState = stateString();

    // scenario 1: check if a book move exists. Opening move
    int bookMove = getBookMove(currentState);
    if (bookMove != -1) {
        // if exists use it
        ChessSquare* target = getLowestEmptySquareInColumn(bookMove);
        if (target) {
            actionForEmptyHolder(*target);
            return; 
        }
    }

    // scenario 2 and 3 are in negamax.

    // get the current player number depending on who started
    int AINumber = getCurrentPlayer()->playerNumber();
    char AI = (AINumber == RED_PLAYER) ? '1' : '2';
    char Human = (AINumber == RED_PLAYER) ? '2' : '1';

    uint64_t aiBits = stateStringToBitboard(currentState, AI);
    uint64_t humanBits = stateStringToBitboard(currentState, Human);

    int bestMove = -100000;
    int bestColumn = -1;

    // attempt each column (use existing helper to find landing square)
    for (int col = 0; col < 7; col++){
        ChessSquare* landing = getLowestEmptySquareInColumn(col);
        if (!landing) continue;

        int row = landing->getRow();
        // make the move on the current state string
        int idx = row * 7 + col;
        currentState[idx] = AI;

        int bitIndex = col * 7 + row;
        uint64_t newAiBits = aiBits | (1ULL << bitIndex);

        // get opponent number
        int humanPlayerNumber = (AINumber == RED_PLAYER) ? YELLOW_PLAYER : RED_PLAYER;
        int newValue = -negamax(currentState, humanBits, newAiBits, 0, -100000, 100000, humanPlayerNumber, AINumber);
        // record the best move
        if (newValue > bestMove){
            bestColumn = col;
            bestMove = newValue;
        }
        //undo
        currentState[idx] = '0';
    }

    //play the best move
    if (bestColumn != -1) {
        ChessSquare* target = getLowestEmptySquareInColumn(bestColumn);
        if (target) {
            actionForEmptyHolder(*target);
        }
    }
}

bool aiTestForTerminalState(std::string &state){
    return (state.find('0') == std::string::npos);
}

int aiBoardEval(uint64_t AI, uint64_t human) {

    // Check if AI won
    if (check_win(AI)) return 1;

    // Check if human won
    if (check_win(human)) return -1;

    return 0;
}

int ConnectFour::negamax(std::string& state, uint64_t currentBits, uint64_t opponentBits, int depth, int alpha, int beta, int playerNumber, int aiPlayerNumber){

    uint64_t aiBits = (playerNumber == aiPlayerNumber) ? currentBits : opponentBits;
    uint64_t humanBits = (playerNumber == aiPlayerNumber) ? opponentBits : currentBits;

    int boardVal = aiBoardEval(aiBits, humanBits);
    if (boardVal != 0){
        int score = (boardVal == 1) ? 1000 - depth : -1000 + depth;


        if (playerNumber == aiPlayerNumber) {
            return score;
        } else {
            return -score;
        }
    }

    // if depth is reached we return the highest value possible fromt he board
    if ( depth >= 9) {
        char currentPlayerChar = (playerNumber == RED_PLAYER) ? '1' : '2';
        return evaluatePosition(state, currentPlayerChar);
    }

    if (aiTestForTerminalState(state)) {
        return 0; // Draw
    }

    int bestVal = -10000;
    // apparently changinf the column order improves pruning
    int columnOrder[] = {3, 2, 4, 1, 5, 0, 6};
    

    // Try each column 
    for (int i = 0; i < 7; i++){
        int col = columnOrder[i];
        // Find the lowest empty position in this column
        int dropIdx = -1; 
        int dropRow = -1;
        for (int row = 5; row >= 0; row--) {
            int idx = row * 7 + col;
            if (state[idx] == '0') {
                dropIdx = idx;
                dropRow = row;
                break;
            }
        }
        
        if (dropIdx != -1) {
            //make the move for the current player
            char currentPlayer = (playerNumber == RED_PLAYER) ? '1' : '2';
            state[dropIdx] = currentPlayer;

            int bitIndex = col * 7 + dropRow;
            uint64_t newCurrentBits = currentBits | (1ULL << bitIndex);
            
            // switch to the other player and call negamax recursively
            int nextPlayer = (playerNumber == RED_PLAYER) ? YELLOW_PLAYER : RED_PLAYER;

            int newVal = -negamax(state, opponentBits, newCurrentBits, depth+1, -beta, -alpha, nextPlayer, aiPlayerNumber);
            
            state[dropIdx] = '0';
            if (newVal > bestVal){
                bestVal = newVal;
            }
            if (bestVal > alpha) {
                alpha = bestVal;
            }
            //alpha beta pruning, if alpha is greater than beta than we cut off that branch
            if (alpha >= beta){
                break;
            }
        }
    }

    return bestVal;
}


void ConnectFour::loadOpeningBook() {

//== scenario 1: AI starts first ==
    
    // --- MOVE 1 ---
    // AI plays center
    _openingBook[std::string(42, '0')] = 3;


    // --- MOVE 2 (of AI) ---
    // AI played at index 38 (which is center)
    // We check where the Human placed their piece.

    // col 0
    std::string h0 = std::string(42, '0');
    h0[38] = '1'; // AI Base
    h0[35] = '2'; // Human Move
    _openingBook[h0] = 3; // AI plays Center

    // Col 1 
    std::string h1 = std::string(42, '0');
    h1[38] = '1'; 
    h1[36] = '2'; 
    _openingBook[h1] = 5; // AI plays col 5

    // Col 2
    std::string h2 = std::string(42, '0');
    h2[38] = '1'; 
    h2[37] = '2'; 
    _openingBook[h2] = 2; // AI plays col 2

    // col 3
    std::string h3 = std::string(42, '0');
    h3[38] = '1'; 
    h3[31] = '2';
    _openingBook[h3] = 3; // AI plays col 3

    // col 4
    std::string h4 = std::string(42, '0');
    h4[38] = '1'; 
    h4[39] = '2'; 
    _openingBook[h4] = 4; // AI plays Col 4

    // col 5
    std::string h5 = std::string(42, '0');
    h5[38] = '1'; 
    h5[40] = '2'; 
    _openingBook[h5] = 1; // AI plays Col 1

    // col 6
    std::string h6 = std::string(42, '0');
    h6[38] = '1'; 
    h6[41] = '2'; 
    _openingBook[h6] = 3; // AI plays col 3

    // third-move only specific cases

    std::string b = std::string(42, '0');
    b[38] = '1'; 
    b[31] = '2'; 
    b[24] = '1'; 
    b[17] = '2';
    _openingBook[b] = 3; 


    // -- scenario 2: AI second--
    
    b = std::string(42, '0'); 
    b[38]='1';
    _openingBook[b] = 3;


    int firstmoves[] = {35, 36, 37, 39, 40, 41}; 
    for (int idx : firstmoves) {
        b = std::string(42, '0'); b[idx]='1';
        _openingBook[b] = 3;
    }

    // AI second move

    b = std::string(42, '0'); b[38]='1'; b[31]='2'; b[24]='1';
    _openingBook[b] = 3; 

    b = std::string(42, '0'); b[38]='1'; b[31]='2'; b[37]='1';
    _openingBook[b] = 4; 

    b = std::string(42, '0'); b[38]='1'; b[31]='2'; b[39]='1';
    _openingBook[b] = 2;

    b = std::string(42, '0'); b[38]='1'; b[31]='2'; b[35]='1';
    _openingBook[b] = 2;

    b = std::string(42, '0'); b[38]='1'; b[31]='2'; b[36]='1';
    _openingBook[b] = 2; 

    b = std::string(42, '0'); b[38]='1'; b[31]='2'; b[40]='1';
    _openingBook[b] = 4;

    b = std::string(42, '0'); b[38]='1'; b[31]='2'; b[41]='1';
    _openingBook[b] = 4;

}

// Helper to look up moves
int ConnectFour::getBookMove(const std::string& state) {
    if (_openingBook.count(state)) {
        return _openingBook[state];
    }
    return -1; // Not found
}


// most valuable positions to hold
static int SCORE_TABLE[42] = {
    3, 4, 5, 7, 5, 4, 3,  // Row 0 (Top)
    4, 6, 8, 10, 8, 6, 4,
    5, 8, 11, 13, 11, 8, 5,
    5, 8, 11, 13, 11, 8, 5,
    4, 6, 8, 10, 8, 6, 4,
    3, 4, 5, 7, 5, 4, 3   // Row 5 (Bottom)
};

int ConnectFour::evaluatePosition(const std::string& state, char playerChar) {
    int score = 0;
    char opponentChar = (playerChar == '1') ? '2' : '1';

    for (int i = 0; i < 42; i++) {
        if (state[i] == playerChar) {
            score += SCORE_TABLE[i];
        } else if (state[i] == opponentChar) {
            score -= SCORE_TABLE[i];
        }
    }
    return score;
}



