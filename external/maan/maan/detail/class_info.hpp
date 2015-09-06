#ifndef __CLASSINFO_HPP
#define __CLASSINFO_HPP

namespace maan{namespace detail{
    template<class T>
    class ClassInfo{
    public:
        static const void* get_metatable_key(void){
            return static_cast<void*>(&metatable_key);
        }
    private:
        static char metatable_key;
    };

    template<class T>
    char ClassInfo<T>::metatable_key = 0;
}}

#endif
