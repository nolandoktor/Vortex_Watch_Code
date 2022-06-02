#include <Arduino.h>
#include "WatchFaceManager.h"
#include "WatchFace.h"
#include "DoubleBuffer.h"

WatchFaceManager::WatchFaceManager()
{
    for (int i=0; i<NUM_WATCH_FACES; i++) {
        watch_faces[i] = NULL;
    }
    current_face = (watch_face_t)0;
}
int WatchFaceManager::init(watch_face_t start_face)
{
    Serial.println("WatchFace manager init");
    bool init_success = true;
    for (int i=0; i<NUM_WATCH_FACES; i++) {
        if (watch_faces[i] != NULL) {
            if (watch_faces[i]->reset() < 0) {
                init_success = false;
                Serial.print("Failed to init watch face: ");
                Serial.println(watch_faces[i]->get_name());
            }
        }
    }

    //At least one state inits failed
    if (!init_success) {
        return -1;
    }
    if (watch_faces[start_face] == NULL) {
        return -1;
    }
    current_face = start_face;
    return 0;
}
int WatchFaceManager::assign_face(watch_face_t face, WatchFace *element)
{
    watch_faces[face] = element;
    return 0;
}
int WatchFaceManager::change_face(watch_face_t next_face)
{
    if (watch_faces[next_face] == NULL) {
        return -1;
    }
    current_face = next_face;
    watch_faces[current_face]->reset();
    return 0;
}
int WatchFaceManager::update()
{
    if (watch_faces[current_face] == NULL) {
        return -1;
    }
    watch_faces[current_face]->update();
    return 0;
}
int WatchFaceManager::draw(DoubleBuffer *frame_buffer)
{
    if (watch_faces[current_face] == NULL) {
        return -1;
    }
    watch_faces[current_face]->draw(frame_buffer);
    return 0;
}
const char* WatchFaceManager::get_face_name(watch_face_t face)
{
    if (watch_faces[face] == NULL) {
        return NULL;
    }
    return watch_faces[face]->get_name();
}