#include "event.h"

constexpr EventType EVT_ARCBALL_ROTATE("EVT_ARCBALL_ROTATE");
constexpr EventType EVT_SELECTION("EVT_SELECTION");
constexpr EventType EVT_MOUSE_CLICK("EVT_MOUSE_CLICK");
constexpr EventType EVT_MOUSE_MOTION("EVT_MOUSE_MOTION");
constexpr EventType EVT_MOUSE_SCROLL("EVT_MOUSE_SCROLL");
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

struct MouseClickEvent: Event{
    MouseClickEvent(int button_, float x_, float y_, int action_, int mods_):
        x(x_), y(y_),
        button(button_), action(action_), mods(mods_)
    {}

    const EventType& getEventType(void)const{
        return EVT_MOUSE_CLICK;
    }

    ~MouseClickEvent(void){};

    float x, y;
    int button, action, mods;
};

struct MouseMotionEvent: Event{
    MouseMotionEvent(float x_, float y_):
        x(x_), y(y_)
    {}

    const EventType& getEventType(void)const{
        return EVT_MOUSE_MOTION;
    }
    ~MouseMotionEvent(void){};

    float x, y;
};

struct MouseScrollEvent: Event{
    MouseScrollEvent(float dz_):
        dz(dz_)
    {}

    const EventType& getEventType(void)const{
        return EVT_MOUSE_SCROLL;
    }
    ~MouseScrollEvent(void){};

    float dz;
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

