#ifndef GAME_HEADER
#define GAME_HEADER
#ifdef _WIN32
#define EXPORT extern "C" __declspec(dllexport)
#endif
#ifdef __linux__
#define EXPORT extern "C" 
#endif
EXPORT void init_game(void* p);
EXPORT void update_game(void* p);
EXPORT void on_game_reload(void* p);
EXPORT void dispose_game(void *);

#endif // !GAME_HEADER
