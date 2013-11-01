#include "cs.h"
#include "geonames.h"
#include "net.h"

static int compare_populations(const void *p1, const void *p2) {
    const city *c1 = *(city * const *) p1;
    const city *c2 = *(city * const *) p2;

    if(c1->population > c2->population) {
        return -1;
    } else if (c1->population < c2->population) {
        return 1;
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
    while(max >= min) {
        mid = min + ((max - min) / 2);
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

world *world_get_cities_in_bounding_box(world *w, double minlat, double maxlat, double minlng, double maxlng) {
    int minindex, maxindex, i, j;
    city *c;
    world *bbw;

    minindex = world_get_city_index_by_latitude(w, minlat);
    maxindex = world_get_city_index_by_latitude(w, maxlat);
    bbw = world_create();

    for(i = minindex, j = 0; i <= maxindex; i++) {
        c = w->cities[i];
        if(c->longitude >= minlng && c->longitude <= maxlng) {
            world_add_city(bbw, c);
            j++;
        }
    }

    qsort(bbw->cities, bbw->length, sizeof(city *), compare_populations);

    return bbw;
}

world *loaded_world;

int main() {
    printf("City slicker, eh?\n");

    printf("Loading geonames file...\n");
    int start = mstime();
    loaded_world = geonames_load_file("../data/allCountries.txt");
    int elapsed = mstime() - start;
    printf("Geonames file loaded in %.2f seconds.\n", elapsed / 1000.0);
    printf("Cities in the world: %d\n", loaded_world->length);
    printf("The size of the world: %d\n", loaded_world->size);
    
    int port = 8082;
    printf("Starting server on port %d\n", port);
    net_server *s = net_server_start(port);
    
    for (;;) {
        net_poll(s);
    }

    net_server_close(s);
    world_destroy(loaded_world);

    return 0;
}