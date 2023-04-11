#ifndef FLAG_H
#define FLAG_H
#include <atomic>

class Flag{
        std::atomic_bool flag;
    public:
        void initFlag(bool);
        void setFlag();
        void resetFlag();
        bool isFlagSet();
};
inline void Flag::initFlag(bool state = true)
{
    if(state)
        flag = 1;
    else
        flag = 0;
}
inline void Flag::setFlag(){
    flag = 1;
}

inline void Flag::resetFlag(){
    flag = 0;
}

inline bool Flag::isFlagSet(){
    if(flag)
        return true;
    return false;
}

#endif