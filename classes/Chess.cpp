#include "Chess.h"
#include "Evaluate.h"

const int AI_PLAYER = 1;
const int HUMAN_PLAYER = -1;

Chess::Chess()
{
    //set basic gameState. may end up unnessecary depending on how it's handled elsewhere
    //This acts moreso as a safety net in case said handling is not called.
    //update: needed to rehaul it because vs failed to display the error message for a half hour. fun!
    myState = new ChessState();
}

ChessState::ChessState(){
    for(int i = 0; i < 4; i++){ // 4 = num of bools in canCastle
        canCastle[i] = true;
    }
    EnPassantSpace = -1;
    halfMoves = 0;
    totalMoves = 0;
}

ChessState::ChessState(const ChessState& oldState){
    for(int i = 0; i < 4; i++){ // 4 = num of bools in canCastle
        canCastle[i] = oldState.canCastle[i];
    }
    EnPassantSpace = oldState.EnPassantSpace;
    halfMoves = oldState.halfMoves;
    totalMoves = oldState.totalMoves;
    for(int i = 0; i < 64; i++){
        myBoard[i/8][i%8] = oldState.myBoard[i/8][i%8];
    }
}

Chess::~Chess()
{
}

//
// make a chess piece for the player
//
Bit* Chess::PieceForPlayer(const int playerNumber, ChessPiece piece)
{
    const char* pieces[] = { "pawn.png", "knight.png", "bishop.png", "rook.png", "queen.png", "king.png" };

    // depending on playerNumber load the "x.png" or the "o.png" graphic
    Bit* bit = new Bit();
    // should possibly be cached from player class?
    const char* pieceName = pieces[piece - 1];
    std::string spritePath = std::string("chess/") + (playerNumber == 0 ? "w_" : "b_") + pieceName;
    bit->LoadTextureFromFile(spritePath.c_str());
    bit->setOwner(getPlayerAt(playerNumber));
    bit->setSize(pieceSize, pieceSize);

    return bit;
}

void Chess::setUpBoard()
{
    setNumberOfPlayers(2);
    //In the event of a restart:
    if(getCurrentPlayer()->playerNumber() == 1){
        endTurn();
    }
    _gameOptions.rowX = 8;
    _gameOptions.rowY = 8;
    //
    // we want white to be at the bottom of the screen so we need to reverse the board
    //
    char piece[2];
    piece[1] = 0;
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            ImVec2 position((float)(pieceSize * x + pieceSize), (float)(pieceSize * (_gameOptions.rowY - y) + pieceSize));
            _grid[y][x].initHolder(position, "boardsquare.png", x, y);
            _grid[y][x].setGameTag(0);
            piece[0] = bitToPieceNotation(y,x);
            _grid[y][x].setNotation(piece);
        }
    }

    //setup pieces. 0=W,1=B
    FENtoBoard("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    for(int i = 0; i < 4; i++){ // 4 = num of bools in canCastle
        myState->canCastle[i] = true;
    }
    //FENtoBoard("5k2/8/8/8/8/8/8/4K2R w K - 0 1"); // white can castle
    //FENtoBoard("3k4/8/8/8/8/8/8/R3K3 w Q - 0 1"); // white can castle queen side
    //FENtoBoard("r3k2r/1b4bq/8/8/8/8/7B/R3K2R w KQkq - 0 1"); // white can castle both sides
    //FENtoBoard("2K2r2/4P3/8/8/8/8/8/3k4 w - - 0 1"); // white can promote to queen
    //FENtoBoard("4k3/1P6/8/8/8/8/K7/8 w - - 0 1"); // white can promote to queen

    boardToArray(myState->myBoard);
    //generate moves for starting player
    generateMoves();
}

//
// about the only thing we need to actually fill out for tic-tac-toe
//
bool Chess::actionForEmptyHolder(BitHolder &holder)
{
    return false;
}

bool Chess::canBitMoveFrom(Bit &bit, BitHolder &src)
{
    //can only move if it's the current player's piece. the check for this should have been done in generateMoves()
    ChessSquare& srcSquare = static_cast<ChessSquare&>(src);
    bool movable = false;
    clearHighlights(); //clear highlights from prev move
    //std::cout<< potentialMoves[srcSquare.getSquareIndex()]->size() <<std::endl;
    if(potentialMoves[srcSquare.getSquareIndex()] != nullptr){
        movable= true;
        for(int i = 0; i < potentialMoves[srcSquare.getSquareIndex()]->size(); i++){
            int spot = (*potentialMoves[srcSquare.getSquareIndex()])[i] % 128; //remove 128 from special moves
            _grid[spot/8][spot%8].setMoveHighlighted(true);
        }
    }
    return movable;
}

bool Chess::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst)
{
    ChessSquare& srcSquare = static_cast<ChessSquare&>(src);
    ChessSquare& dstSquare = static_cast<ChessSquare&>(dst);
    for (int i =0; i < potentialMoves[srcSquare.getSquareIndex()]->size(); i++){
        if((*potentialMoves[srcSquare.getSquareIndex()])[i] % 128 == dstSquare.getSquareIndex()){
            return true;
        }
    }
    return false;
}

