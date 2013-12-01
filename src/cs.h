#ifndef CS_H
#define CS_H

#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <errno.h>
#include <limits.h>

#define LINE_BUF_SIZE 1024*20
#define COLUMN_BUF_SIZE 1024*20
#define WORLD_MIN_SIZE 10000
#define WORLD_RESIZE_FACTOR 1.5
#define FEATURE_CODE_MAX_LENGTH 10

typedef enum {
    geonames_id,
    geonames_name,
    geonames_asciiname,
    geonames_altnames,
    geonames_latitude,
    geonames_longitude,
    geonames_feature_class,
    geonames_feature_code,
    geonames_country_code,
    geonames_cc2,
    geonames_admin_code_1,
    geonames_admin_code_2,
    geonames_admin_code_3,
    geonames_admin_code_4,
    geonames_population,
    geonames_evelation,
    geonames_dem,
    geonames_timezone,
    geonames_last_mod
} geonames_column_t;

typedef struct city {
    int id;
    char *name;
    double latitude;
    double longitude;
    long long population;
    char feature_code[FEATURE_CODE_MAX_LENGTH + 1];
} city;

typedef struct world {
    int length;
    int size; // allocated size
    city **cities;
} world;

city *city_create();
int city_destroy(city *c);
world *world_create();
int world_destroy(world *w);
int world_add_city(world *w, city *c);
int world_get_city_index_by_latitude(world *w, double latitude);
world *world_get_cities_in_bounding_box(world *w, double minlat, double maxlat, double minlng, double maxlng);

extern world *loaded_world;

#endif