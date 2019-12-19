#include "hashMap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

int hashFunction1(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += key[i];
    }
    return r;
}

int hashFunction2(const char* key)
{
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        r += (i + 1) * key[i];
    }
    return r;
}

/**
 * Creates a new hash table link with a copy of the key string.
 * @param key Key string to copy in the link.
 * @param value Value to set in the link.
 * @param next Pointer to set as the link's next.
 * @return Hash table link allocated on the heap.
 */
HashLink* hashLinkNew(const char* key, int value, HashLink* next)
{
    HashLink* link = malloc(sizeof(HashLink));
    link->key = malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(link->key, key);
    link->value = value;
    link->next = next;
    return link;
}

/**
 * Free the allocated memory for a hash table link created with hashLinkNew.
 * @param link
 */
static void hashLinkDelete(HashLink* link)
{
    free(link->key);
    free(link);
}

/**
 * Initializes a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param map
 * @param capacity The number of table buckets.
 */
void hashMapInit(HashMap* map, int capacity)
{
    map->capacity = capacity;
    map->size = 0;
    map->table = malloc(sizeof(HashLink*) * capacity);
    for (int i = 0; i < capacity; i++)
    {
        map->table[i] = NULL;
    }
}

/**
 * Removes all links in the map and frees all allocated memory. You can use
 * hashLinkDelete to free the links.
 * @param map
 */
void hashMapCleanUp(HashMap* map)
{
    assert(map != 0);
    for(int i=0; i < map->capacity; i++){
        HashLink *cur = map->table[i];
        HashLink *next;
        while(cur != NULL){
            next = cur->next;
            hashLinkDelete(cur);
            map->size--;
            cur = next;
        }
    }
    free(map->table);
    map->size = 0;
}

/**
 * Creates a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param capacity The number of buckets.
 * @return The allocated map.
 */
HashMap* hashMapNew(int capacity)
{
    HashMap* map = malloc(sizeof(HashMap));
    hashMapInit(map, capacity);
    return map;
}

/**
 * Removes all links in the map and frees all allocated memory, including the
 * map itself.
 * @param map
 */
void hashMapDelete(HashMap* map)
{
    hashMapCleanUp(map);
    free(map);
}

/**
 * Returns a pointer to the value of the link with the given key  and skip traversing as well. Returns NULL
 * if no link with that key is in the table.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return Link value or NULL if no matching link.
 */
int* hashMapGet(HashMap* map, const char* key)
{
    assert(map != 0);
    assert(key != 0);
    int index = HASH_FUNCTION(key) % map->capacity;
    if(index < 0){
        index += map->capacity;
    }
    HashLink *temp = map->table[index];
    while(temp != NULL){
        if(strcmp(temp->key, key) == 0){
            return &temp->value;
        }
        temp = temp->next;
    }
    return NULL;
}

/**
 * Resizes the hash table to have a number of buckets equal to the given 
 * capacity (double of the old capacity). After allocating the new table, 
 * all of the links need to rehashed into it because the capacity has changed.
 * 
 * Remember to free the old table and any old links if you use hashMapPut to
 * rehash them.
 * 
 * @param map
 * @param capacity The new number of buckets.
 */
void resizeTable(HashMap* map, int capacity)
{
    assert(map != 0);
    assert(capacity > 0);
    HashMap *newMap = hashMapNew(capacity);
    assert(newMap != 0);
    HashLink *temp;
    for(int i=0; i < map->capacity; i++){
        temp = map->table[i];
        while(temp != NULL){
            hashMapPut(newMap, temp->key, temp->value);
            temp = temp->next;
        }
    }
    hashMapCleanUp(map);
    map->table = newMap->table;
    map->size = newMap->size;
    map->capacity = newMap->capacity;
    free(newMap);
    newMap = 0;
}

/**
 * Updates the given key-value pair in the hash table. If a link with the given
 * key already exists, this will just update the value and skip traversing. Otherwise, it will
 * create a new link with the given key and value and add it to the table
 * bucket's linked list. You can use hashLinkNew to create the link.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket.
 * 
 * @param map
 * @param key
 * @param value
 */
