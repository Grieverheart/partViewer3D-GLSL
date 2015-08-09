#include "event.h"

constexpr EventType EVT_ARCBALL_ROTATE("EVT_ARCBALL_ROTATE");
constexpr EventType EVT_ARCBALL_START("EVT_ARCBALL_START");
constexpr EventType EVT_ARCBALL_END("EVT_ARCBALL_END");

constexpr EventType EVT_SELECTION("EVT_SELECTION");

constexpr EventType EVT_ZOOM("EVT_ZOOM");

struct ArcballStartEvent: Event{
    const EventType& getEventType(void)const{
        return EVT_ARCBALL_START;
    }
    ~ArcballStartEvent(void){};
};

struct ArcballEndEvent: Event{
    const EventType& getEventType(void)const{
        return EVT_ARCBALL_END;
    }
    ~ArcballEndEvent(void){};
};

struct ArcballRotateEvent: Event{
    ArcballRotateEvent(const glm::mat4& rot):
        rotation(rot)
    {}

    const EventType& getEventType(void)const{
        return EVT_ARCBALL_ROTATE;
    }
    ~ArcballRotateEvent(void){};

    glm::mat4 rotation;
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

