#ifndef __GLOBAL_H_INCLUDED
#define __GLOBAL_H_INCLUDED

#include <cassert>
#include <thread>
#include <mutex>
#include <memory>


using std::mutex;
using std::thread;
using std::lock_guard;
//using std::make_shared;
using std::shared_ptr;


#define IN_RAGNE(Value, Begin, End) (((Value) >= (Begin)) && ((Value) < (End)))
#define ARRAY_SIZE(Array) (sizeof((Array)) / sizeof((Array)[0]))
#define ASSERT(Condition) assert(Condition)

#define STORAGE_SIZE	1024


#endif // __GLOBAL_H_INCLUDED
