/*
 Copyright (c) 1997, 1998 Carnegie Mellon University.  All Rights
 Reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
 this list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 3. The name of the author may not be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The AODV code developed by the CMU/MONARCH group was optimized and tuned by Samir Das and Mahesh Marina, University of Cincinnati. The work was partially done in Sun Microsystems.
 */

#include "aodvetx_rtable.h"

/*
 The Routing Table
 */

aodvetx_rt_entry::aodvetx_rt_entry() {
    int i;

    rt_req_timeout = 0.0;
    rt_req_cnt = 0;

    rt_dst = 0;
    rt_seqno = 0;
    rt_hops = rt_last_hop_count = INFINITY2;
    rt_etx = rt_last_etx = INFINITY2;
    rt_nexthop = 0;
    LIST_INIT(&rt_pclist);
    rt_expire = 0.0;
    rt_flags = RTF_DOWN;

    /*
     rt_errors = 0;
     rt_error_time = 0.0;
     */

    for (i = 0; i < MAX_HISTORY; i++) {
        rt_disc_latency[i] = 0.0;
    }
    hist_indx = 0;
    rt_req_last_ttl = 0;

    LIST_INIT(&rt_nblist);

}

aodvetx_rt_entry::~aodvetx_rt_entry() {
    AODVETX_Neighbor *nb;
    while ((nb = rt_nblist.lh_first)) {
        LIST_REMOVE(nb, nb_link);
        delete nb;
    }

    AODVETX_Precursor *pc;
    while ((pc = rt_pclist.lh_first)) {
        LIST_REMOVE(pc, pc_link);
        delete pc;
    }
}

void aodvetx_rt_entry::nb_insert(nsaddr_t id) {
    AODVETX_Neighbor *nb = new AODVETX_Neighbor(id);
    assert(nb);

    nb->nb_expire = 0;
    LIST_INSERT_HEAD(&rt_nblist, nb, nb_link);
}

AODVETX_Neighbor*
aodvetx_rt_entry::nb_lookup(nsaddr_t id) {
    AODVETX_Neighbor *nb = rt_nblist.lh_first;
    for (; nb; nb = nb->nb_link.le_next) {
        if (nb->nb_addr == id)
            break;
    }

    return nb;
}

void aodvetx_rt_entry::pc_insert(nsaddr_t id) {
    if (pc_lookup(id) == NULL) {
        AODVETX_Precursor *pc = new AODVETX_Precursor(id);
        assert(pc);

        LIST_INSERT_HEAD(&rt_pclist, pc, pc_link);
    }
}

AODVETX_Precursor*
aodvetx_rt_entry::pc_lookup(nsaddr_t id) {
    AODVETX_Precursor *pc = rt_pclist.lh_first;
    for (; pc; pc = pc->pc_link.le_next) {
        if (pc->pc_addr == id)
            return pc;
    }

    return NULL;
}

void aodvetx_rt_entry::pc_delete(nsaddr_t id) {
    AODVETX_Precursor *pc = rt_pclist.lh_first;
    for (; pc; pc = pc->pc_link.le_next) {
        if (pc->pc_addr == id) {
            LIST_REMOVE(pc, pc_link);
            delete pc;
            break;
        }
    }
}

void aodvetx_rt_entry::pc_delete(void) {
    AODVETX_Precursor *pc;
    while ((pc = rt_pclist.lh_first)) {
        LIST_REMOVE(pc, pc_link);
        delete pc;
    }
}

bool aodvetx_rt_entry::pc_empty(void) {
    AODVETX_Precursor *pc;
    if ((pc = rt_pclist.lh_first))
        return false;
    else
        return true;
}

/*
 The Routing Table
 */

aodvetx_rt_entry* aodvetx_rtable::rt_lookup(nsaddr_t id) {
    aodvetx_rt_entry *rt = rthead.lh_first;
    for (; rt; rt = rt->rt_link.le_next) {
        if (rt->rt_dst == id)
            break;
    }
    return rt;
}

void aodvetx_rtable::rt_delete(nsaddr_t id) {
    aodvetx_rt_entry *rt = rt_lookup(id);
    if (rt) {
        LIST_REMOVE(rt, rt_link);
        delete rt;
    }
}

aodvetx_rt_entry* aodvetx_rtable::rt_add(nsaddr_t id) {
    assert(rt_lookup(id) == 0);

    aodvetx_rt_entry *rt = new aodvetx_rt_entry;
    assert(rt);

    rt->rt_dst = id;
    LIST_INSERT_HEAD(&rthead, rt, rt_link);
    return rt;
}
