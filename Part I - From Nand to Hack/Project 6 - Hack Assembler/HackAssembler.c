#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAX_LABEL_LENGTH 100

typedef struct Symbol_Table_Node {
    char symbol[MAX_LABEL_LENGTH + 1];
    int value;
    struct Symbol_Table_Node* link;
}Node;


//initializing translation tables
#define CMD_START_INDX 0
#define DEST_START_INDX 28
#define JUMP_START_INDX 36
#define SIZE_OF_TRANS_TABLE 44
char translation_table[44][2][8] = {

            //Command codes
            {"0" , "0101010"}, {"1" , "0111111"}, {"-1" , "0111010"},
            {"D" , "0001100"}, {"A" , "0110000"}, {"!D" , "0001101"}, {"!A" , "0110001"},
            {"-D" , "0001111"}, {"-A" , "0110011"}, {"D+1" , "0011111"}, {"A+1" , "0110111"},
            {"D-1" , "0001110"}, {"A-1" , "0110010"}, {"D+A" , "0000010"}, {"D-A" , "0010011"},
            {"A-D" , "0000111"}, {"D&A" , "0000000"}, {"D|A" , "0010101"}, {"M" , "1110000"},
            {"!M" , "1110001"}, {"-M" , "1110011"}, {"M+1" , "1110111"}, {"M-1" , "1110010"},
            {"D+M" , "1000010"}, {"D-M" , "1010011"}, {"M-D" , "1000111"}, {"D&M" , "1000000"},
            {"D|M" , "1010101"},

            //Destination codes
            {"null" , "000"}, {"M" , "001"}, {"D" , "010"}, {"MD" , "011"}, 
            {"A" , "100"}, {"AM" , "101"}, {"AD" , "110"}, {"AMD" , "111"},

            //Jump codes
            {"null" , "000"}, {"JGT" , "001"}, {"JEQ" , "010"}, {"JGE" , "011"}, 
            {"JLT" , "100"}, {"JNE" , "101"}, {"JLE" , "110"}, {"JMP" , "111"}

        };


//Function forward declarations
int isSpace(char x);
int isNumber(char x);
void strcat_Char(char* buff, char x);
char* int_to_15bitstr(int x);
void init_Symbol_Table (Node** head, Node** last);
int search_symbols(Node* head, char* symbol);
char* search_translations(char table[44][2][8], char* word, int start_ind, int end_ind);
void add_symtab_entry(Node** last, char* item, int address, Node* head);
void dealloc_symtab(Node* head);
void Preprocessor(char* path, Node* head, Node** last);
void Parser(char* line, char parts[3][16]);
char* Decoder (char parts[3][16], Node* head, Node** last);


int main (int argc, char* argv[]) {


    //initializing symbol table 
    Node* symbol_table_head = NULL;
    Node* symbol_table_last = NULL;
    init_Symbol_Table(&symbol_table_head, &symbol_table_last);


    //Taking command line argument of file path into name
    if (argc != 2) {
        printf("Expected 1 argument (file path of .asm file)");
        exit(-1);
    }
    char name[150];
    strcpy(name, argv[1]);


    //Stripping comments, saving labels into symbol table and preprocessed code in temp.txt
    Preprocessor(argv[1], symbol_table_head, &symbol_table_last);


    //Opening .hack file in same directory as .asm file
    int length = strlen(name);
    strcpy(name + (length-3), "hack");
    FILE* fp_to_hack = fopen(name, "w");


    //Translating preprocessed code
    char line[103];
    char binary[18];
    char parts[3][16] = {"null", "", "null"};       //Format: {destination, command, jump}
    FILE* fp_to_temp = fopen("temp.txt", "r");

    while (fscanf(fp_to_temp, "%s", line) != EOF) {
        //Reading a full line from temp.txt and breaking it into parts
        Parser(line, parts);

        //Getting binary equivalent of the parts
        strcpy(binary, Decoder(parts, symbol_table_head, &symbol_table_last));

        //Writing binary into .hack file
        binary[16] = '\n';
        binary[17] = '\0';
        fprintf(fp_to_hack, binary);

        //Resetting parts back to {"null", "", "null"}
        strcpy(parts[0], "null");
        parts[1][0] = '\0';
        strcpy(parts[2], "null");
    }


    //Closing file pointers, deleting temp.txt, deallocating the symbol table
    fclose(fp_to_temp);
    fclose(fp_to_hack);
    char file_to_remove[] = "temp.txt";
    remove(file_to_remove);
    dealloc_symtab(symbol_table_head);
}



//FUNCTIONS

