#include "cs.h"

static int compare_cities(const void *p1, const void *p2) {
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

static int geonames_load_column(city *c, geonames_column_t col, const char *valbuf, size_t len) {
    char *eptr;
    switch(col) {
    case geonames_id:
        c->id = atoi(valbuf);
        break;
    case geonames_name:
        c->name = strdup(valbuf);
        break;
    case geonames_latitude:
        c->latitude = strtod(valbuf, &eptr);
        break;
    case geonames_longitude:
        c->longitude = strtod(valbuf, &eptr);
        break;
    case geonames_population:
        c->population = strtoll(valbuf, &eptr, 10);
        break;
    case geonames_feature_class:
        c->feature_class = valbuf[0];
        break;
    case geonames_feature_code:
        if(len <= FEATURE_CODE_MAX_LENGTH) {
            memcpy(c->feature_code, valbuf, len);
            c->feature_code[len] = '\0';
        }
        break;
    default:
        // a few columns are ignored.
        break;
    }

    return 0;
}

static city *geonames_read_line(const char *buffer, size_t len) {
    city *c = city_create();

    char valbuf[COLUMN_BUFFER_SIZE];
    int i, j = 0, vlen = 0;

    for(i = 0; i < len; i++) {
        if(buffer[i] != '\t') {
            valbuf[vlen] = buffer[i];
            vlen++;
        } else {
            valbuf[vlen] = '\0';
            geonames_load_column(c, j, valbuf, vlen);
            j++;
            vlen = 0;
        }
    }

    return c;
}

static int geonames_is_valid_feature_code(const char *feature_code) {
    const char *tbl[] = {
        "PPL",
        "PCLI",
        "PPLA",
        "ADM1",
        "PPLL",
        "PPLC",
        "PPLF"
    };

    int i;
    for(i = 0; i < (sizeof(tbl) / sizeof(tbl[0])); i++) {
        if(!strcmp(feature_code, tbl[i]))
            return 1;
    }

    return 0;
}

world *geonames_load_file(const char *filename) {
    FILE *f = fopen(filename, "r");

    char buf[LINE_BUFFER_SIZE];
    char linebuf[LINE_BUFFER_SIZE];
    int num_read, linelen = 0;
    int i = 0, j = 0;

    world *w = world_create();

    while((num_read = fread(buf, 1, sizeof(buf), f))) {
        for(i = 0; i < num_read; i++) {
            if(buf[i] != '\n') {
                linebuf[linelen] = buf[i];
                linelen++;
            } else {
                city *c = geonames_read_line(linebuf, linelen);

                if(geonames_is_valid_feature_code(c->feature_code))
                    world_add_city(w, c);

                j++;
                linelen = 0;
            }
        }
    }

    fclose(f);

    qsort(w->cities, w->length, sizeof(city *), compare_cities);

    return w;
}