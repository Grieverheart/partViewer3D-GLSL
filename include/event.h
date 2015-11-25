#ifndef _EVENT_H
#define _EVENT_H

#include <cstdint>


//A Hashed String. Stores the string and its hash which is calculated uppon construction
class HashedString{
public:
	template<size_t length>
    explicit constexpr HashedString(const char (&text)[length])noexcept:
        text_(text),
        hash_(calcHash(text))
    {};
    constexpr uint32_t getHash(void)const{
        return hash_;
    };
    constexpr const char* getText(void)const{
        return text_;
    };
    //Use of the FNV-1a algorithm for hashing
	template<size_t length>
    static constexpr uint32_t calcHash(const char (&text)[length]){
        return FNV1aRec<length>(text, FNVoffset);
    };

    //Comparison operators for use with set
    bool operator< (const HashedString &other)const{
        return (hash_ < other.getHash()); //possibly use reinterpret cast
    };
    bool operator== (const HashedString &other)const{
        return (hash_ == other.getHash()); //possibly use reinterpret cast
    };

private:
    static constexpr uint32_t FNVoffset = 2166136261u;
    static constexpr uint32_t FNVprime  = 16777619u;
	//TODO: This more complex than intended because of VS not handling const char* parameters
	//to constexpr functions. It also cannot handle recursive templates. Try to find a better
	//way to write this and comply with VS.
	template<size_t idx>
	struct some_struct {};
	template<size_t length>
    static constexpr uint32_t FNV1aRec(const char (&text)[length], uint32_t hash){
        //NOTE: case sensitive!
        return FNV1aRecRec<length>(text, hash, some_struct<0>());
    }
	template<size_t length, size_t idx>
	static constexpr uint32_t FNV1aRecRec(const char(&text)[length], uint32_t hash, some_struct<idx>) {
		//NOTE: case sensitive!
		return FNV1aRecRec<length>(text, (hash ^ uint32_t(text[idx])) * FNVprime, some_struct<idx + 1>());
	}
	template<size_t length>
	static constexpr uint32_t FNV1aRecRec(const char(&text)[length], uint32_t hash, some_struct<length - 1>) {
		//NOTE: case sensitive!
		return hash;
	}
    const char *text_;
    const uint32_t hash_;
};
typedef HashedString EventType;

//An abstract event. Parties should implement their own events as
//they see fit i.e ActorMoved would contain actor id and evtType "actor_moved"
struct Event{
    virtual const EventType& getEventType(void)const = 0;
    virtual ~Event(void) = 0;
};

inline Event::~Event(void){}

#endif
