#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
//05210000237_05210000233
#define MAX_IDENTIFIER_SIZE 30 //constant sabitler
#define MAX_INT_CONST_SIZE 10

enum TokenType { //token olarak ekleyeceğimiz karakterler veya karakter dizilerinin türleri
    Identifier,
    IntConst,
    Operator,
    LeftPar,
    RightPar,
    LeftSquareBracket,
    RightSquareBracket,
    LeftCurlyBracket,
    RightCurlyBracket,
    StringConst,
    Keyword,
    EndOfLine,
};

const char* tokenNames[] = {
        "Identifier",
        "IntConst",
        "Operator",
        "LeftPar",
        "RightPar",
        "LeftSquareBracket",
        "RightSquareBracket",
        "LeftCurlyBracket",
        "RightCurlyBracket",
        "StringConst",
        "Keyword",
        "EndOfLine"
};

void addToken(enum TokenType type, const char* input, FILE* outputFile) {
    if (type == Identifier || type == IntConst || type == Operator || type == StringConst || type == Keyword){
        fprintf(outputFile, "%s(%s)\n", tokenNames[type], input); } //burada lexe yazdirirken parantezin içinde inputun olması gereken tokenler yaziliyor IntConst(20) gibi
    else{
        fprintf(outputFile, "%s\n", tokenNames[type]);//burada lexe yazdirirken paranteze gerek olmayan tokenler yaziliyor EndOfLine gibi
    }
}

int isKeyword(const char* input) {//daha sonra kullanilmak üzere gelen inputun keyword dizimizde olup olmadığını kontrol eden fonksiyon
    const char* keywords[] = {
            "break", "case", "char", "const", "continue", "do", "else", "enum", "float", "for", "goto", "if",
            "int", "long", "record", "return", "static", "while"
    };
    int numKeywords = sizeof(keywords) / sizeof(keywords[0]); //keywords dizisindeki eleman sayisini hesaplama islemi

    for (int i = 0; i < numKeywords; i++) {
        if (strcasecmp(input, keywords[i]) == 0) { //girdiyle dizi üyelerini kiyasliyor
            return 1;
        }
    }
    return 0;
}

void processIdentifier(FILE* inputFile, FILE* outputFile) { //identifierları token olarak eklediğimiz metot
    char input[MAX_IDENTIFIER_SIZE + 1];
    int inputIndex = 0;
    int currentChar;

    while (((currentChar = fgetc(inputFile)) != EOF) && ((isalnum(currentChar) || currentChar == '_') )) {//dosya sonunda değilsek ve karakter,sayı veya underscore ise
        input[inputIndex++] = (char)tolower(currentChar);//characterleri sıra sıra küçülterek diziye ekliyor
    }

    input[inputIndex] = '\0';
    fseek(inputFile, -1, SEEK_CUR);
    if (isKeyword(input)) {//keywordse keyword olarak ekliyor iskeyword methodu yukarda oluşturduğumuz
        addToken(Keyword, input, outputFile);
    }
    else if (input[0] == '_' || isdigit(input[0])){// //başlangıcı alfabetik karakter değil ise durumuna bakiyor
        printf("Lexical error: Identifiers must begin with an alphabetic character\n");
    }
    else {
        if (strlen(input) > 30){ //uzunluğunun 30u gecip gecmedigine bakiyor
            printf("Lexical error: Identifiers can't be larger than 30 characters\n");
        }
        else{
            addToken(Identifier, input, outputFile);
        }
    }
}

void processIntConst(FILE* inputFile, FILE* outputFile) { //tam sayı sabitleri token olarak eklediğimiz metot
    char input[MAX_INT_CONST_SIZE + 1];
    int inputIndex = 0;
    int currentChar;
    while (((currentChar = fgetc(inputFile)) != EOF) && isdigit(currentChar)) { //sıradaki karakteri alip sayi mi diye kontrol eden döngü
        input[inputIndex++] = (char)currentChar; //input dizisine ekliyor karakteri
    }

    input[inputIndex] = '\0';
    fseek(inputFile, -1, SEEK_CUR);//fseek dosya akışına ait dosya konum göstergesini origin parametre değerinde yer alan değere göre offset parametre değeri ile gösterilen değere ayarlar.
    if (strtoll(input,0,10) <= 10000000000) {//burda inputu long long türüne çevirip en küçük 11 basamaklı sayıdan küçükse diye kontrol ediyor psi++ max 10 basamakli destekledigi icin
        addToken(IntConst, input, outputFile);
    }
    else{
        printf("Lexical error: Integer constants can't be more than 10 digits\n");
    }
}

void processOperator(FILE* inputFile, FILE* outputFile) {//operator tokenlerini ekledigimiz metot
    int currentChar = fgetc(inputFile);//siradaki karakteri dosyadan aliyor
    //tüm operatörleri karakterleri ve gerektiğinde kendisinden sonraki karaktere bakarak algılayıp token olarak ekliyor + ise ++ mı diye kontrol ediyor mesela
    if (currentChar == '+') {
        int nextChar = fgetc(inputFile);//bi sonraki karakteri aliyor
        if (nextChar == '+') {
            addToken(Operator, "++", outputFile);
        } else {
            fseek(inputFile, -1, SEEK_CUR);
            addToken(Operator, "+", outputFile);
        }
    } else if (currentChar == '-') {
        int nextChar = fgetc(inputFile);if (nextChar == '-') {
            addToken(Operator, "--", outputFile);
        } else {

            fseek(inputFile, -1, SEEK_CUR);
            addToken(Operator, "-", outputFile);
        }
    } else if (currentChar == '*') {
        addToken(Operator, "*", outputFile);
    } else if (currentChar == '/') {
        int nextChar = fgetc(inputFile);
        if (nextChar == '/') {
            fseek(inputFile, -1, SEEK_CUR);
            addToken(Operator, "/", outputFile);
        }
    } else if (currentChar == ':') {
        int nextChar = fgetc(inputFile);
        if (nextChar == '=') {
            addToken(Operator, ":=", outputFile);
        } else {
            fseek(inputFile, -1, SEEK_CUR);
            printf("Lexical error: Invalid operator '%c'\n", currentChar); //eğerki iki noktadan sonra = gelmezse iki nokta anlamsız operatör olacağı için
            return;
        }
    } else {
        // Invalid character //yanlış karakterlere bakıyor burada
        printf("Lexical error: Invalid character '%c'\n", currentChar);
        return;
    }
}