int isSpace(char x) {
    if ((x == ' ') || (x == '\t') || (x == '\n') || (x == '\v') || (x == '\f') || (x == '\r')) {
        return 1;
    }
    else return 0;
}

int isNumber(char x) {
    if ((x == '0') || (x == '1') || (x == '2') || (x == '3') || (x == '4') || (x == '5') || 
    (x == '6') || (x == '7') || (x == '8') || (x == '9')) {
        return 1;
    }
    return 0;
}

void strcat_Char(char* buff, char x) {
    //Adds a character to the end of the string and terminates with null terminator
    //Unsafe - doesn't check size of buffer
    while (*buff != '\0') {
        buff += 1;
    }
    *buff = x;
    *(buff+1) = '\0';
}

char* int_to_15bitstr(int x) {
    //Function assumes that the address is capable of being stored in a 15 bit number
    static char bits[16];
    bits[15] = '\0';
    for (int i=0; i<15; i++) {
        bits[i] = '0';
    }

    int ind = 14;
    do {
        if (x - (x/2)*2 == 1) {
            bits[ind] = '1';
        }
        x=x/2;
        ind--;
    } while (x != 0);
    return bits;
}

void init_Symbol_Table (Node** head, Node** last) {

    //First node allocation has to be handled separately because symbol table is empty
    Node* first = (Node*)malloc(sizeof(Node));
    if (first == NULL) {
        //Allocation failed, exiting with exit code -1
        exit(-1);
    }
    strcpy(first->symbol, "R0");
    first->value = 0;
    first->link = NULL;
    *head = first;
    *last = first;

    //Allocating the rest of the nodes
    add_symtab_entry(last, "R1", 1, *head);
    add_symtab_entry(last, "R2", 2, *head);
    add_symtab_entry(last, "R3", 3, *head);
    add_symtab_entry(last, "R4", 4, *head);
    add_symtab_entry(last, "R5", 5, *head);
    add_symtab_entry(last, "R6", 6, *head);
    add_symtab_entry(last, "R7", 7, *head);
    add_symtab_entry(last, "R8", 8, *head);
    add_symtab_entry(last, "R9", 9, *head);
    add_symtab_entry(last, "R10", 10, *head);
    add_symtab_entry(last, "R11", 11, *head);
    add_symtab_entry(last, "R12", 12, *head);
    add_symtab_entry(last, "R13", 13, *head);
    add_symtab_entry(last, "R14", 14, *head);
    add_symtab_entry(last, "R15", 15, *head);
    add_symtab_entry(last, "SCREEN", 16384, *head);
    add_symtab_entry(last, "KBD", 24576, *head);
    add_symtab_entry(last, "SP", 0, *head);
    add_symtab_entry(last, "LCL", 1, *head);
    add_symtab_entry(last, "ARG", 2, *head);
    add_symtab_entry(last, "THIS", 3, *head);
    add_symtab_entry(last, "THAT", 4, *head);
}

int search_symbols(Node* head, char* symbol) {
    //Returns value associated with symbol if found, otherwise -1
    while (head!=NULL) {
        if (strcmp(head->symbol, symbol) == 0) {
            return head->value;
        }
        head = head->link;
    }
    return -1;
}

char* search_translations(char table[44][2][8], char* word, int start_ind, int end_ind) {
    //Returns the binary string equivalent of parts of a C-instruction 
    for (int i=start_ind; i<end_ind; i++) {
        if (strcmp(table[i][0], word) == 0) {
            return table[i][1];
        }
    }
}

void add_symtab_entry(Node** last, char* item, int address, Node* head) {
    //Adds entry to the end of the symbol table
    //head is only passed as an argument so that deallocation can happen if required. It serves no other purpose

    static Node* new;
    new = (Node*)malloc(sizeof(Node));
    if (new == NULL) {
        //Allocation failed, deallocating symbol table and exiting with exit code -1
        dealloc_symtab(head); 
        exit(-1);
    }

    if (item[0] == '(') {
        //Removing ( and ) before writing the label into symbol table
        item+=1;
        int length = strlen(item);
        *(item + length - 1) = '\0';
    }
    
    //Adding new variable/label to end of symbol table
    strcpy(new->symbol, item);
    new->value = address;
    new->link = NULL;
    (*last)->link = new;
    *last = new;
}

void dealloc_symtab(Node* head) {
    Node* dummy;
    while (head!=NULL) {
        dummy = head->link;
        free(head);
        head = dummy;
    }
}

