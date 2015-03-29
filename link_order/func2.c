#include <stdio.h>

void my_func() {
    printf("%s %s:%d\n", __FILE__,  __FUNCTION__, __LINE__);
}
