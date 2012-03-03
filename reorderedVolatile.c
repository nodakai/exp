volatile int Ready;

int Message[100];

void foo( int i ) {
    Message[i/10] = 42;
    Ready = 1;

}
