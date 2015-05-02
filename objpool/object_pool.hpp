#ifndef _OBJECT_POOL_HPP_
#define _OBJECT_POOL_HPP_

#include <vector>
#include <mutex>

// Object pool used to collect object newed
// release one time.
class ObjectPool {
public:
    ~ObjectPool() {
        for (auto& iter : _objects) {
            delete iter;
            iter = nullptr;
        }
        _objects.clear();
    }

    template<typename T>
    void add(T* t) {
        std::lock_guard<std::mutex> l(_lock);
        _objects.push_back(new Object<T>(t));
    }

private:
    class ObjectBase {
    public:
        virtual ~ObjectBase() {
        }
    };

    template<typename T>
    class Object : public ObjectBase {
    public:
        Object(T* obj) : _obj(obj) {
        }

        virtual ~Object() {
            delete _obj;
            _obj = nullptr;
        }

    private:
        T* _obj;
    };

    // used to store all object
    std::mutex _lock;
    std::vector<ObjectBase*> _objects;
};

#endif
