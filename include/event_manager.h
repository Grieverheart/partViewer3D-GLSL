#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <map>
#include <vector>
#include <functional>
#include "event.h"

typedef std::function<void(const Event&)> EventHandler;

//A helper signal class for handler functions
class EventSignal{
public:
	EventSignal(void);
	int connect(EventHandler);
	void disconnect(int key);
	void operator() (const Event&)const;
	bool empty(void)const;
	
private:
	int next_key_;
	std::map<int, EventHandler> handlers_;
};

//The Event Manager: Registers Listeners to Events and handles new events through a queue
class EventManager{
public:
	enum{nQueues = 2};
	
	explicit EventManager(void):
		active_queue_(0)
	{};
	
	~EventManager(void){
		for(int i : {0, 1}){
			//Delete events if any remaining
			if(!queue_[i].empty()){
				for(auto event : queue_[i]) delete event;
			}
		}
	};
	
	int addHandler(EventHandler, const EventType&);
	bool rmHandler(int, const EventType&);
	bool queueEvent(Event*);
	void processQueue(void);
	
private:
	int active_queue_;
	std::map<EventType, EventSignal> evt_assocs_;
	std::vector<Event*> queue_[nQueues];
};

#endif
