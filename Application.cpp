#include "Application.h"
#include "imgui/imgui.h"
#include "classes/Chess.h"

namespace ClassGame {
        //
        // our global variables
        //
        Chess *game = nullptr;
        bool gameOver = false;
        bool startUp = true;
        int gameWinner = -1;

        //
        // game starting point
        // this is called by the main render loop in main.cpp
        //
        void GameStartUp() 
        {
            game = new Chess();
            game->setUpBoard();
        }

        //
        // game render loop
        // this is called by the main render loop in main.cpp
        //
        void RenderGame() 
        {
                ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

                //ImGui::ShowDemoWindow();

                ImGui::Begin("Settings");
                //ImGui::Text("Current Player Number: %d", game->getCurrentPlayer()->playerNumber());
                //ImGui::Text("Current Board State: %s", game->stateString().c_str());

                if (gameOver) {
                    ImGui::Text("Game Over!");
                    //ImGui::Text("Winner: %d", gameWinner);
                    if(gameWinner == 0){
                        ImGui::Text("Winner: White");
                    }
                    else if(gameWinner == 1){
                        ImGui::Text("Winner: Black");
                    }
                    if (ImGui::Button("Reset Game (Player vs Player)")) {
                        game->stopGame();
                        game->setUpBoard();
                        gameOver = false;
                        gameWinner = -1;
                    }
                    if (ImGui::Button("Reset Game (Player vs AI)")) {
                        game->stopGame();
                        game->setUpBoard();
                        game->setAIPlayer(1);
                        gameOver = false;
                        gameWinner = -1;
                    }
                }
                else if(startUp){
                    ImGui::Text("Welcome to Chess!");
                    ImGui::Text("Select a mode to begin.");
                    if (ImGui::Button("Player vs Player")) {
                        game->stopGame();
                        game->setUpBoard();
                        //no AI player!
                        startUp = false;
                    }
                    if (ImGui::Button("Player vs AI")) {
                        game->stopGame();
                        game->setUpBoard();
                        game->setAIPlayer(1);
                        startUp = false;
                    }
                }
                ImGui::End();

                ImGui::Begin("GameWindow");
                game->drawFrame();
                ImGui::End();
        }

        //
        // end turn is called by the game code at the end of each turn
        // this is where we check for a winner
        //
        void EndOfTurn() 
        {
            Player *winner = game->checkForWinner();
            if (winner)
            {
                gameOver = true;
                gameWinner = winner->playerNumber();
            }
            if (game->checkForDraw()) {
                gameOver = true;
                gameWinner = -1;
            }
        }
}
