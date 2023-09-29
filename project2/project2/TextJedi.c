#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_IDENTIFIER_SIZE 31 //Max identifier size
#define MAX_STRING_SIZE 1000 //Stringe de size koydum
#define MAX_INT_CONST_SIZE 4294967295 //proje kağıdında istenen max unsigned int
#define MAX_VARIABLES 10000 //max değişken sayısı rastgele yüksek bir değer yazdım

typedef enum { //token türlerini tanımladım
    TOKEN_IDENTIFIER,
    TOKEN_INTEGER_CONSTANT,
    TOKEN_STRING_CONSTANT,
    TOKEN_OPERATOR,
    TOKEN_ASSIGNMENT,
    TOKEN_KEYWORD,
    TOKEN_END_OF_LINE,
    TOKEN_COMMENT,
    TOKEN_INVALID,
    TOKEN_LEFTPAR,
    TOKEN_RIGHTPAR,
    TOKEN_COMMA,
    TOKEN_END_OF_FILE
} TokenType;

typedef enum { //değişkenin değer tipleri int veya string
    VALUE_INT,
    VALUE_STRING
} ValueType;

typedef struct { //variable yapısı isim değer tipi ve değer içeren
    char name[MAX_IDENTIFIER_SIZE];
    ValueType type;
    union {
        int intValue;
        char stringValue[MAX_STRING_SIZE];
    } value;
} Variable;

typedef struct { // token yapısı tip, text, ve variable mı onu iceren
    TokenType type;
    char lexeme[MAX_STRING_SIZE];
    bool isVariable;
} Token;

//method declarationları
Token getNextToken(FILE* file);
Token processIdentifier(FILE* inputFile);
Token processIntConst(FILE* inputFile);
Token processOperator(FILE* inputFile);
Token processStringConst(FILE* inputFile);
int isKeyword(const char* input);
void addToList(Variable variable);
void parseDeclaration(FILE* inputFile, Token token);
void parseAssignment(FILE* inputFile, Token token);
void parseOutput(FILE* inputFile, Token token);
void interpret(FILE* inputFile);
void processAdditionorSubtraction(Token prevToken, Token token, FILE* inputFile,int variableIndex, char*variableName);
int processSize(FILE* inputFile, int variableIndex, char* variableName);
char* processSubs(FILE* inputFile, int variableIndex, char* variableName);
int processLocate(FILE* inputFile);
int locate(char* bigText, char* smallText, int start);
int findVariableIndex(const char* variableName);
char* processInsert(FILE* inputFile);
char* processOverride(FILE* inputFile);
char* processAsString(FILE* inputFile);
char* removeSubstring(char* s, const char* toRemove);
int processAsInt(FILE* inputFile);
char* insert(const char* myText, int location, const char* insertText);

Variable variableList[MAX_VARIABLES]; //variable list oluşumu ve başlangıçtaki variable sayısı
int variableCount = 0;

int main(int argc, char* argv[]) {
    if (argc < 2) {  //çalıştırmak için gereken kodlar
        printf("Usage: TextJedi <filename>\n");
        return 1;
    }

    char* filename = argv[1];

    FILE* inputFile = fopen(filename, "r");
    if (inputFile == NULL) {
        printf("Error opening file: %s\n", filename);
        return 1;
    }

    interpret(inputFile); //genel olarak her şeyin yapıldığı method

    fclose(inputFile);
    return 0;
}

