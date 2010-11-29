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

 The MAODV code developed by the CMU/MONARCH group was optimized and tuned by
 Samir Das and Mahesh Marina, University of Cincinnati. The work was partially
 done in Sun Microsystems.
 */

#ifndef __nsmiracle_aodv_h__
#define __nsmiracle_aodv_h__

#include <cmu-trace.h>
#include <priqueue.h>
#include <classifier/classifier-port.h>

#include "nsmiracle_aodv_rtable.h"
#include "nsmiracle_aodv_rqueue.h"

/*
 Allows local repair of routes
 */
#define MAODV_LOCAL_REPAIR

/*
 Allows MAODV to use link-layer (802.11) feedback in determining when
 links are up/down.
 */
#define MAODV_LINK_LAYER_DETECTION

/*
 Causes MAODV to apply a "smoothing" function to the link layer feedback
 that is generated by 802.11.  In essence, it requires that RT_MAX_ERROR
 errors occurs within a window of RT_MAX_ERROR_TIME before the link
 is considered bad.
 */
#define MAODV_USE_LL_METRIC

/*
 Only applies if MAODV_USE_LL_METRIC is defined.
 Causes MAODV to apply omniscient knowledge to the feedback received
 from 802.11.  This may be flawed, because it does not account for
 congestion.
 */
//#define MAODV_USE_GOD_FEEDBACK


class MAODV;

#define MADOV_ROUTE_TIMEOUT        10                      	// 100 seconds
#define MADOV_ACTIVE_ROUTE_TIMEOUT    10				// 50 seconds
#define MADOV_REV_ROUTE_LIFE          6				// 5  seconds
#define MADOV_BCAST_ID_SAVE           6				// 3 seconds
// No. of times to do network-wide search before timing out for
// MADOV_MAX_RREQ_TIMEOUT sec.
#define MADOV_RREQ_RETRIES            3
// timeout after doing network-wide search MADOV_RREQ_RETRIES times
#define MADOV_MAX_RREQ_TIMEOUT	10.0 //sec
/* Various constants used for the expanding ring search */
#define MADOV_TTL_START     5
#define MADOV_TTL_THRESHOLD 7
#define MADOV_TTL_INCREMENT 2

// This should be somewhat related to arp timeout
#define MADOV_NODE_TRAVERSAL_TIME     0.03             // 30 ms
#define MADOV_LOCAL_REPAIR_WAIT_TIME  0.15 //sec
// Should be set by the user using best guess (conservative)
#define MADOV_NETWORK_DIAMETER        30             // 30 hops
// Must be larger than the time difference between a node propagates a route
// request and gets the route reply back.

//#define MADOV_RREP_WAIT_TIME     (3 * MADOV_NODE_TRAVERSAL_TIME * MADOV_NETWORK_DIAMETER) // ms
//#define MADOV_RREP_WAIT_TIME     (2 * MADOV_REV_ROUTE_LIFE)  // seconds
#define MADOV_RREP_WAIT_TIME         1.0  // sec
#define MADOV_ID_NOT_FOUND    0x00
#define MADOV_ID_FOUND        0x01
//#define INFINITY        0xff

// The followings are used for the forward() function. Controls pacing.
#define MADOV_DELAY 1.0           // random delay
#define MADOV_NO_DELAY -1.0       // no delay
// think it should be 30 ms
#define MADOV_ARP_DELAY 0.01      // fixed delay to keep arp happy
#define MADOV_HELLO_INTERVAL        1               // 1000 ms
#define MADOV_ALLOWED_HELLO_LOSS    3               // packets
#define MADOV_BAD_LINK_LIFETIME     3               // 3000 ms
#define MADOV_MAX_HELLO_INTERVAL    (1.25 * MADOV_HELLO_INTERVAL)
#define MADOV_MIN_HELLO_INTERVAL    (0.75 * MADOV_HELLO_INTERVAL)

/*
 Timers (Broadcast ID, Hello, Neighbor Cache, Route Cache)
 */
class MAODVBroadcastTimer: public Handler {
public:
    MAODVBroadcastTimer(MAODV* a) :
        agent(a) {
    }
    void handle(Event*);
private:
    MAODV *agent;
    Event intr;
};

class MAODVHelloTimer: public Handler {
public:
    MAODVHelloTimer(MAODV* a) :
        agent(a) {
    }
    void handle(Event*);
private:
    MAODV *agent;
    Event intr;
};

class MAODVNeighborTimer: public Handler {
public:
    MAODVNeighborTimer(MAODV* a) :
        agent(a) {
    }
    void handle(Event*);
private:
    MAODV *agent;
    Event intr;
};

