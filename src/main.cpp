#include <cstdint>
#include <iostream>
#include <string>

#include <SDL.h>
#include <getopt.h>
#include <zmq.hpp>

class Options {
public:
  enum class Args { endpoint = 1, width, height, scale, bpp };

  std::string endpoint = "tcp://*:42024";
  int width = 128;
  int height = 128;
  int scale = 8;
  int bpp = 24;

  static Options from_args(int argc, char *argv[]) {
    Options server_options;

    static struct option long_opts[]{
        {"zmq-endpoint", optional_argument, nullptr,
         static_cast<int>(Options::Args::endpoint)},
        {"width", optional_argument, nullptr,
         static_cast<int>(Options::Args::width)},
        {"height", optional_argument, nullptr,
         static_cast<int>(Options::Args::height)},
        {"scale", optional_argument, nullptr,
         static_cast<int>(Options::Args::scale)},
        {"bpp", optional_argument, nullptr,
         static_cast<int>(Options::Args::bpp)},
        {nullptr, 0, nullptr, 0}};

    int optCode;
    int optIndex;
    while ((optCode = getopt_long(argc, argv, "", long_opts, &optIndex)) !=
           -1) {
      switch (optCode) {
      case static_cast<int>(Options::Args::endpoint):
        server_options.endpoint = std::string(optarg);
        break;

      case static_cast<int>(Options::Args::width):
        server_options.width = std::stoi(optarg);
        break;

      case static_cast<int>(Options::Args::height):
        server_options.height = std::stoi(optarg);
        break;

      case static_cast<int>(Options::Args::scale):
        server_options.scale = std::stoi(optarg);
        break;

      case static_cast<int>(Options::Args::bpp):
        server_options.bpp = std::stoi(optarg);
        break;
      }
    }

    return server_options;
  }
};

int main(int argc, char *argv[]) {
  const auto options = Options::from_args(argc, argv);

  const auto windowWidth = options.width * options.scale;
  const auto windowHeight = options.height * options.scale;

  SDL_Init(SDL_INIT_VIDEO);
  auto *window =
      SDL_CreateWindow("led-matrix-zmq-virtual", SDL_WINDOWPOS_UNDEFINED,
                       SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, 0);
  auto *renderer = SDL_CreateRenderer(window, -1, 0);
  auto *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24,
                                    SDL_TEXTUREACCESS_STREAMING, options.width,
                                    options.height);

  zmq::context_t zmqCtx;
  zmq::socket_t zmqSock(zmqCtx, zmq::socket_type::rep);
  zmqSock.bind(options.endpoint);

  auto running = true;
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
    zmqSock.recv(texturePixels,
                 options.width * options.height * options.bpp / 8);
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