void Chess::bitMovedFromTo(Bit &bit, BitHolder &src, BitHolder &dst)
{

    //update state/promote
    ChessSquare& srcSquare = static_cast<ChessSquare&>(src);
    ChessSquare& dstSquare = static_cast<ChessSquare&>(dst);
    //PROMOTE
    std::cout<<"hello"<<srcSquare.getSquareIndex()<<std::endl;
    if(bit.gameTag() % 128 == Pawn && (dstSquare.getRow() == 0 || dstSquare.getRow() == 7)){
        setPiece(dstSquare.getSquareIndex(), bit.gameTag() / 128, Queen);
    }
    //STATE UPDATE
    //EN PASSANT
    if(bit.gameTag() % 128 == Pawn && dstSquare.getSquareIndex() == myState->EnPassantSpace){
        if(bit.gameTag()/128 == 0){ //W capturing B
            std::cout<<dstSquare.getRow() + 1<<std::endl;
            _grid[dstSquare.getRow() - 1][dstSquare.getColumn()].destroyBit();
        }
        else{
            _grid[dstSquare.getRow() + 1][dstSquare.getColumn()].destroyBit();
        }
    }
    myState->EnPassantSpace = -1;
    if(bit.gameTag() % 128 == Pawn && (srcSquare.getRow() - dstSquare.getRow()) % 2 == 0){ //when a pawn jumps by 2
        myState->EnPassantSpace = srcSquare.getSquareIndex() + 8 - (16 * (bit.gameTag()/128));//up a row if W, down a row if B
    }
    //CASTLING
    //below statements always ran, since they start true and can't become true again
    switch(srcSquare.getSquareIndex()){ //when the piece moves
        //towers
        case 0:
            myState->canCastle[0] = false; break;
        case 7:
            myState->canCastle[1] = false; break;
        case 56:
            myState->canCastle[2] = false; break;
        case 63:
            myState->canCastle[3] = false; break;
        //kings
        case 4: //W
            //if move left, queenside
            if(dstSquare.getSquareIndex() == 2 && myState->canCastle[0] == true){
                //move the tower piece
                _grid[0][3].setBit(_grid[0][0].bit());
                _grid[0][3].bit()->setPosition(_grid[0][3].getPosition());
                _grid[0][0].setBit(nullptr);
            }//if right, kingside
            else if(dstSquare.getSquareIndex() == 6 && myState->canCastle[1] == true){
                _grid[0][5].setBit(_grid[0][7].bit());
                _grid[0][5].bit()->setPosition(_grid[0][5].getPosition());
                _grid[0][7].setBit(nullptr);
            }
            myState->canCastle[0] = false;
            myState->canCastle[1] = false;
            break;
        case 60: //B
            //if move left, queenside
            if(dstSquare.getSquareIndex() == 58 && myState->canCastle[2] == true){
                //move the tower piece
                _grid[7][3].setBit(_grid[7][0].bit());
                _grid[7][3].bit()->setPosition(_grid[7][3].getPosition());
                _grid[7][0].setBit(nullptr);
            }//if right, kingside
            else if(dstSquare.getSquareIndex() == 62 && myState->canCastle[3] == true){
                _grid[7][5].setBit(_grid[7][7].bit());
                _grid[7][5].bit()->setPosition(_grid[7][5].getPosition());
                _grid[7][7].setBit(nullptr);
            }
            myState->canCastle[2] = false;
            myState->canCastle[3] = false;
            break;
    }
    switch(dstSquare.getSquareIndex()){ //when the piece is captured
        //towers
        case 0:
            myState->canCastle[0] = false; break;
        case 7:
            myState->canCastle[1] = false; break;
        case 56:
            myState->canCastle[2] = false; break;
        case 63:
            myState->canCastle[3] = false; break;
        //kings
        case 4: //W
            myState->canCastle[0] = false; break;
            myState->canCastle[1] = false; break;
        case 59: //B
            myState->canCastle[2] = false; break;
            myState->canCastle[3] = false; break;
    }
    //MOVE COUNTER
    myState->halfMoves += 1;
    myState->totalMoves = myState->halfMoves / 2;
    //Update array rep in state
    boardToArray(myState->myBoard);
    //wrap up turn
    clearHighlights(); //clear highlights from move
    endTurn();
    generateMoves(); //generate moves for new current player
    //if AI, run as start of following turn
    if (gameHasAI() && getCurrentPlayer() && getCurrentPlayer()->isAIPlayer()) 
    {
        updateAI();
    }
}

