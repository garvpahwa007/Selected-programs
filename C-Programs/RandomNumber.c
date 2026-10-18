#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    // Seed the random number generator
    srand(time(NULL));

    int min = 1;
    int max = 100;

    int randomNumber = (rand() % (max - min + 1)) + min;

    printf("Random Number: %d\n", randomNumber);
    return 0;
}