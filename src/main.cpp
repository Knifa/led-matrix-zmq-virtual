#include <SDL.h>
#include <cstdint>
#include <iostream>
#include <zmq.hpp>

int main(int argc, char *argv[]) {
  const int matrixWidth = 128;
  const int matrixHeight = 128;
  const int matrixBpp = 24;
  const int displayScale = 8;

  const int windowWidth = matrixWidth * displayScale;
  const int windowHeight = matrixHeight * displayScale;

  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window =
      SDL_CreateWindow("led-matrix-zmq-virtual", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, 0);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
  SDL_Texture *texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                        SDL_TEXTUREACCESS_STREAMING, matrixWidth, matrixHeight);

  zmq::context_t zmqCtx;
  zmq::socket_t zmqSock(zmqCtx, zmq::socket_type::rep);
  zmqSock.bind("tcp://*:42024");

  bool running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      }
    }

    void *texturePixels;
    int texturePitch;
    SDL_LockTexture(texture, nullptr, &texturePixels, &texturePitch);
    zmqSock.recv(texturePixels, matrixWidth * matrixHeight * matrixBpp / 8);
    zmqSock.send(nullptr, 0);
    SDL_UnlockTexture(texture);

    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
  }

  zmqSock.close();
  zmqCtx.close();

  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
