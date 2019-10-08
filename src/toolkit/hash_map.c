#include "hash_map.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

hashentry_t * hashentry_init(int key, void * value) {
    hashentry_t * hashentry = malloc(sizeof(hashentry_t));

    hashentry->key = key;
    hashentry->value = value;
    hashentry->next = NULL;

    return hashentry;
}


int hash_map_put(hashmap_t * hashmap, int key, void * value) {
    int index = hashmap->hashcode(hashmap, key);


    hashentry_t ** hashentry = hashmap->hashentry + index;

    while (*hashentry != NULL)
    {
        if ((*hashentry)->key == key) {
            (*hashentry)->value = value;
            return 0;
        } 
        hashentry = &((*hashentry)->next);
    }

    if (hashmap->size >= hashmap->buckets * hashmap->resize_value) {
        hashmap->scale(hashmap);
    }

    hashentry = hashmap->hashentry + index;

    while (*hashentry != NULL)
    {
        hashentry = &(*hashentry)->next;
    }
    
    *hashentry = hashentry_init(key, value);
    hashmap->size++;

    return 0;
};


void * hash_map_get(hashmap_t * hash_map, int key) {
    int index = hash_map->hashcode(hash_map, key);

    hashentry_t ** hashentry = hash_map->hashentry + index;

    while (*hashentry != NULL)
    {
        if ((*hashentry)->key == key) {
            return (*hashentry)->value;
        } 
        hashentry = &((*hashentry)->next);
    }

    return NULL;
};


int hash_map_delete(hashmap_t * hash_map, int key) {
    int index = hash_map->hashcode(hash_map, key);

    hashentry_t ** hashentry = hash_map->hashentry + index;


    while (*hashentry)
    {
        if ((*hashentry)->key == key) {
            hashentry_t * p = *hashentry;
            *hashentry = p->next;
            free(p);

            hash_map->size--;
            
            return 0;
        } 
        hashentry = &((*hashentry)->next);
    }

    return -1;
};

int hashcode(hashmap_t * hash_map, int key) {
    int buckets = hash_map->buckets;
    long code = (long) key;

    return code % hash_map->buckets;
};

int hashentry_isempty(hashmap_t * hashmap) {
    return hashmap->size == 0;
}

int hashentry_set(hashmap_t * hashmap) {
    int p_size = sizeof(hashentry_t *);
    int buckets = p_size * hashmap->buckets;
    hashmap->hashentry = malloc(buckets);
    memset(hashmap->hashentry, 0, buckets);

    return 0;
}

int hash_map_scale(hashmap_t* hashmap) {
    hashentry_t ** hashentry = hashmap->hashentry;
    
    hashentry_set(hashmap);
    int p_size = sizeof(hashentry_t *);
    int buckets = p_size * hashmap->buckets;

    memcpy(hashmap->hashentry, hashentry, buckets);

    free(hashentry);

    return 0;
}

int hash_map_foreach(hashmap_t* hashmap, callback_t callback) {
    for (int i = 0; i < hashmap->buckets; i++)
    {
        hashentry_t * entry = hashmap->hashentry[i];

        while (entry != NULL)
        {
            callback(entry->key, entry->value);
            entry = entry->next;
        }
    }

    return 0;
}

int * hash_map_keys(hashmap_t* hashmap) {
    int * keys = malloc(sizeof(void * ) * hashmap->size);

    int key_i = 0;;

    for (int i = 0; i < hashmap->buckets; i++)
    {
        hashentry_t * entry = hashmap->hashentry[i];

        while (entry != NULL)
        {
            keys[key_i] = entry->key;
            key_i++;
            entry = entry->next;
        }
    }

    return keys;
}

int hash_map_init(hashmap_t * hashmap) {
    hashmap->size = 0;
    hashmap->buckets = 100;
    hashmap->resize_value = 0.6;

    hashentry_set(hashmap);

    hashmap->hashcode = hashcode;
    hashmap->get = hash_map_get;
    hashmap->put = hash_map_put;
    hashmap->del = hash_map_delete;
    hashmap->isempty = hashentry_isempty;
    hashmap->scale = hash_map_scale;
    hashmap->foreach = hash_map_foreach;
    hashmap->keys = hash_map_keys;

    return 0;
};