int isKeyword(const char* input) { //keyword kontrolü
    const char* keywords[] = {
            "new", "int", "text", "size", "subs", "locate", "insert", "override",
            "read", "write", "from", "to", "input", "output", "asText", "asString"
    };
    int numKeywords = sizeof(keywords) / sizeof(keywords[0]);

    for (int i = 0; i < numKeywords; i++) {
        if (strcasecmp(input, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

Token processIdentifier(FILE* inputFile) { //identifierları tokenize ettigimiz yer
    char input[MAX_IDENTIFIER_SIZE];
    int inputIndex = 0;
    int currentChar;
    Token token;
    token.type = TOKEN_INVALID;

    while (((currentChar = fgetc(inputFile)) != EOF) && (isalnum(currentChar) || currentChar == '_')) {
        input[inputIndex++] = (char)tolower(currentChar);
    }

    input[inputIndex] = '\0';
    fseek(inputFile, -1, SEEK_CUR);
    if (!isKeyword(input)) {
        if (!isalpha(input[0]) && input[0] != '_') {
            printf("Lexical error: Identifiers must begin with an alphabetic character\n");
            exit(0);
        }
        if (strlen(input) > MAX_IDENTIFIER_SIZE) {
            printf("Lexical error: Identifiers can't be larger than 30 characters\n");
            exit(0);
        } else {
            token.type = TOKEN_IDENTIFIER;
            strcpy(token.lexeme, input);
        }
    } else {
        token.type = TOKEN_KEYWORD;
        strcpy(token.lexeme, input);
    }
    return token;
}

Token processIntConst(FILE* inputFile) { //int sabitleri tokenize ettiğimiz yer
    Token token;
    token.type = TOKEN_INVALID;
    char input[11];
    int inputIndex = 0;
    int currentChar;
    while (((currentChar = fgetc(inputFile)) != EOF) && isdigit(currentChar)) {
        input[inputIndex++] = (char)currentChar;
    }
    input[inputIndex] = '\0';
    fseek(inputFile, -1, SEEK_CUR);
    if (strtoll(input, 0, 10) <= MAX_INT_CONST_SIZE) {
        token.type = TOKEN_INTEGER_CONSTANT;
        strcpy(token.lexeme, input);
    } else {
        printf("Lexical error: Integer constants can't be more than unsigned int const length digits\n");
        exit(0);
    }
    return token;
}

Token processOperator(FILE* inputFile) { // operatörleri tokenize ettiğimiz yer
    Token token;
    token.type = TOKEN_INVALID;

    int currentChar = fgetc(inputFile);
    if (currentChar == '+' || currentChar == '-') {
        token.type = TOKEN_OPERATOR;
        sprintf(token.lexeme, "%c", currentChar);
    } else if (currentChar == ':') {
        int nextChar = fgetc(inputFile);
        if (nextChar == '=') {
            token.type = TOKEN_ASSIGNMENT;
        } else {
            fseek(inputFile, -1, SEEK_CUR);
            printf("Lexical error: Invalid operator '%c'\n", currentChar);
            exit(0);
        }
    } else {
        printf("Lexical error: Invalid character '%c'\n", currentChar);
        exit(0);
    }
    return token;
}

Token processStringConst(FILE* inputFile) { // string sabitleri tokenize ettiğimiz yer
    Token token;
    token.type = TOKEN_INVALID;
    char input[MAX_STRING_SIZE + 1];
    int inputIndex = 0;
    int currentChar;
    input[inputIndex++] = '"';
    if (((currentChar = fgetc(inputFile)) == '"')) {
        while ((currentChar = fgetc(inputFile)) != EOF) {
            input[inputIndex++] = (char)currentChar;
            if (currentChar == '"') {
                input[inputIndex] = '\0';
                token.type = TOKEN_STRING_CONSTANT;
                strcpy(token.lexeme, input);
                break;
            }
        }
        if (currentChar == EOF) {
            printf("Lexical error: String constant not terminated before end of file\n");
            exit(0);
        }
    }
    return token;
}

void addToList(Variable variable) { //değişkenleri listeye eklediğimiz yer
    if (variableCount < MAX_VARIABLES) {
        variableList[variableCount] = variable;
        variableCount++;
    } else {
        printf("Variable list is full. Cannot add more variables.\n");
        exit(0);
    }
}

void parseDeclaration(FILE* inputFile, Token token) { //isim olarak değişkeni tanımlama işlemi yaptığımız yer
    if (token.type == TOKEN_KEYWORD && strcmp(token.lexeme, "new") == 0) {
        token = getNextToken(inputFile);
        if (token.type == TOKEN_IDENTIFIER) {
            Variable newVariable;
            strcpy(newVariable.name, token.lexeme);

            token = getNextToken(inputFile);
            if (token.type == TOKEN_KEYWORD && strcmp(token.lexeme, "int") == 0) { //intse value int stringse value string oluyor
                newVariable.type = VALUE_INT;
            } else if (token.type == TOKEN_KEYWORD && strcmp(token.lexeme, "text") == 0) {
                newVariable.type = VALUE_STRING;
            } else {
                printf("Syntax error: Invalid variable type\n");
                exit(0);
                return;
            }

            token = getNextToken(inputFile);
            if (token.type == TOKEN_END_OF_LINE) { //satır bitmiş mi diye kontrol ediyor
                addToList(newVariable);
            } else {
                printf("Syntax error: Expected end of line\n");
                exit(0);
            }

        } else {
            printf("Syntax error: Expected variable identifier\n");
            exit(0);
        }
    } else {
        printf("Syntax error: Expected 'new' keyword\n");
        exit(0);
    }
}

void parseAssignment(FILE* inputFile, Token token) { //değer atamaların yapıldığı yer onemli metot
    //bazı fonksiyonları burada çağırdık size, subs, locate, insert gibi çünkü bir değer döndürmesi istenmiş o metotlardan ondan mantıken returnlü metotları değişkene eşitlemek gerekir.
    //inserti hem burada hem interpret metodunda çağırdık insert değer döndürmesine rağmen kendi içinde de değişken üzerinde değişiklik yapabiliyor çünkü

    if (token.type == TOKEN_IDENTIFIER) {
        char variableName[MAX_IDENTIFIER_SIZE];
        strcpy(variableName, token.lexeme);

        token = getNextToken(inputFile);
        if (token.type == TOKEN_ASSIGNMENT) {
            token = getNextToken(inputFile);

            int variableIndex = -1;
            for (int i = 0; i < variableCount; i++) {
                if (strcmp(variableList[i].name, variableName) == 0) {
                    variableIndex = i;
                    break;
                }
            }
            if (variableIndex != -1) {
                Token prevToken = token;
                Token nextToken = getNextToken(inputFile);
                if (token.type == TOKEN_INTEGER_CONSTANT && nextToken.type == TOKEN_END_OF_LINE) { //sadece belirli bir değer atamaların yapıldığı if bu ve bundan sonraki 1 else if
                    if (variableList[variableIndex].type == VALUE_INT) {
                        variableList[variableIndex].value.intValue = atoi(token.lexeme);
                    } else {
                        printf("Type mismatch in variable assignment\n");
                        exit(0);
                    }
                } else if (token.type == TOKEN_STRING_CONSTANT && nextToken.type == TOKEN_END_OF_LINE) {
                    if (variableList[variableIndex].type == VALUE_STRING) {
                        strcpy(variableList[variableIndex].value.stringValue, token.lexeme);
                    } else {
                        printf("Type mismatch in variable assignment\n");
                        exit(0);
                    }

                } else if (nextToken.type == TOKEN_OPERATOR) {//operatör mü diye bakıyor token tipi + veya - olabiliyor zaten
                    processAdditionorSubtraction(token, nextToken, inputFile, variableIndex, variableName);
                    token = getNextToken(inputFile);
                    if (token.type != TOKEN_END_OF_LINE) { //satır bitmiş mi diye kontrol ediyor
                        printf("Syntax error: Expected end of line\n");
                        exit(0);
                    }
                } else if (prevToken.type == TOKEN_KEYWORD){//keyword mü diye bakıyor ona göre ilgili fonksiyona giriyor
                    if (strcmp(prevToken.lexeme,"size")==0){
                        variableList[variableIndex].value.intValue =processSize(inputFile,variableIndex,variableName);
                        token = getNextToken(inputFile);
                        if (token.type != TOKEN_END_OF_LINE) { //satır bitmiş mi diye kontrol ediyor
                            printf("Syntax error: Expected end of line\n");
                            exit(0);
                        }
                    }
                    else if (strcmp(prevToken.lexeme,"subs")==0){
                        strcpy(variableList[variableIndex].value.stringValue,processSubs(inputFile,variableIndex,variableName));
                        token = getNextToken(inputFile);
                        if (token.type != TOKEN_END_OF_LINE) { //satır bitmiş mi diye kontrol ediyor
                            printf("Syntax error: Expected end of line\n");
                            exit(0);
                        }
                    } else if (strcmp(prevToken.lexeme,"locate")==0){
                        variableList[variableIndex].value.intValue = processLocate(inputFile);
                        token = getNextToken(inputFile);
                        if (token.type != TOKEN_END_OF_LINE) { //satır bitmiş mi diye kontrol ediyor
                            printf("Syntax error: Expected end of line\n");
                            exit(0);
                        }
                    } else if (strcmp(prevToken.lexeme,"insert")==0){
                        strcpy(variableList[variableIndex].value.stringValue,processInsert(inputFile));
                        token = getNextToken(inputFile); token = getNextToken(inputFile);
                        if (token.type != TOKEN_END_OF_LINE) { //satır bitmiş mi diye kontrol ediyor
                            printf("Syntax error: Expected end of line\n");
                            exit(0);
                        }
                    } else if (strcmp(prevToken.lexeme,"override")==0){
                        strcpy(variableList[variableIndex].value.stringValue,processOverride(inputFile));
                        token = getNextToken(inputFile); token = getNextToken(inputFile);
                        if (token.type != TOKEN_END_OF_LINE) { //satır bitmiş mi diye kontrol ediyor
                            printf("Syntax error: Expected end of line\n");
                            exit(0);
                        }
                    } else if (strcmp(prevToken.lexeme,"asstring")==0){
                        strcpy(variableList[variableIndex].value.stringValue,processAsString(inputFile));
                        token = getNextToken(inputFile); token = getNextToken(inputFile);
                        if (token.type != TOKEN_END_OF_LINE) { //satır bitmiş mi diye kontrol ediyor
                            printf("Syntax error: Expected end of line\n");
                            exit(0);
                        }
                    } else if (strcmp(prevToken.lexeme,"astext")==0){
                        variableList[variableIndex].value.intValue = processAsInt(inputFile);
                        token = getNextToken(inputFile); token = getNextToken(inputFile);
                        if (token.type != TOKEN_END_OF_LINE) { //satır bitmiş mi diye kontrol ediyor
                            printf("Syntax error: Expected end of line\n");
                            exit(0);
                        }
                    }
                }
                else {
                    printf("Syntax error: Expected identifier or constant\n");
                    exit(0);
                }
            } else {
                printf("Variable '%s' not found\n", variableName);
                exit(0);
            }
        } else {
            printf("Syntax error: Expected assignment operator\n");
            exit(0);
        }
    } else {
        printf("Syntax error: Expected variable identifier\n");
        exit(0);
    }
}

int asInt( char* myString) { //stringi inte çeviren fonksiyon
    return atoi(myString);
}

void executeRead(FILE* inputFile) {//read keywordü ile başlanıp hocanın istediği formatta dosya okuma işlemi dosyanın önceden var olması gerekiyor
    Token variableToken = getNextToken(inputFile);
    Token fromToken = getNextToken(inputFile);
    Token fileNameToken = getNextToken(inputFile);
    if (variableToken.type == TOKEN_IDENTIFIER) {
        int variableIndex = -1;
        for (int i = 0; i < variableCount; i++) {
            if (strcmp(variableList[i].name, variableToken.lexeme) == 0) {
                variableIndex = i;
                break;
            }
        }

        if (strcmp(fromToken.lexeme, "from") == 0) {
            char* fileName = strcat(fileNameToken.lexeme,".txt");
            FILE* file = fopen(fileName, "r");
            if (file) {
                fseek(file, 0, SEEK_END);
                long fileSize = ftell(file);
                rewind(file);

                char* buffer = (char*)malloc(fileSize + 1);
                if (buffer) {
                    fread(buffer, 1, fileSize, file);
                    buffer[fileSize] = '\0';

                    strcpy(variableList[variableIndex].value.stringValue,buffer);
                    variableList[variableIndex].type = VALUE_STRING;

                    fclose(file);
                } else {
                    printf("Memory allocation failed\n"); exit(0);
                }
            } else {
                printf("Failed to open %s \n", fileName); exit(0);
            }
        } else {
            printf("from keyword not found\n"); exit(0);
        }
    } else {
        printf("Syntax error: Expected variable identifier\n"); exit(0);
    }
}

void executeWrite(FILE* inputFile) {//dosyaya değer yazma işlemi dosyanın var olması gerekiyor
    Token variableToken = getNextToken(inputFile);
    Token toToken = getNextToken(inputFile);
    Token fileNameToken = getNextToken(inputFile);
    if (variableToken.type == TOKEN_IDENTIFIER || variableToken.type == TOKEN_STRING_CONSTANT) {
        int variableIndex = -1;
        for (int i = 0; i < variableCount; i++) {
            if (strcmp(variableList[i].name, variableToken.lexeme) == 0) {
                variableIndex = i;
                break;
            }
        }
        if (strcmp(toToken.lexeme,"to") == 0) {
            char* fileName = strcat(fileNameToken.lexeme,".txt");
            FILE* file = fopen(fileName, "w");
            if (file) {
                const char* myText;
                if (variableToken.type == TOKEN_STRING_CONSTANT){
                    myText = variableToken.lexeme;
                }else{
                    myText = variableList[variableIndex].value.stringValue;
                }
                fprintf(file, "%s", myText);
                fclose(file);
            } else {
                printf("Failed to open %s\n",fileName); exit(0);
            }
        } else {
            printf("to keyword not found\n"); exit(0);
        }
    } else {
        printf("Syntax error: Expected variable identifier\n"); exit(0);
    }
}

void executeInput(FILE* inputFile) {//prompttext mesajını gösterip konsoldan input alan metot direkt olarak yazdırmaz output ile yazdırmalısınız
    Token variableToken = getNextToken(inputFile);
    Token promptToken = getNextToken(inputFile);
    Token promptTextToken = getNextToken(inputFile);
    if (variableToken.type == TOKEN_IDENTIFIER) {
        int variableIndex = -1;
        for (int i = 0; i < variableCount; i++) {
            if (strcmp(variableList[i].name, variableToken.lexeme) == 0) {
                variableIndex = i;
                break;
            }
        }
        if (variableIndex == -1){
            printf("Variable not declared"); exit(0);
            exit(0);
        }
        if (strcmp(promptToken.lexeme,"prompt") == 0) {
            char* promptText;
            if (promptTextToken.type == TOKEN_STRING_CONSTANT){
                promptText = promptTextToken.lexeme;
            } else {
                int variableIndex1 = -1;
                for (int i = 0; i < variableCount; i++) {
                    if (strcmp(variableList[i].name, promptTextToken.lexeme) == 0) {
                        variableIndex1 = i;
                        break;
                    }
                }
                if (variableIndex1 < 0){
                    printf("Prompt txt degiskeniniz bulunamadi"); exit(0);
                    exit(0);
                }
                promptText = variableList[variableIndex1].value.stringValue;
            }
            printf(strcat(promptText," "), variableToken.lexeme);
            char input[100];
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = '\0';

            strncpy(variableList[variableIndex].value.stringValue, input, MAX_STRING_SIZE);
            variableList[variableIndex].type = VALUE_STRING;
        } else {
            printf("prompt keyword not found\n"); exit(0);
        }
    } else {
        printf("Syntax error: Expected variable identifier\n"); exit(0);
    }
}

int processAsInt(FILE* inputFile) { //stringi inte çevirme metodunun dilimiz için uyarlandığı yer

    Token token = getNextToken(inputFile);
    if (token.type == TOKEN_IDENTIFIER || token.type == TOKEN_STRING_CONSTANT) {
        int variableIndex = -1;
        for (int i = 0; i < variableCount; i++) {
            if (strcmp(variableList[i].name, token.lexeme) == 0) {
                variableIndex = i;
                break;
            }
        }

        if (variableList[variableIndex].type == VALUE_STRING && token.type != TOKEN_STRING_CONSTANT) {

            char* myString = variableList[variableIndex].value.stringValue;
            myString = removeSubstring(myString,"\""); //gereksiz tırnakları kaldırıyor
            myString = removeSubstring(myString,"\""); //gereksiz tırnakları kaldırıyor
            int myInt = asInt(myString);
            return myInt;
        } else if (token.type == TOKEN_STRING_CONSTANT) {
            char* myString = token.lexeme;
            myString = removeSubstring(myString,"\""); //gereksiz tırnakları kaldırıyor
            myString = removeSubstring(myString,"\""); //gereksiz tırnakları kaldırıyor
            int myInt = asInt(myString);
            return myInt;
        }
        else {
            printf("Type mismatch: Expected variable of type string\n"); exit(0);
        }
    } else {
        printf("Variable '%s' not found\n", token.lexeme); exit(0);
    }
}

char* asString(int myInt) { //inti stringi çevirme fonksiyonu
    char* result = malloc(MAX_STRING_SIZE);
    snprintf(result, MAX_STRING_SIZE, "%d", myInt);
    return result;
}

char* processAsString(FILE* inputFile) { //inti stringe çevirme metodunun kendi dilimizde uyarlandığı yer
    Token token = getNextToken(inputFile);
    if (token.type == TOKEN_IDENTIFIER || token.type == TOKEN_INTEGER_CONSTANT) {
        int variableIndex = -1;
        for (int i = 0; i < variableCount; i++) {
            if (strcmp(variableList[i].name, token.lexeme) == 0) {
                variableIndex = i;
                break;
            }
        }

        if (variableList[variableIndex].type == VALUE_INT && token.type != TOKEN_INTEGER_CONSTANT) {
            int myInt = variableList[variableIndex].value.intValue;
            char* myString = asString(myInt);
            myString = insert(myString,0,"\"");
            myString = insert(myString, strlen(myString),"\"");
            return myString;
        } else if (token.type == TOKEN_INTEGER_CONSTANT){
            int myInt = atoi(token.lexeme);
            char* myString = asString(myInt);
            myString = insert(myString,0,"\"");
            myString = insert(myString, strlen(myString),"\"");
            return myString;
        }else {
            printf("Type mismatch: Expected variable of type int or int const\n"); exit(0);
        }

    } else {
        printf("Syntax error: Expected variable identifier\n"); exit(0);
    }

}

char* override(const char* myText, int location, const char* ovrText) {//genel override mantığı metodu
    int myTextLength = strlen(myText);
    int ovrTextLength = strlen(ovrText);

    if (location < 0) {
        printf("Error: Invalid location\n"); exit(0);
        return NULL;
    }

    // Calculate the end position to write ovrText
    int endPosition = location + ovrTextLength;

    // If endPosition exceeds the length of myText, truncate ovrText
    if (endPosition > myTextLength) {
        endPosition = myTextLength;
        ovrTextLength = myTextLength - location;
    }

    char* result = malloc(myTextLength + 1);  // Allocate memory for the result
    strncpy(result, myText, myTextLength);    // Copy myText to result
    strncpy(result + location, ovrText, ovrTextLength);  // Override ovrText onto result
    result[myTextLength] = '\0';  // Null-terminate the result string

    return result;
}

char* processOverride(FILE* inputFile) {//overrideın kendi dilimize uygulandığı yer
    Token typeToken = getNextToken(inputFile);
    if (typeToken.type == TOKEN_IDENTIFIER || typeToken.type == TOKEN_STRING_CONSTANT) {
        char variableName[MAX_IDENTIFIER_SIZE];
        strcpy(variableName, typeToken.lexeme);

        Token token = getNextToken(inputFile);
        if (token.type == TOKEN_COMMA) {
            token = getNextToken(inputFile);
            if (token.type == TOKEN_INTEGER_CONSTANT) {
                int location = atoi(token.lexeme);

                token = getNextToken(inputFile);
                if (token.type == TOKEN_COMMA) {
                    token = getNextToken(inputFile);
                    if (token.type == TOKEN_STRING_CONSTANT || token.type == TOKEN_IDENTIFIER) {
                        char variableName1[MAX_IDENTIFIER_SIZE];
                        strcpy(variableName1, token.lexeme);
                        int variableIndex1 = -1;
                        for (int i = 0; i < variableCount; i++) {
                            if (strcmp(variableList[i].name, variableName1) == 0) {
                                variableIndex1 = i;
                                break;
                            }
                        }
                        char* overrideText;
                        if (token.type == TOKEN_STRING_CONSTANT){
                            overrideText= removeSubstring(token.lexeme, "\""); //gereksiz tırnakları kaldırıyor
                            overrideText = removeSubstring(overrideText, "\""); //gereksiz tırnakları kaldırıyor
                        }
                        else{
                            overrideText= removeSubstring(variableList[variableIndex1].value.stringValue, "\""); //gereksiz tırnakları kaldırıyor
                            overrideText= removeSubstring(overrideText, "\""); //gereksiz tırnakları kaldırıyor
                        }

                        int variableIndex = -1;
                        for (int i = 0; i < variableCount; i++) {
                            if (strcmp(variableList[i].name, variableName) == 0) {
                                variableIndex = i;
                                break;
                            }
                        }
                        if (variableIndex != -1 || typeToken.type == TOKEN_STRING_CONSTANT) {
                            if (variableList[variableIndex].type == VALUE_STRING || typeToken.type == TOKEN_STRING_CONSTANT) {
                                char* myText;
                                if (typeToken.type == TOKEN_IDENTIFIER){
                                    myText = variableList[variableIndex].value.stringValue;
                                } else {
                                    myText = typeToken.lexeme;
                                }
                                char* result;

                                if (strlen(myText) - 2 <= strlen(overrideText)){ //boyutu uzun olursa diye
                                    result = overrideText;
                                } else{
                                    result = override(myText, location + 1, overrideText); //tırnaktan dolayı yine
                                }

                                if (typeToken.type == TOKEN_IDENTIFIER){
                                    strcpy(variableList[variableIndex].value.stringValue, result);
                                }else{
                                    strcpy(typeToken.lexeme, result);
                                }
                                return result;
                            } else {
                                printf("Type mismatch: Variable '%s' should be of type string\n", variableName); exit(0);
                            }
                        } else {
                            printf("Variable '%s' not found\n", variableName); exit(0);
                        }
                        free(overrideText);
                    } else {
                        printf("Syntax error: Expected string constant\n"); exit(0);
                    }
                } else {
                    printf("Syntax error: Expected comma\n"); exit(0);
                }
            } else {
                printf("Syntax error: Expected integer constant\n"); exit(0);
            }
        } else {
            printf("Syntax error: Expected comma\n"); exit(0);
        }
    } else {
        printf("Syntax error: Expected variable identifier\n"); exit(0);
    }

    return NULL;
}

char* insert(const char* myText, int location, const char* insertText) { //basit halde insert metodu
    int myTextLength = strlen(myText);
    int insertTextLength = strlen(insertText);

    if (location < 0 || location > myTextLength) {
        printf("Error: Invalid location\n"); exit(0);
        return NULL;
    }

    char* result = malloc((myTextLength + insertTextLength + 1) * sizeof(char));
    if (result == NULL) {
        printf("Error: Memory allocation failed\n"); exit(0);
        return NULL;
    }

    strncpy(result, myText, location);
    strncpy(result + location, insertText, insertTextLength);
    strcpy(result + location + insertTextLength, myText + location);

    return result;
}

char* processInsert(FILE* inputFile) { //programa uyarlanmış hali insertün
    Token token = getNextToken(inputFile);
    if (token.type == TOKEN_IDENTIFIER) {
        char variableName[MAX_IDENTIFIER_SIZE];
        strcpy(variableName, token.lexeme);

        token = getNextToken(inputFile);
        if (token.type == TOKEN_COMMA) {
            token = getNextToken(inputFile);
            if (token.type == TOKEN_INTEGER_CONSTANT) {
                int location = atoi(token.lexeme);

                token = getNextToken(inputFile);
                if (token.type == TOKEN_COMMA) {
                    token = getNextToken(inputFile);
                    if (token.type == TOKEN_STRING_CONSTANT || token.type == TOKEN_IDENTIFIER) {
                        char variableName1[MAX_IDENTIFIER_SIZE];
                        strcpy(variableName1, token.lexeme);
                        int variableIndex1 = -1;
                        for (int i = 0; i < variableCount; i++) {
                            if (strcmp(variableList[i].name, variableName1) == 0) {
                                variableIndex1 = i;
                                break;
                            }
                        }
                        char* insertText;
                        if (token.type == TOKEN_STRING_CONSTANT){
                            insertText= removeSubstring(token.lexeme, "\""); //gereksiz tırnakları kaldırıyor
                            insertText = removeSubstring(insertText, "\""); //gereksiz tırnakları kaldırıyor
                        }
                        else{
                            insertText= removeSubstring(variableList[variableIndex1].value.stringValue, "\""); //gereksiz tırnakları kaldırıyor
                            insertText= removeSubstring(insertText, "\"");
                        }

                        int variableIndex = -1;
                        for (int i = 0; i < variableCount; i++) {
                            if (strcmp(variableList[i].name, variableName) == 0) {
                                variableIndex = i;
                                break;
                            }
                        }


                        if (variableIndex != -1) {
                            if (variableList[variableIndex].type == VALUE_STRING) {
                                char* updatedText = insert(variableList[variableIndex].value.stringValue, location + 1, insertText); // +1 ilk textteki tırnaktan dolayı
                                if (updatedText != NULL) {
                                    strcpy(variableList[variableIndex].value.stringValue, updatedText);
                                    token = getNextToken(inputFile);
                                    return updatedText;
                                }
                            } else {
                                printf("Type mismatch: Variable '%s' should be of type string\n", variableName); exit(0);
                            }
                        } else {
                            printf("Variable '%s' not found\n", variableName); exit(0);
                        }
                        free(insertText);
                    } else {
                        printf("Syntax error: Expected string constant\n"); exit(0);
                    }
                } else {
                    printf("Syntax error: Expected comma\n"); exit(0);
                }
            } else {
                printf("Syntax error: Expected integer constant\n"); exit(0);
            }
        } else {
            printf("Syntax error: Expected comma\n"); exit(0);
        }
    } else {
        printf("Syntax error: Expected variable identifier %s\n", token.lexeme); exit(0);
    }
}

int locate(char *bigText, char *smallText, int start) { //basit haldeki locate metodu
    // Check if start is beyond the length of bigText
    if (start >= strlen(bigText)) {
        return 1; //process methodunda 1 çıkardık ondan 1 ekledim normalde 0
    }

    char *foundPos = strstr(bigText + start, smallText);
    if (foundPos != NULL) {
        return (foundPos - bigText) + 1; // +1 to adjust to 1-based indexing
    } else {
        return 1; //process methodunda 1 çıkardık ondan 1 ekledim normalde 0
    }
}

int processLocate(FILE* inputFile) { //locate metodunun uyarlanmış hali
    //empler virgüller parantez o tarz tokenler
    Token bigTextToken = getNextToken(inputFile);
    Token emp1 = getNextToken(inputFile);
    Token smallTextToken = getNextToken(inputFile);
    Token emp2 = getNextToken(inputFile);
    Token startToken = getNextToken(inputFile);
    Token emp3 = getNextToken(inputFile);

    if ((bigTextToken.type == TOKEN_IDENTIFIER || bigTextToken.type == TOKEN_STRING_CONSTANT) && (smallTextToken.type == TOKEN_IDENTIFIER || smallTextToken.type == TOKEN_STRING_CONSTANT) && startToken.type == TOKEN_INTEGER_CONSTANT) {
        char variableName1[MAX_IDENTIFIER_SIZE];
        char variableName2[MAX_IDENTIFIER_SIZE];
        strcpy(variableName1, bigTextToken.lexeme);
        strcpy(variableName2, smallTextToken.lexeme);

        int variableIndex1 = findVariableIndex(variableName1);
        int variableIndex2 = findVariableIndex(variableName2);

        if (variableIndex1 != -1 && variableIndex2 != -1) {
            if (variableList[variableIndex1].type == VALUE_STRING && variableList[variableIndex2].type == VALUE_STRING) {
                int start = atoi(startToken.lexeme);
                if (start < 0) {
                    printf("Start index cannot be negative\n"); exit(0);
                    return -1;
                }
                char* bigText;
                char* smallText;
                if (bigTextToken.type == TOKEN_IDENTIFIER){
                    bigText = variableList[variableIndex1].value.stringValue;
                }
                else{
                    bigText = bigTextToken.lexeme;
                }
                if (smallTextToken.type == TOKEN_IDENTIFIER){
                    smallText = variableList[variableIndex2].value.stringValue;
                }
                else{
                    smallText = bigTextToken.lexeme;
                }
                strcpy(smallText,removeSubstring(smallText,"\"")); //gereksiz tırnakları kaldırıyor
                strcpy(smallText,removeSubstring(smallText,"\"")); //gereksiz tırnakları kaldırıyor

                int position = locate(bigText, smallText, start);
                return position - 1; // -1in sebebi tırnak işaretini de alması
            } else {
                printf("Type mismatch in locate method\n"); exit(0);
                return -2;
            }
        } else {
            printf("Variable(s) not found\n"); exit(0);
            return -3;
        }
    } else {
        printf("Syntax error: Invalid arguments for locate method\n"); exit(0);
        return -4;
    }

    return -1;  // Return -1 to indicate an error occurred
}

// index bulan fonksiyon
int findVariableIndex(const char* variableName) {
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variableList[i].name, variableName) == 0) {
            return i;
        }
    }
    return -1;  // bulunamazsa diye
}


char* substr(const char *src, int m, int n) //substringin basit hali
{
    // get the length of the destination string
    int len = n - m;

    // allocate (len + 1) chars for destination (+1 for extra null character)
    char *dest = (char*)malloc(sizeof(char) * (len + 1));

    // extracts characters between m'th and n'th index from source string
    // and copy them into the destination string
    for (int i = m; i < n && (*(src + i) != '\0'); i++)
    {
        *dest = *(src + i);
        dest++;
    }

    // null-terminate the destination string
    *dest = '\0';

    // return the destination string
    return dest - len;
}

char* processSubs(FILE* inputFile, int variableIndex, char* variableName){ //substring metodunun uyarlanmış hali
    Token token = getNextToken(inputFile);
    if (variableList[variableIndex].type == VALUE_STRING){
        if (token.type == TOKEN_IDENTIFIER || token.type == TOKEN_STRING_CONSTANT){
            char variableNameOperand1[MAX_IDENTIFIER_SIZE];
            strcpy(variableNameOperand1, token.lexeme);
            int variableIndex1 = -1;
            for (int i = 0; i < variableCount; i++) {
                if (strcmp(variableList[i].name, variableNameOperand1) == 0) {
                    variableIndex1 = i;
                    break;
                }
            }
            char* myText;
            if (token.type == TOKEN_IDENTIFIER){
                myText = variableList[variableIndex1].value.stringValue;
            } else{
                myText = token.lexeme;
            }
            Token nextToken = getNextToken(inputFile);
            if(nextToken.type == TOKEN_COMMA){
                nextToken = getNextToken(inputFile);
                if (nextToken.type == TOKEN_INTEGER_CONSTANT || nextToken.type == TOKEN_IDENTIFIER){
                    int begin = atoi(nextToken.lexeme);
                    if (nextToken.type == TOKEN_IDENTIFIER){
                        int variableIndex2 = -1;
                        for (int i = 0; i < variableCount; i++) {
                            if (strcmp(variableList[i].name, nextToken.lexeme) == 0) {
                                variableIndex2 = i;
                                break;
                            }
                        }
                        begin = variableList[variableIndex2].value.intValue;
                    }
                    nextToken = getNextToken(inputFile);
                    if (nextToken.type != TOKEN_COMMA){
                        printf("Error: comma"); exit(0);
                    }
                    nextToken = getNextToken(inputFile);
                    if (nextToken.type == TOKEN_INTEGER_CONSTANT || nextToken.type == TOKEN_IDENTIFIER){
                        int end = atoi(nextToken.lexeme);
                        if (nextToken.type == TOKEN_IDENTIFIER){
                            int variableIndex3 = -1;
                            for (int i = 0; i < variableCount; i++) {
                                if (strcmp(variableList[i].name, nextToken.lexeme) == 0) {
                                    variableIndex3 = i;
                                    break;
                                }
                            }
                            end = variableList[variableIndex3].value.intValue;
                        }
                        token = getNextToken(inputFile);
                        if (token.type != TOKEN_RIGHTPAR) {
                            printf("Error: RightPar"); exit(0);
                        }
                        myText = removeSubstring(myText, "\""); myText = removeSubstring(myText, "\""); //gereksiz tırnakları kaldırıyor
                        char* myTextSubbed = substr(myText,begin,end);
                        strcat(myTextSubbed,"\"");
                        myTextSubbed = insert(myTextSubbed,0,"\"");
                        return myTextSubbed;

                    } else{
                        printf("Error: Your end parameter must be an integer constant."); exit(0);
                    }
                }
                else{
                    printf("Error: Your begin parameter must be an integer constant."); exit(0);
                }
            }
            else{
                printf("Error: comma"); exit(0);
                return 0;
            }

        }
        else{
            printf("Error given input should be an identifier or string const"); exit(0);
            return 0;
        }
    } else {
        printf("Error: variable type should be int"); exit(0);
        return 0;
    }
}

int processSize(FILE* inputFile, int variableIndex, char* variableName){ //size metodunun uyarlanmış hali
    Token token = getNextToken(inputFile);
    if (variableList[variableIndex].type == VALUE_INT){
        if (token.type == TOKEN_IDENTIFIER){
            char variableNameOperand1[MAX_IDENTIFIER_SIZE];
            strcpy(variableNameOperand1, token.lexeme);
            int variableIndex1 = -1;
            for (int i = 0; i < variableCount; i++) {
                if (strcmp(variableList[i].name, variableNameOperand1) == 0) {
                    variableIndex1 = i;
                    break;
                }
            }
            Token nextToken = getNextToken(inputFile);
            if(nextToken.type == TOKEN_RIGHTPAR){
                return strlen(variableList[variableIndex1].value.stringValue) - 2; //kesme işaretlerinden dolayı 2 çıkardık
            }
            else{
                printf("Error: parenthesis"); exit(0);
                return 0;
            }

        }
        else if (token.type == TOKEN_STRING_CONSTANT){
            Token nextToken = getNextToken(inputFile);
            if(nextToken.type == TOKEN_RIGHTPAR){
                return strlen(token.lexeme) - 2; //kesme işaretlerinden dolayı 2 çıkardık
            }
            else{
                printf("Error: parenthesis"); exit(0);
                return 0;
            }

        }
        else{
            printf("Error given input should be an identifier or string const"); exit(0);
            return 0;
        }
    } else {
        printf("Error: variable type should be int"); exit(0);
        return 0;
    }

}

void processAddition(Token prevToken, Token token, FILE* inputFile, int variableIndex, char* variableName){ //ekleme işlemi önce tokenlerin türlerini buluyo ona göre işlem yaptırıyo
    Token nextToken = getNextToken(inputFile);
    char variableNameOperand1[MAX_IDENTIFIER_SIZE];
    strcpy(variableNameOperand1, prevToken.lexeme);
    char variableNameOperand2[MAX_IDENTIFIER_SIZE];
    strcpy(variableNameOperand2, nextToken.lexeme);
    int variableIndex1 = -1;
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variableList[i].name, variableNameOperand1) == 0) {
            variableIndex1 = i;
            break;
        }
    }
    int variableIndex2 = -1;
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variableList[i].name, variableNameOperand2) == 0) {
            variableIndex2 = i;
            break;
        }
    }
    if (prevToken.type == TOKEN_INTEGER_CONSTANT && nextToken.type == TOKEN_INTEGER_CONSTANT){
        if (variableList[variableIndex].type == VALUE_INT) {
            variableList[variableIndex].value.intValue = atoi(prevToken.lexeme) + atoi(nextToken.lexeme);
        } else {
            printf("Type mismatch in variable assignment\n");
            exit(0);
        }

    } else if (prevToken.type == TOKEN_IDENTIFIER && nextToken.type == TOKEN_INTEGER_CONSTANT){
        if (variableList[variableIndex].type == VALUE_INT) {
            variableList[variableIndex].value.intValue = variableList[variableIndex1].value.intValue + atoi(nextToken.lexeme);
        } else {
            printf("Type mismatch in variable assignment\n");
            exit(0);
        }

    } else if (prevToken.type == TOKEN_INTEGER_CONSTANT && nextToken.type == TOKEN_IDENTIFIER){
        if (variableList[variableIndex].type == VALUE_INT) {
            variableList[variableIndex].value.intValue = atoi(prevToken.lexeme) + variableList[variableIndex2].value.intValue;
        } else {
            printf("Type mismatch in variable assignment\n");
            exit(0);
        }

    } else if (prevToken.type == TOKEN_STRING_CONSTANT && nextToken.type == TOKEN_STRING_CONSTANT){
        if (variableList[variableIndex].type == VALUE_STRING) {
            strcat(variableList[variableIndex].value.stringValue, prevToken.lexeme);
            strcat(variableList[variableIndex].value.stringValue, nextToken.lexeme);
            strcpy(variableList[variableIndex].value.stringValue,removeSubstring(variableList[variableIndex].value.stringValue, "\"\""));

        } else {
            printf("Type mismatch in variable assignment\n");
            exit(0);
        }
    } else if (prevToken.type == TOKEN_IDENTIFIER && nextToken.type == TOKEN_STRING_CONSTANT){
        if (variableList[variableIndex].type == VALUE_STRING) {
            strcat(variableList[variableIndex].value.stringValue, variableList[variableIndex1].value.stringValue);
            strcat(variableList[variableIndex].value.stringValue, nextToken.lexeme);
            strcpy(variableList[variableIndex].value.stringValue,removeSubstring(variableList[variableIndex].value.stringValue, "\"\""));

        } else {
            printf("Type mismatch in variable assignment\n");
            exit(0);
        }

    } else if (prevToken.type == TOKEN_STRING_CONSTANT && nextToken.type == TOKEN_IDENTIFIER){
        if (variableList[variableIndex].type == VALUE_STRING) {
            strcat(variableList[variableIndex].value.stringValue, prevToken.lexeme);
            strcat(variableList[variableIndex].value.stringValue, variableList[variableIndex2].value.stringValue);
            strcpy(variableList[variableIndex].value.stringValue,removeSubstring(variableList[variableIndex].value.stringValue, "\"\"")); //gereksiz tırnakları kaldırıyor

        } else {
            printf("Type mismatch in variable assignment\n");
            exit(0);
        }

    } else if (prevToken.type == TOKEN_IDENTIFIER && nextToken.type == TOKEN_IDENTIFIER){
        if (variableList[variableIndex].type == VALUE_INT) {
            variableList[variableIndex].value.intValue = variableList[variableIndex1].value.intValue + variableList[variableIndex2].value.intValue;
        } else {
            strcat(variableList[variableIndex].value.stringValue, variableList[variableIndex1].value.stringValue);
            strcat(variableList[variableIndex].value.stringValue, variableList[variableIndex2].value.stringValue);
            strcpy(variableList[variableIndex].value.stringValue,removeSubstring(variableList[variableIndex].value.stringValue, "\"\"")); //gereksiz tırnakları kaldırıyor

        }
    } else{
        printf("Error: type error \n");
        exit(0);
    }
}

