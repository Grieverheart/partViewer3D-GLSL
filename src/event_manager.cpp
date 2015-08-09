#include "include/event_manager.h"
#include <cstring>

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

int EventManager::addHandler(EventHandler handler, const EventType& type){
    //Check if the event type already exists in Map
    EventSignal &signal = evt_assocs_[type];
    //Duplicates are allowed since we can't check if the handlers are equal
    //Add handler to the signal
    return signal.connect(handler);
}

bool EventManager::rmHandler(int handler_key, const EventType& type){
    auto em_itr = evt_assocs_.find(type);
    //Return false if the event does not exist
    if(em_itr == evt_assocs_.end()){
        return false;
    }
    EventSignal &signal = em_itr->second;
    signal.disconnect(handler_key);
    //If no handlers left, delete entry
    if(signal.empty()) evt_assocs_.erase(em_itr);
    return true;
}

bool EventManager::queueEvent(Event *event){
    queue_[active_queue_].push_back(event);
    return true;
}

void EventManager::processQueue(void){
    if(!queue_[active_queue_].empty()){
        for(auto event_ptr: queue_[active_queue_]){
            //Find where the type of the event is stored
            auto EventPairItr = evt_assocs_.find(event_ptr->getEventType());
            if(EventPairItr != evt_assocs_.end()){
                const EventSignal &signal = EventPairItr->second;
                //call handlers
                signal(*event_ptr);
            }
        }
        queue_[active_queue_].clear();
    }
    active_queue_ = (active_queue_ + 1)%nQueues;
}


