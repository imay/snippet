#include "object_pool.hpp"

struct abc {
    int a;
};

struct bcd {
    int b;
};

int main() {
    ObjectPool pool;

    pool.add(new abc);
    pool.add(new bcd);
    return 0;
}