void hashMapPut(HashMap* map, const char* key, int value)
{
    assert(map != 0);
    assert(key != 0);
    int index = HASH_FUNCTION(key) % map->capacity;
    if(index < 0){
        index += map->capacity;
    }
    HashLink *temp = map->table[index];
    // bucket is empty - create new link
    if(temp == NULL){
        HashLink *new = hashLinkNew(key, value, map->table[index]);
        map->table[index] = new;
        map->size++;
    } else if(hashMapContainsKey(map, key) == 1){
        // bucket not empty, key found, update value
        while(temp != NULL){
            // key found
            if(strcmp(temp->key, key) == 0){
                temp->value = value;
                break;
            }
            temp = temp->next;
        }
    } else {
        // bucket not empty, key not found, create new link
        HashLink *new = hashLinkNew(key, value, map->table[index]);
        map->table[index] = new;
        map->size++;
    }
    if(hashMapTableLoad(map) >= MAX_TABLE_LOAD){
        resizeTable(map, (map->capacity * 2));
    }
}

/**
 * Removes and frees the link with the given key from the table. If no such link
 * exists, this does nothing. Remember to search the entire linked list at the
 * bucket. You can use hashLinkDelete to free the link.
 * @param map
 * @param key
 */
void hashMapRemove(HashMap* map, const char* key)
{
    assert(map != 0);
    assert(key != 0);
    int index = HASH_FUNCTION(key) % map->capacity;
    if(index < 0){
        index += map->capacity;
    }
    HashLink *cur = map->table[index];
    HashLink *prev = 0;
    while(cur != NULL){
        if(strcmp(cur->key, key) == 0){
            // if found at first link
            if(prev == 0){
                map->table[index] = cur->next;
                hashLinkDelete(cur);
                map->size--;
                break;
            // value found in middle
            } else {
                prev->next = cur->next;
                hashLinkDelete(cur);
                map->size--;
                break;
            }
        }
        prev = cur;
        cur = cur->next;
    }
}

/**
 * Returns 1 if a link with the given key is in the table and 0 otherwise.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return 1 if the key is found, 0 otherwise.
 */
int hashMapContainsKey(HashMap* map, const char* key)
{
    assert(map != 0);
    assert(key != 0);
    int index = HASH_FUNCTION(key) % map->capacity;
    if(index < 0){
        index += map->capacity;
    }
    HashLink *temp = map->table[index];
    while(temp != NULL){
        // value found
        if(strcmp(temp->key, key) == 0){
            return 1;
        }
        temp = temp->next;
    }
    return 0;
}

/**
 * Returns the number of links in the table.
 * @param map
 * @return Number of links in the table.
 */
int hashMapSize(HashMap* map)
{
    assert(map != 0);
    return map->size;
}

/**
 * Returns the number of buckets in the table.
 * @param map
 * @return Number of buckets in the table.
 */
int hashMapCapacity(HashMap* map)
{
    assert(map != 0);
    return map->capacity;
}

/**
 * Returns the number of table buckets without any links.
 * @param map
 * @return Number of empty buckets.
 */
int hashMapEmptyBuckets(HashMap* map)
{
    assert(map != 0);
    int buckets = 0;
    for(int i=0; i < map->capacity; i++){
        if(map->table[i] == NULL){
            buckets++;
        }
    }
    return buckets;
}

/**
 * Returns the ratio of (number of links) / (number of buckets) in the table.
 * Remember that the buckets are linked lists, so this ratio tells you nothing
 * about the number of empty buckets. Remember also that the load is a floating
 * point number, so don't do integer division.
 * @param map
 * @return Table load.
 */
float hashMapTableLoad(HashMap* map)
{
    assert(map != 0);
    float ratio = (float)map->size / (float)map->capacity;
    return ratio;
}

/**
 * Prints all the links in each of the buckets in the table.
 * @param map
 */
void hashMapPrint(HashMap* map)
{
    assert(map != 0);
    for(int i=0; i < map->capacity; i++){
        printf("[%d] ", i);
        HashLink *temp = map->table[i];
        while(temp != NULL){
            printf("[%s: %d] ", temp->key, temp->value);
            temp = temp->next;
        }
        if(temp == NULL){
                printf("\n");
        }
    }
}
