#include <SDL2/SDL.h>          // Biblioteca principal da SDL2
#include <SDL_events.h>        // Eventos de entrada (teclado, mouse, etc.)
#include <SDL_keycode.h>       // Códigos de teclas (SDLK_w, SDLK_s, etc.)
#include <SDL_rect.h>          // Estrutura SDL_Rect usada para retângulos
#include <SDL_render.h>        // Funções de renderização
#include <SDL_timer.h>         // Controle de tempo e delays
#include <cstdint>             // Tipos inteiros de tamanho fixo
#include <iostream>            // Entrada e saída padrão em C++
#include <random>              // Geração de números aleatórios

int winW {500};                // Largura da janela
int winH {500};                // Altura da janela

// Função que gera um número float aleatório entre min e max
float randomfloat(float min, float max) {
    static std::random_device rd;             // Dispositivo para gerar semente
    static std::mt19937 gen(rd());            // Gerador pseudoaleatório Mersenne Twister
    std::uniform_real_distribution<float> dis(min, max); // Distribuição uniforme
    return dis(gen);                          // Retorna valor sorteado
}

// Função que retorna -1 ou +1 de forma aleatória
int randomSign() {
    static std::random_device rd;             // Semente
    static std::mt19937 gen(rd());            // Gerador
    std::uniform_int_distribution<int> dis(0, 1); // Sorteia 0 ou 1
    return dis(gen) == 0 ? -1 : 1;            // Retorna -1 se for 0, senão +1
}

