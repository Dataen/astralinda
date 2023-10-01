#include <Game.h>
#include <memory>

int main()
{
    GameSpecification game_spec{};
    game_spec.width = 2048;
    game_spec.height = 2048;
    game_spec.resizable_window = true;

    auto game = std::make_unique<Game>(game_spec);
    game->run();

    return 0;
}
