#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#endif

/* ---------- Colors ---------- */
#define RST "\033[0m"
#define BLD "\033[1m"
#define DIM "\033[2m"
#define RED "\033[31m"
#define GRN "\033[32m"
#define YEL "\033[33m"
#define CYN "\033[36m"

#define MAX_CART 20
#define MAX_QTY 10
#define DELIVERY_FEE 40
#define FREE_DELIVERY_ABOVE 500
#define INNER 60 /* visible width of box content */

/* ---------- Data ---------- */
typedef struct
{
    int id;
    char name[32];
    float mrp;
    float price;
    int stock;
} Product;
typedef struct
{
    int p;
    int qty;
} CartItem;
typedef struct
{
    float mrpTotal, subtotal, slabRate, slabDisc, couponDisc, delivery, grand;
} Bill;

Product products[] = {
    {101, "Wireless Headphones", 3999, 1299, 25},
    {102, "Cotton T-Shirt", 999, 399, 100},
    {103, "Running Shoes", 3499, 1799, 40},
    {104, "Backpack 30L", 1999, 899, 60},
    {105, "Steel Water Bottle", 699, 349, 80},
    {106, "Power Bank 10000mAh", 1999, 999, 4},
    {107, "Smart Watch", 5999, 2499, 15},
    {108, "Notebook Pack (5)", 450, 275, 0}};
const int PRODUCT_COUNT = sizeof(products) / sizeof(products[0]);

CartItem cart[MAX_CART];
int cartCount = 0;
char coupon[16] = "";

/* ---------- Terminal setup ---------- */
static void initTerminal(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(h, &mode))
        SetConsoleMode(h, mode | 0x0004);
#endif
}

/* ---------- Box drawing helpers ---------- */
/* visible length: skips ANSI codes, counts UTF-8 characters once */
static int visLen(const char *s)
{
    int n = 0;
    while (*s)
    {
        if (*s == '\033')
        {
            while (*s && *s != 'm')
                s++;
            if (*s)
                s++;
            continue;
        }
        if ((*s & 0xC0) != 0x80)
            n++;
        s++;
    }
    return n;
}

static void hr(const char *l, const char *m, const char *r)
{
    printf(CYN "%s", l);
    for (int i = 0; i < INNER + 2; i++)
        printf("%s", m);
    printf("%s" RST "\n", r);
}
#define TOP() hr("╭", "─", "╮")
#define MID() hr("├", "─", "┤")
#define BOT() hr("╰", "─", "╯")

static void boxRow(const char *s, int center)
{
    int pad = INNER - visLen(s);
    if (pad < 0)
        pad = 0;
    int left = center ? pad / 2 : 0, right = pad - left;
    printf(CYN "│" RST " %*s%s%*s " CYN "│" RST "\n", left, "", s, right, "");
}

static void row(const char *fmt, ...)
{
    char buf[512];
    va_list a;
    va_start(a, fmt);
    vsnprintf(buf, sizeof buf, fmt, a);
    va_end(a);
    boxRow(buf, 0);
}

static void rowC(const char *fmt, ...)
{
    char buf[512];
    va_list a;
    va_start(a, fmt);
    vsnprintf(buf, sizeof buf, fmt, a);
    va_end(a);
    boxRow(buf, 1);
}

static void twoCols(const char *a, const char *b)
{
    char buf[256];
    int pad = 30 - visLen(a);
    snprintf(buf, sizeof buf, "%s%*s%s", a, pad < 0 ? 0 : pad, "", b);
    boxRow(buf, 0);
}

/* ---------- Messages ---------- */
static void ok(const char *fmt, ...)
{
    va_list a;
    va_start(a, fmt);
    printf("  " GRN "✔ ");
    vprintf(fmt, a);
    printf(RST "\n");
    va_end(a);
}
static void err(const char *fmt, ...)
{
    va_list a;
    va_start(a, fmt);
    printf("  " RED "✖ ");
    vprintf(fmt, a);
    printf(RST "\n");
    va_end(a);
}
static void warn(const char *fmt, ...)
{
    va_list a;
    va_start(a, fmt);
    printf("  " YEL "! ");
    vprintf(fmt, a);
    printf(RST "\n");
    va_end(a);
}

