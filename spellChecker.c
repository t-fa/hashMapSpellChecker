#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * @param file
 * @return Allocated string or NULL.
 */
char* nextWord(FILE* file)
{
    int maxLength = 16;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
            }
            word[length] = c;
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}

/**
 * Loads the contents of the file into the hash map.
 * @param file
 * @param map
 */
void loadDictionary(FILE* file, HashMap* map)
{
    assert(file != 0);
    assert(map != 0);
    char* word = nextWord(file);
    while(word != NULL){
        hashMapPut(map, word, 1);
        free(word);
        word = nextWord(file);
    }
}

// with assistance from:
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C
#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))

int levenshtein(char *s1, char *s2) {
    unsigned int s1len, s2len, x, y, lastdiag, olddiag;
    s1len = strlen(s1);
    s2len = strlen(s2);
    unsigned int column[s1len+1];
    for (y = 1; y <= s1len; y++)
        column[y] = y;
    for (x = 1; x <= s2len; x++) {
        column[0] = x;
        for (y = 1, lastdiag = x-1; y <= s1len; y++) {
            olddiag = column[y];
            column[y] = MIN3(column[y] + 1, column[y-1] + 1, lastdiag + (s1[y-1] == s2[x-1] ? 0 : 1));
            lastdiag = olddiag;
        }
    }
    return(column[s1len]);
}

// updates all values in dictionary compared to levenshtein distance of input word
void updateValues(HashMap* map, char* inputWord){
    assert(map != 0);
    assert(inputWord != 0);
    for(int i=0; i < map->capacity; i++){
        HashLink *temp = map->table[i];
        while(temp != NULL){
            int value = levenshtein(temp->key, inputWord);
            hashMapPut(map, temp->key, value);
            temp = temp->next;
        }
    }
}

struct suggestedArray
{
    char word[5][100];
    int size;
};

// searches through hashmap for words with the lowest levenshtein distance and adds them to
// a struct suggestedArray. returns that struct
struct suggestedArray *suggestions(HashMap* map){
    assert(map != 0);

    struct suggestedArray *suggestedWords = malloc(sizeof(struct suggestedArray));
    assert(suggestedWords != 0);

    suggestedWords->size = 0;

    int lowestValue = 1;
    while(suggestedWords->size < 5){
        for(int i=0; i < map->capacity; i++){
            HashLink *temp = map->table[i];
            while(temp != NULL && suggestedWords->size < 5){
                if(temp->value == lowestValue){
                    strcpy(suggestedWords->word[suggestedWords->size], temp->key);
                    suggestedWords->size++;
                }
                temp = temp->next;
            }
        }
        lowestValue++;
    }
    return suggestedWords;
}


/**
 * Checks the spelling of the word provded by the user. If the word is spelled incorrectly,
 * print the 5 closest words as determined by a metric like the Levenshtein distance.
 * Otherwise, indicate that the provded word is spelled correctly. Use dictionary.txt to
 * create the dictionary.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char** argv)
{
    HashMap* map = hashMapNew(1000);

    FILE* file = fopen("dictionary.txt", "r");
    clock_t timer = clock();
    loadDictionary(file, map);
    timer = clock() - timer;
    printf("Dictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);

    char inputBuffer[256];
    int quit = 0;
    while (!quit)
    {
        printf("Enter a word or \"quit\" to quit: ");
        scanf("%s", inputBuffer);
        
        for(int i=0; i <= strlen(inputBuffer); i++)
        {
            inputBuffer[i] = tolower(inputBuffer[i]);
        }

        // Implement the spell checker code here..
        if(hashMapContainsKey(map, inputBuffer) == 1){
            printf("The inputted word %s is spelled correctly.\n", inputBuffer);
        } else {
            printf("The inputted word %s is spelled incorrectly.\n", inputBuffer);
            printf("Did you mean:\n");

            updateValues(map, inputBuffer);
            struct suggestedArray *array = suggestions(map);
            for(int i=0; i < array->size; i++){
                printf("%s\n", array->word[i]);
            }
            free(array);
        }

        if (strcmp(inputBuffer, "quit") == 0)
        {
            quit = 1;
        }
    }

    hashMapDelete(map);
    return 0;
}
