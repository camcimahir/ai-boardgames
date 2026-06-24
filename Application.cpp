#include "Application.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h" // DockBuilder API for the default layout
#include <chrono>

#include "classes/TicTacToe.h"
#include "classes/ConnectFour.h"
#include "classes/Chess.h"
#include "classes/AstroBots.h"

namespace ClassGame {

    enum class GameType {
        None,
        TicTacToe,
        ConnectFour,
        Chess,
        AstroBots,
    };

    Game *game = nullptr;
    GameType currentGameType = GameType::None;
    GameType pendingGameType = GameType::None;
    bool gameOver = false;
    int gameWinner = -1;

    // AstroBots ticks its simulation off the render loop at ~30Hz rather than per-turn.
    static auto lastAstroBotsUpdate = std::chrono::steady_clock::now();
    static constexpr double ASTROBOTS_UPDATE_INTERVAL_MS = 1000.0 / 30.0;

    static void quitToMainMenu()
    {
        if (game) {
            game->stopGame();
            delete game;
            game = nullptr;
        }
        currentGameType = GameType::None;
        pendingGameType = GameType::None;
        gameOver = false;
        gameWinner = -1;
    }

    static void startGame(GameType type, Game *newGame)
    {
        if (game) {
            game->stopGame();
            delete game;
        }
        game = newGame;
        currentGameType = type;
        pendingGameType = GameType::None;
        gameOver = false;
        gameWinner = -1;
        if (game) game->setUpBoard();
        if (type == GameType::AstroBots) {
            lastAstroBotsUpdate = std::chrono::steady_clock::now();
        }
    }

    void GameStartUp()
    {
        game = nullptr;
        currentGameType = GameType::None;
        pendingGameType = GameType::None;
    }

    // Full-width menu button that wraps long labels onto multiple lines and grows in height.
    static bool MenuButton(const char* label, float width)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        const float wrap_width = ImMax(1.0f, width - style.FramePadding.x * 2.0f);
        const ImVec2 text_size = ImGui::CalcTextSize(label, nullptr, false, wrap_width);
        const ImVec2 size(width, text_size.y + style.FramePadding.y * 2.0f);

        const ImVec2 pos = window->DC.CursorPos;
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(size, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        bool hovered = false;
        bool held = false;
        const bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

        const ImU32 col = ImGui::GetColorU32(
            (held && hovered) ? ImGuiCol_ButtonActive :
            hovered ? ImGuiCol_ButtonHovered :
            ImGuiCol_Button);
        ImGui::RenderNavHighlight(bb, id);
        ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
        const ImVec2 text_pos(bb.Min.x + style.FramePadding.x, bb.Min.y + style.FramePadding.y);
        ImGui::RenderTextWrapped(text_pos, label, nullptr, wrap_width);

        return pressed;
    }

    static void renderMainMenu()
    {
        // Scale up all text and buttons in the menu so they are easy to read at a glance.
        const float MENU_FONT_SCALE = 1.5f;
        ImGui::SetWindowFontScale(MENU_FONT_SCALE);

        ImGui::TextUnformatted("AI Boardgames");
        ImGui::Separator();

        if (pendingGameType == GameType::None) {
            ImGui::TextUnformatted("Pick a game:");
            ImGui::Spacing();

            const float panelW = ImGui::GetContentRegionAvail().x;

            if (MenuButton("Tic-Tac-Toe", panelW)) {
                startGame(GameType::TicTacToe, new TicTacToe());
            }
            ImGui::Spacing();
            if (MenuButton("Connect Four", panelW)) {
                pendingGameType = GameType::ConnectFour;
            }
            ImGui::Spacing();
            if (MenuButton("Chess", panelW)) {
                pendingGameType = GameType::Chess;
            }
            ImGui::Spacing();
            if (MenuButton("AstroBots (Robots)", panelW)) {
                startGame(GameType::AstroBots, new AstroBots());
            }

            ImGui::SetWindowFontScale(1.0f);
            return;
        }

        if (pendingGameType == GameType::ConnectFour) {
            ImGui::TextUnformatted("Connect Four — pick mode:");
            ImGui::Spacing();
            const float panelW = ImGui::GetContentRegionAvail().x;

            if (MenuButton("Human vs Human", panelW)) {
                ConnectFour *c4 = new ConnectFour();
                c4->setNumberOfPlayers(2);
                startGame(GameType::ConnectFour, c4);
            }
            ImGui::Spacing();
            if (MenuButton("Human vs AI (you go first)", panelW)) {
                ConnectFour *c4 = new ConnectFour();
                c4->setNumberOfPlayers(2);
                c4->setAIPlayer(1);
                startGame(GameType::ConnectFour, c4);
            }
            ImGui::Spacing();
            if (MenuButton("AI vs Human (AI goes first)", panelW)) {
                ConnectFour *c4 = new ConnectFour();
                c4->setNumberOfPlayers(2);
                c4->setAIPlayer(0);
                startGame(GameType::ConnectFour, c4);
            }
            ImGui::Spacing();
            if (MenuButton("Back", panelW)) {
                pendingGameType = GameType::None;
            }

            ImGui::SetWindowFontScale(1.0f);
            return;
        }

        if (pendingGameType == GameType::Chess) {
            ImGui::TextUnformatted("Chess — pick mode:");
            ImGui::Spacing();
            const float panelW = ImGui::GetContentRegionAvail().x;

            if (MenuButton("Human vs Human", panelW)) {
                Chess *chess = new Chess();
                chess->setUseAI(false);
                startGame(GameType::Chess, chess);
            }
            ImGui::Spacing();
            if (MenuButton("Human (White) vs AI (Black)", panelW)) {
                Chess *chess = new Chess();
                chess->setUseAI(true);
                startGame(GameType::Chess, chess);
            }
            ImGui::Spacing();
            if (MenuButton("Back", panelW)) {
                pendingGameType = GameType::None;
            }

            ImGui::SetWindowFontScale(1.0f);
            return;
        }
    }

