#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <random>
#include <string>
#include <cmath>
#include <algorithm>

using namespace std;

// Constants
const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 700;
const int GRID_SIZE = 4;
const int TILE_SIZE = 120;
const int TILE_MARGIN = 15;
const int GRID_OFFSET_X = 60;
const int GRID_OFFSET_Y = 150;
const int ANIMATION_FRAMES = 10;

// Colors for different tile values
struct Color {
    Uint8 r, g, b, a;
};

Color getTileColor(int value) {
    switch(value) {
        case 0:    return {205, 193, 180, 255}; // Empty
        case 2:    return {238, 228, 218, 255};
        case 4:    return {237, 224, 200, 255};
        case 8:    return {242, 177, 121, 255};
        case 16:   return {245, 149, 99, 255};
        case 32:   return {246, 124, 95, 255};
        case 64:   return {246, 94, 59, 255};
        case 128:  return {237, 207, 114, 255};
        case 256:  return {237, 204, 97, 255};
        case 512:  return {237, 200, 80, 255};
        case 1024: return {237, 197, 63, 255};
        case 2048: return {237, 194, 46, 255};
        default:   return {60, 58, 50, 255};   // Higher values
    }
}

Color getTextColor(int value) {
    if (value <= 4) return {119, 110, 101, 255}; // Dark text for light tiles
    return {249, 246, 242, 255}; // Light text for dark tiles
}

// Animation structure
struct TileAnimation {
    int startRow, startCol;
    int endRow, endCol;
    int value;
    int frame;
    bool active;
};

class Game2048 {
private:
    vector<vector<int>> board;
    vector<vector<int>> prevBoard;
    int score;
    bool gameOver;
    mt19937 gen;
    
    vector<TileAnimation> animations;
    
public:
    Game2048() : board(4, vector<int>(4, 0)), score(0), gameOver(false), gen(42) {
        spawnTile();
        spawnTile();
    }
    
    void spawnTile() {
        vector<pair<int,int>> empty;
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                if (board[r][c] == 0) empty.emplace_back(r, c);
        
        if (empty.empty()) return;
        
        uniform_int_distribution<> pos_dist(0, empty.size()-1);
        uniform_int_distribution<> val_dist(1, 10);
        
        auto [r, c] = empty[pos_dist(gen)];
        board[r][c] = (val_dist(gen) == 1 ? 4 : 2);
    }
    
    vector<int> compressRow(const vector<int>& row) {
        vector<int> compressed;
        copy_if(row.begin(), row.end(), back_inserter(compressed), [](int x) { return x != 0; });
        compressed.resize(row.size(), 0);
        return compressed;
    }
    
    vector<int> mergeRow(vector<int> row) {
        for(auto i = 0; i < row.size() - 1; i++){
            if(row[i] != 0 && row[i] == row[i + 1]){
                row[i] *= 2;
                score += row[i];
                row[i + 1] = 0;
                i++;
            }
        }
        return compressRow(row);
    }
    
    bool moveLeft() {
        prevBoard = board;
        bool moved = false;
        for(auto& row : board){
            auto temp = mergeRow(compressRow(row));
            moved = moved || (temp != row);
            row = temp;
        }
        return moved;
    }
    
    bool moveRight() {
        prevBoard = board;
        bool moved = false;
        for(auto& row : board){
            auto temp = row;
            reverse(temp.begin(), temp.end());
            temp = mergeRow(compressRow(temp));
            reverse(temp.begin(), temp.end());
            moved = moved || (temp != row);
            row = temp;
        }
        return moved;
    }
    
    bool moveUp() {
        prevBoard = board;
        bool moved = false;
        for(int c = 0; c < 4; c++){
            vector<int> col;
            for(int r = 0; r < 4; r++) col.push_back(board[r][c]);
            auto temp = mergeRow(compressRow(col));
            if(temp != col) moved = true;
            for(int r = 0; r < 4; r++) board[r][c] = temp[r];
        }
        return moved;
    }
    
    bool moveDown() {
        prevBoard = board;
        bool moved = false;
        for(int c = 0; c < 4; c++){
            vector<int> col;
            for(int r = 0; r < 4; r++) col.push_back(board[r][c]);
            reverse(col.begin(), col.end());
            auto temp = mergeRow(compressRow(col));
            reverse(temp.begin(), temp.end());
            vector<int> original(4);
            for(int r = 0; r < 4; r++) original[r] = board[r][c];
            if(temp != original) moved = true;
            for(int r = 0; r < 4; r++) board[r][c] = temp[r];
        }
        return moved;
    }
    
    bool canMove() {
        // Check for empty cells
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                if (board[r][c] == 0) return true;
        
        // Check for possible merges
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++) {
                if (c < 3 && board[r][c] == board[r][c+1]) return true;
                if (r < 3 && board[r][c] == board[r+1][c]) return true;
            }
        
        return false;
    }
    
    const vector<vector<int>>& getBoard() const { return board; }
    int getScore() const { return score; }
    bool isGameOver() const { return gameOver; }
    void checkGameOver() { gameOver = !canMove(); }
    
    void undo() {
        if (!prevBoard.empty()) {
            board = prevBoard;
            prevBoard.clear();
        }
    }
    
    void reset() {
        board = vector<vector<int>>(4, vector<int>(4, 0));
        score = 0;
        gameOver = false;
        spawnTile();
        spawnTile();
    }
};

