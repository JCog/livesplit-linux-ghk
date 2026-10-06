#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define ARRAY_LENGTH(arr) (int)(sizeof(arr) / sizeof(arr[0]))

#define HOST "127.0.0.1"
#define PORT 16834

enum ServerCommands {
    CMD_SPLIT,
    CMD_SKIP_SPLIT,
    CMD_UNDO_SPLIT,
    CMD_RESET,
    CMD_PREV_COMP,
    CMD_NEXT_COMP,
};

struct CommandMap {
    const enum ServerCommands cmd;
    const char *msg;
    int key;
    int modKey;
};

static int keyPressed(const int key, const char* devicePath) {
    FILE *kbd = fopen(devicePath, "r");
    char keyMap[KEY_MAX/8 + 1];
    ioctl(fileno(kbd), EVIOCGKEY(sizeof(keyMap)), keyMap);
    return keyMap[key/8] & (1 << (key % 8));
}

int main(const int argc, char *argv[]) {
    if (argc == 1) {
        printf("arg0: full path to input device under /dev/input\n"
               "arg1: LiveSplit TCP Server IP (default 127.0.0.1)\n"
               "arg2: LiveSplit TCP Server Port (default 16834)\n");
        return 0;
    }
    const char* devicePath = argv[1];
    const char* host = argc >= 3 ? argv[2] : HOST;
    const int port = argc >= 4 ? atoi(argv[3]) : PORT;

    const struct CommandMap commandMaps[] = {
        {CMD_SPLIT,         "startorsplit",     KEY_KPPLUS,     -1},
        {CMD_SKIP_SPLIT,    "skipsplit",        KEY_KP2,        -1},
        {CMD_UNDO_SPLIT,    "unsplit",          KEY_KP8,        -1},
        {CMD_RESET,         "reset",            KEY_KPMINUS,    KEY_LEFTSHIFT},
        {CMD_PREV_COMP,     "setcomparison ",   KEY_KP4,        -1},
        {CMD_NEXT_COMP,     "setcomparison ",   KEY_KP6,        -1},
    };
    
    // for some reason LiveSplit doesn't have a server command to cycle through the enabled comparisons, so this is my
    // workaround. edit the list to change which comparisons are active.
    const char *comparisons[] = {
        "Personal Best",
        "Best Segments",
        "Best Split Times",
        "Average Segments",
        "Median Segments",
        "Worst Segments",
        "Balanced PB",
        "Latest Run",
        "None",
    };
    const int compCount = ARRAY_LENGTH(comparisons);
    int comp = 0;
    
    int fd = open(devicePath, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open input device");
        return 1;
    }
    
    // LiveSplit server connection
    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(host);
    servAddr.sin_port = htons(port);
    int client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0) {
        perror("Error creating socket");
        return 1;
    }
    if (connect(client, (void*)&servAddr, sizeof(servAddr)) != 0) {
        perror("Can't connect to LiveSplit. Did you enable the TCP Server?");
        return 1;
    }
    
    // read keyboard events
    while (1) {
        struct input_event event;
        if (read(fd, &event, sizeof(struct input_event)) != sizeof(struct input_event)) {
            perror("Failed to read input event");
            break;
        }
    
        if (event.type != EV_KEY) {
            continue;
        }
        if (event.value == 1) { // key pressed
            for (int i = 0; i < ARRAY_LENGTH(commandMaps); i++) {
                struct CommandMap cmdMap = commandMaps[i];
                if (event.code == cmdMap.key && (cmdMap.modKey == -1 || keyPressed(cmdMap.modKey, devicePath))) {
                    send(client, cmdMap.msg, strlen(cmdMap.msg), 0);
                    if (cmdMap.cmd == CMD_PREV_COMP) {
                        comp = (comp - 1 + compCount) % compCount;
                        send(client, comparisons[comp], strlen(comparisons[comp]), 0);
                    } else if (cmdMap.cmd == CMD_NEXT_COMP) {
                        comp = (comp + 1) % compCount;
                        send(client, comparisons[comp], strlen(comparisons[comp]), 0);
                    }
                    send(client, "\r\n", 2, 0);
                    break;
                }
            }
        }
    }
    
    // close input device but not the TCP connection due to library naming conflicts. doesn't seem to cause issues.
    close(fd);
    return 0;
}
