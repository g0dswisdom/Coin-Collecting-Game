#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL_ttf.h>

/*
    This is a simple coin collecting game created in SDL
*/

bool checkCollision(SDL_Rect rectA, SDL_Rect rectB) {
    return (rectA.x < rectB.x + rectB.w &&
            rectA.x + rectA.w > rectB.x &&
            rectA.y < rectB.y + rectB.h &&
            rectA.y + rectA.h > rectB.y);
}

enum {
    PLAY,
    EXIT,
    BUTTON_COUNT
};

int main() {
    // init libraries

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("Could not initialize SDL! %s\n", SDL_GetError());
    }
    if (TTF_Init() != 0) {
        printf("Could not initialize SDL text library! %s\n", TTF_GetError());
    }
 
    SDL_Window* window = SDL_CreateWindow("Coin Collecting", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 700, 0);
    Uint32 render_flags = SDL_RENDERER_ACCELERATED;
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, render_flags);

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    TTF_Font* font = TTF_OpenFont("./assets/JetBrainsMono-Bold.ttf", 28);
    if (font == NULL) {
        printf("Couldnt load font! %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Color textColor = {0, 0, 0, 255};
    SDL_Surface* text = TTF_RenderText_Solid(font, "Coins: 0", textColor);
    if (text == NULL) {
        printf("Couldnt make text! %s\n", TTF_GetError());
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_FreeSurface(text);
    if (textTexture == NULL) {
        printf("Could not create text texture! %s\n", TTF_GetError());
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    int textWidth, textHeight;
    SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);
    SDL_Rect textRect = {(windowWidth - textWidth) / 2, 10, textWidth, textHeight}; // positions the text at the top

    // main menu
    SDL_Rect buttons[BUTTON_COUNT];
    int buttonX = (windowWidth - 200) / 2;

    buttons[PLAY] = (SDL_Rect){
        buttonX,
        200,
        200,
        50
    };

    buttons[EXIT] = (SDL_Rect) {
        buttonX,
        200 + 50 + 20,
        200,
        50
    };

    int selectedButton = PLAY;
    bool inMenu = true;
    while (inMenu) {
        // button stuff
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    inMenu = false;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_UP:
                            selectedButton = (selectedButton - 1 + BUTTON_COUNT) % BUTTON_COUNT;
                            break;
                        case SDL_SCANCODE_DOWN:
                            selectedButton = (selectedButton + 1) % BUTTON_COUNT;
                            break;
                        case SDL_SCANCODE_RETURN:
                            if (selectedButton == PLAY) {
                                inMenu = false;
                            } else if (selectedButton == EXIT) {
                                TTF_Quit();
                                SDL_Quit();
                                return -1;
                            }
                            break;
                    }
                    break;
            }
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        SDL_Surface* playTextSurface = TTF_RenderText_Solid(font, "Play", textColor);
        SDL_Texture* playTextTexture = SDL_CreateTextureFromSurface(renderer, playTextSurface);
        
        SDL_Surface* exitTextSurface = TTF_RenderText_Solid(font, "Exit", textColor);
        SDL_Texture* exitTextTexture = SDL_CreateTextureFromSurface(renderer, exitTextSurface);

        int playTextWidth, playTextHeight, exitTextWidth, exitTextHeight;
        SDL_QueryTexture(playTextTexture, NULL, NULL, &playTextWidth, &playTextHeight);
        SDL_QueryTexture(exitTextTexture, NULL, NULL, &exitTextWidth, &exitTextHeight);

        SDL_Rect playTextRect = {
            buttons[PLAY].x + (buttons[PLAY].w - playTextWidth) / 2,
            buttons[PLAY].y + (buttons[PLAY].h - playTextHeight) / 2,
            playTextWidth,
            playTextHeight
        };

        SDL_Rect exitTextRect = {
            buttons[EXIT].x + (buttons[EXIT].w - exitTextWidth) / 2,
            buttons[EXIT].y + (buttons[EXIT].h - exitTextHeight) / 2,
            exitTextWidth,
            exitTextHeight
        };

        for(int i = 0; i < BUTTON_COUNT; ++i) {
            if (i == selectedButton) {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
            }
            SDL_RenderFillRect(renderer, &buttons[i]);
        }
        SDL_RenderCopy(renderer, playTextTexture, NULL, &playTextRect);
        SDL_RenderCopy(renderer, exitTextTexture, NULL, &exitTextRect);
        SDL_RenderPresent(renderer);
        SDL_FreeSurface(playTextSurface);
        SDL_FreeSurface(exitTextSurface);
        SDL_DestroyTexture(playTextTexture);
        SDL_DestroyTexture(exitTextTexture);
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    int squareX = (windowHeight - 50) / 2;
    int squareY = (windowWidth - 50) / 2;
    
    SDL_Rect square = {
        (windowHeight - 50) / 2,
        (windowWidth - 50) / 2,
        50,
        50
    };

    SDL_Rect coin = {
        (windowHeight - 50) / 2,
        400,
        50,
        50
    };

    SDL_Event event;
    int speed = 5; // the speed that the player has
    int quit = 0;
    int coins = 0;
    char coinAmount[50]; // used for updating the text

    // game

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.scancode) {
                        // movement
                        case SDL_SCANCODE_UP:
                            square.y -= speed;
                            break;
                        case SDL_SCANCODE_DOWN:
                            square.y += speed;
                            break;
                        case SDL_SCANCODE_RIGHT:
                            square.x += speed;
                            break;
                        case SDL_SCANCODE_LEFT:
                            square.x -= speed;
                            break;
                    }
            }
        }

        bool coinCollision = checkCollision(square, coin);
        if (coinCollision) {
            coins++;
            printf("Coin collected!\n");
            // reposition coin somewhere else
            coin.x = rand() % (windowWidth / 50);
            coin.y = rand() % (windowHeight - 50);
            // update the text
            sprintf(coinAmount, "Coins: %d", coins);

            SDL_Surface* updatedText = TTF_RenderText_Solid(font, coinAmount, textColor);
            SDL_DestroyTexture(textTexture);
            textTexture = SDL_CreateTextureFromSurface(renderer, updatedText);
            SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);

            textRect.x = (windowWidth - textWidth) / 2,
            textRect.y = 10;
            textRect.w = textWidth;
            textRect.h = textHeight;

            SDL_FreeSurface(updatedText);
        }

        SDL_SetRenderDrawColor(renderer, 2, 204, 254, 255);
        SDL_RenderClear(renderer);
        // render text
        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        // render game objects
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 
        SDL_RenderFillRect(renderer, &square); 
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderFillRect(renderer, &coin);

        SDL_RenderPresent(renderer);
    }
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}