cd ~/Desktop/Selected-programs
cat > README.md << 'EOF'
# Selected Programs

A curated collection of programs I've written and chosen to keep, organized by language. This is not a dump of everything I've practiced, only the ones I want to show.

## Structure
Selected-programs                                                                                                    
├── C/ # Selected C programs                                                                                   
└── README.md
More language folders will be added over time.

## Contents

### [C](./C)

| File | Description |
|------|-------------|
| `shoppingcartDiscount.c` | Calculates a shopping cart total with discounts applied |
| `NumberGuessingGame.c` | A number guessing game played in the terminal |

## How to run the C programs

```bash
cd C
clang ProgramName.c -o ProgramName
./ProgramName
```

## Notes

- Compiled binaries and `.dSYM` folders are not committed, only source files.
- Programs are added as I finish and review them.

## Author

Garv Pahwa ([@garvpahwa007](https://github.com/garvpahwa007))
B.Tech CSE, DIT University, Dehradun
EOF