void processStringConst(FILE* inputFile, FILE* outputFile) { //karakter dizilerini token olarak ekleyen metot
    char input[2048]; //C maksimum string byte yaklaşık 2048 diye böyle yazıldı
    int inputIndex = 0;
    int currentChar;
    input[inputIndex++] = '"';
    if (((currentChar = fgetc(inputFile)) == '"')) { //sonraki karakter " ise diye bakiyor
        while ((currentChar = fgetc(inputFile)) != EOF) { //dosya sonu değilse sonraki karakter
            input[inputIndex++] = (char)currentChar; //sonraki karakterleri birleştiriyor
            if (currentChar == '"') {//burası string consttan çıkılacağı zamanı belirtiyor yani bir daha " görmesini ve tokeni eklemesini
                input[inputIndex] = '\0';
                addToken(StringConst, input, outputFile);
                break;
            }
        }
        if (currentChar == EOF) { //eof ise lexical error yani " kapanmamissa bu ife giriyor
            printf("Lexical error: String constant not terminated before end of file\n");
            return;
        }
    }
}

void processAllTokens(FILE* inputFile, FILE* outputFile) { //geri kalan token işlemlerini yapan ve diğer token oluşturma metotlarını çağırdığımız metot
    int currentChar;
    while ((currentChar = fgetc(inputFile)) != EOF) {//sonraki karakter space ise ilerliyor bir şey yapmadan
        int prevChar = currentChar;
        if (isspace(currentChar)) {
            continue;
        } else if (isalpha(currentChar) || currentChar == '_') { //karakterimiz harfse veya underscore ise identifier token oluşturmaya gidiyor
            fseek(inputFile, -1, SEEK_CUR);
            processIdentifier(inputFile, outputFile);
        } else if (isdigit(currentChar)) {//karakterimiz sayiysa intconst oluşturmaya gidiyor
            fseek(inputFile, -1, SEEK_CUR);
            processIntConst(inputFile, outputFile);
        } else if (currentChar == '/' && (currentChar = fgetc(inputFile)) == '*') { // sonraki iki karakterimiz /* ise
            while ((currentChar = fgetc(inputFile)) != EOF) {//end of file değilken ilerliyor döngü
                if (currentChar == '*') {
                    if ((currentChar = fgetc(inputFile)) == '/') { //en son 2 karakterimiz */ ise döngüden çıkıyor commentleri lexe yazdırmadığımız için bir şey yapmiyor
                        break;
                    }
                }
            }
            if (currentChar == EOF) {//eofa kadar comment kısmı kapanmazsa lexical error veriyor
                printf("Lexical error: Comment not terminated before end of file\n");
                return;
            }
        } else if (currentChar == '(') { //parantezleri token olarak ekliyor
            addToken(LeftPar, "", outputFile);
        } else if (currentChar == ')') {
            addToken(RightPar, "", outputFile);
        } else if (currentChar == '[') {
            addToken(LeftSquareBracket, "", outputFile);
        } else if (currentChar == ']') {
            addToken(RightSquareBracket, "", outputFile);
        } else if (currentChar == '{') {
            addToken(LeftCurlyBracket, "", outputFile);
        } else if (currentChar == '}') {
            addToken(RightCurlyBracket, "", outputFile);
        } else if (currentChar == '"') { //string constları token olarak ekleme metodunu çağırıyor ilk karakter " ise
            fseek(inputFile, -1, SEEK_CUR);
            processStringConst(inputFile, outputFile);
        } else if (currentChar == ';') {//end of line tokenini ekliyor
            addToken(EndOfLine, "", outputFile);
        } else if (currentChar == '+' || currentChar == '-' || currentChar == '*' || currentChar == '/' || currentChar == ':') {//operator token ekleme metodunu çağırıyor
            fseek(inputFile, -1, SEEK_CUR);
            processOperator(inputFile, outputFile);
        } else {//diğer türlü de karakter yoktur diyor
            printf("Lexical error: Invalid character '%c'\n",prevChar);

        }
    }
}

int main() {//gerekli dosyaları açıyor read ve write modunda dosya yoksa hata veriyor
    FILE* inputFile = fopen("code.psi", "r");
    FILE* outputFile = fopen("code.lex", "w");
    if (inputFile == NULL) {
        printf("Failed to open input file\n");
    }
    if (outputFile == NULL) {
        printf("Failed to create output file\n");
    }
    processAllTokens(inputFile, outputFile); //tüm tokenleri oluşturma metodumuzu çağırıyor
    //dosyaları kapatıyor
    fclose(inputFile);
    fclose(outputFile);

    printf("Lexical analysis completed\n");//leksikal analiz bitti mesaji veriyor.
}