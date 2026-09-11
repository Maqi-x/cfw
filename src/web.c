#include <web.h>

#if WEB

EM_JS(void, LocalStorageSet, (const char* key, const char* value, usize length), {
    const keyStr = UTF8ToString(key);
    let valStr;

    if (length === 0) {
        valStr = UTF8ToString(value);
    } else {
        valStr = UTF8ToString(value, length);
    }

    try {
        localStorage.setItem(keyStr, valStr);
    } catch (e) {
        console.error("LocalStorageSet failed:", e);
    }
});

EM_JS(usize, LocalStorageGet, (const char* key, char** outValue), {
    const keyStr = UTF8ToString(key);
    const val = localStorage.getItem(keyStr);

    if (val === null) {
        setValue(outValue, 0, 'i32');
        return 0;
    }

    const lengthBytes = lengthBytesUTF8(val);
    const ptr = _malloc(lengthBytes + 1);

    stringToUTF8(val, ptr, lengthBytes + 1);
    setValue(outValue, ptr, 'i32');

    return lengthBytes;
});

#endif
