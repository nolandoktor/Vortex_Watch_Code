#pragma once

#include "WatchFace.h"
#include "DoubleBuffer.h"

class WatchFaceManager {
    private:
        WatchFace *watch_faces[NUM_WATCH_FACES];
        watch_face_t current_face;
    public:
        WatchFaceManager();
        int init(watch_face_t start_face);
        int assign_face(watch_face_t face, WatchFace *element);
        int change_face(watch_face_t face);
        int update();
        int draw(DoubleBuffer *frame_buffer);
        const char* get_face_name(watch_face_t face);
};