void processSubtraction(Token prevToken, Token token, FILE* inputFile,int variableIndex, char* variableName){ //çıkarma işlemi önce tokenlerin türlerini buluyo ona göre işlem yaptırıyo
    Token nextToken = getNextToken(inputFile);
    char variableNameOperand1[MAX_IDENTIFIER_SIZE];
    strcpy(variableNameOperand1, prevToken.lexeme);
    char variableNameOperand2[MAX_IDENTIFIER_SIZE];
    strcpy(variableNameOperand2, nextToken.lexeme);
    int variableIndex1 = -1;
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variableList[i].name, variableNameOperand1) == 0) {
            variableIndex1 = i;
            break;
        }
    }
    int variableIndex2 = -1;
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variableList[i].name, variableNameOperand2) == 0) {
            variableIndex2 = i;
            break;
        }
    }
    if (prevToken.type == TOKEN_INTEGER_CONSTANT && nextToken.type == TOKEN_INTEGER_CONSTANT){
        if (variableList[variableIndex].type == VALUE_INT) {
            variableList[variableIndex].value.intValue = atoi(prevToken.lexeme) - atoi(nextToken.lexeme);
        } else {
            printf("Type mismatch in variable assignment\n");
        }

    } else if (prevToken.type == TOKEN_IDENTIFIER && nextToken.type == TOKEN_INTEGER_CONSTANT){
        if (variableList[variableIndex].type == VALUE_INT) {
            variableList[variableIndex].value.intValue = variableList[variableIndex1].value.intValue - atoi(nextToken.lexeme);
        } else {
            printf("Type mismatch in variable assignment\n");
        }

    } else if (prevToken.type == TOKEN_INTEGER_CONSTANT && nextToken.type == TOKEN_IDENTIFIER){
        if (variableList[variableIndex].type == VALUE_INT) {
            variableList[variableIndex].value.intValue = atoi(prevToken.lexeme) - variableList[variableIndex2].value.intValue;
        } else {
            printf("Type mismatch in variable assignment\n");
        }

    } else if (prevToken.type == TOKEN_STRING_CONSTANT && nextToken.type == TOKEN_STRING_CONSTANT){
        if (variableList[variableIndex].type == VALUE_STRING) {
            strcpy(nextToken.lexeme,removeSubstring(nextToken.lexeme,"\""));
            strcpy(nextToken.lexeme,removeSubstring(nextToken.lexeme,"\""));
            strcpy(variableList[variableIndex].value.stringValue, removeSubstring(prevToken.lexeme,nextToken.lexeme));
        } else {
            printf("Type mismatch in variable assignment\n");
        }
    } else if (prevToken.type == TOKEN_IDENTIFIER && nextToken.type == TOKEN_STRING_CONSTANT){
        if (variableList[variableIndex].type == VALUE_STRING) {
            strcpy(nextToken.lexeme,removeSubstring(nextToken.lexeme,"\""));
            strcpy(nextToken.lexeme,removeSubstring(nextToken.lexeme,"\""));
            strcpy(variableList[variableIndex].value.stringValue,removeSubstring(variableList[variableIndex1].value.stringValue, nextToken.lexeme));
        } else {
            printf("Type mismatch in variable assignment\n");
        }

    } else if (prevToken.type == TOKEN_STRING_CONSTANT && nextToken.type == TOKEN_IDENTIFIER){
        if (variableList[variableIndex].type == VALUE_STRING) {
            strcpy(variableList[variableIndex2].value.stringValue,removeSubstring(variableList[variableIndex2].value.stringValue,"\""));
            strcpy(variableList[variableIndex2].value.stringValue,removeSubstring(variableList[variableIndex2].value.stringValue,"\""));
            strcpy(variableList[variableIndex2].value.stringValue,removeSubstring(variableList[variableIndex2].value.stringValue," "));
            strcpy(variableList[variableIndex].value.stringValue,removeSubstring(prevToken.lexeme, variableList[variableIndex2].value.stringValue));

        } else {
            printf("Type mismatch in variable assignment\n");
        }

    } else if (prevToken.type == TOKEN_IDENTIFIER && nextToken.type == TOKEN_IDENTIFIER){
        if (variableList[variableIndex].type == VALUE_INT) {
            variableList[variableIndex].value.intValue = variableList[variableIndex1].value.intValue - variableList[variableIndex2].value.intValue;
        } else {
            strcpy(variableList[variableIndex2].value.stringValue,removeSubstring(variableList[variableIndex2].value.stringValue,"\""));
            strcpy(variableList[variableIndex2].value.stringValue,removeSubstring(variableList[variableIndex2].value.stringValue,"\""));
            strcpy(variableList[variableIndex2].value.stringValue,removeSubstring(variableList[variableIndex2].value.stringValue," "));
            strcpy(variableList[variableIndex].value.stringValue,removeSubstring(variableList[variableIndex1].value.stringValue, variableList[variableIndex2].value.stringValue));

        }
    } else{
        printf("Error: \n");
    }

}