//helper function, records what moves the piece can perform into the possibleMoves variable.
//returns how many moves there are.
std::vector<int>* Chess::getPossibleMoves(Bit &bit, BitHolder &src){
    ChessSquare& srcSquare = static_cast<ChessSquare&>(src);
    int moveCount = 0;
    std::vector<int>* moveList = new std::vector<int>;
    switch (bit.gameTag() % 128) // get remainder after 128 to get piece num
    {
    case Pawn:
    {
        int direction = 1;
        //0=W,1=B
        if(bit.gameTag() / 128 == 1){
            direction *= -1; //B goes down, W goes up
        }
        //add the 1 move forward(s)
        if(0 <= srcSquare.getRow() + direction && 7 >= srcSquare.getRow() + direction){
            if(_grid[srcSquare.getRow() + direction][srcSquare.getColumn()].empty()){
                possibleMoves[moveCount] = 8*(srcSquare.getRow() + direction) + srcSquare.getColumn();
                moveList->push_back(8*(srcSquare.getRow() + direction) + srcSquare.getColumn());
                moveCount += 1;
            }

            //add diagonals (done here since we know there's room vertically)
            for(int i=-1;i<=1;i+=2){
                if(0 <= srcSquare.getColumn() + i && 7 >= srcSquare.getColumn() + i){
                    if((!_grid[srcSquare.getRow() + direction][srcSquare.getColumn() + i].empty() //diagonal for piece capture only
                    && _grid[srcSquare.getRow() + direction][srcSquare.getColumn() + i].bit()->gameTag() / 128 != bit.gameTag() / 128)
                    || myState->EnPassantSpace == 8*(srcSquare.getRow() + direction) + srcSquare.getColumn() + i){ //..or en passant
                        possibleMoves[moveCount] = 8*(srcSquare.getRow() + direction) + srcSquare.getColumn() + i;
                        moveList->push_back(8*(srcSquare.getRow() + direction) + srcSquare.getColumn() + i);
                        moveCount += 1;
                    }
                }
            }
        }
        //add the 2 move forward
        if(srcSquare.getRow() == 1 + 5 * (bit.gameTag() / 128)){ //2 when W, 6 when B
            if(0 <= srcSquare.getRow() + 2*direction && 7 >= srcSquare.getRow() + 2*direction
                && _grid[srcSquare.getRow() + 2*direction][srcSquare.getColumn()].empty()
                && _grid[srcSquare.getRow() + direction][srcSquare.getColumn()].empty()){ //only when empty space between
                    possibleMoves[moveCount] = 8*(srcSquare.getRow() + 2*direction) + srcSquare.getColumn();
                    moveList->push_back(8*(srcSquare.getRow() + 2*direction) + srcSquare.getColumn());
                    moveCount += 1;
            }
        }
    }
        break;
    case Knight:
    {
        int directions[4] = {-2,-1,1,2};
        for(int x : directions){
            for(int y : directions){
                if(abs(x) == abs(y)){ //skip when two 2s or two 1s; it should always be a 2 and a 1
                    continue;
                }
                //skip when out of range
                if(0>srcSquare.getRow() + x || 7<srcSquare.getRow() + x
                    || 0>srcSquare.getColumn() + y || 7<srcSquare.getColumn() + y){
                    continue;
                }

                if(_grid[srcSquare.getRow() + x][srcSquare.getColumn() + y].empty()
                    || _grid[srcSquare.getRow() + x][srcSquare.getColumn() + y].bit()->gameTag() / 128 != bit.gameTag() / 128){
                    possibleMoves[moveCount] = 8*(srcSquare.getRow() + x) + srcSquare.getColumn() + y;
                    moveList->push_back(8*(srcSquare.getRow() + x) + srcSquare.getColumn() + y);
                    moveCount += 1;
                }
            }
        }
    }
        break;
    case Bishop:
    {
        //variables for readability/shorter lines
        int row = srcSquare.getRow();
        int col = srcSquare.getColumn();
        for(int x = -1; x <= 1; x++){
            for(int y = -1; y <= 1; y++){
                if((x == 0 && y == 0)
                    || !(x != 0 && y != 0)){ //as a bishop, skip any combos that wouldn't give diagonals
                    continue;
                }

                for(int i = 1; i < 8; i++){
                    if(0>col+i*y || 7<col+i*y || 0>row+i*x || 7<row+i*x){ //stop when out of range
                        break;
                    }
                    else if(_grid[row+i*x][col+i*y].empty()){
                        possibleMoves[moveCount] = 8*(row+i*x) + col+i*y;
                        moveList->push_back(8*(row+i*x) + col+i*y);
                        moveCount += 1;
                    }
                    else{ //not empty, will break
                        //add move if the non-empty spot has an enemy piece
                        if(_grid[row+i*x][col+i*y].bit()->gameTag() / 128 != bit.gameTag() / 128){
                            possibleMoves[moveCount] = 8*(row+i*x) + col+i*y;
                            moveList->push_back(8*(row+i*x) + col+i*y);
                            moveCount += 1;
                        }
                        break;
                    }
                }
            }
        }
    }
        break;
    case Rook:
    {
        //variables for readability/shorter lines
        int row = srcSquare.getRow();
        int col = srcSquare.getColumn();
        for(int x = -1; x <= 1; x++){
            for(int y = -1; y <= 1; y++){
                if((x == 0 && y == 0)
                    || !(x == 0 || y == 0)){ //as a rook, skip any combos that would give diagonals
                    continue;
                }

                for(int i = 1; i < 8; i++){
                    if(0>col+i*y || 7<col+i*y || 0>row+i*x || 7<row+i*x){ //stop when out of range
                        break;
                    }
                    else if(_grid[row+i*x][col+i*y].empty()){
                        possibleMoves[moveCount] = 8*(row+i*x) + col+i*y;
                        moveList->push_back(8*(row+i*x) + col+i*y);
                        moveCount += 1;
                    }
                    else{ //not empty, will break
                        //add move if the non-empty spot has an enemy piece
                        if(_grid[row+i*x][col+i*y].bit()->gameTag() / 128 != bit.gameTag() / 128){
                            possibleMoves[moveCount] = 8*(row+i*x) + col+i*y;
                            moveList->push_back(8*(row+i*x) + col+i*y);
                            moveCount += 1;
                        }
                        break;
                    }
                }
            }
        }
    }
        break;
    case Queen:
    {
        //variables for readability/shorter lines
        int row = srcSquare.getRow();
        int col = srcSquare.getColumn();
        for(int x = -1; x <= 1; x++){
            for(int y = -1; y <= 1; y++){
                if((x == 0 && y == 0)){ //skip the 0,0 direction, as it's not a real direction
                    continue;
                }

                for(int i = 1; i < 8; i++){
                    if(0>col+i*y || 7<col+i*y || 0>row+i*x || 7<row+i*x){ //stop when out of range
                        break;
                    }
                    else if(_grid[row+i*x][col+i*y].empty()){
                        possibleMoves[moveCount] = 8*(row+i*x) + col+i*y;
                        moveList->push_back(8*(row+i*x) + col+i*y);
                        moveCount += 1;
                    }
                    else{ //not empty, will break
                        //add move if the non-empty spot has an enemy piece
                        if(_grid[row+i*x][col+i*y].bit()->gameTag() / 128 != bit.gameTag() / 128){
                            possibleMoves[moveCount] = 8*(row+i*x) + col+i*y;
                            moveList->push_back(8*(row+i*x) + col+i*y);
                            moveCount += 1;
                        }
                        break;
                    }
                }
            }
        }
    }
        break;
    case King:
        for(int x = -1; x <= 1; x++){
            if(0 > srcSquare.getRow() + x || 7 < srcSquare.getRow() + x){
                continue; //skip if out of board
            }
            for(int y = -1; y <= 1; y++){
                if(0 > srcSquare.getColumn() + y || 7 < srcSquare.getColumn() + y){
                    continue; //skip if out of board
                }
                //if a neightboring spot is empty, or if enemies, then it's a valid move
                if(_grid[srcSquare.getRow() + x][srcSquare.getColumn() + y].empty()
                    || _grid[srcSquare.getRow() + x][srcSquare.getColumn() + y].bit()->gameTag() / 128 != bit.gameTag() / 128){
                    possibleMoves[moveCount] = 8*(srcSquare.getRow() + x) + srcSquare.getColumn() + y;
                    moveList->push_back(8*(srcSquare.getRow() + x) + srcSquare.getColumn() + y);
                    moveCount += 1;
                }
            }
        }
        //consider castling
        if(myState->canCastle[2*(bit.gameTag() / 128)]){ //left castle
            int i = srcSquare.getSquareIndex(); //i = index. used for readability
            //if all 3 between pieces are empty, can castle
            std::cout<<_grid[(i-1)/8][(i-1)%8].empty()<<_grid[(i-2)/8][(i-2)%8].empty()<<_grid[(i-3)/8][(i-3)%8].empty()<<std::endl;
            if(_grid[(i-1)/8][(i-1)%8].empty() && _grid[(i-2)/8][(i-2)%8].empty() && _grid[(i-3)/8][(i-3)%8].empty()){
                moveList->push_back(8*(srcSquare.getRow()) + srcSquare.getColumn() - 2);
            }
        
        }
        if(myState->canCastle[2*(bit.gameTag() / 128) + 1]){ //right castle
            int i = srcSquare.getSquareIndex(); //i = index. used for readability
            //if all 3 between pieces are empty, can castle
            if(_grid[(i+1)/8][(i+1)%8].empty() && _grid[(i+2)/8][(i+2)%8].empty()){
                moveList->push_back(8*(srcSquare.getRow()) + srcSquare.getColumn() + 2);
            }
        }
        break;
    default:
        printf("default");
        break;
    }
    possibleMoves[moveCount] = -1; // -1 signals the end of the list; this way we avoid needing to handle memory
                                   // inefficent memory use? probably. messy? definitely. Functional? enough!
    
    std::vector<int>* moveListLocation = moveList;
    return moveListLocation;
}

