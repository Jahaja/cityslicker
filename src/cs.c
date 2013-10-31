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

int main() {
    printf("City slicker, eh?\n");

    world *w = geonames_load_file("../data/allCountries.txt");
    printf("Cities in the world: %d\n", w->length);
    printf("The size of the world: %d\n", w->size);

    net_server *server = net_start_server("127.0.0.1", 8082);
    for (;;) {
        int cfd;
        net_command *cmd;

        if((cfd = net_get_client(net_server)) {
            if((cmd = net_read_command(cfd))) {
                printf("Got command from fd %d\n", cfd);
                // if(cmd->type == net_command_type_bounding_box) {
                //     double *coords = (double *) cmd->ptr;
                //     world *res = world_get_cities_in_bounding_box(w, coords[0], coords[1], coords[2], coords[3]);
                // }
            }
        }

        net_close_client(cfd);
    }

    long long start = mstime();
    int num_lookups = 10000;
    for(i = 0; i < num_lookups; i++) {
        world_get_cities_in_bounding_box(w, 59.1509666443, 59.6238327026, 17.5449085236, 18.6245002747);
    }
    long long elapsed = mstime() - start;

    printf("Time elapsed for %d lookups: %lld ms (%.2fms per lookup)\n", num_lookups, elapsed, (elapsed / (float) num_lookups));

    char inputbuf[100];
    printf("Inputz pls:\n");
    fgets(inputbuf, sizeof(inputbuf), stdin);

    world_destroy(w);

    return 0;
}