void processAdditionorSubtraction(Token prevToken, Token token, FILE* inputFile, int variableIndex, char* variableName){//eklemeye veya toplamaya yolluyor
    if (strcmp(token.lexeme, "+") == 0){
        processAddition(prevToken,token,inputFile, variableIndex, variableName);
    } else if(strcmp(token.lexeme, "-") == 0){
        processSubtraction(prevToken,token,inputFile,variableIndex, variableName);
    }
}

void parseOutput(FILE* inputFile, Token token) { //çıktıyı ekrana yazdırmak için

    if (token.type == TOKEN_KEYWORD && strcmp(token.lexeme, "output") == 0) {
        token = getNextToken(inputFile);
        if (token.type == TOKEN_IDENTIFIER) {
            int variableIndex = -1;
            for (int i = 0; i < variableCount; i++) {
                if (strcmp(variableList[i].name, token.lexeme) == 0) {
                    variableIndex = i;
                    break;
                }
            }

            if (variableIndex != -1) {
                if (variableList[variableIndex].type == VALUE_INT) {
                    printf("Output of %s: %d\n",variableList[variableIndex].name, variableList[variableIndex].value.intValue);
                } else if (variableList[variableIndex].type == VALUE_STRING) {
                    printf("Output of %s: %s\n", variableList[variableIndex].name, variableList[variableIndex].value.stringValue);
                }
            } else {
                printf("Variable '%s' not found\n", token.lexeme);
            }
        } else {
            printf("Syntax error: Expected variable identifier\n");
        }
    } else {
        printf("Syntax error: Expected 'output' keyword\n");
    }
}

