// This header provides wrappers for common Web APIs
#ifdef __EMSCRIPTEN__
    #define WEB true
    #include <emscripten/html5.h>
    #include <emscripten.h>
#else
    #define WEB false
#endif

#include <defs.h>
#if WEB
    // length = 0 for null terminated
    void  LocalStorageSet(const char* key, const char* value, usize length);
    usize LocalStorageGet(const char* key, char** outValue);
#endif