//helper function, records what moves the piece can perform into the possibleMoves variable.
//returns how many moves there are. USES AN ARRAY REP OF THE BOARD
//bit = location on board[][]
std::vector<int>* getPossibleMovesFromArray(int bit, int board[8][8], ChessState& cState){
    std::vector<int>* moveList = new std::vector<int>;
    //bitGT = gametag. pulled here since we need it often
    int bitGT = board[bit/8][bit%8];
    //bitR is row and BitC is column. also pulled here due to frequently using it below
    int bitR = bit / 8;
    int bitC = bit % 8;
    switch (bitGT % 128) // get remainder after 128 to get piece num
    {
    case Pawn:
    {
        int direction = 1;
        //0=W,1=B
        if(bitGT / 128 == 1){
            direction *= -1; //B goes down, W goes up
        }
        //add the 1 move forward(s)
        if(0 <= bitR + direction && 7 >= bitR + direction){
            if(board[bitR + direction][bitC] == 0){
                moveList->push_back(bit + 8*direction);
            }

            //add diagonals (done here since we know there's room vertically)
            for(int i=-1;i<=1;i+=2){
                if(0 <= bitC + i && 7 >= bitC + i){
                    if(board[bitR + direction][bitC + i] != 0 //diagonal for piece capture only
                    && board[bitR + direction][bitC + i] / 128 != bitGT / 128){
                        moveList->push_back(bit + 8*direction + i);
                    }
                    else if(cState.EnPassantSpace == bit + 8*direction + i){ //..or en passant
                        moveList->push_back(bit + 8*direction + i + 128); //128 for special moves
                    }
                }
            }
        }
        //add the 2 move forward
        if(bitR == 1 + 5 * (bitGT / 128)){ //2 when W, 6 when B
            if(board[bitR + 2*direction][bitC] == 0 && board[bitR + direction][bitC] == 0){ //only when empty space between
                    moveList->push_back(bit + 16*direction);
            }
        }   
    }
        break;
    case Knight:
    {
        int directions[4] = {-2,-1,1,2};
        for(int x : directions){
            for(int y : directions){
                if(abs(x) == abs(y)){ //skip when two 2s or two 1s; it should always be a 2 and a 1
                    continue;
                }
                //skip when out of range
                if(0>bitR + x || 7<bitR + x
                    || 0>bitC + y || 7<bitC + y){
                    continue;
                }

                if(board[bitR + x][bitC + y] == 0
                    || board[bitR + x][bitC + y] / 128 != bitGT / 128){
                    moveList->push_back(bit + 8*x + y);
                }
            }
        }
    }
        break;
    case Bishop:
    {
        moveList = getStraightPaths(bit,board);
    }
        break;
    case Rook:
    {
        moveList = getStraightPaths(bit,board);
    }
        break;
    case Queen:
    {
        moveList = getStraightPaths(bit,board);
    }
        break;
    case King:
        for(int x = -1; x <= 1; x++){
            if(0 > bitR + x || 7 < bitR + x){
                continue; //skip if out of board
            }
            for(int y = -1; y <= 1; y++){
                if(0 > bitC + y || 7 < bitC + y){
                    continue; //skip if out of board
                }
                //if a neightboring spot is empty, or if enemies, then it's a valid move
                if(board[bitR + x][bitC + y] == 0
                    || board[bitR + x][bitC + y] / 128 != bitGT / 128){
                    moveList->push_back(bit + 8*x + y);
                }
            }
        }
        //consider castling
        if(cState.canCastle[2*(bitGT / 128)]){ //left castle
            //if all 3 between pieces are empty, can castle
            //std::cout<<(board[bitR][bitC-1] == 0)<<(board[bitR][bitC-2] == 0)<<(board[bitR][bitC-3] == 0)<<std::endl;
            if(board[bitR][bitC-1] == 0 && board[bitR][bitC-2] == 0 && board[bitR][bitC-3] == 0){
                moveList->push_back(bit - 2 + 128); //128 to signal special move
            }
        
        }
        if(cState.canCastle[2*(bitGT / 128) + 1]){ //right castle
            //if all 3 between pieces are empty, can castle
            if(board[bitR][bitC+1] == 0 && board[bitR][bitC+2] == 0){
                moveList->push_back(bit + 2 + 128); //128 to signal special move
            }
        }
        break;
    default:
        std::cout<<"default error"<<std::endl;
        break;
    }
    std::vector<int>* moveListLocation = moveList;
    return moveListLocation;
}

