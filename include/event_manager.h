#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <functional>
#include "event.h"

typedef std::function<void(const Event&)> EventHandler;

//The Event Manager: Registers Listeners to Events and handles new events through a queue
class EventManager{
public:
	enum{nQueues = 2};
	
	EventManager(void);
	~EventManager(void);
	
	int addHandler(EventHandler, const EventType&);
	bool rmHandler(int, const EventType&);
	bool queueEvent(Event*);
	void processQueue(void);
	
private:
    struct Impl;
    Impl* impl_;
};

#endif
