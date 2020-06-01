#pragma once

#include "hime2d.h"

NS_HIME_BEGIN

/**
 *  Wrapper around an enum that allows simple use of bitwise logic operations.
 */
class Flags {
	Flags(unsigned int value) : _flags(value) {}
	inline void set(unsigned int type) { _flags |= type; }
	inline void unset(unsigned int type) { _flags &= ~type; }
	inline bool isset(unsigned int type) const { return (_flags & type) != 0; }
	operator unsigned int() const {
		return static_cast<unsigned int>(_flags);
	}
private:
    unsigned int _flags;
};

/**
 *  Helper template class for auto singleton. 
 */
template <typename T>
class singleton {
public:
    static T* getInstance()
    {
        if (!_instance) {
            _instance = new (std::nothrow)T;
        }
        return _instance;
    }
    static void destroyInstance()
    {
        HM_SAFE_DELETE(_instance);
    }
protected:
    static T* _instance;
};
template <typename T>
T *singleton<T>::_instance;


/**
 *  Compile time string hash
 */
namespace util
{
    inline constexpr size_t hash(char const* input)
    {
        return *input ? *input + 33ull * hash(input + 1) : 5381;
    }
    inline size_t hash(const char* input, size_t size, int index)
    {
        return index < size ? input[index] + 33ull * hash(input, size, index + 1) : 5381;
    }
    inline size_t hash(const std::string& str)
    {
        return hash(str.c_str(), str.size(), 0);
    }
}
inline size_t constexpr operator "" _sid(const char* s, size_t)
{
    return util::hash(s);
}


#ifdef min
    #undef min
#endif
#ifdef max
    #undef max
#endif

namespace util
{
    template <class T>
    inline T clamp(T value, T minVal, T maxVal)
    {
        auto pair = std::minmax(minVal, maxVal);
        return std::max(std::min(value, pair.second), pair.first);
    }
}

NS_HIME_END