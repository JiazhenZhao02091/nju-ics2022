#ifndef WATCHPOINT_H
#define WATCHPOINT_H

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
    int NO;
    struct watchpoint *next;

    // TODO: Add more members if necessary

    bool flag; // use / unuse
    char expr[100];
    int new_value;
    int old_value;

} WP;

extern WP wp_pool[NR_WP];
static WP *head __attribute__((unused));
static WP *free_ __attribute__((unused));

void init_wp_pool();
WP* new_wp();
void free_wp(WP *wp);


#endif

