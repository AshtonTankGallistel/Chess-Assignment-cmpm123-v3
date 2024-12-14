(Additions to this README are compiled at the bottom, seperated by thin lines [ --- ].)

# Chess AI Implementation Project

![Chess Board](https://raw.githubusercontent.com/zaphodgjd/class-chess-123/main/chess/w_king.png)

## 🎯 Project Overview
This repository contains a skeleton implementation of a Chess AI engine written in C++. The project is designed to teach fundamental concepts of game AI, including board representation, move generation, and basic game tree search algorithms.

### 🎓 Educational Purpose
This project serves as a teaching tool for computer science students to understand:
- Game state representation
- Object-oriented design in C++
- Basic AI concepts in game playing
- Bitboard operations and chess piece movement
- FEN (Forsyth–Edwards Notation) for chess position representation

## 🔧 Technical Architecture

### Key Components
1. **Chess Class**: Core game logic implementation
   - Board state management
   - Move validation
   - Game state evaluation
   - AI player implementation

2. **Piece Representation**
   - Unique identifiers for each piece type
   - Sprite loading and rendering
   - Movement pattern definitions

3. **Board Management**
   - 8x8 grid representation
   - Piece positioning
   - Move history tracking
   - FEN notation support

## 🚀 Getting Started

### Prerequisites
- C++ compiler with C++11 support or higher
- Image loading library for piece sprites
- CMake 3.10 or higher

### Building the Project
```bash
mkdir build
cd build
cmake ..
make
```

### Running Tests
```bash
./chess_tests
```

## 📝 Implementation Details

### Current Features
- Basic board setup and initialization
- Piece movement validation framework
- FEN notation parsing and generation
- Sprite loading for chess pieces
- Player turn management

### Planned Features
- [ ] AI move generation
- [ ] Position evaluation
- [ ] Opening book integration
- [ ] Advanced search algorithms
- [ ] Game state persistence

## 🔍 Code Examples

### Piece Movement Validation
```cpp
bool Chess::canBitMoveFromTo(Bit& bit, BitHolder& src, BitHolder& dst) {
    // TODO: Implement piece-specific movement rules
    return false;
}
```

### FEN Notation Generation
```cpp
const char Chess::bitToPieceNotation(int row, int column) const {
    if (row < 0 || row >= 8 || column < 0 || column >= 8) {
        return '0';
    }
    // Implementation details for FEN notation
}
```

## 📚 Class Assignment Structure

### Phase 1: Board Setup
- Implement piece placement
- Setup initial board state
- Validate board representation

### Phase 2: Move Generation
- Implement basic piece movements
- Add move validation
- Implement special moves (castling, en passant)

### Phase 3: AI Implementation
- Develop position evaluation
- Implement minimax algorithm
- Add alpha-beta pruning
- Basic opening book

## 🤝 Contributing
Students are encouraged to:
1. Fork the repository
2. Create a feature branch
3. Implement assigned components
4. Submit their fork for review

## 🔒 Code Style and Standards
- Use consistent indentation (4 spaces)
- Follow C++ naming conventions
- Document all public methods
- Include unit tests for new features

## 📄 License
This project is licensed under the MIT License.

## 👥 Contributors
- [Your Name] - Initial work
- [Student Names] - Implementation and testing

## 🙏 Acknowledgments
- Chess piece sprites from [Wikipedia](https://en.wikipedia.org/wiki/Chess_piece)
- Original game engine framework by [ocornut](https://github.com/ocornut/imgui)

---
*This README is part of an educational project and is intended to serve as an example of good documentation practices.*
---

Hello, apologies if I am putting this in the incorrect place.
Here is the required screenshots for this project.
The board, on startup, looks as such.
![Chess Board](https://raw.githubusercontent.com/AshtonTankGallistel/class-chess-123/refs/heads/main/screenshots/chessStart.png)

However, I was sadly unable to do the random move generation nor array in time, as I had not noticed it until shortly before the due time, and there's no description of how we are supposed to do either. As such, I've attached a screenshot of 20 moves, randomly performed by me, instead.
![Chess Board](https://raw.githubusercontent.com/AshtonTankGallistel/class-chess-123/refs/heads/main/screenshots/chess20Moves.png)

I apologize for the poor quality and shortness of my additions to this readme. Thank you for reading regardless.
-Ashton Gallistel

---

Hello, this is my addition to the followup assignment. It's again very short and likely in the wrong place, as I again failed to manage my time properly. I have, at the time of writing, managed to add:
-FENToBoard function
-pawn promotion (to queen only)
I am attempting to add castling right now, but am unlikely to before the deadline. I apologize for failing to properly manage my time, and thank you for reading.

---

# Results Description (12/13/2024)

I am happy to report that the final assignment of the three regarding my chess engine has gone well! All written requirments have been successfully met as of writing this.

## Catch Up

Something I forgot to mention in my previous submission was that the move generation was modified to find all moves at the start of the turn, rather than the moves for each piece when they're picked up. This function, generateMoves(), worked for implementing the remaining functionality later on, including castling and en passant. Specifically, it called getPossibleMoves() for each grid with a piece belonging to the current player.

I also wrote the check for if a player had taken another one's king, resulting in a win. Sadly, I did not write code to look for checkmates, as I was focused on purely written requirements, and did not have time to add it. There was also the concern of how much time it would add to the AI, which I was already worried about the efficiency of in the current system. As such, the game is winnable by capturing the king, but there is no system for checkmates nor draws.

## Changes needed for the AI

Firstly, I quickly realized that I was relying on the chess square _grid and chessState of the chess object in all of my code, which chessAI wouldn't use. As such, I had to modify it to use a different structure. Originally, I had wanted to use bitboards as the teacher had recommended, but to implement them would involve massively restructuring a lot of my code, so I chose to go with a 2D, 8 by 8 array of ints instead. On review, I likely should have just used a 1D array of 64 ints instead, but at the time I wanted the change to be as straightforward as possible. I also made the chessState into a public structure seperate from the Chess class, so that both Chess and ChessAI could access it. As both would have the structure, I chose to store the board array inside of it. With this change, I implemented a function boardToArray(), which copied the _grid board into Chess's array.

As for getPossibleMoves(), the way I had coded it involved checking the status of the _grid array in the Chess object. To allow both Chess and the AI to use it via there new arrays, I made a new function to replace it, getPossibleMovesFromArray(). This function worked similarly, but instead of determining moves using the Chess's _grid and state, it would take in pointers to an array and ChessState struct to use instead. While doing this, I also added a helper function, getStraightPaths(), which involved a loop that could be used for rooks, bishops, and queens. I had intended to make this helper function for the original getPossibleMoves(), but previously did not have the time. Both of these functions were eventually moved to outside of the Chess class, so that both Chess and ChessAI could access them.

## AI Additions

### Performing and un-Performing Moves

Unlike TicTacToe, which had rather simple moves, I had to add additional functions for both performing moves, and undoing the result of those moves. performMoveOnArray() Takes an inputted location on the board (called bit), move, and array, and performs the move from that location inside of the array. In order to easily detect if the move was a special move, like castling or en passant, I decided to have 128 added to the move integer, similarly to how detecting the player color was done with game tags. This involved having to go back and add this change in the move generator function, as well as account for it in any functions that reacted to moves.The function also returns the game tag of any captured piece, or 0 if there was no capture.

In order to undo the results of that function, an additional function was needed, unperformMove(). This function had the same inputs as performMove(), alongside an additional integer representing the captured piece. The function is unable to undo any changes to the gameState tracking en passant or castling, so the code to do so is handled in updateAI() and negamax instead().

### Implementing negamax

The AI is heavily based on the one I made for TicTacToe, with me starting by copying and pasting the code in from that assignment. The AI starts at the top level inside of updateAI(), using the list of moves generated at the end of bitMovedFromTo(). It then calls negamax inside, which then loops recursively until reaching the end of the game or the stated cap for the depth. Additionally, on each layer, it starts by writing down the state of castling and en passant, then resets the game state's values to those after each negamax call, specifically after the move is un-performed.

In order to determine which moves are available, negamax has a built in move generator. The generateMoves() in the Chess class wasn't accessible from the AI, and made use of _grid, so I chose to take the quicker route and just copy the code into negamax, and modify from there. I originally hoped to try and combine them more, skipping unnessecary checks and loops, but I ended up not having the time to.

### Evaluation

I added the provided Evaluate.h to the folder, and used the provided grids in an evaluateBoard() function. The function loops over every spot on the board, and for spots with pieces, it performs a switch function to determine which piece it is. For each piece type, it adds the associated value, alongside the added or lowered value from the related grid. The value is positive if it's a black piece, and negative if it's white, with the final result merely being made negative if the AI is playing white instead of black.

## Challenges, Depth, and AI Skill

I knew early on from my choice to use arrays that the AI would likely be at risk of running slow, but as I was concerned about my ability to finish in time, I chose to go through with it. Thankfully, the speed is just fast enough to accomplish a depth of 3, 1 layer in the updateAI() level and 2 in negamax, in under a second. At a depth of 4, it surpasses a second, and even seems to encounter a memory issue on some turns resulting in a crash. If I were to guess, this is likely due to my heavy use of arrays. Thankfully, this issue does not appear to occur on a depth of 3.

My AI also appears to do rather well. While I haven't had the time to play a match where I fully think through my moves, simply choosing the moves I feel would possibly work tends to result in me getting quickly defeated. The main trait I noticed is that the AI seems to have a preference for getting the queen out relatively early, which may be because I often left my pieces open and easy to take. While I do wish I got the depth higher, I'm honestly rather happy with how skilled my AI seems to be with such a low depth.

Regarding my challenges, I would say that my main issue was time management. I tend to prioritize assignments that are due sooner rather than later, which resulted in me not dedicating the time I should have to this assignment back around Thanksgiving, despite knowing that I should have been working on it. I also feel like I did a poor job at efficiency during this assignment, especially due to my common usage of arrays over bytes. While this was mostly done to get the project finished in time, there are multiple times where I can think of where I could have likely used bytes or ints over arrays with no issues. If I were to continue this assignment, I would likely prioritize improving the efficiency in order to get a better depth, as well finally implement checkmate.

Thank you for taking the time to read this, and I hope you have a great Winter break!
- Ashton Gallistel