//Helper function for getPossibleMovesFromArray(), handling rook/bishop/queen, which use the same math.
//Calling for a diff piece than one of those can result in bugs
std::vector<int>* getStraightPaths(int bit, int board[8][8]){
    std::vector<int>* moveList = new std::vector<int>;
    //bitGT = gametag. pulled here since we need it often
    int bitGT = board[bit/8][bit%8];
    int row = bit / 8;
    int col = bit % 8;
    for(int x = -1; x <= 1; x++){
        for(int y = -1; y <= 1; y++){
            if(bitGT % 128 == Bishop){
                if((x == 0 && y == 0)
                    || !(x != 0 && y != 0)){ //as a bishop, skip any combos that wouldn't give diagonals
                    continue;
                }
            }
            else if(bitGT % 128 == Rook){
                if((x == 0 && y == 0)
                    || !(x == 0 || y == 0)){ //as a rook, skip any combos that would give diagonals
                        continue;
                }
            }
            if((x == 0 && y == 0)){ //skip the 0,0 direction, as it's not a real direction. Done for all pieces
                continue;
            }

            for(int i = 1; i < 8; i++){
                if(0>col+i*y || 7<col+i*y || 0>row+i*x || 7<row+i*x){ //stop when out of range
                    break;
                }
                else if(board[row+i*x][col+i*y] == 0){
                    moveList->push_back(8*(row+i*x) + col+i*y);
                }
                else{ //not empty, will break
                    //add move if the non-empty spot has an enemy piece
                    if(board[row+i*x][col+i*y] / 128 != bitGT / 128){
                        moveList->push_back(8*(row+i*x) + col+i*y);
                    }
                    break;
                }
            }
        }
    }
    std::vector<int>* moveListLocation = moveList;
    return moveListLocation;
}

//helper function, clears all highlights on the board
void Chess::clearHighlights(){
    for(int i = 0; i < 64; i++){
        _grid[i/8][i%8].setMoveHighlighted(false);
    }
}

//The below function generates all possible moves that can be chosen by the player this turn
//The moves are stored as an std::vector of vectors. Each spot is tied to a spot on the board (starting from spot 0 to 64),
//And the value stored is a (pointer to) std::vector of ints listing all possible spots it can move to.
//If there's no spots, value=nullptr. This is all stored in class var potentialMoves
void Chess::generateMoves(){
    //potentialMoves setup in Chess constructor. All spots will be replaced, so no need to call a clear func
    for(int i = 0;i<64;i++){
        if(_grid[i/8][i%8].empty()){ //if empty, no moves
            potentialMoves[i] = nullptr;
        } //if not player's piece, no moves
        else if(getCurrentPlayer()->playerNumber() != _grid[i/8][i%8].bit()->gameTag() / 128){
            potentialMoves[i] = nullptr;
        } //if neither, then it's the player's piece, and could make a move
        else{
            std::vector<int>* myMoveList = nullptr;
            //myMoveList = getPossibleMoves(*_grid[i/8][i%8].bit(),_grid[i/8][i%8]);
            myMoveList = getPossibleMovesFromArray(i, myState->myBoard, *myState);
            if(myMoveList->size() == 0){ //if piece cannot move, nullptr
                potentialMoves[i] = nullptr;
            }
            else{ //if piece can move, add the list for that spot
                //std::cout<<i<<std::endl;
                potentialMoves[i] = myMoveList;
            }
        }
    }
}

//function for setting the board based on FEN string
void Chess::FENtoBoard(std::string FEN){
    int gameState = -1; //tracks if we need to handle gameState info from FEN.
                       //-1 means we don't, otherwise it correlates to the pos in the string with gameState
    int boardPos = 0; //ties to the position on the board of the next piece.
    for(int i = 0; i < FEN.length(); i++){
        //EMPTY SPACE HANDLER
        if(48 <= FEN[i] && FEN[i] <=57){
            boardPos += FEN[i] - 48; // FEN - 48 so as to change the char into it's correlated int value
            continue;
        }
        if(FEN[i] == 47){ // 47 = / . Represents a transition to the next line. not needed here
            continue;
        }
        if(FEN[i] == 32){ // 32 = space. Represents when board position starts.
            gameState = i;
            break; //break so as to have seperate loop handle it.
        }
        
        //upper = white = 0. all upper chars have val < 95, so remainder = player num
        ChessPiece myPiece;
        switch(FEN[i] % 32){ //char's num value. doing overcomplicated math to get both upper/lowercase
            case 16: // p = 80, 112
                myPiece = Pawn;
                break;
            case 18: // r = 82, 114
                myPiece = Rook;
                break;
            case 14: // n = 78, 110
                myPiece = Knight;
                break;
            case 2: // b = 66, 98
                myPiece = Bishop;
                break;
            case 17: // q = 81, 113
                myPiece = Queen;
                break;
            case 11: // k = 75, 107
                myPiece = King;
                break;
            default:
                std::cout <<"error: improper Fen input" <<std::endl;
                break;
        }
        //equation needed since FEN starts at top left, but ours starts at bottom left.
        int position = (8 - boardPos/8)*8 - (8 - boardPos%8);
        // FEN / 95 since < 95 means 0 meaning W, and >95 means 1 meaning B
        Chess::setPiece(position, (int)FEN[i] / 95, myPiece);
        boardPos += 1; //increment for next pos
    }

    //set default game state. some may be overriden, and we can't fully tell which will/won't as we go.
    //As such, all are set to false to be safe. These can be overriden manually after calling this if needed
    for(int i = 0; i < 4; i++){ // 4 = num of bools in canCastle
        myState->canCastle[i] = false;
    }
    myState->EnPassantSpace = -1;
    myState->halfMoves = 0;
    myState->totalMoves = 0;

    //handle board pos if needed (sorry this is so long...)
    if(gameState != -1){
        gameState++; //move from space to player
        //CURRENT PLAYER
        //ok ok. hear me out. there's no dedicated way to manually set the player.
        //but b always goes second. and this should be called at the start of a game. so...
        if(gameState == 'b'){
            endTurn(); //if there's a better way to do this, i apologize.
        }
        gameState += 2; //skip to castle info.
        //CASTLING
        while(FEN[gameState] != ' '){
            switch(FEN[gameState]){
                case 'Q':
                    myState->canCastle[0] = true; break;
                case 'K':
                    myState->canCastle[1] = true; break;
                case 'q':
                    myState->canCastle[2] = true; break;
                case 'k':
                    myState->canCastle[3] = true; break;
                default: break;
            }
            gameState++;
        }
        gameState++;
        //EN PASSANT
        if(FEN[gameState] != '-'){
            //en passant goes L#, where L=letter=col pos and #=number=row pos. 
            myState->EnPassantSpace = (FEN[gameState] - 97) + ((FEN[gameState+1] - 48)*8);
        }
        gameState += 2;

        //MOVE COUNTERS
        while(FEN[gameState] != '-' && FEN[gameState] != ' '){
            myState->halfMoves *= 10;
            myState->halfMoves = FEN[gameState] - 48;
            gameState++;
        }
        gameState++;
        if(FEN[gameState] != ' '){ //if last was a -, you'd still be on a _, so skip forward 1
            gameState++;
        }
        myState->totalMoves = FEN[gameState] - 48;
        while(gameState < FEN.length()){
            myState->halfMoves *= 10;
            myState->halfMoves = FEN[gameState] - 48;
            gameState++;
        }
        //ok so you know what i said about setting turns earlier? how there's no dedicated way to set them?
        //I was going to do a loop to add the num of turns accounted for above.
        //but, that would likely result in a lot of loading time. Additionally, afaik, other than player...
        //nothing actually needs the num of turns, outside of it being tracked in gamestate.
        //as such, I'm not adding a loop here, unless it turns out I need to later. sry for the wall of text
    }
}

