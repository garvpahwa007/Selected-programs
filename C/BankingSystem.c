#include <stdio.h>
#include <math.h>

void checkBalance(float balance);
float deposit(float balance);
float withdraw(float balance);
void clearInput(void);

int main(void)
{
    // BANKING SYSTEM

    int choice = 0;
    float balance = 0.0;

    printf("WELCOME TO THE BANKING SYSTEM\n");

    do
    {
        printf("Choose an option:\n");
        printf("1. Check Balance\n");
        printf("2. Deposit\n");
        printf("3. Withdraw\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        fflush(stdout);

        int result = scanf("%d", &choice);
        if (result == EOF)
        {
            break;
        }
        clearInput();
        if (result != 1)
        {
            printf("Invalid choice! Please enter a number from 1 to 4.\n");
            continue;
        }

        switch (choice)
        {
        case 1:
            checkBalance(balance);
            break;
        case 2:
            balance = deposit(balance);
            break;
        case 3:
            balance = withdraw(balance);
            break;
        case 4:
            break;
        default:
            printf("Invalid choice! Please try again.\n");
            break;
        }

    } while (choice != 4);

    printf("Exiting the banking system. Goodbye!\n");
    return 0;
}

void checkBalance(float balance)
{
    printf("Your current balance is: $%.2f\n", balance);
}
float deposit(float balance)
{
    float amount;
    printf("Enter the amount to deposit: $");
    fflush(stdout);
    int result = scanf("%f", &amount);
    clearInput();
    if (result != 1 || !isfinite(amount) || amount <= 0)
    {
        printf("Invalid deposit amount!\n");
        return balance;
    }
    if (!isfinite(balance + amount))
    {
        printf("Deposit amount is too large!\n");
        return balance;
    }

    balance += amount;
    printf("Deposited: $%.2f\n", amount);
    printf("Your new balance is: $%.2f\n", balance);
    return balance;
}
float withdraw(float balance)
{
    float amount;
    printf("Enter the amount to withdraw: $");
    fflush(stdout);
    int result = scanf("%f", &amount);
    clearInput();
    if (result != 1 || !isfinite(amount) || amount <= 0)
    {
        printf("Invalid withdrawal amount!\n");
        return balance;
    }
    if (amount > balance)
    {
        printf("Insufficient funds!\n");
        return balance;
    }

    balance -= amount;
    printf("Withdrawn: $%.2f\n", amount);
    printf("Your new balance is: $%.2f\n", balance);
    return balance;
}

void clearInput(void)
{
    int character;
    while ((character = getchar()) != '\n' && character != EOF)
    {
        // Discard the rest of the input line.
    }
}
