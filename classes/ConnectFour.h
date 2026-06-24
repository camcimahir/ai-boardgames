#pragma once

#include "Game.h"
#include "Grid.h"

class ConnectFour : public Game
{
public:
    ConnectFour();
    ~ConnectFour();

    // Required virtual methods from Game base class
    void    setUpBoard() override;
    Player* checkForWinner() override;
    bool    checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void    setStateString(const std::string &s) override;
    bool    actionForEmptyHolder(BitHolder &holder) override;
    bool    canBitMoveFrom(Bit &bit, BitHolder &src) override;
    bool    canBitMoveFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    void    stopGame() override;
    void    bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override; // Not used in C4, but required by base

    // AI methods
    void    updateAI() override;
    bool    gameHasAI() override { 
        if (_players.size() >= 2) {
            return _players[0]->isAIPlayer() || _players[1]->isAIPlayer();
        }
        return false;
    }
    Grid* getGrid() override { return _grid; }
    int   negamax(std::string& state, uint64_t currentBits, uint64_t opponentBits, int depth, int alpha, int beta, int playerNumber, int aiPlayerNumber);

private:
    // Constants for piece types (similar to Checkers)
    static const int EMPTY = 0;
    static const int RED_PIECE = 1;
    static const int YELLOW_PIECE = 2;

    // Player constants
    static const int RED_PLAYER = 0;
    static const int YELLOW_PLAYER = 1;

    // Grid
    Grid* _grid;
    bool _deferAIMove = false;

    // Helper methods
    ChessSquare* getLowestEmptySquareInColumn(int column);
    Bit* createPiece(int pieceType);

    std::unordered_map<std::string, int> _openingBook;
    void loadOpeningBook();
    int getBookMove(const std::string& state);

    int evaluatePosition(const std::string& state, char playerChar);
};

