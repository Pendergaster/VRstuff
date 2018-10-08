#ifndef GAME_HEADER
#define GAME_HEADER

#define EXPORT extern "C" __declspec(dllexport)

EXPORT void init_game(void* p);
EXPORT void update_game(void* p);
EXPORT void dispose_game(void *);

#endif // !GAME_HEADER
