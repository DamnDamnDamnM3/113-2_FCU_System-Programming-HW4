#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILES 10
#define MAX_SYMBOLS 100

typedef struct {
    char name[10];      
    int address;        
} Symbol;

typedef struct {
    char name[10];      
    int address;        
    int length;         
} ControlSection;

Symbol ESTAB[MAX_SYMBOLS];
int symbolCount = 0;
ControlSection CS[MAX_FILES];
int csCount = 0;

int strToInt(char *hex) {
    int val;
    sscanf(hex, "%x", &val);
    return val;
}

void addSymbol(char *name, int address) {
    for (int i = 0; i < symbolCount; i++) {
        if (strcmp(ESTAB[i].name, name) == 0) {
            printf("Duplicate symbol error: %s\n", name);
            return;
        }
    }
    strcpy(ESTAB[symbolCount].name, name);
    ESTAB[symbolCount].address = address;
    symbolCount++;
}

char* DealWithFile(const char *filename) {
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s.txt", filename);
    return buffer;
}

void Pass1(char *filenames[], int fileCount, int progaddr) {
    int csaddr = progaddr;
    for (int i = 0; i < fileCount; i++) {
        const char *modifiedFilename = DealWithFile(filenames[i]);
        FILE *fp = fopen(modifiedFilename, "r");
        if (!fp) {
            printf("Cannot open file: %s\n", modifiedFilename);
            exit(1);
        }

        char line[1024];
        while (fgets(line, sizeof(line), fp)) {
            line[strcspn(line, "\n")] = 0;

            if (line[0] == 'H') {
                char *token = strtok(line, "^");
                token = strtok(NULL, "^");
                char csName[10];
                strcpy(csName, token);
                token = strtok(NULL, "^");
                token = strtok(NULL, "^");
                int length = hexToInt(token);

                strcpy(CS[csCount].name, csName);
                CS[csCount].address = csaddr;
                CS[csCount].length = length;
                csCount++;

                addSymbol(csName, csaddr);
            } else if (line[0] == 'D') {
                char *token = strtok(line, "^");
                while ((token = strtok(NULL, "^")) != NULL) {
                    char symbol[10];
                    strcpy(symbol, token);
                    token = strtok(NULL, "^");
                    int addr = hexToInt(token) + csaddr;
                    addSymbol(symbol, addr);
                }
            } else if (line[0] == 'E') {
                break;
            }
        }

        fclose(fp);
        csaddr += CS[csCount - 1].length;
    }
}

void printLoadMap() {
    for (int i = 0; i < csCount; i++) {
        printf("%-16s\t\t%04X\t%04X\n", CS[i].name, CS[i].address, CS[i].length);
        for (int j = 0; j < symbolCount; j++) {
            if (strcmp(ESTAB[j].name, CS[i].name) != 0 &&
                ESTAB[j].address >= CS[i].address &&
                ESTAB[j].address < CS[i].address + CS[i].length) {
                printf("\t\t%-12s\t%04X\n", ESTAB[j].name, ESTAB[j].address);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int progaddr;
    sscanf(argv[1], "%x", &progaddr);

    Pass1(&argv[2], argc - 2, progaddr);
    printLoadMap();

    return 0;
}
