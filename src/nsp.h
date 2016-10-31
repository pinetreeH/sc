//
// Created by pinetree on 16-10-29.
//

#ifndef SC_NAMESPACE_NSP_H
#define SC_NAMESPACE_NSP_H

struct nsp;
struct room;
struct nsp_iterator;

extern struct nsp *nsp_new(void);

extern int nsp_add_room(struct nsp *n, struct room *r);

extern int nsp_del_room(struct nsp *n, struct room *r);

extern struct room *nsp_get_room(struct nsp *n, const char *room_name);

extern int nsp_del(struct nsp *n);

// iterate all rooms in namespace
extern struct nsp_iterator *nsp_iterator_new(struct nsp *n);

extern int nsp_iterator_del(struct nsp_iterator *it);

extern int nsp_valid_iterator(struct nsp_iterator *it);

extern int nsp_next_room(struct nsp *n,
                         struct nsp_iterator **it,
                         struct room **r);

#endif
