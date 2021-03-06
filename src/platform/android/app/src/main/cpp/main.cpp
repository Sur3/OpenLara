#include <jni.h>
#include <android/log.h>
#include <sys/time.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>

#include "game.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_org_xproger_openlara_Wrapper_##method_name

// timing
time_t startTime;

int osGetTime() {
    timeval t;
    gettimeofday(&t, NULL);
    return int((t.tv_sec - startTime) * 1000 + t.tv_usec / 1000);
}

// joystick
bool osJoyReady(int index) {
    return index == 0;
}

void osJoyVibrate(int index, float L, float R) {
    //
}

extern "C" {

char Stream::cacheDir[255];
char Stream::contentDir[255];

JNI_METHOD(void, nativeInit)(JNIEnv* env, jobject obj, jstring contentDir, jstring cacheDir) {
    timeval t;
    gettimeofday(&t, NULL);
    startTime = t.tv_sec;	

    const char* str;

    Stream::contentDir[0] = Stream::cacheDir[0] = 0;

    str = env->GetStringUTFChars(contentDir, NULL);
    strcat(Stream::contentDir, str);
    env->ReleaseStringUTFChars(contentDir, str);

    str = env->GetStringUTFChars(cacheDir, NULL);
    strcat(Stream::cacheDir, str);
    env->ReleaseStringUTFChars(cacheDir, str);

    Game::init();
}

JNI_METHOD(void, nativeFree)(JNIEnv* env) {
    Game::deinit();
}

JNI_METHOD(void, nativeReset)(JNIEnv* env) {
//  core->reset();
}

JNI_METHOD(void, nativeUpdate)(JNIEnv* env) {
    Game::update();
}

JNI_METHOD(void, nativeRender)(JNIEnv* env) {
    Game::render();
}

JNI_METHOD(void, nativeResize)(JNIEnv* env, jobject obj, jint w, jint h) {
    Core::width  = w;
    Core::height = h;
}

float DeadZone(float x) {
    return x = fabsf(x) < 0.2f ? 0.0f : x;
}

InputKey keyToInputKey(int code) {
    int codes[] = {
        21, 22, 19, 20, 62, 61, 66, 111, 59, 113, 57,
        7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
        29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
        42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54,
    };

    for (int i = 0; i < sizeof(codes) / sizeof(codes[0]); i++)
        if (codes[i] == code)
            return (InputKey)(ikLeft + i);
    return ikNone;
}

JNI_METHOD(void, nativeTouch)(JNIEnv* env, jobject obj, jint id, jint state, jfloat x, jfloat y) {
// gamepad / keyboard
    if (state < 0) {
        switch (state) {
            case -3 : Input::setJoyPos(id, jkL, vec2(DeadZone(x), DeadZone(y))); break;
            case -4 : Input::setJoyPos(id, jkR, vec2(DeadZone(x), DeadZone(y))); break;
            default : {
                int btn = int(x);
                LOG("key %d = %d\n", btn, state);
                if (btn < 0)
                    Input::setJoyDown(id, JoyKey(jkNone - btn), state != -1);
                else
                    Input::setDown(keyToInputKey(btn), state != -1);
            }
        }
        return;
    }

    if (id == -100) {
        /*
        switch (state) {
            case 0 : Input::head.basis.rot.x = x; Input::head.basis.rot.y = y; break;
            case 1 : Input::head.basis.rot.z = x; Input::head.basis.rot.w = y; Input::head.set(); break;
        }
        */
        return;
    }

// touch
    InputKey key = Input::getTouch(id);
    if (key == ikNone) return;
    Input::setPos(key, vec2(x, y));
    if (state == 1 || state == 2)
        Input::setDown(key, state == 2);
}

JNI_METHOD(void, nativeSoundFill)(JNIEnv* env, jobject obj, jshortArray buffer) {
    jshort *frames = env->GetShortArrayElements(buffer, NULL);
    jsize count = env->GetArrayLength(buffer) / 2;
    Sound::fill((Sound::Frame*)frames, count);
    env->ReleaseShortArrayElements(buffer, frames, 0);
}

}