//helper funciton. runs the needed code to place a piece on the board.
void Chess::setPiece(int pos, int player, ChessPiece piece){
    Bit* bit = PieceForPlayer(player, piece);
    bit->setPosition(_grid[pos / 8][pos % 8].getPosition());
    bit->setParent(&_grid[pos / 8][pos % 8]);
    bit->setGameTag(piece + player * 128);
    _grid[pos / 8][pos % 8].setBit(bit);
}

//helper function, returns an array representing the board
void Chess::boardToArray(int targetArray[8][8]){
    for(int i = 0; i<64;i++){
        if(_grid[i / 8][i % 8].empty()){
            targetArray[i / 8][i % 8] = 0;
        }
        else{
            targetArray[i / 8][i % 8] = _grid[i / 8][i % 8].bit()->gameTag();
        }
    }
}

//
// free all the memory used by the game on the heap
//
void Chess::stopGame()
{
}

Player* Chess::checkForWinner()
{
    // check to see if either player has won
    bool BKing = false;
    bool WKing = false;
    for(int i = 0; i <64; i++){
        if(!_grid[i/8][i%8].empty()){
            switch(_grid[i/8][i%8].bit()->gameTag()){
                case King:
                    WKing = true; break;
                case King + 128:
                    BKing = true; break;
            }
        }
    }
    if(!WKing){ // no white king, black wins
        return getPlayerAt(1);
    }
    else if(!BKing){ // no black king, white wins
        return getPlayerAt(0);
    }
    return nullptr;
}

bool Chess::checkForDraw()
{
    // check to see if the board is full
    return false;
}

//
// add a helper to Square so it returns out FEN chess notation in the form p for white pawn, K for black king, etc.
// this version is used from the top level board to record moves
//
const char Chess::bitToPieceNotation(int row, int column) const {
    if (row < 0 || row >= 8 || column < 0 || column >= 8) {
        return '0';
    }

    const char* wpieces = { "?PNBRQK" };
    const char* bpieces = { "?pnbrqk" };
    unsigned char notation = '0';
    Bit* bit = _grid[row][column].bit();
    if (bit) {
        notation = bit->gameTag() < 128 ? wpieces[bit->gameTag()] : bpieces[bit->gameTag() & 127];
    } else {
        notation = '0';
    }
    return notation;
}

//
// state strings
//
std::string Chess::initialStateString()
{
    return stateString();
}

//
// this still needs to be tied into imguis init and shutdown
// we will read the state string and store it in each turn object
//
std::string Chess::stateString()
{
    std::string s;
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            s += bitToPieceNotation(y, x);
        }
    }
    return s;
}

//
// this still needs to be tied into imguis init and shutdown
// when the program starts it will load the current game from the imgui ini file and set the game state to the last saved state
//
void Chess::setStateString(const std::string &s)
{
    for (int y = 0; y < _gameOptions.rowY; y++) {
        for (int x = 0; x < _gameOptions.rowX; x++) {
            int index = y * _gameOptions.rowX + x;
            int playerNumber = s[index] - '0';
            if (playerNumber) {
                _grid[y][x].setBit(PieceForPlayer(playerNumber - 1, Pawn));
            } else {
                _grid[y][x].setBit(nullptr);
            }
        }
    }
}


