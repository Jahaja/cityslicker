#include "cs.h"
#include "geonames.h"

city *city_create(void) {
    city *c = malloc(sizeof(city));
    memset(c, 0, sizeof(city));
    return c;
}

int city_destroy(city *c) {
    free(c->name);
    free(c);
    return 0;
}

world *world_create(void) {
    world *w = malloc(sizeof(world));
    w->cities = malloc(sizeof(city *) * WORLD_MIN_SIZE);
    w->size = WORLD_MIN_SIZE;
    w->length = 0;
    return w;
}

int world_destroy(world *w) {
    int i;
    for (i = 0; i < w->length; i++) {
        city_destroy(w->cities[i]);
    }
    free(w->cities);
    free(w);
    return 0;
}

int world_add_city(world *w, city *c) {
    if(w->length >= w->size) {
        int newsize = w->size * WORLD_RESIZE_FACTOR;
        printf("Resizing to %d\n", newsize);
        city **tc = realloc(w->cities, sizeof(city *) * newsize);
        if(!tc) {
            // handle memory error
            return -1;
        }

        w->cities = tc;
        w->size *= WORLD_RESIZE_FACTOR;
    }

    w->cities[w->length++] = c;

    return w->length;
}

int world_get_city_index_by_latitude(world *w, double latitude) {
    int mid = -1, min = 0;
    int max = w->length - 1;
    while(max > min) {
        int mid = min + ((max - min) / 2);
        city *c = w->cities[mid];

        if(c->latitude < latitude) {
            min = mid + 1;
        } else if (c->latitude > latitude) {
            max = mid - 1;
        } else {
            return mid;
        }
    }

    // always return the closest.
    return mid;
}

int compare_cities(const void *p1, const void *p2) {
    const city *c1 = *(city * const *) p1;
    const city *c2 = *(city * const *) p2;

    if(c1->latitude > c2->latitude) {
        return 1;
    } else if (c1->latitude < c2->latitude) {
        return -1;
    } else {
        return 0;
    }
}

long long ustime(void) {
    struct timeval tv;
    long long ust;

    gettimeofday(&tv, NULL);
    ust = ((long long) tv.tv_sec) * 1000000;
    ust += tv.tv_usec;
    return ust;
}

long long mstime(void) {
    return ustime() / 1000;
}

int main() {
    printf("City slicker, eh?\n");

    world *w = geonames_load_file("../data/SE.txt");
    printf("Cities in the world: %d\n", w->length);
    printf("The size of the world: %d\n", w->size);

    printf("Sorting...\n");
    qsort(w->cities, w->length, sizeof(city *), compare_cities);
    printf("Done.\n");

    int i;
    for(i = 0; i < 20; i++) {
        city *c = w->cities[i];
        printf("%d %s %.3f\n", c->id, c->name, c->latitude);
    }

    double minlat = 59.8585;
    double maxlat = 61.2545;
    int city_index;
    city *found;
    
    city_index = world_get_city_index_by_latitude(w, minlat);
    found = w->cities[city_index];
    printf("Index for %f: %d, %d - %s\n", minlat, city_index, found->id, found->name);

    city_index = world_get_city_index_by_latitude(w, maxlat);
    printf("Index: %d\n", city_index);
    found = w->cities[city_index];
    printf("Index for %f: %d, %d - %s\n", maxlat, city_index, found->id, found->name);

    long long start = ustime();
    int num_lookups = 10000;
    for(i = 0; i < num_lookups; i++) {
        world_get_city_index_by_latitude(w, minlat);
    }
    long long elapsed = ustime() - start;

    printf("Time elapsed for %d latitude lookups %lld us\n", num_lookups, elapsed);

    world_destroy(w);

    return 0;
}