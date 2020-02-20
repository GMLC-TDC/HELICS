
#include <stdio.h>

class A {
  public:
    template<typename... Args>
    virtual void doInitialize(Args... args);
};

public static void main(int argc, char** argv) {
    printf("Hello World\n");
}
