#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_IDENTIFIER_LENGTH 10
#define MAX_INTEGER_LENGTH 8
#define MAX_STRING_LENGTH 256

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_OPERATOR,
    TOKEN_STRING,
    TOKEN_INTEGER,
    TOKEN_END_OF_LINE,
    TOKEN_COMMA,
    TOKEN_LEFT_CURLY_BRACKET,
    TOKEN_RIGHT_CURLY_BRACKET,
    TOKEN_ERROR
} TokenType;

// Represents a token in the code, with type and value
typedef struct {
    TokenType type;
    char value[MAX_STRING_LENGTH + 1];
} Token;

// Global definitions for keywords and operators
const char* keywords[] = {
    "int", "text", "is", "loop", "times", "read", "write", "newLine"
};

const char operators[] = {'+', '-', '*', '/'};

// Checks if the given character is an operator
int is_operator(char ch) {
    return strchr(operators, ch) != NULL;
}

// Checks if the given identifier is a keyword
int is_keyword(const char* identifier) {
    for (int i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(identifier, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Function to remove comments and update is_comment_open flag
int strip_comments(char* line, int* is_comment_open) {
    char* comment_start = strstr(line, "/*");
    char* comment_end = strstr(line, "*/");

    // If a comment is already open and there is no closing, clear the line
    if (*is_comment_open) {
        if (comment_end) {
            memmove(line, comment_end + 2, strlen(comment_end + 2) + 1);
            *is_comment_open = 0;
        } else {
            line[0] = '\'\0';
            return 0;
        }
    }

    // Check new comment start and end in line
    while (comment_start) {
        if (comment_end && comment_end > comment_start) {
            memmove(comment_start, comment_end + 2, strlen(comment_end + 2) + 1);
            comment_start = strstr(comment_start, "/*");
        } else {
            *is_comment_open = 1;
            line[comment_start - line] = '\'\0';
            break;
        }
    }

    return 1;  // Comments are disabled or absent
}

// Handles operator tokens
void handle_operator(FILE* outputFile, char op) {
    fprintf(outputFile, "Operator(%c)\n", op);
}

// Handles end-of-line tokens
void handle_end_of_line(FILE* outputFile) {
    fprintf(outputFile, "EndOfLine\n");
}

// Handles comma tokens
void handle_comma(FILE* outputFile) {
    fprintf(outputFile, "Comma\n");
}

// Handles curly bracket tokens (left or right)
void handle_curly_bracket(FILE* outputFile, char bracket) {
    if (bracket == '{') {
        fprintf(outputFile, "LeftCurlyBracket\n");
    } else if (bracket == '}') {
        fprintf(outputFile, "RightCurlyBracket\n");
    }
}

// Handles identifier tokens, checking if they are keywords or regular identifiers
void handle_identifier(FILE* outputFile, const char* start, int length) {
    char identifier[MAX_IDENTIFIER_LENGTH + 1];
    strncpy(identifier, start, length);
    identifier[length] = '\0';

    if (!isalpha(identifier[0])) {
        fprintf(outputFile, "Error: Invalid identifier.\n");
    } else if (is_keyword(identifier)) {
        fprintf(outputFile, "Keyword(%s)\n", identifier);
    } else {
        fprintf(outputFile, "Identifier(%s)\n", identifier);
    }
}

// Handles integer tokens
void handle_integer(FILE* outputFile, const char* start, int length) {
    char integer[MAX_INTEGER_LENGTH + 1];
    strncpy(integer, start, length);
    integer[length] = '\0';
    fprintf(outputFile, "IntConst(%s)\n", integer);
}

// Handles string tokens, checks for unclosed or invalid strings
void handle_string(FILE* outputFile, const char* start, int length) {
    char string[MAX_STRING_LENGTH + 1];
    strncpy(string, start, length);
    string[length] = '\0';
    fprintf(outputFile, "String(%s)\n", string);
}

// Processes a line of code, identifying and classifying tokens
void process_line(const char* line, FILE* outputFile,int* is_comment_open) {
    int i = 0;
    int len = strlen(line);

    if (*is_comment_open) { // using flag for comments
        char* comment_end = strstr(line, "*/");
        if (comment_end) {
            *is_comment_open = 0;  // Comment closed
            i = comment_end - line + 2;  // Continue processing
        } else {
            return;  // Comment is still open
        }
    }

    while (i < len) {
        while (isspace(line[i])) i++; // Skip whitespace
        if (line[i] == '\0') continue;  // If at the end of the line

        // Handle potential negative integers
        if (line[i] == '-' && isdigit(line[i + 1])) {
            fprintf(outputFile, "Error: Negative integer.\n");
            i++;
            while (isdigit(line[i])) i++; // Skip the rest of the integer
            continue;
        }

         // Handle operator tokens
        if (is_operator(line[i])) {
            handle_operator(outputFile, line[i]);
            i++;
            continue;
        }

        // Handle various other token types
        switch (line[i]) {
            case '.':  // End-of-line token
                handle_end_of_line(outputFile);
                i++;
                break;
            case ',': // Comma token
                handle_comma(outputFile);
                i++;
                break;
            case '{': // Left curly bracket token
            case '}': // Right curly bracket token
                handle_curly_bracket(outputFile, line[i]);
                i++;
                break;
            default:
                // Handle identifiers and keywords
                if (isalpha(line[i])) {
                    int start = i;
                    int length = 0;
                    while (isalnum(line[i]) || line[i] == '_') {
                        length++;
                        if (length > MAX_IDENTIFIER_LENGTH) {
                            fprintf(outputFile, "Error: Identifier too long.\n");
                            while (isalnum(line[i]) || line[i] == '_') {  // Skip the long identifier
                                i++;
                            }
                            break;
                        }
                        i++;
                    }
                    if (length <= MAX_IDENTIFIER_LENGTH) {
                        handle_identifier(outputFile, line + start, length);
                    }
                    continue;
                }

                 // Handle integer tokens
                if (isdigit(line[i])) {
                    int start = i;
                    int length = 0;
                    while (isdigit(line[i])) {
                        length++;
                        if (length > MAX_INTEGER_LENGTH) {
                            fprintf(outputFile, "Error: Integer too long.\n");
                            while (isdigit(line[i])) i++;  // Skip the long integer
                            break;
                        }
                        i++;
                    }
                    if (length <= MAX_INTEGER_LENGTH) {
                        handle_integer(outputFile, line + start, length);
                    }
                    continue;
                }

                // Handle string tokens, check for unclosed or invalid strings
                if (line[i] == '"') {
                    int start = i;
                    int length = 1;
                    i++;
                    int string_too_long = 0;

                    while (line[i] != '"' && i < len) {
                        length++;
                        if (length > MAX_STRING_LENGTH) {
                            string_too_long = 1;
                            break;
                        }
                        i++;
                    }

                    if (string_too_long) {
                        fprintf(outputFile, "Error: String too long.\n");
                        while (line[i] != '\0' && line[i] != '"') i++; // Skip until end of the string or line
                        if (line[i] == '"') i++; // Skip the closing quote if present
                        continue;
                    }

                    if (line[i] == '"') {
                        length++;  // Include the closing quote

                        // Check if the string contains a double quote inside it
                        int isInvalidString = 0;

                        handle_string(outputFile, line + start, length); // Handle the valid string

                        for (int j = i+1; j < strlen(line); j++) {
                            if (line[j] == '"') {
                                isInvalidString = 1;
                                break;
                            }
                            i = j+2; // Move past the string

                        }

                        if (isInvalidString) {
                            fprintf(outputFile, "Error: String contains double quotes.\n");
                            continue;
                        }



                        } else {
                        fprintf(outputFile, "Error: Unclosed string.\n"); // If the string isn't closed
                    }
                    continue;
                }

                fprintf(outputFile, "Error: Unrecognized token.\n"); // If none of the conditions were met


                i++;


                break;
        }
    }
}

// Main lexical analysis function, reads from an input file and writes tokens to an output file
void lexicalAnalyzer(const char* inputFilePath, const char* outputFilePath) {
    FILE* inputFile = fopen(inputFilePath, "r");
    if (!inputFile) {
        fprintf(stderr, "Error: Could not open input file.\n");
        return;
    }

    FILE* outputFile = fopen(outputFilePath, "w");
    if (!outputFile) {
        fclose(inputFile);
        fprintf(stderr, "Error: Could not open output file.\n");
        return;
    }

    int is_comment_open = 0;  // Flag for comments
    char line[1024];
    while (fgets(line, sizeof(line), inputFile)) {
        if (!strip_comments(line, &is_comment_open)) {  // If the comment is not closed, do not continue
            continue;  //If the comment is not closed, check the next line
        }

        // If comment is closed, commit current line
        if (!is_comment_open) {
            process_line(line, outputFile, &is_comment_open);
        }
    }

    // If there is still an open comment at the end of the file, print the error message
    if (is_comment_open) {
        fprintf(outputFile, "Error: Unclosed comment.\n");
    }


    fclose(inputFile);
    fclose(outputFile);
}


int main() {
    lexicalAnalyzer("code.sta", "code.lex");
    return 0;
}