    static const char* currentGameLabel()
    {
        switch (currentGameType) {
            case GameType::TicTacToe:   return "Tic-Tac-Toe";
            case GameType::ConnectFour: return "Connect Four";
            case GameType::Chess:       return "Chess";
            case GameType::AstroBots:   return "AstroBots";
            default:                    return "(none)";
        }
    }

    static void renderInGameSettings()
    {
        ImGui::Text("Now playing: %s", currentGameLabel());
        ImGui::Separator();

        if (gameOver) {
            ImGui::TextUnformatted("Game Over!");
            if (gameWinner == -1) {
                ImGui::TextUnformatted("Result: Draw");
            } else if (currentGameType == GameType::ConnectFour) {
                ImGui::Text("Winner: %s", gameWinner == 0 ? "Red Player" : "Yellow Player");
            } else if (currentGameType == GameType::Chess) {
                ImGui::Text("Winner: %s", gameWinner == 0 ? "White Player" : "Black Player");
            } else {
                ImGui::Text("Winner: Player %d", gameWinner);
            }
            if (ImGui::Button("Reset Game")) {
                game->stopGame();
                game->setUpBoard();
                gameOver = false;
                gameWinner = -1;
            }
            ImGui::SameLine();
        }

        if (ImGui::Button("Quit to Main Menu")) {
            quitToMainMenu();
            return;
        }

        ImGui::Separator();

        if (currentGameType == GameType::AstroBots) {
            ImGui::TextUnformatted("(AstroBots is a real-time simulation)");
        } else if (game && game->getCurrentPlayer()) {
            ImGui::Text("Current Player: %d", game->getCurrentPlayer()->playerNumber());
            std::string stateString = game->stateString();
            int stride = game->_gameOptions.rowX;
            int height = game->_gameOptions.rowY;
            if (stride > 0 && height > 0 && (int)stateString.size() >= stride * height) {
                for (int y = 0; y < height; y++) {
                    ImGui::Text("%s", stateString.substr(y * stride, stride).c_str());
                }
            } else {
                ImGui::Text("State: %s", stateString.c_str());
            }
        }
    }

    void RenderGame()
    {
        ImGuiID dockspace_id = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        // Build a sensible default layout the first time the app runs (and on the web every
        // launch, since no imgui.ini is persisted): the game board fills the main area and the
        // Settings panel is docked on the right, so players immediately see where to click.
        static bool s_dockLayoutInitialized = false;
        if (!s_dockLayoutInitialized) {
            s_dockLayoutInitialized = true;
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

            ImGuiID settings_id = 0;
            ImGuiID game_id = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.78f, nullptr, &settings_id);

            ImGui::DockBuilderDockWindow("GameWindow", game_id);
            ImGui::DockBuilderDockWindow("Settings", settings_id);
            ImGui::DockBuilderFinish(dockspace_id);
        }

        ImGui::Begin("Settings");
        if (!game) {
            renderMainMenu();
        } else {
            renderInGameSettings();
        }
        ImGui::End();

        ImGui::Begin("GameWindow");
        if (game) {
            if (currentGameType == GameType::AstroBots) {
                AstroBots *astroGame = dynamic_cast<AstroBots*>(game);
                if (astroGame) {
                    auto now = std::chrono::steady_clock::now();
                    double elapsedMs = std::chrono::duration<double, std::milli>(now - lastAstroBotsUpdate).count();
                    if (elapsedMs >= ASTROBOTS_UPDATE_INTERVAL_MS) {
                        astroGame->endTurn();
                        lastAstroBotsUpdate = now;
                    }
                }
            } else if (!gameOver && game->gameHasAI() &&
                       (game->getCurrentPlayer()->isAIPlayer() || game->_gameOptions.AIvsAI)) {
                game->updateAI();
            }
            game->drawFrame();
        }
        ImGui::End();
    }

    void EndOfTurn()
    {
        if (!game) return;
        Player *winner = game->checkForWinner();
        if (winner) {
            gameOver = true;
            gameWinner = winner->playerNumber();
        }
        if (game->checkForDraw()) {
            gameOver = true;
            gameWinner = -1;
        }
    }
}
