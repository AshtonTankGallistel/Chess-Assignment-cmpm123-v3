#pragma once
#include "Game.h"
#include "ChessSquare.h"

const int pieceSize = 64;

enum ChessPiece {
    NoPiece = 0,
    Pawn = 1,
    Knight,
    Bishop,
    Rook,
    Queen,
    King
};

//GAMESTATE STRUCT. USED FOR MAIN GAMESTATE, AND AI SEARCHING GAMESTATES
struct ChessState{
    bool canCastle[4]; //{leftWhite,RightWhite,LeftBlack,RightBlack}
    int EnPassantSpace; //The space where an EnPassant can be performed. -1 means there's no space to do so. 
                            //There can only ever be 1 enPassant-able space at a time, hence it being an int.
    int halfMoves; //num of moves since the last time a pawn moved or a piece was captured (1 per player)
    int totalMoves; //num of moves total (1 per both players)
    int myBoard[8][8]; //Current state of board
    ChessState(); //constructor
    ChessState(const ChessState& oldState); //copy/duplicate state, used by AI to get current state for itself
};

struct ChessAI
{
    ChessState* myState;
    int evaluateBoard();
    int negamax(ChessAI* state, int depth, int playerColor, int alpha, int beta);
    int ownerAt(int index ) const;
    int AICheckForWinner();
    //UPDATES ARRAY BASED ON INPUTTED MOVE, RETURNS GAMETAG OF CAPTURED PIECE (0 IF NONE)
    int performMoveOnArray(int bit, int move, int targetArray[8][8]);
    //UNDOES CHANGE THAT WOULD HAVE BEEN PERFORMED BY MOVE.
    //DOES NOT UNDO CASTLE/E.P.SPACE CHANGES, MUST BE TRACKED ELSEWHERE
    void unperformMove(int bit, int move, int targetArray[8][8], int capturedPiece);

    int AIPlayerNumber; //which player is the AI
};
//HELPER FUNCTION FOR FINDING MOVES FOR A PIECE USING AN INT REPRESENTATION OF THE BOARD
std::vector<int>*   getPossibleMovesFromArray(int bit, int board[8][8], ChessState& cState);
//HELPER FUNCTION FOR GETPOSSIBLEMOVESFROMARRAY. GETS MOVES FOR BISHOP/ROOK/QUEEN, WHICH USE THE SAME MATH
std::vector<int>*   getStraightPaths(int bit, int board[8][8]);

//
// the main game class
//
class Chess : public Game
{
public:
    Chess();
    ~Chess();

    // set up the board
    void        setUpBoard() override;

    Player*     checkForWinner() override;
    bool        checkForDraw() override;
    std::string initialStateString() override;
    std::string stateString() override;
    void        setStateString(const std::string &s) override;
    bool        actionForEmptyHolder(BitHolder& holder) override;
    bool        canBitMoveFrom(Bit& bit, BitHolder& src) override;
    bool        canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) override;
    void        bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst) override;
    //HELPER FUNCTION FOR ABOVE 4 FUNCS, FINDS OUT WHICH MOVES A PIECE CAN PERFORM
    std::vector<int>*   getPossibleMoves(Bit &bit, BitHolder &src);
    //HELPER FUNCTION, CLEARS ALL HIGHLIGHTS
    void        clearHighlights();
    //MOVE GENERATOR FUNCTION. GETS ALL POSSIBLE MOVES FOR THE CURRENT TURN
    void        generateMoves();
    //BOARD SETUP FUNCTION. SETS UP BOARD BASED ON FEN STRING
    void        FENtoBoard(std::string FEN);
    //HELPER FUNCTION, SETS A PIECE ONTO THE BOARD
    void        setPiece(int pos, int player, ChessPiece piece);
    //HELPER FUNCTION, CONVERTS BOARD TO AN int[8][8]
    void        boardToArray(int targetArray[8][8]);

    //makes an Ai.
    ChessAI*    clone(int AInum);

    void        stopGame() override;
    BitHolder& getHolderAt(const int x, const int y) override { return _grid[y][x]; }

	void        updateAI() override;
    bool        gameHasAI() override { return true; }

    
private:
    Bit *       PieceForPlayer(const int playerNumber, ChessPiece piece);
    const char  bitToPieceNotation(int row, int column) const;

    ChessSquare      _grid[8][8];

    //ADDED VAR, TRACKS WHICH INDEXES THE PIECE CAN BE MOVED TO. MAX OF 28 (A piece can only have 27 spots total, so that+1)
    //ENDS AT -1.
    int         possibleMoves[28];
    std::vector<int>* potentialMoves[64];

    //MAIN GAMESTATE VAR. TRACKS THE CURRENT GAMESTATE.
    ChessState* myState;
};