//
// this is the function that will be called by the AI
//
void Chess::updateAI() 
{
    int target = -1; //impossible value, will get replaced in below loop
    int movedBit = -1; //the bit moved. the target is the move performed to move it
    int bestVal = -999999; //lower than all other vals
    //std::cout << target <<" "<< bestVal << std::endl;
    ChessAI* myAI = clone(1);
    int enemyPlayer = 1 - myAI->AIPlayerNumber; //the playerNumber of the enemy player

    //state tracker. keeps track of default state. done here since it can't be reasonably tracked in unperformMove()
    int currentCastles[4];
    for(int i = 0; i < 4; i++){ // 4 = num of bools in canCastle
        currentCastles[i] = myAI->myState->canCastle[i];
    }
    int currEPSpace = myAI->myState->EnPassantSpace;
    for(int i = 0; i < 64; i++){
        //9999 = infinity in this loop, as no other value should be higher than it
        int topAlpha = -999999;
        int topBeta = 999999;
        if(potentialMoves[i] != nullptr){
            for(int move : *(potentialMoves[i])){
                //std::cout << "move"<<move;
                //perform move
                int capturedPiece = myAI->performMoveOnArray(i, move, myAI->myState->myBoard);
                int turnVal = -myAI->negamax(myAI,0,enemyPlayer, -topBeta, -topAlpha);
                //un-perform move
                myAI->unperformMove(i, move, myAI->myState->myBoard, capturedPiece);
                //revert state
                for(int i = 0; i < 4; i++){ // 4 = num of bools in canCastle
                    myAI->myState->canCastle[i] = currentCastles[i];
                }
                myAI->myState->EnPassantSpace = currEPSpace;
                //std::cout << " "<<turnVal;
                //if turnVal is bigger then current best turn, replace it
                if(turnVal > bestVal){
                    bestVal = turnVal;
                    target = move;
                    movedBit = i;

                    //only updates when bestVal is bigger, so only need to run check inside of here
                    topAlpha = std::max(topAlpha, bestVal);
                    //no break check comparing alpha>=beta needed, as it's impossible for topBeta to decrease at the top level,
                    //making such a check impossible to pass
                }
            }
        }
    }
    std::cout<<"best:"<<bestVal<<std::endl;

    _grid[(target%128) / 8][(target%128) % 8].setBit(_grid[movedBit/8][movedBit%8].bit());
    _grid[(target%128) / 8][(target%128) % 8].bit()->setPosition(_grid[(target%128) / 8][(target%128) % 8].getPosition());
    _grid[movedBit/8][movedBit%8].setBit(nullptr);

    //endTurn(); //done in bitMovedFromTo(), which also runs all the other nessecary checks that we need.
    bitMovedFromTo(*_grid[(target%128) / 8][(target%128) % 8].bit(),_grid[movedBit/8][movedBit%8],_grid[(target%128) / 8][(target%128) % 8]);
}

int ChessAI::negamax(ChessAI* myAI, int depth, int playerColor, int alpha, int beta){
    int result = -99999; //lower than all possible negamax results
    //if board is full, set result to 0 (overriden later if there's a win)
    //if(myAI->isBoardFull()){
    //    result = 0;
    //}
    //if either player won, that's the result
    int boardState = myAI->evaluateBoard();
    if(/*boardState != 0 || */depth >= 3){ //stops when reaching the 3rd layer of depth (0 at base, 1&2 in negamax)
        if(playerColor == AIPlayerNumber){
            result = boardState;
        }
        else{ //opponent performed gameending move, meaning the value of the results is inverted
            result = -boardState;
        }
    }
    else if(result == -99999){ //If no winner (nor a tie), run through possible turns.
        //generate moves
        std::vector<int>* turnMoves[64];
        for(int i = 0;i<64;i++){
            if(myAI->myState->myBoard[i/8][i%8] == 0){ //if empty, no moves
                turnMoves[i] = nullptr;
            } //if not player's piece, no moves
            else if(playerColor != myAI->myState->myBoard[i/8][i%8] / 128){
                turnMoves[i] = nullptr;
            } //if neither, then it's the player's piece, and could make a move
            else{
                std::vector<int>* myMoveList = nullptr;
                //myMoveList = getPossibleMoves(*_grid[i/8][i%8].bit(),_grid[i/8][i%8]);
                myMoveList = getPossibleMovesFromArray(i, myAI->myState->myBoard, *(myAI->myState));
                if(myMoveList->size() == 0){ //if piece cannot move, nullptr
                    turnMoves[i] = nullptr;
                }
                else{ //if piece can move, add the list for that spot
                    //std::cout<<i<<std::endl;
                    turnMoves[i] = myMoveList;
                }
            }
        }
        //perform negamax
        //state tracker. keeps track of default state. done here since it can't be reasonably tracked in unperformMove()
        int currentCastles[4];
        for(int i = 0; i < 4; i++){ // 4 = num of bools in canCastle
            currentCastles[i] = myAI->myState->canCastle[i];
        }
        int currEPSpace = myAI->myState->EnPassantSpace;
        //std::cout << depth<< " ";
        for(int i = 0;i<64;i++){
            if(turnMoves[i] != nullptr){
                for(int move : *(turnMoves[i])){
                    //std::cout << myAI->_grid[i/3][i%3];
                    //perform move
                    int capturedPiece = myAI->performMoveOnArray(i, move, myAI->myState->myBoard);
                    //negamax
                    result = std::max(result, -negamax(myAI,depth + 1, 1 - playerColor, -beta, -alpha));
                    //un-perform move
                    myAI->unperformMove(i, move, myAI->myState->myBoard, capturedPiece);
                    //revert state
                    for(int i = 0; i < 4; i++){ // 4 = num of bools in canCastle
                        myAI->myState->canCastle[i] = currentCastles[i];
                    }
                    myAI->myState->EnPassantSpace = currEPSpace;

                    alpha = std::max(alpha, result);
                    if(alpha >= beta){ //optimal move found, no need for further checking of moves (Alpha-beta pruning)
                        //std::cout << "a"<< alpha<<"b"<< beta << std::endl;
                        break;
                    }
                }
            }
        }
        if(result == -99999){ //result is still base val, meaning no moves were performed
            result = 0;
        }
    }
    return result;
}

//makes the AI
ChessAI* Chess::clone(int AInum) 
{
    ChessAI* newGame = new ChessAI();
    //setup board
    newGame->myState = new ChessState(*myState);
    //boardToArray(newGame->myState->myBoard); //sadly can't access myBoard for some reason
    //as such we instead just copy from the current state board
    //Tracking which player is AI, as there's no way to check from inside the AI
    newGame->AIPlayerNumber = AInum;
    return newGame;
}