char* removeSubstring(char* s, const char* toRemove) {// bir stringten belirli bir stringi çıkaran metod
    char *result = malloc(strlen(s) + 1); // +1 for the null-terminator
    strcpy(result, s);

    size_t len = strlen(toRemove);
    if(len > 0) {
        char* p = result;
        while((p = strstr(p, toRemove)) != NULL) {
            memmove(p, p + len, 1 + strlen(p + len));
            break;  // Remove only the first occurrence
        }
    }
    return result;
}

void interpret(FILE* inputFile) { // ana metod gelen tokenin türüne göre işleme sokan metod
    Token token;
    while ((token = getNextToken(inputFile)).type != TOKEN_END_OF_FILE) {
        if (token.type == TOKEN_KEYWORD && strcmp(token.lexeme, "new") == 0) {
            parseDeclaration(inputFile,token);
        } else if (token.type == TOKEN_IDENTIFIER) {
            //Token nextToken = getNextToken(inputFile);
            parseAssignment(inputFile, token);
        } else if (token.type == TOKEN_KEYWORD) {
            if (strcmp(token.lexeme, "output") == 0) {
                parseOutput(inputFile, token);
                token = getNextToken(inputFile);
            }
            else if (strcmp(token.lexeme, "read") == 0){
                executeRead(inputFile);
                token = getNextToken(inputFile);
            } else if (strcmp(token.lexeme, "write") == 0){
                executeWrite(inputFile);
                token = getNextToken(inputFile);

            } else if (strcmp(token.lexeme, "input") == 0){
                executeInput(inputFile);
                token = getNextToken(inputFile);
            } else if (strcmp(token.lexeme, "insert") == 0){
                token = getNextToken(inputFile);
                processInsert(inputFile);
                token = getNextToken(inputFile);
            } else if (strcmp(token.lexeme, "override") == 0){
                token = getNextToken(inputFile);
                processOverride(inputFile);
                token = getNextToken(inputFile); token = getNextToken(inputFile);
            }

        } else if(token.type == TOKEN_END_OF_LINE){
            return;
        }
        else{
            printf("Syntax error: Invalid statement\n");
            exit(0);
            return;
        }
    }
}

