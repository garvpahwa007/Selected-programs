#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    // NUMBER GUESSING GAME

    srand(time(NULL)); // Seed the random number generator
    int guess = 0;
    int tries = 0;
    int min = 1;
    int max = 100;
    int answer = (rand() % (max - min + 1)) + min;
    printf("NUMBER GUESSING GAME\n");

    do
    {
        printf("Enter your guess (between %d and %d): ", min, max);
        scanf("%d", &guess);
        tries++;

        if (guess < answer)
        {
            printf("Too low! Try again.\n");
        }
        else if (guess > answer)
        {
            printf("Too high! Try again.\n");
        }
        else
        {
            printf("Congratulations! You guessed the number %d in %d tries.\n", answer, tries);
        }
    } while (guess != answer);
    printf("the answer is %d\n", answer);
    printf("It took you %d tries.\n", tries);
    return 0;
}