int ChessAI::performMoveOnArray(int bit, int move, int targetArray[8][8]){
    int bitGT = targetArray[bit/8][bit%8];
    int pos = move % 128; //remove 128 that signals a special move if needed
    int capturedPiece = targetArray[pos/8][pos%8];
    targetArray[pos/8][pos%8] = bitGT;
    targetArray[bit/8][bit%8] = 0;
    //check if pawn reached end of board to promote
    if(bit % 128 == Pawn && (pos/8 == 0 || pos/8 == 7)){
        bit = 128*(bit/128) + Queen;
    }
    if(move / 128 == 1){ //special move handler
        if(bitGT % 128 == Pawn){ //en passant
            //if bitGT=0, then W captures B (on row 4). if 1, then B cap W (on row 3)
            targetArray[4 - bitGT / 128][pos%8] = 0;
        }
        else if(bitGT % 128 == King){ //castling
            switch(bit){//kings
            case 4: //W
                //if move left, queenside
                if(pos == 2){
                    //move the tower piece
                    targetArray[0][3] = targetArray[0][0];
                    targetArray[0][0] = 0;
                }//if right, kingside
                else if(pos == 6){
                    targetArray[0][5] = targetArray[0][7];
                    targetArray[0][7] = 0;
                }
                myState->canCastle[0] = false;
                myState->canCastle[1] = false;
                break;
            case 60: //B
                //if move left, queenside
                if(pos == 58){
                    //move the tower piece
                    targetArray[7][3] = targetArray[7][0];
                    targetArray[7][0] = 0;
                }//if right, kingside
                else if(pos == 62){
                    targetArray[7][5] = targetArray[7][7];
                    targetArray[7][7] = 0;
                }
                myState->canCastle[2] = false;
                myState->canCastle[3] = false;
                break;
            }
        }
    }
    //update state
    //castling
    //CASTLING
    //below statements always ran, since they start true and can't become true again
    switch(bit){ //when the piece moves
        //towers
        case 0:
            myState->canCastle[0] = false; break;
        case 7:
            myState->canCastle[1] = false; break;
        case 56:
            myState->canCastle[2] = false; break;
        case 63:
            myState->canCastle[3] = false; break;
        //kings
        case 4: //W
            myState->canCastle[0] = false;
            myState->canCastle[1] = false;
            break;
        case 60: //B
            myState->canCastle[2] = false;
            myState->canCastle[3] = false;
            break;
    }
    switch(pos){ //when the piece is captured
        //towers
        case 0:
            myState->canCastle[0] = false; break;
        case 7:
            myState->canCastle[1] = false; break;
        case 56:
            myState->canCastle[2] = false; break;
        case 63:
            myState->canCastle[3] = false; break;
        //kings
        case 4: //W
            myState->canCastle[0] = false; break;
            myState->canCastle[1] = false; break;
        case 59: //B
            myState->canCastle[2] = false; break;
            myState->canCastle[3] = false; break;
    }
    //en passant
    if(bitGT % 128 == Pawn && (bit/8) - (pos/8) == 2){ //2 row diff = pawn jumped 2 and is en passantable
        myState->EnPassantSpace = bit + 8;
    }
    else{
        myState->EnPassantSpace = -1;
    }
    myState->halfMoves += 1;
    myState->totalMoves = myState->halfMoves / 2;
    return capturedPiece;
}

void ChessAI::unperformMove(int bit, int move, int targetArray[8][8], int capturedPiece){
    int pos = move % 128; //remove 128 that signals a special move if needed
    targetArray[bit/8][bit%8] = targetArray[pos/8][pos%8];
    targetArray[pos/8][pos%8] = capturedPiece;
    if(move / 128 == 1){ //special move handler. DOES NOT REVERT STATE TRACKING, MUST BE DONE ELSEWHERE
        int bitGT = targetArray[bit/8][bit%8];
        if(bitGT % 128 == Pawn){ //en passant
            //if bitGT=0, then W captures B (on row 4). if 1, then B cap W (on row 3)
            targetArray[4 - bitGT / 128][pos%8] = 0;
        }
        else if(bitGT % 128 == King){ //castling
            switch(bit){//kings
            case 4: //W
                //if move left, queenside
                if(pos == 2){
                    //move the tower piece
                    targetArray[0][0] = targetArray[0][3];
                    targetArray[0][3] = 0;
                }//if right, kingside
                else if(pos == 6){
                    targetArray[0][7] = targetArray[0][5];
                    targetArray[0][5] = 0;
                }
                break;
            case 60: //B
                //if move left, queenside
                if(pos == 58){
                    //move the tower piece
                    targetArray[7][0] = targetArray[7][3];
                    targetArray[7][3] = 0;
                }//if right, kingside
                else if(pos == 62){
                    targetArray[7][7] = targetArray[7][5];
                    targetArray[7][5] = 0;
                }
                break;
            }
        }
    }
    myState->halfMoves += 1;
    myState->totalMoves = myState->halfMoves / 2;
}

int ChessAI::evaluateBoard(){
    //Use the tables from Evaluate.h to evaluate results. the numbers are how good they are for black
    //piece scores taken from provided Evaluate.h
    //loop through the board, score each piece
    int score = 0;
    for(int i = 0; i < 64; i++){
        int bitGT = myState->myBoard[i/8][i%8];
        //note: (bitGT / 64 - 1) is because I need -1 for W and 1 for B.
        //2*(bitGT/128) gets 0 and 2, but I'm able to simplify it (bitGT/64) since no details are stored in the 64 bit
        //then -1 for the offset needed
        switch(bitGT % 128){
            case Pawn:
                score += (100 + pawnTable[i]) * (bitGT / 64 - 1); break;
            case Knight:
                score += (200 + knightTable[i]) * (bitGT / 64 - 1); break;
            case Bishop:
                score += (230 + bishopTable[i]) * (bitGT / 64 - 1); break;
            case Rook:
                score += (400 + rookTable[i]) * (bitGT / 64 - 1); break;
            case Queen:
                score += (900 + queenTable[i]) * (bitGT / 64 - 1); break;
            case King:
                score += (2000 + kingTable[i]) * (bitGT / 64 - 1); break;
            default:
                break;
        }
    }
    if(AIPlayerNumber != 1){ //if AI is not playing B, flip the score
        return -score;
    }
    return score;
}