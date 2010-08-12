/*
 * Definitions and Declarations for tuple.
 *
 * 16 Dec 2003: Yasuyuki Kozakai @USAGI <yasuyuki.kozakai@toshiba.co.jp>
 *	- generalize L3 protocol dependent part.
 *
 * Derived from include/linux/netfiter_ipv4/ip_conntrack_tuple.h
 */

#ifndef _NF_CONNTRACK_TUPLE_H
#define _NF_CONNTRACK_TUPLE_H

#include <linux/netfilter/nf_conntrack_tuple_common.h>

/* A `tuple' is a structure containing the information to uniquely
  identify a connection.  ie. if two packets have the same tuple, they
  are in the same connection; if not, they are not.

  We divide the structure along "manipulatable" and
  "non-manipulatable" lines, for the benefit of the NAT code.
*/

#define NF_CT_TUPLE_L3SIZE	4

/* The l3 protocol-specific manipulable parts of the tuple: always in
   network order! */
union nf_conntrack_address {
	u_int32_t all[NF_CT_TUPLE_L3SIZE];
	__be32 ip;
	__be32 ip6[4];
};

/* The protocol-specific manipulable parts of the tuple: always in
   network order! */
union nf_conntrack_man_proto
{
	/* Add other protocols here. */
	u_int16_t all;

	struct {
		__be16 port;
	} tcp;
	struct {
		__be16 port;
	} udp;
	struct {
		__be16 id;
	} icmp;
	struct {
		__be16 port;
	} sctp;
	struct {
		__be16 key;	/* GRE key is 32bit, PPtP only uses 16bit */
	} gre;
};

/* The manipulable part of the tuple. */
struct nf_conntrack_man
{
	union nf_conntrack_address u3;
	union nf_conntrack_man_proto u;
	/* Layer 3 protocol */
	u_int16_t l3num;
};

/* This contains the information to distinguish a connection. */
struct nf_conntrack_tuple
{
	struct nf_conntrack_man src;

	/* These are the parts of the tuple which are fixed. */
	struct {
		union nf_conntrack_address u3;
		union {
			/* Add other protocols here. */
			u_int16_t all;

			struct {
				__be16 port;
			} tcp;
			struct {
				__be16 port;
			} udp;
			struct {
				u_int8_t type, code;
			} icmp;
			struct {
				__be16 port;
			} sctp;
			struct {
				__be16 key;
			} gre;
		} u;

		/* The protocol. */
		u_int8_t protonum;

		/* The direction (for tuplehash) */
		u_int8_t dir;
	} dst;
};

#ifdef __KERNEL__

#define NF_CT_DUMP_TUPLE(tp)						    \
DEBUGP("tuple %p: %u %u " NIP6_FMT " %hu -> " NIP6_FMT " %hu\n",	    \
	(tp), (tp)->src.l3num, (tp)->dst.protonum,			    \
	NIP6(*(struct in6_addr *)(tp)->src.u3.all), ntohs((tp)->src.u.all), \
	NIP6(*(struct in6_addr *)(tp)->dst.u3.all), ntohs((tp)->dst.u.all))

/* If we're the first tuple, it's the original dir. */
#define NF_CT_DIRECTION(h)						\
	((enum ip_conntrack_dir)(h)->tuple.dst.dir)

//--SZ Angela 09.03 Add for QOS {
struct ip_track 
{
	u_int8_t number;
	u_int8_t large_packet;
	u_int8_t flag;
};
//--SZ angela 09.03 Add for QOS }

/* Connections have two entries in the hash table: one for each way */
struct nf_conntrack_tuple_hash
{
	struct list_head list;

	struct nf_conntrack_tuple tuple;

	struct ip_track track; //--SZ angela 09.03
};

#endif /* __KERNEL__ */

static inline int __nf_ct_tuple_src_equal(const struct nf_conntrack_tuple *t1,
					  const struct nf_conntrack_tuple *t2)
{ 
	return (t1->src.u3.all[0] == t2->src.u3.all[0] &&
		t1->src.u3.all[1] == t2->src.u3.all[1] &&
		t1->src.u3.all[2] == t2->src.u3.all[2] &&
		t1->src.u3.all[3] == t2->src.u3.all[3] &&
		t1->src.u.all == t2->src.u.all &&
		t1->src.l3num == t2->src.l3num);
}

static inline int __nf_ct_tuple_dst_equal(const struct nf_conntrack_tuple *t1,
					  const struct nf_conntrack_tuple *t2)
{
	return (t1->dst.u3.all[0] == t2->dst.u3.all[0] &&
		t1->dst.u3.all[1] == t2->dst.u3.all[1] &&
		t1->dst.u3.all[2] == t2->dst.u3.all[2] &&
		t1->dst.u3.all[3] == t2->dst.u3.all[3] &&
		t1->dst.u.all == t2->dst.u.all &&
		t1->dst.protonum == t2->dst.protonum);
}

static inline int nf_ct_tuple_equal(const struct nf_conntrack_tuple *t1,
				    const struct nf_conntrack_tuple *t2)
{
	return __nf_ct_tuple_src_equal(t1, t2) &&
	       __nf_ct_tuple_dst_equal(t1, t2);
}

static inline int nf_ct_tuple_mask_cmp(const struct nf_conntrack_tuple *t,
				       const struct nf_conntrack_tuple *tuple,
				       const struct nf_conntrack_tuple *mask)
{
	int count = 0;

        for (count = 0; count < NF_CT_TUPLE_L3SIZE; count++){
                if ((t->src.u3.all[count] ^ tuple->src.u3.all[count]) &
                    mask->src.u3.all[count])
                        return 0;
        }

        for (count = 0; count < NF_CT_TUPLE_L3SIZE; count++){
                if ((t->dst.u3.all[count] ^ tuple->dst.u3.all[count]) &
                    mask->dst.u3.all[count])
                        return 0;
        }

        if ((t->src.u.all ^ tuple->src.u.all) & mask->src.u.all ||
            (t->dst.u.all ^ tuple->dst.u.all) & mask->dst.u.all ||
            (t->src.l3num ^ tuple->src.l3num) & mask->src.l3num ||
            (t->dst.protonum ^ tuple->dst.protonum) & mask->dst.protonum)
                return 0;

        return 1;
}

#endif /* _NF_CONNTRACK_TUPLE_H */