void Preprocessor(char* path, Node* head, Node** last) {
    //This preprocessor also removes the pseudo-commands (i.e. the ones initializing labels)
    //Responsible for adding labels and their corresponding addresses to the symbol table

    FILE* fp = fopen(path, "r");
    FILE* fp2 = fopen("temp.txt", "w");

    int new_word = 0;
    char curchar = getc(fp);
    char word[MAX_LABEL_LENGTH + 4]= " ";        //Accomodates label along with (, ), \n and \0
    int address = 0;

    //Moving past the leading spaces in file if present
    if (isSpace(curchar)) {
        do {
            curchar = getc(fp);
        } while (isSpace(curchar));
    }

    //Reading the contents of the file
    while (1) {
        new_word = 0;
        //Remove comment
        if (curchar == '/') {
            while (curchar != '\n') {
                curchar = getc(fp);
            }
            new_word = 1;
        }
        //Remove spaces after comment
        if (isSpace(curchar)) {
            do {
                curchar = getc(fp);
            } while (isSpace(curchar));
            new_word = 1;
        }
        //If next line is also comment, skip to next iteration to remove comment
        if (curchar == '/') {
            continue;
        }

        //If end of file reached, check if word contains a command (not a label)
        //If it does, write it into temp.txt
        //break loop
        if (curchar == EOF) {
            if ((word[0] != '\0') && (word[0] != '(')) {
                fprintf(fp2, word);
            }
            break;
        }

        //If new_word is true, word is holding a completed command/label
        //If new_word is false, continue appending curchar to word
        if (new_word) {
            if ((word[0] != '(') && (word[0] != ' ')) {     
                //Writing non-empty commands into temp.txt
                strcat_Char(word, '\n');
                fprintf(fp2, word);
                address++;
            }
            else {
                if (word[0] == '(') {                       
                    //Adding labels to symbol table
                    add_symtab_entry(last, word, address, head);
                }
            }
            word[0] = curchar;
            word[1] = '\0';
        }
        else {
            strcat_Char(word, curchar);
        }

        curchar = getc(fp);
    } 

    fclose(fp);
    fclose(fp2);
}

void Parser(char* line, char parts[3][16]) {
    //Reads a line from temp.txt (preprocessed code) and breaks it into parts
    //parts = {destination, command, jump}

    if (line[0] == '@') {           
        //encountered an A-instruction
        strcpy(parts[0], "@");
        line+=1;
        strcpy(parts[1], line);
    }
    else {                          
        //encountered a C-instruction
        int jump = 0;
        char word[4] = "";
        while (line[0] != '\0') {
            if (line[0] != '=' && line[0] != ';') {
                strcat_Char(word, line[0]);
            }
            else {
                if (line[0] == '=') {
                    strcpy(parts[0], word);     
                } 
                else {
                    strcpy(parts[1], word);
                    jump=1;
                }                     
                word[0] = '\0';
            }
            line+=1;
        }

        if (word[0] != '\0') {
            if (jump) {
                strcpy(parts[2], word);
            }
            else {
                strcpy(parts[1], word);
            }
        }
    }
}

char* Decoder (char parts[3][16], Node* head, Node** last) {
    //Responsible for adding variables and their addresses to the symbol table

    static int address = 16;
    static char decoded[17] = "";

    if (parts[0][0] == '@') {    
        //encountered an A-instruction, need to convert whatever address follows '@' to binary
        decoded[0] = '0';

        if (!isNumber(parts[1][0])) {
            //encountered a variable name
            static int addr;
            addr = search_symbols(head, parts[1]);

            if (addr != -1) {
                //If present in symbol table, fetch address
                strcpy(decoded+1, int_to_15bitstr(addr));
            }
            else {
                //If not present, assign address and add to symbol table
                strcpy(decoded+1, int_to_15bitstr(address));
                add_symtab_entry(last, parts[1], address, head);
                address++;
            }

        }
        else {          
            //encountered a normal, numerical address. Convert to binary string directly
            strcpy(decoded+1, int_to_15bitstr(atoi(parts[1])));
        }
    }
    else {
        //encountered a C-instruction
        decoded[0] = '1';
        decoded[1] = '1';
        decoded[2] = '1';
        strcpy(decoded + 3, search_translations(translation_table, parts[1], CMD_START_INDX, DEST_START_INDX));
        strcpy(decoded + 10, search_translations(translation_table, parts[0], DEST_START_INDX, JUMP_START_INDX));
        strcpy(decoded + 13, search_translations(translation_table, parts[2], JUMP_START_INDX, SIZE_OF_TRANS_TABLE));
    }
    return decoded;
}
