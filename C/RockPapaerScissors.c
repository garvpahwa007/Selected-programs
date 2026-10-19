#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int getUserChoice();
int getComputerChoice();
void checkWinner(int userChoice, int computerChoice);

int main()
{
    // ROCK PAPER SCISSORS GAME
    srand(time(NULL));

    printf("ROCK PAPER SCISSORS GAME\n");

    int userChoice = getUserChoice();
    int computerChoice = getComputerChoice();

    switch (userChoice)
    {
    case 1:
        printf("You chose Rock.\n");
        break;
    case 2:
        printf("You chose Paper.\n");
        break;
    case 3:
        printf("You chose Scissors.\n");
        break;
    }
    switch (computerChoice)
    {
    case 1:
        printf("Computer chose Rock.\n");
        break;
    case 2:
        printf("Computer chose Paper.\n");
        break;
    case 3:
        printf("Computer chose Scissors.\n");
        break;
    }

    checkWinner(userChoice, computerChoice);

    return 0;
}
int getUserChoice()
{
    return (rand() % 3) + 1; // 1 = Rock, 2 = Paper, 3 = Scissors
}
int getComputerChoice()
{
    int choice = 0;
    do
    {
        printf("Choose an option:\n");
        printf("1. Rock\n");
        printf("2. Paper\n");
        printf("3. Scissors\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
    } while (choice < 1 || choice > 3);

    return choice;
}
void checkWinner(int userChoice, int computerChoice)
{
    if (userChoice == computerChoice)
    {
        printf("It's a tie!\n");
    }
    else if ((userChoice == 1 && computerChoice == 3) || (userChoice == 2 && computerChoice == 1) || (userChoice == 3 && computerChoice == 2))
    {
        printf("You win!\n");
    }
    else
    {
        printf("Computer wins!\n");
    }
}