Token getNextToken(FILE* inputFile) { //sonraki tokeni almamızı sağlayan metod
    //bu metodu çok kez kullandım her zaman yorum eklemedim ama tokenleri almak veya atlamak için kullanıldı.
    Token token;
    token.type = TOKEN_INVALID;
    memset(token.lexeme, 0, sizeof(token.lexeme));

    int currentChar;
    while ((currentChar = fgetc(inputFile)) != EOF) {
        int prevChar = currentChar;
        if (isspace(currentChar)) {
            continue;
        } else if(currentChar == '(' || currentChar == ')') {
            if (currentChar == '('){
                token.type = TOKEN_LEFTPAR;
                strcpy(token.lexeme, "(");
            }
            else{
                token.type = TOKEN_RIGHTPAR;
                strcpy(token.lexeme, ")");
            }
            return token;
        } else if(currentChar == ',') {
            token.type = TOKEN_COMMA;
            strcpy(token.lexeme,",");
            return token;
        } else if (isalpha(currentChar) || currentChar == '_') {
            fseek(inputFile, -1, SEEK_CUR);
            token = processIdentifier(inputFile);
            token.isVariable = true;
            return token;
        } else if (isdigit(currentChar)) {
            fseek(inputFile, -1, SEEK_CUR);
            token = processIntConst(inputFile);
            return token;
        } else if (currentChar == '/') {
            int nextChar = fgetc(inputFile);
            if (nextChar == '/') {
                while ((currentChar = fgetc(inputFile)) != '\n' && currentChar != EOF);
                continue;
            } else if (nextChar == '*') {
                bool isCommentClosed = false;
                while ((currentChar = fgetc(inputFile)) != EOF) {
                    if (currentChar == '*') {
                        if ((currentChar = fgetc(inputFile)) == '/') {
                            isCommentClosed = true;
                            break;
                        }
                    }
                }
                if (!isCommentClosed) {
                    printf("Lexical error: Comment not terminated before end of file\n");
                    exit(0);
                    break;
                }
                continue;
            } else {
                fseek(inputFile, -1, SEEK_CUR);
                token = processOperator(inputFile);
                return token;
            }
        } else if (currentChar == '"') {
            fseek(inputFile, -1, SEEK_CUR);
            token = processStringConst(inputFile);
            return token;
        } else if (currentChar == ';') {
            token.type = TOKEN_END_OF_LINE;
            sprintf(token.lexeme, ";");
            return token;
        } else if (currentChar == '+' || currentChar == '-') {
            token.type = TOKEN_OPERATOR;
            sprintf(token.lexeme, "%c", currentChar);
            return token;
        } else if (currentChar == ':') {
            int nextChar = fgetc(inputFile);
            if (nextChar == '=') {
                token.type = TOKEN_ASSIGNMENT;
                sprintf(token.lexeme, ":=");
                return token;
            } else {
                fseek(inputFile, -1, SEEK_CUR);
                printf("Lexical error: Invalid operator '%c'\n", currentChar);
                exit(0);
                break;
            }
        } else {
            printf("Lexical error: Invalid character '%c'\n", prevChar);
            exit(0);
            break;
        }
    }
    if (currentChar == EOF) {
        token.type = TOKEN_END_OF_FILE;

    }
    return token;
}


