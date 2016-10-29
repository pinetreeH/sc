//
// Created by pinetree on 16-10-29.
//

#ifndef SC_NAMESPACE_NSP_H
#define SC_NAMESPACE_NSP_H

struct nsp;
struct room;

extern struct nsp *nsp_new(void);

extern int nsp_add_room(struct nsp *nsp, struct room *r);

extern int nsp_del_room(struct nsp *nsp, struct room *r);

extern int nsp_del(struct nsp *nsp);

extern int nsp_set_name(struct nsp *nsp, const char *name);

extern const char *nsp_get_name(struct nsp *nsp);

#endif