/* ---------- Input helpers ---------- */
static int readInt(const char *prompt, int *out)
{
    char buf[64];
    printf("  " YEL ">" RST " %s", prompt);
    if (!fgets(buf, sizeof buf, stdin))
        exit(0);
    return sscanf(buf, "%d", out) == 1;
}

static void readWord(const char *prompt, char *out, int size)
{
    char buf[64];
    printf("  " YEL ">" RST " %s", prompt);
    if (!fgets(buf, sizeof buf, stdin))
        exit(0);
    buf[strcspn(buf, "\n")] = 0;
    for (int i = 0; buf[i]; i++)
        buf[i] = toupper((unsigned char)buf[i]);
    strncpy(out, buf, size - 1);
    out[size - 1] = 0;
}

/* ---------- Lookups ---------- */
static int findProduct(int id)
{
    for (int i = 0; i < PRODUCT_COUNT; i++)
        if (products[i].id == id)
            return i;
    return -1;
}

static int findInCart(int p)
{
    for (int i = 0; i < cartCount; i++)
        if (cart[i].p == p)
            return i;
    return -1;
}

static int cartUnits(void)
{
    int u = 0;
    for (int i = 0; i < cartCount; i++)
        u += cart[i].qty;
    return u;
}

/* ---------- Offers & billing ---------- */
static float getSlabRate(float amt)
{
    if (amt >= 20000)
        return 20;
    if (amt >= 10000)
        return 15;
    if (amt >= 5000)
        return 10;
    if (amt >= 1000)
        return 5;
    return 0;
}

/* status: 0 = ok, 1 = invalid code, 2 = minimum order not met */
static float couponValue(const char *code, float amt, int *status)
{
    float d = 0, min = 0;
    *status = 0;
    if (strcmp(code, "SAVE10") == 0)
    {
        min = 2000;
        d = amt * 0.10;
        if (d > 500)
            d = 500;
    }
    else if (strcmp(code, "FLAT200") == 0)
    {
        min = 1500;
        d = 200;
    }
    else if (strcmp(code, "WELCOME100") == 0)
    {
        min = 999;
        d = 100;
    }
    else
    {
        *status = 1;
        return 0;
    }
    if (amt < min)
    {
        *status = 2;
        return 0;
    }
    return d;
}

static Bill calcBill(int *couponStatus)
{
    Bill b = {0};
    for (int i = 0; i < cartCount; i++)
    {
        b.mrpTotal += products[cart[i].p].mrp * cart[i].qty;
        b.subtotal += products[cart[i].p].price * cart[i].qty;
    }
    b.slabRate = getSlabRate(b.subtotal);
    b.slabDisc = b.subtotal * b.slabRate / 100;
    float afterSlab = b.subtotal - b.slabDisc;

    *couponStatus = 0;
    if (coupon[0])
        b.couponDisc = couponValue(coupon, afterSlab, couponStatus);

    float payable = afterSlab - b.couponDisc;
    b.delivery = (payable >= FREE_DELIVERY_ABOVE || cartCount == 0) ? 0 : DELIVERY_FEE;
    b.grand = payable + b.delivery;
    return b;
}

/* ---------- Screens ---------- */
static void printBanner(void)
{
    printf("\n");
    TOP();
    rowC(BLD YEL "★  ONLINE SHOPPING STORE  ★" RST);
    rowC(DIM "Free delivery above ₹%d  |  Bigger cart = bigger savings" RST, FREE_DELIVERY_ABOVE);
    BOT();
}

