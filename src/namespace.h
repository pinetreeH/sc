//
// Created by pinetree on 16-10-29.
//

#ifndef SC_NAMESPACE_NSP_H
#define SC_NAMESPACE_NSP_H

struct namespace;
struct room;

extern struct namespace *nsp_new(void);

extern int nsp_add_room(struct namespace *nsp, struct room *r);

extern int nsp_del_room(struct namespace *nsp, struct room *r);

extern int nsp_del(struct namespace *nsp);

#endif
