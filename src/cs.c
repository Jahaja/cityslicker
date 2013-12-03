/*
* cityslicker by Joakim Hamren
* 
* To the extent possible under law, the person who associated CC0 with
* cityslicker has waived all copyright and related or neighboring rights
* to cityslicker.
*
* You should have received a copy of the CC0 legalcode along with this
* work. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include "cs.h"
#include "geonames.h"
#include "net.h"
#include "util.h"

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

city *city_create(void) {
    city *c = malloc(sizeof(city));
    if(!c) return NULL;
    memset(c, 0, sizeof(city));
    return c;
}

int city_destroy(city *c) {
    if(!c) return -1;
    free(c->name);
    free(c);
    return 0;
}

world *world_create(void) {
    world *w = malloc(sizeof(world));
    if(!w) goto err;

    w->cities = malloc(sizeof(city *) * WORLD_MIN_SIZE);
    if(!w->cities) goto err;

    w->size = WORLD_MIN_SIZE;
    w->length = 0;
    return w;
err:
    if(w) free(w);
    return NULL;
}

int world_destroy(world *w) {
    if(!w) return -1;
    int i;
    for (i = 0; i < w->length; i++) {
        city_destroy(w->cities[i]);
    }
    free(w->cities);
    free(w);
    return 0;
}

int world_add_city(world *w, city *c) {
    if(!w || !c) return -1;

    if(w->length >= w->size) {
        int newsize = (int) (w->size * WORLD_RESIZE_FACTOR);
        city **tc = realloc(w->cities, sizeof(city *) * newsize);
        if(!tc) return -1;

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


int main(const int argc, const char *argv[]) {
    log_info("City Slicker version: 0.1.0\n");

    if(argc < 2) {
        log_fatal("Invalid number of arguments, database file must be specified.\n");
    }

    const char *db = argv[1];

    int port;
    if(argc > 2) {
        port = atoi(argv[2]);
        if(port < 1 || port > USHRT_MAX) {
            log_fatal("Invalid port number: %s\n", argv[2]);
        }
    } else {
        port = 8082;
    }

    log_info("Loading geonames file...\n");
    int start = mstime();
    loaded_world = geonames_load_file(db);
    if(!loaded_world) {
        log_fatal("Could not load geonames database (%s).\n", strerror(errno));
    }
    int elapsed = mstime() - start;
    log_info("Geonames file loaded in %.2f seconds.\n", elapsed / 1000.0);
    log_info("Cities loaded: %d (%d slots allocated)\n", loaded_world->length, loaded_world->size);
    
    log_info("Starting TCP server on port %d\n", port);
    net_server *s = net_server_start(port);
    if(!s) {
        log_fatal("Failed to start TCP server: %s\n", strerror(errno));
    }
    
    for (;;) {
        net_poll(s);
    }

    net_server_close(s);
    world_destroy(loaded_world);

    return 0;
}