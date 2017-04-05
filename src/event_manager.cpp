#include "include/event_manager.h"
#include <cstring>
#include <map>
#include <vector>

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

EventSignal::EventSignal(void):
    next_key_(0)
{}

int EventSignal::connect(EventHandler handler){
    int k = next_key_++;
    handlers_[k] = handler;
    return k;
}

void EventSignal::disconnect(int key){
    handlers_.erase(key);
}

void EventSignal::operator() (const Event& event)const{
    for(auto handler_pair: handlers_){
        const EventHandler& handler = handler_pair.second;
        handler(event);
    }
}

bool EventSignal::empty(void)const{
    return handlers_.empty();
}

struct EventManager::Impl{
	int active_queue_;
	std::map<EventType, EventSignal> evt_assocs_;
	std::vector<Event*> queue_[nQueues];
};

	
EventManager::EventManager(void):
    impl_(new Impl())
{
    impl_->active_queue_ = 0;
}

EventManager::~EventManager(void){
    for(int i : {0, 1}){
        //Delete events if any remaining
        if(!impl_->queue_[i].empty()){
            for(auto event : impl_->queue_[i]) delete event;
        }
    }
    delete impl_;
}

int EventManager::addHandler(EventHandler handler, const EventType& type){
    //Check if the event type already exists in Map
    EventSignal &signal = impl_->evt_assocs_[type];
    //Duplicates are allowed since we can't check if the handlers are equal
    //Add handler to the signal
    return signal.connect(handler);
}

bool EventManager::rmHandler(int handler_key, const EventType& type){
    auto em_itr = impl_->evt_assocs_.find(type);
    //Return false if the event does not exist
    if(em_itr == impl_->evt_assocs_.end()){
        return false;
    }
    EventSignal &signal = em_itr->second;
    signal.disconnect(handler_key);
    //If no handlers left, delete entry
    if(signal.empty()) impl_->evt_assocs_.erase(em_itr);
    return true;
}

bool EventManager::queueEvent(Event *event){
    impl_->queue_[impl_->active_queue_].push_back(event);
    return true;
}

void EventManager::processQueue(void){
    if(!impl_->queue_[impl_->active_queue_].empty()){
        for(auto event_ptr: impl_->queue_[impl_->active_queue_]){
            //Find where the type of the event is stored
            auto EventPairItr = impl_->evt_assocs_.find(event_ptr->getEventType());
            if(EventPairItr != impl_->evt_assocs_.end()){
                const EventSignal &signal = EventPairItr->second;
                //call handlers
                signal(*event_ptr);
            }
        }
        impl_->queue_[impl_->active_queue_].clear();
    }
    impl_->active_queue_ = (impl_->active_queue_ + 1)%nQueues;
}