void drawText(SDL_Renderer* renderer, TTF_Font* font, const string& text, 
              int x, int y, Color color, bool centered = false) {
    SDL_Color sdlColor = {color.r, color.g, color.b, color.a};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), sdlColor);
    if (!surface) return;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }
    
    SDL_Rect rect;
    rect.w = surface->w;
    rect.h = surface->h;
    
    if (centered) {
        rect.x = x - rect.w / 2;
        rect.y = y - rect.h / 2;
    } else {
        rect.x = x;
        rect.y = y;
    }
    
    SDL_RenderCopy(renderer, texture, nullptr, &rect);
    
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void drawTile(SDL_Renderer* renderer, TTF_Font* font, int value, int x, int y) {
    Color bgColor = getTileColor(value);
    
    // Draw tile background with rounded corners (approximated with filled rect)
    SDL_Rect tileRect = {x, y, TILE_SIZE, TILE_SIZE};
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &tileRect);
    
    // Draw value if not empty
    if (value > 0) {
        Color textColor = getTextColor(value);
        string text = to_string(value);
        drawText(renderer, font, text, x + TILE_SIZE/2, y + TILE_SIZE/2, textColor, true);
    }
}

void render(SDL_Renderer* renderer, TTF_Font* largeFont, TTF_Font* smallFont, 
            const Game2048& game) {
    // Clear screen with background color
    SDL_SetRenderDrawColor(renderer, 250, 248, 239, 255);
    SDL_RenderClear(renderer);
    
    // Draw title
    Color titleColor = {119, 110, 101, 255};
    drawText(renderer, largeFont, "2048", 60, 30, titleColor);
    
    // Draw score
    string scoreText = "Score: " + to_string(game.getScore());
    drawText(renderer, smallFont, scoreText, 400, 50, titleColor);
    
    // Draw grid background
    SDL_Rect gridBg = {GRID_OFFSET_X - TILE_MARGIN, GRID_OFFSET_Y - TILE_MARGIN,
                       4 * TILE_SIZE + 5 * TILE_MARGIN, 4 * TILE_SIZE + 5 * TILE_MARGIN};
    SDL_SetRenderDrawColor(renderer, 187, 173, 160, 255);
    SDL_RenderFillRect(renderer, &gridBg);
    
    // Draw tiles
    const auto& board = game.getBoard();
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int x = GRID_OFFSET_X + c * (TILE_SIZE + TILE_MARGIN);
            int y = GRID_OFFSET_Y + r * (TILE_SIZE + TILE_MARGIN);
            drawTile(renderer, largeFont, board[r][c], x, y);
        }
    }
    
    // Draw game over overlay
    if (game.isGameOver()) {
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 238, 228, 218, 200);
        SDL_RenderFillRect(renderer, &gridBg);
        
        Color gameOverColor = {119, 110, 101, 255};
        drawText(renderer, largeFont, "Game Over!", WINDOW_WIDTH/2, WINDOW_HEIGHT/2 - 30, 
                 gameOverColor, true);
        drawText(renderer, smallFont, "Press R to restart", WINDOW_WIDTH/2, WINDOW_HEIGHT/2 + 30, 
                 gameOverColor, true);
    }
    
    // Draw controls hint
    Color hintColor = {119, 110, 101, 255};
    drawText(renderer, smallFont, "Arrow keys to move | R to restart | U to undo", 
             WINDOW_WIDTH/2, WINDOW_HEIGHT - 30, hintColor, true);
    
    SDL_RenderPresent(renderer);
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "SDL initialization failed: " << SDL_GetError() << endl;
        return 1;
    }
    
    if (TTF_Init() < 0) {
        cerr << "TTF initialization failed: " << TTF_GetError() << endl;
        SDL_Quit();
        return 1;
    }
    
    SDL_Window* window = SDL_CreateWindow("2048 Game",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        cerr << "Window creation failed: " << SDL_GetError() << endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        cerr << "Renderer creation failed: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    // Load fonts (you'll need to have a TTF font file)
    TTF_Font* largeFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 48);
    TTF_Font* smallFont = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 24);
    
    if (!largeFont || !smallFont) {
        cerr << "Font loading failed: " << TTF_GetError() << endl;
        cerr << "Make sure fonts are installed in /usr/share/fonts/truetype/dejavu/" << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    Game2048 game;
    bool running = true;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                bool moved = false;
                
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                    case SDLK_a:
                        moved = game.moveLeft();
                        break;
                    case SDLK_RIGHT:
                    case SDLK_d:
                        moved = game.moveRight();
                        break;
                    case SDLK_UP:
                    case SDLK_w:
                        moved = game.moveUp();
                        break;
                    case SDLK_DOWN:
                    case SDLK_s:
                        moved = game.moveDown();
                        break;
                    case SDLK_u:
                        game.undo();
                        break;
                    case SDLK_r:
                        game.reset();
                        break;
                    case SDLK_ESCAPE:
                    case SDLK_q:
                        running = false;
                        break;
                }
                
                if (moved && !game.isGameOver()) {
                    game.spawnTile();
                    game.checkGameOver();
                }
            }
        }
        
        render(renderer, largeFont, smallFont, game);
        SDL_Delay(16); // ~60 FPS
    }
    
    TTF_CloseFont(largeFont);
    TTF_CloseFont(smallFont);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}
