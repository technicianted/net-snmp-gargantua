/*******************************
 *
 *      net-snmp/utils.h
 *
 *      Net-SNMP library - General utilities
 *
 *******************************/

#ifndef _NET_SNMP_UTILS_H
#define _NET_SNMP_UTILS_H


        /* Buffer handling */

#define NETSNMP_BUFFER_RESIZE            0x01
#define NETSNMP_BUFFER_NOFREE            0x02
#define NETSNMP_BUFFER_NULLTERM          0x04
#define NETSNMP_BUFFER_REVERSE           0x08
#define NETSNMP_BUFFER_NOCOPY            0x10

typedef struct netsnmp_buf_s {
    char *string;
    int   cur_len;
    int   max_len;
    int   flags;
} netsnmp_buf;


#include <stdio.h>
#include <net-snmp/struct.h>

netsnmp_buf* buffer_new( char *string, unsigned int len, unsigned int flags);
int          buffer_append(        netsnmp_buf *buf, char *string, int len  );
int          buffer_append_string( netsnmp_buf *buf, char *string           );
int          buffer_append_char(   netsnmp_buf *buf, char  ch               );
int          buffer_append_bufstr( netsnmp_buf *buf, netsnmp_buf *str       );
int          buffer_append_int(    netsnmp_buf *buf, int i                  );
int          buffer_append_oid(    netsnmp_buf *buf, netsnmp_oid *oid       );
char*        buffer_string(        netsnmp_buf *buf                         );
int          buffer_compare(       netsnmp_buf *one, netsnmp_buf *two       );
netsnmp_buf* buffer_copy(          netsnmp_buf *buf                         );
int          buffer_set_string(    netsnmp_buf *buf, char *string, int len  );
void         buffer_free(          netsnmp_buf *buf                         );

        /*
         * The 'buffer_append' calls are frequently used within routines
         *   that similarly return a -ve value to indicate failure.
         * The following "convenience macro" can be used to propogate this
         *   error indication, without detracting from the code readability.
         */
#define __B( x )        if ( x < 0 ) { return -1; }

	/*
	 * Certain compilers complain loudly about literal strings
	 *   being passed as 'char*' parameters.
	 * This is somewhat clunky, but it shuts them up!
	 */
#define buffer_append_string(b,s)	buffer_append_string(b, (char*)s)

char* list_add_token(     char *list, char *token,  char sep );
char* list_remove_token(  char *list, char *token,  char sep );
char* list_remove_tokens( char *list, char *remove, char sep );

#endif /* _NET_SNMP_UTILS_H */