static void browseProducts(void)
{
    printf("\n");
    TOP();
    row(BLD YEL "PRODUCTS" RST);
    MID();
    row(BLD "%-4s  %-21s %7s %7s %6s  %-8s" RST, "ID", "Product", "MRP", "Price", "Off", "Stock");
    MID();
    for (int i = 0; i < PRODUCT_COUNT; i++)
    {
        Product *p = &products[i];
        const char *col;
        char stk[16];
        if (p->stock == 0)
        {
            col = RED;
            strcpy(stk, "Sold out");
        }
        else if (p->stock <= 10)
        {
            col = YEL;
            snprintf(stk, sizeof stk, "Only %d", p->stock);
        }
        else
        {
            col = GRN;
            strcpy(stk, "In Stock");
        }
        row("%-4d  %-21.21s %s%7.0f%s %7.0f %s%5.0f%%%s  %s%-8s%s",
            p->id, p->name, DIM, p->mrp, RST, p->price,
            GRN, (p->mrp - p->price) / p->mrp * 100, RST, col, stk, RST);
    }
    BOT();
}

static void billRows(Bill b, int cs)
{
    char lbl[64];
    int units = cartUnits();
    float payable = b.grand - b.delivery;

    snprintf(lbl, sizeof lbl, "Price (%d item%s)", units, units == 1 ? "" : "s");
    row("%-44s  ₹%13.2f", lbl, b.mrpTotal);
    row(GRN "%-44s- ₹%13.2f" RST, "Product Discount", b.mrpTotal - b.subtotal);
    if (b.slabDisc > 0)
    {
        snprintf(lbl, sizeof lbl, "Cart Value Offer (%.0f%%)", b.slabRate);
        row(GRN "%-44s- ₹%13.2f" RST, lbl, b.slabDisc);
    }
    if (coupon[0])
    {
        if (cs == 0)
        {
            snprintf(lbl, sizeof lbl, "Coupon (%s)", coupon);
            row(GRN "%-44s- ₹%13.2f" RST, lbl, b.couponDisc);
        }
        else if (cs == 1)
        {
            row(RED "Coupon %s is not a valid code" RST, coupon);
        }
        else
        {
            row(YEL "Coupon %s: minimum order value not met" RST, coupon);
        }
    }
    if (b.delivery == 0)
        row("%-44s%s%16s%s", "Delivery Charges", GRN, "FREE", RST);
    else
        row("%-44s+ ₹%13.2f", "Delivery Charges", b.delivery);

    MID();
    row(BLD "%-44s  ₹%13.2f" RST, "TOTAL PAYABLE", b.grand);
    row(GRN "You save ₹%.2f (%.0f%% off MRP)" RST,
        b.mrpTotal - payable, (b.mrpTotal - payable) / b.mrpTotal * 100);

    if (payable >= FREE_DELIVERY_ABOVE)
    {
        row(GRN "✔ Free delivery unlocked" RST);
    }
    else
    {
        char bar[128] = "";
        int filled = (int)(payable / FREE_DELIVERY_ABOVE * 20);
        for (int i = 0; i < 20; i++)
            strcat(bar, i < filled ? "█" : "░");
        row(YEL "Add ₹%.0f more for FREE delivery" RST, FREE_DELIVERY_ABOVE - payable);
        row(YEL "%s" RST, bar);
    }
}

static void viewCart(void)
{
    printf("\n");
    TOP();
    row(BLD YEL "YOUR CART" RST);
    MID();
    if (cartCount == 0)
    {
        rowC(DIM "Your cart is empty" RST);
        rowC(DIM "Browse products and add something you like!" RST);
        BOT();
        return;
    }
    row(BLD "%-4s %-21s %7s %7s %4s %12s" RST, "No", "Product", "MRP", "Price", "Qty", "Amount");
    MID();
    for (int i = 0; i < cartCount; i++)
    {
        Product *p = &products[cart[i].p];
        row("%-4d %-21.21s %s%7.0f%s %7.0f %4d %12.2f", i + 1, p->name,
            DIM, p->mrp, RST, p->price, cart[i].qty, p->price * cart[i].qty);
    }
    MID();
    int cs;
    Bill b = calcBill(&cs);
    billRows(b, cs);
    BOT();
}

