#include "event.h"

constexpr EventType EVT_ARCBALL_ROTATE("EVT_ARCBALL_ROTATE");
constexpr EventType EVT_SELECTION("EVT_SELECTION");
constexpr EventType EVT_ZOOM("EVT_ZOOM");
constexpr EventType EVT_WINDOW_SIZE_CHANGED("EVT_WINDOW_SIZE_CHANGED");

struct ArcballRotateEvent: Event{
    ArcballRotateEvent(float angle_, const glm::vec3& axis_):
        angle(angle_), axis(axis_)
    {}

    const EventType& getEventType(void)const{
        return EVT_ARCBALL_ROTATE;
    }
    ~ArcballRotateEvent(void){};

    float angle;
    glm::vec3 axis;
};

struct SelectionEvent: Event{
    SelectionEvent(float x_, float y_):
        x(x_), y(y_)
    {}

    const EventType& getEventType(void)const{
        return EVT_SELECTION;
    }
    ~SelectionEvent(void){};

    float x, y;
};

struct ZoomEvent: Event{
    ZoomEvent(float dz_):
        dz(dz_)
    {}

    const EventType& getEventType(void)const{
        return EVT_ZOOM;
    }
    ~ZoomEvent(void){};

    float dz;
};

struct WindowSizeEvent: Event{
    WindowSizeEvent(int width_, int height_):
        width(width_), height(height_)
    {}

    const EventType& getEventType(void)const{
        return EVT_WINDOW_SIZE_CHANGED;
    }
    ~WindowSizeEvent(void){};

    int width, height;
};

