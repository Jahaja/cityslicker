#include "cs.h"

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
    default:
        // a few columns are ignored.
        break;
    }

    return 0;
}

static city *geonames_read_line(const char *buffer, size_t len) {
    city *c = city_create();

    char valbuf[BUFFER_SIZE];
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

world *geonames_load_file(const char *filename) {
    FILE *f = fopen(filename, "r");

    char buf[BUFFER_SIZE];
    char linebuf[BUFFER_SIZE];
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
                world_add_city(w, c);
                j++;
                linelen = 0;
            }
        }
    }

    fclose(f);
    return w;
}