static void showMenu(void)
{
    int cs, units = cartUnits();
    Bill b = calcBill(&cs);
    char a[96], c[96];

    printf("\n");
    TOP();
    if (coupon[0] && cs == 0)
        row(DIM "Cart:" RST " %d item%s  " DIM "|" RST "  Total: " BLD "₹%.0f" RST "  " DIM "|" RST "  Coupon: " GRN "%s" RST,
            units, units == 1 ? "" : "s", b.grand, coupon);
    else
        row(DIM "Cart:" RST " %d item%s  " DIM "|" RST "  Total: " BLD "₹%.0f" RST,
            units, units == 1 ? "" : "s", b.grand);
    MID();
    twoCols(YEL "[1]" RST " Browse Products", YEL "[5]" RST " Remove Item");
    twoCols(YEL "[2]" RST " Add to Cart", YEL "[6]" RST " Apply Coupon");
    snprintf(a, sizeof a, YEL "[3]" RST " View Cart (%d)", units);
    snprintf(c, sizeof c, YEL "[7]" RST " Checkout");
    twoCols(a, c);
    twoCols(YEL "[4]" RST " Update Quantity", RED "[8]" RST " Exit");
    BOT();
}

/* ---------- Cart actions ---------- */
static void addToCart(void)
{
    int id, qty;
    if (!readInt("Enter product ID: ", &id))
    {
        err("Please enter a number.");
        return;
    }
    int p = findProduct(id);
    if (p < 0)
    {
        err("Product not found.");
        return;
    }
    if (products[p].stock == 0)
    {
        err("%s is sold out.", products[p].name);
        return;
    }
    if (!readInt("Quantity: ", &qty) || qty < 1)
    {
        err("Invalid quantity.");
        return;
    }

    int c = findInCart(p);
    int newQty = qty + (c >= 0 ? cart[c].qty : 0);
    if (newQty > MAX_QTY)
    {
        err("Max %d units per product allowed.", MAX_QTY);
        return;
    }
    if (newQty > products[p].stock)
    {
        err("Only %d units in stock.", products[p].stock);
        return;
    }

    if (c >= 0)
        cart[c].qty = newQty;
    else
    {
        if (cartCount >= MAX_CART)
        {
            err("Cart is full.");
            return;
        }
        cart[cartCount].p = p;
        cart[cartCount++].qty = qty;
    }
    ok("Added to cart: %s x%d", products[p].name, qty);
}

static void updateQty(void)
{
    int n, qty;
    if (cartCount == 0)
    {
        warn("Your cart is empty.");
        return;
    }
    viewCart();
    if (!readInt("Item number to update: ", &n) || n < 1 || n > cartCount)
    {
        err("Invalid item number.");
        return;
    }
    if (!readInt("New quantity (0 to remove): ", &qty) || qty < 0)
    {
        err("Invalid quantity.");
        return;
    }
    Product *p = &products[cart[n - 1].p];
    if (qty > MAX_QTY)
    {
        err("Max %d units per product allowed.", MAX_QTY);
        return;
    }
    if (qty > p->stock)
    {
        err("Only %d units in stock.", p->stock);
        return;
    }
    if (qty == 0)
    {
        for (int i = n - 1; i < cartCount - 1; i++)
            cart[i] = cart[i + 1];
        cartCount--;
        ok("Item removed from cart.");
    }
    else
    {
        cart[n - 1].qty = qty;
        ok("Quantity updated.");
    }
}