int main(){
    // Inicializa todos os subsistemas da SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0){
        SDL_Log("ERRO", SDL_GetError());      // Loga erro se falhar
        return 1;                             // Sai do programa
    }

    // Cria uma janela com título "Pong"
    SDL_Window *window = SDL_CreateWindow(
        "Pong",                               // Título
        SDL_WINDOWPOS_CENTERED,               // Posição X centralizada
        SDL_WINDOWPOS_CENTERED,               // Posição Y centralizada
        winW,                                 // Largura
        winH,                                 // Altura
        SDL_WINDOW_SHOWN                      // Exibir janela
    );
    if (!window) {                            // Se falhar ao criar a janela
        std::cerr << "Erro ao criar janela: " << SDL_GetError() << std::endl;
        SDL_Quit();                           // Fecha SDL
        return 1;                             // Sai do programa
    }

    // Cria um renderizador para a janela
    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,                               // Janela associada
        -1,                                   // Driver padrão
        SDL_RENDERER_ACCELERATED              // Usa aceleração de hardware
    );
    if (!renderer) {                          // Se falhar ao criar renderer
        std::cerr << "Erro ao criar renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);            // Destroi janela
        SDL_Quit();                           // Fecha SDL
        return 1;                             // Sai do programa
    }

    bool run {true};                          // Controle do loop principal
    SDL_Event event;                          // Estrutura para eventos

    bool wPress {false};                      // Flag se tecla W está pressionada
    bool sPress {false};                      // Flag se tecla S está pressionada

    float speed {1.2f};                       // Velocidade vertical do jogador
    float maxSpeed {5.0f};                    // Velocidade máxima
    float acceleration {0.1f};                // Aceleração
    float friction {0.2};                     // Atrito para desaceleração
    float posY {0.0f};                        // Deslocamento do jogador

    int cubeHeight = 100;                     // Altura da raquete do jogador

    const int ballW {10};                     // Largura da bola
    const int ballH {10};                     // Altura da bola

    float startingPositionX = winW / 2.0f;    // Posição inicial X da bola
    float startingPositionY = winH / 2.0f;    // Posição inicial Y da bola

    float ballX = startingPositionX;          // Posição atual X da bola
    float ballY = startingPositionY;          // Posição atual Y da bola

    float ballSpeed = 5.0f;                   // Velocidade total da bola

    // Define velocidade Y aleatória da bola
    float ballSpeedY = randomfloat(0.0f, ballSpeed) * randomSign();
    // Calcula velocidade X restante
    float ballSpeedX = (ballSpeed - std::abs(ballSpeedY)) * randomSign();

    // Variáveis do oponente (IA)
    int opponentX = winW - 20;                // Posição X fixa da raquete adversária
    float opponentY = winH / 2 - cubeHeight / 2; // Posição inicial Y
    float opponentCenter = opponentY + cubeHeight / 2; // Centro da raquete
    float aiResponsiveness = 0.05f;           // Responsividade da IA

    bool reset = false;                       // Flag para resetar a bola

    // Loop principal do jogo
    while (run) {
        // Processa eventos
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) run = false; // Fecha janela

            // Detecta tecla pressionada
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_w) wPress = true;
                if (event.key.keysym.sym == SDLK_s) sPress = true;
                if (event.key.keysym.sym == SDLK_r) reset = true;
            }
            // Detecta tecla liberada
            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_w) wPress = false;
                if (event.key.keysym.sym == SDLK_s) sPress = false;
                if (event.key.keysym.sym == SDLK_r) reset = false;
            }
        }

        // Movimento do jogador
        if (wPress) {                         // Se W pressionado, sobe
            speed -= acceleration;
            if (speed < -maxSpeed) speed = -maxSpeed;
        }
        else if (sPress) {                    // Se S pressionado, desce
            speed += acceleration;
            if (speed > maxSpeed) speed = maxSpeed;
        } else {                              // Caso contrário aplica atrito
            if (speed > 0) {
                speed -= friction;
                if (speed < 0) speed = 0;
            } else if (speed < 0) {
                speed += friction;
                if (speed > 0) speed = 0;
            }
        }

        // Define cor de fundo preta
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);            // Limpa tela

        posY += speed;                        // Atualiza posição do jogador

        // Define retângulo da raquete do jogador
        SDL_Rect rect = {
            10,                               // X
            (winH / 2 - cubeHeight / 2) + static_cast<int>(posY), // Y
            10,                               // Largura
            cubeHeight                        // Altura
        };

        // Limita movimento do jogador à tela
        if (rect.y < 0) {
            rect.y = 0;
            posY = rect.y - (winH / 2 - cubeHeight / 2);
        }
        if (rect.y + rect.h > winH) {
            rect.y = winH - rect.h;
            posY = rect.y - (winH / 2 - cubeHeight / 2);
        }

        // Movimento do oponente (IA)
        float opponentCenter = opponentY + cubeHeight / 2;
        float ballCenter = ballY + ballH / 2;
        float delta = ballCenter - opponentCenter;

        // Só move quando bola está na metade direita e indo para direita
        if (ballX > (winW / 3) && ballSpeedX > 0) {
            opponentY += delta * aiResponsiveness;
        }

        // Limita oponente à tela
        if (opponentY < 0) opponentY = 0;
        if (opponentY + cubeHeight > winH) opponentY = winH - cubeHeight;

        // Retângulo do oponente
        SDL_Rect opponentRect = {
            opponentX,
            static_cast<int>(opponentY),
            10,
            cubeHeight
        };
        
        // Movimento da bola
        ballY += ballSpeedY;
        ballX += ballSpeedX;

        // Rebater nas bordas superior e inferior
        if ((ballY + ballH) > winH) {
            ballSpeedY = -ballSpeedY;
        } else if (ballY < 0) {
            ballSpeedY = -ballSpeedY;
        }

        // Resetar bola se sair da tela ou se apertar R
        if (ballX > winW || ballX < 0 || reset) {
            ballX = startingPositionX;
            ballY = startingPositionY;
            ballSpeedY = randomfloat(0.0f, ballSpeed) * randomSign();
            ballSpeedX = (ballSpeed - std::abs(ballSpeedY)) * randomSign();
        }
        
        // Define retângulo da bola
        SDL_Rect ball = {
            static_cast<int>(ballX),
            static_cast<int>(ballY),
            ballW,
            ballH
        };

        // Colisão da bola com jogador
        if (SDL_HasIntersection(&rect, &ball)) {
            float hitPos = (ballY + ballH/2) - (rect.y + rect.h/2); // Distância do centro
            ballSpeedY = hitPos * 0.1f;          // Ajusta ângulo
            ballSpeedX = std::abs(ballSpeedX);   // Bola vai para direita
        }

        // Colisão da bola com oponente
        if (SDL_HasIntersection(&opponentRect, &ball)) {
            ballSpeedX = -ballSpeedX;            // Bola vai para esquerda
        }

        // Desenha jogador (branco)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &rect);

        // Desenha bola (vermelha)
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &ball);

        // Desenha oponente (ciano)
        SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        SDL_RenderFillRect(renderer, &opponentRect);

        SDL_RenderPresent(renderer);             // Atualiza tela

        SDL_Delay(16);                           // Delay ~60 FPS
    }

    // Libera recursos
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;                                    // Sai com sucesso
}