class MAODVRouteCacheTimer: public Handler {
public:
    MAODVRouteCacheTimer(MAODV* a) :
        agent(a) {
    }
    void handle(Event*);
private:
    MAODV *agent;
    Event intr;
};

class MAODVLocalRepairTimer: public Handler {
public:
    MAODVLocalRepairTimer(MAODV* a) :
        agent(a) {
    }
    void handle(Event*);
private:
    MAODV *agent;
    Event intr;
};

/*
 Broadcast ID Cache
 */
class MAODVBroadcastID {
    friend class MAODV;
public:
    MAODVBroadcastID(nsaddr_t i, u_int32_t b) {
        src = i;
        id = b;
    }
protected:
    LIST_ENTRY(MAODVBroadcastID) link;
    nsaddr_t src;
    u_int32_t id;
    double expire; // now + MADOV_BCAST_ID_SAVE s
};

LIST_HEAD(maodv_bcache, MAODVBroadcastID);

/*
 The Routing Agent
 */
class MAODV: public Agent {

    /*
     * make some friends first
     */

    friend class maodv_rt_entry;
    friend class MAODVBroadcastTimer;
    friend class MAODVHelloTimer;
    friend class MAODVNeighborTimer;
    friend class MAODVRouteCacheTimer;
    friend class MAODVLocalRepairTimer;

public:
    MAODV(nsaddr_t id);

    void recv(Packet *p, Handler *);

protected:
    int command(int, const char * const *);
    int initialized() {
        return 1 && target_;
    }

    /*
     * Route Table Management
     */
    void rt_resolve(Packet *p);
    void rt_update(maodv_rt_entry *rt, u_int32_t seqnum, u_int16_t metric,
            nsaddr_t nexthop, double expire_time);
    void rt_down(maodv_rt_entry *rt);
    void local_rt_repair(maodv_rt_entry *rt, Packet *p);
public:
    void rt_ll_failed(Packet *p);
    void handle_link_failure(nsaddr_t id);
protected:
    void rt_purge(void);

    void enque(maodv_rt_entry *rt, Packet *p);
    Packet* deque(maodv_rt_entry *rt);

    /*
     * Neighbor Management
     */
    void nb_insert(nsaddr_t id);
    MAODV_Neighbor* nb_lookup(nsaddr_t id);
    void nb_delete(nsaddr_t id);
    void nb_purge(void);

    /*
     * Broadcast ID Management
     */

    void id_insert(nsaddr_t id, u_int32_t bid);
    bool id_lookup(nsaddr_t id, u_int32_t bid);
    void id_purge(void);

    /*
     * Packet TX Routines
     */
    void forward(maodv_rt_entry *rt, Packet *p, double delay);
    void sendHello(void);
    void sendRequest(nsaddr_t dst);

    void sendReply(nsaddr_t ipdst, u_int32_t hop_count, nsaddr_t rpdst,
            u_int32_t rpseq, u_int32_t lifetime, double timestamp);
    void sendError(Packet *p, bool jitter = true);

    /*
     * Packet RX Routines
     */
    void recvMAODV(Packet *p);
    void recvHello(Packet *p);
    void recvRequest(Packet *p);
    void recvReply(Packet *p);
    void recvError(Packet *p);

    /*
     * History management
     */

    double PerHopTime(maodv_rt_entry *rt);

    nsaddr_t index; // IP Address of this node
    u_int32_t seqno; // Sequence Number
    int bid; // Broadcast ID

    maodv_rtable rthead; // routing table
    maodv_ncache nbhead; // Neighbor Cache
    maodv_bcache bihead; // Broadcast ID Cache

    /*
     * Timers
     */
    MAODVBroadcastTimer btimer;
    MAODVHelloTimer htimer;
    MAODVNeighborTimer ntimer;
    MAODVRouteCacheTimer rtimer;
    MAODVLocalRepairTimer lrtimer;

    /*
     * Routing Table
     */
    maodv_rtable rtable;
    /*
     *  A "drop-front" queue used by the routing layer to buffer
     *  packets to which it does not have a route.
     */
    maodv_rqueue rqueue;

    /*
     * A mechanism for logging the contents of the routing
     * table.
     */
    Trace *logtarget;

    /*
     * A pointer to the network interface queue that sits
     * between the "classifier" and the "link layer".
     */
    PriQueue *ifqueue;

    /*
     * Logging stuff
     */
    void log_link_del(nsaddr_t dst);
    void log_link_broke(Packet *p);
    void log_link_kept(nsaddr_t dst);

    /* for passing packets up to agents */
    PortClassifier *dmux_;

};

#endif /* __nsmiracle_aodv_h__ */