static void removeItem(void)
{
    int n;
    if (cartCount == 0)
    {
        warn("Your cart is empty.");
        return;
    }
    viewCart();
    if (!readInt("Item number to remove: ", &n) || n < 1 || n > cartCount)
    {
        err("Invalid item number.");
        return;
    }
    for (int i = n - 1; i < cartCount - 1; i++)
        cart[i] = cart[i + 1];
    cartCount--;
    ok("Item removed from cart.");
}

static void applyCoupon(void)
{
    char code[16];
    printf("\n");
    TOP();
    row(BLD YEL "AVAILABLE COUPONS" RST);
    MID();
    row(GRN "%-12s" RST "10%% off up to ₹500 on orders above ₹2000", "SAVE10");
    row(GRN "%-12s" RST "Flat ₹200 off on orders above ₹1500", "FLAT200");
    row(GRN "%-12s" RST "Flat ₹100 off on orders above ₹999", "WELCOME100");
    BOT();
    readWord("Enter coupon code (blank to remove): ", code, sizeof code);
    if (code[0] == 0)
    {
        coupon[0] = 0;
        ok("Coupon removed.");
        return;
    }

    int cs;
    strcpy(coupon, code);
    calcBill(&cs);
    if (cs == 0)
        ok("Coupon %s applied!", coupon);
    else if (cs == 1)
    {
        err("Invalid coupon code.");
        coupon[0] = 0;
    }
    else
        warn("Coupon %s saved, but your cart is below its minimum order value.", coupon);
}

static void checkout(void)
{
    int pay;
    char confirm[8];
    if (cartCount == 0)
    {
        warn("Your cart is empty.");
        return;
    }
    viewCart();

    printf("\n  " YEL "[1]" RST " UPI    " YEL "[2]" RST " Card    " YEL "[3]" RST " Cash on Delivery\n");
    if (!readInt("Choose payment method: ", &pay) || pay < 1 || pay > 3)
    {
        err("Invalid payment method.");
        return;
    }
    readWord("Place order? (Y/N): ", confirm, sizeof confirm);
    if (confirm[0] != 'Y')
    {
        warn("Order not placed.");
        return;
    }

    int cs;
    Bill b = calcBill(&cs);
    for (int i = 0; i < cartCount; i++)
        products[cart[i].p].stock -= cart[i].qty;

    const char *modes[] = {"UPI", "Card", "Cash on Delivery"};
    char date[32];
    time_t t = time(NULL) + 4 * 24 * 3600;
    strftime(date, sizeof date, "%a, %d %b %Y", localtime(&t));

    printf("\n");
    TOP();
    rowC(BLD GRN "ORDER CONFIRMED" RST);
    MID();
    row("Order ID       OD%08d", rand() % 100000000);
    row("Payment        %s", modes[pay - 1]);
    row("%-14s " BLD "₹%.2f" RST, pay == 3 ? "Amount to pay" : "Amount paid", b.grand);
    row("Delivery by    %s", date);
    row(GRN "You saved      ₹%.2f" RST, b.mrpTotal - (b.grand - b.delivery));
    MID();
    rowC(DIM "Thank you for shopping with us!" RST);
    BOT();

    cartCount = 0;
    coupon[0] = 0;
}

/* ---------- Main ---------- */
int main(void)
{
    int choice;
    initTerminal();
    srand((unsigned)time(NULL));
    printBanner();

    while (1)
    {
        showMenu();
        if (!readInt("Choose an option: ", &choice))
        {
            err("Please enter a number from 1 to 8.");
            continue;
        }
        switch (choice)
        {
        case 1:
            browseProducts();
            break;
        case 2:
            addToCart();
            break;
        case 3:
            viewCart();
            break;
        case 4:
            updateQty();
            break;
        case 5:
            removeItem();
            break;
        case 6:
            applyCoupon();
            break;
        case 7:
            checkout();
            break;
        case 8:
            printf("\n  " GRN "Thanks for visiting. Goodbye!" RST "\n\n");
            return 0;
        default:
            err("Choose a number from 1 to 8.");
        }
    }
}