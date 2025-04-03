#include <SDL.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <vector>
#include <deque>
#include <string>

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 1000;
const int TILE_SIZE = 10;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;

enum Direction { UP, DOWN, LEFT, RIGHT };
int dir = RIGHT;
SDL_Rect head{500, 500, TILE_SIZE, TILE_SIZE};

std::deque<SDL_Rect> snake;
int size = 1;

std::vector<SDL_Rect> apples;

int score = 0;
int highScore = 0;
bool paused = false;

// Initializes SDL and other component
bool init() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) return false;
    if (TTF_Init() == -1) return false;

    window = SDL_CreateWindow("Snake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) return false;

    font = TTF_OpenFont("SuperPixel-m2L8j.ttf", 24);
    if (!font) return false;

    return true;
}

// Renders text on screen
void renderText(const std::string& text, int x, int y) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), white);
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);

    SDL_Rect destRect = {x, y, 200, 50};
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_DestroyTexture(texture);
}

// Reset the game when snake collides with its' own body or the wall
void resetGame() {
    head.x = SCREEN_WIDTH / 2;
    head.y = SCREEN_HEIGHT / 2;
    dir = RIGHT;
    size = 1;
    score = 0;
    snake.clear();
}

// game loop
void gameLoop() {
    SDL_Event event;
    bool running = true;
    int speedDelay = 25;  // Base speed delay

    // Generate initial apples
    for (int i = 0; i < 10; ++i) {
        apples.emplace_back(SDL_Rect{rand() % (SCREEN_WIDTH / TILE_SIZE) * TILE_SIZE,
                                     rand() % (SCREEN_HEIGHT / TILE_SIZE) * TILE_SIZE, TILE_SIZE, TILE_SIZE});
    }

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_UP && dir != DOWN) dir = UP;
                if (event.key.keysym.sym == SDLK_DOWN && dir != UP) dir = DOWN;
                if (event.key.keysym.sym == SDLK_LEFT && dir != RIGHT) dir = LEFT;
                if (event.key.keysym.sym == SDLK_RIGHT && dir != LEFT) dir = RIGHT;
                if (event.key.keysym.sym == SDLK_SPACE) paused = !paused;
            }
        }

        if (paused) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            renderText("PAUSED", SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2);
            SDL_RenderPresent(renderer);
            SDL_Delay(100); // Prevent CPU overload
            continue;
        }

        // Update snake position
        switch (dir) {
            case UP: head.y -= TILE_SIZE; break;
            case DOWN: head.y += TILE_SIZE; break;
            case LEFT: head.x -= TILE_SIZE; break;
            case RIGHT: head.x += TILE_SIZE; break;
        }

        // Wall collision check
        if (head.x < 0 || head.x >= SCREEN_WIDTH || head.y < 0 || head.y >= SCREEN_HEIGHT) {
            if (score > highScore) highScore = score;
            resetGame();
        }

        // Apple collision
        for (auto& apple : apples) {
            if (head.x == apple.x && head.y == apple.y) {
                size += 5;
                score += 10;
                apple.x = rand() % (SCREEN_WIDTH / TILE_SIZE) * TILE_SIZE;
                apple.y = rand() % (SCREEN_HEIGHT / TILE_SIZE) * TILE_SIZE;
            }
        }

        // Snake self-collision check
        for (const auto& segment : snake) {
            if (head.x == segment.x && head.y == segment.y) {
                if (score > highScore) highScore = score;
                resetGame();
            }
        }

        // Add new head to snake body
        snake.push_front(head);
        while (snake.size() > static_cast<size_t>(size)) {
            snake.pop_back();
        }

        // Adjust speed based on score
        speedDelay = std::max(5, 25 - score / 5);  // Minimum delay of 5ms

        // Render the game
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (const auto& segment : snake) {
            SDL_RenderFillRect(renderer, &segment);
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (const auto& apple : apples) {
            SDL_RenderFillRect(renderer, &apple);
        }
        
        renderText("Score: " + std::to_string(score), 10, 10);
        renderText("High Score: " + std::to_string(highScore), 10, 50);
        renderText("Speed: " + std::to_string(25 - speedDelay), 10, 90);
        renderText("Press SPACE to Pause", 10, 130);

        SDL_RenderPresent(renderer);
        SDL_Delay(speedDelay);
    }
}

void cleanup() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    if (!init()) {
        printf("Failed at init()");
        return 1;
    }

    gameLoop();
    cleanup();
    return 0;
}
