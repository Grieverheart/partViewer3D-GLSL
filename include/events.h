#include "event.h"
constexpr EventType EVT_MOUSE_CLICK("EVT_MOUSE_CLICK");
constexpr EventType EVT_MOUSE_MOTION("EVT_MOUSE_MOTION");
constexpr EventType EVT_MOUSE_SCROLL("EVT_MOUSE_SCROLL");
constexpr EventType EVT_WINDOW_SIZE_CHANGED("EVT_WINDOW_SIZE_CHANGED");

struct MouseClickEvent: Event{
    MouseClickEvent(float x_, float y_, int button_, int action_, int mods_):
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

