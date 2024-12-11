#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/ioctl.h>

#endif

// Cross-platform structure definitions
//update deneme
typedef struct {
    int columns;
    int rows;
    int column; //kodu bozdum bu kodu sil
} ConsoleSize;

typedef struct MenuItem {
    char* name;
    char* description;
    struct MenuItem* parent;
    char** menuItems;
    int itemCount;
} MenuItem, *pmenu;

// Global variables
ConsoleSize consoleSize;
#ifdef _WIN32
    WORD styleDefault = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    WORD styleHiglight = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
#else
    #define styleDefault "\033[0m"
    #define styleHiglight "\033[7m"
#endif

ConsoleSize refreshSize() {
    ConsoleSize size;
    #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO buffer;
        HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        GetConsoleScreenBufferInfo(stdOut, &buffer);
        size.columns = buffer.srWindow.Right - buffer.srWindow.Left;
        size.rows = buffer.srWindow.Bottom - buffer.srWindow.Top;
    #else
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        size.columns = w.ws_col;
        size.rows = w.ws_row;
    #endif
    consoleSize = size;
    return size;
}

void clear() {
    #ifdef _WIN32
        HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD start = {0, 0};
        DWORD nChars;
        refreshSize();
        FillConsoleOutputCharacter(stdOut, ' ', consoleSize.columns * consoleSize.rows, start, &nChars);
        SetConsoleCursorPosition(stdOut, start);
    #else
        printf("\033[2J");
        printf("\033[H");
    #endif
}

int waitKeys() {
    #ifdef _WIN32
        return _getch();
    #else
        struct termios old_tio, new_tio;
        int pressed_key;

        tcgetattr(STDIN_FILENO, &old_tio);
        new_tio = old_tio;

        new_tio.c_lflag &= (~ICANON & ~ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

        pressed_key = getchar();

        tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);

        return pressed_key;
    #endif
}

void displayMenu(pmenu menu, int itemIndex) {
    clear();
    refreshSize();

    if (itemIndex > menu->itemCount) {
        itemIndex = menu->itemCount;
    }

    printf("================================\n");

    int padding = (consoleSize.columns - strlen(menu->name)) / 2;
    for (int i = 0; i < padding; i++) printf(" ");
    printf("%s\n\n", menu->name);

    if (menu->description[0] != '\n') {
        printf("%s\n", menu->description);
        printf("================================\n\n");
    } else {
        printf("================================\n\n");
        printf("%s\n", menu->description);
    }

    for (int i = 0; i < menu->itemCount; i++) {
        if (i == itemIndex) {
            #ifdef _WIN32
                SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), styleHiglight);
            #else
                printf("%s", styleHiglight);
            #endif
        }
        printf("> %s\n", menu->menuItems[i]);
        #ifdef _WIN32
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), styleDefault);
        #else
            printf("%s", styleDefault);
        #endif
    }

    if (itemIndex > menu->itemCount - 1) {
        #ifdef _WIN32
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), styleHiglight);
        #else
            printf("%s", styleHiglight);
        #endif
    }

    if (menu->parent != NULL) {
        printf("< %s\n", menu->parent->name);
    } else {
        printf("< Exit\n");
    }

    #ifdef _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), styleDefault);
    #else
        printf("%s", styleDefault);
    #endif
}

MenuItem* createMenu(char* name, char* description, MenuItem* parent, char** items, int itemCount) {
    MenuItem* menu = (MenuItem*)malloc(sizeof(MenuItem));
    menu->name = name;
    menu->description = description;
    menu->parent = parent;
    menu->menuItems = items;
    menu->itemCount = itemCount;
    return menu;
}

int main() {
    char* mainMenuItems[] = {
        "Start Game",
        "Options",
        "Help"
    };

    char* optionsMenuItems[] = {
        "Sound Settings",
        "Display Settings",
        "Controls"
    };

    MenuItem* mainMenu = createMenu(
        "Main Menu",
        "Welcome to the Demo Program\n",
        NULL,
        mainMenuItems,
        3
    );

    MenuItem* optionsMenu = createMenu(
        "Options Menu",
        "Configure your settings below:\n",
        mainMenu,
        optionsMenuItems,
        3
    );

    MenuItem* currentMenu = mainMenu;
    int selectedIndex = 0;
    int running = 1;

    while (running) {
        displayMenu(currentMenu, selectedIndex);

        int key = waitKeys();

        switch (key) {
            case 'w':
            case 'W':
                selectedIndex = (selectedIndex > 0) ? selectedIndex - 1 : currentMenu->itemCount;
                break;

            case 's':
            case 'S':
                selectedIndex = (selectedIndex < currentMenu->itemCount) ? selectedIndex + 1 : 0;
                break;

            case '\r':
            case '\n':
                if (selectedIndex == currentMenu->itemCount) {
                    if (currentMenu->parent != NULL) {
                        currentMenu = currentMenu->parent;
                        selectedIndex = 0;
                    } else {
                        running = 0;
                    }
                } else if (currentMenu == mainMenu && selectedIndex == 1) {
                    currentMenu = optionsMenu;
                    selectedIndex = 0;
                }
                break;

            case 'q':
            case 'Q':
                running = 0;
                break;
        }
    }

    free(optionsMenu);
    free(mainMenu);

    return 0;
}