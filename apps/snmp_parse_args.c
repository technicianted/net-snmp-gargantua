#include <config.h>

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#include <stdio.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <ctype.h>
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if TIME_WITH_SYS_TIME
# ifdef WIN32
#  include <sys/timeb.h>
# else
#  include <sys/time.h>
# endif
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#if HAVE_WINSOCK_H
#include <winsock.h>
#else
#include <netdb.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#include "asn1.h"
#include "snmp_api.h"
#include "snmp_impl.h"
#include "snmp_client.h"
#include "mib.h"
#include "snmp.h"
#include "keytools.h"

#ifdef USE_V2PARTY_PROTOCOL
#include "party.h"
#include "context.h"
#include "view.h"
#include "acl.h"
#endif /* USE_V2PARTY_PROTOCOL */

#include "snmp_parse_args.h"
#include "version.h"
#include "system.h"

int random_access = 0;

#define USM_AUTH_PROTO_MD5_LEN 10
static oid usmHMACMD5AuthProtocol[]  = { 1,3,6,1,6,3,10,1,1,2 };
#define USM_AUTH_PROTO_SHA_LEN 10
static oid usmHMACSHA1AuthProtocol[] = { 1,3,6,1,6,3,10,1,1,3 };
#define USM_PRIV_PROTO_DES_LEN 10
static oid usmDESPrivProtocol[]      = { 1,3,6,1,6,3,10,1,2,2 };

void usage __P((void));

void
snmp_parse_args_usage(outf)
  FILE *outf;
{
  fprintf(outf,
        "[-v 1|2c|2p|3] [-h] [-d] [-q] [-R] [-D] [-m <MIBS>] [-M <MIDDIRS>] [-p <P>] [-t <T>] [-r <R>] [-c <S> <D>] [-e <E>] [-n <N>] [-u <U>] [-l <L>] [-a <A>] [-A <P>] [-x <X>] [-X <P>] hostname> <community>|{<srcParty> <dstParty> <context>}");
}

void
snmp_parse_args_descriptions(outf)
  FILE *outf;
{
  fprintf(outf, "  -v 1|2c|2p|3\tspecifies snmp version to use.\n");
  fprintf(outf, "  -h\t\tthis help message.\n");
  fprintf(outf, "  -V\t\tdisplay version number.\n");
  fprintf(outf, "  -d\t\tdump input/output packets.\n");
  fprintf(outf, "  -q\t\tquick print output for easier parsing ability.\n");
  fprintf(outf, "  -f\t\tprint full object identifiers on output.\n");
  fprintf(outf, "  -s\t\tprint only last element of object identifiers.\n");
  fprintf(outf, "  -S\t\tmodule id plus last element of object identifiers.\n");
  fprintf(outf, "  -R\t\tuse \"random access\" to the mib tree.\n");
  fprintf(outf, "  -D\t\tturn on debugging output.\n");
  fprintf(outf, "  -m <MIBS>\tuse MIBS list instead of the default mib list.\n");
  fprintf(outf, "  -M <MIBDIRS>\tuse MIBDIRS as the location to look for mibs.\n");
  fprintf(outf, "  -p <P>\tuse port P instead of the default port.\n");
  fprintf(outf, "  -t <T>\tset the request timeout to T.\n");
  fprintf(outf, "  -r <R>\tset the number of retries to R.\n");
  fprintf(outf,
          "  -c <S> <D>\tset the source/destination clocks for v2p requests.\n");
  fprintf(outf, "  -e <E>\tengine ID (e.g., 800000020109840301).\n");
  fprintf(outf, "  -n <N>\tcontext name (e.g., bridge1).\n");
  fprintf(outf, "  -u <U>\tsecurity name (e.g., bert).\n");
  fprintf(outf, "  -l <L>\tsecurity level (noAuthNoPriv|authNoPriv|authPriv).\n");
  fprintf(outf, "  -a <A>\tauthentication protocol (MD5|SHA)\n");
  fprintf(outf, "  -A <P>\tauthentication protocol pass phrase.\n");
  fprintf(outf, "  -x <X>\tprivacy protocol (DES).\n");
  fprintf(outf, "  -X <P>\tprivacy protocol pass phrase\n");
}
#define BUF_SIZE 512
int
snmp_parse_args(argc, argv, session, type)
  int argc;
  char **argv;
  struct snmp_session *session;
  char *type;
{
  int arg;
  char *psz;
  u_char buf[BUF_SIZE];
  int bsize;
#ifdef USE_V2PARTY_PROTOCOL
  static oid src[MAX_NAME_LEN];
  static oid dst[MAX_NAME_LEN];
  static oid context[MAX_NAME_LEN];
  struct partyEntry *pp;
  struct contextEntry *cxp;
  struct hostent *hp;
  char ctmp[300];
  in_addr_t destAddr;
  int clock_flag = 0;
  u_long srcclock = 0;
  u_long dstclock = 0;
#endif

  /* initialize session to default values */
  memset(session, 0, sizeof(struct snmp_session));
  session->remote_port = SNMP_DEFAULT_REMPORT;
  session->timeout = SNMP_DEFAULT_TIMEOUT;
  session->retries = SNMP_DEFAULT_RETRIES;
  session->authenticator = NULL;
  session->peername = NULL;

  /* get the options */
  for(arg = 1; (arg < argc) && (argv[arg][0] == '-'); arg++){
    switch(argv[arg][1]){
      case 'd':
        snmp_set_dump_packet(1);
        break;

      case 'R':
        random_access = 1;
        break;

      case 'q':
        snmp_set_quick_print(1);
        break;

      case 'D':
        snmp_set_do_debugging(1);
        break;
        
      case 'm':
        if (argv[arg][2] != 0)
          setenv("MIBS",&argv[arg][2], 1);
        else if (++arg < argc)
          setenv("MIBS",argv[arg], 1);
        else {
          fprintf(stderr,"Need MIBS after -m flag.\n");
          usage();
          exit(1);
        }
        break;

      case 'M':
        if (argv[arg][2] != 0)
          setenv("MIBDIRS",&argv[arg][2], 1);
        else if (++arg < argc)
          setenv("MIBDIRS",argv[arg], 1);
        else {
          fprintf(stderr,"Need MIBDIRS after -M flag.\n");
          usage();
          exit(1);
        }
        break;
        
      case 'f':
	snmp_set_full_objid(1);
	break;

      case 's':
	snmp_set_suffix_only(1);
	break;

      case 'S':
	snmp_set_suffix_only(2);
	break;

      case 'p':
        if (isdigit(argv[arg][2]))
          session->remote_port = atoi(&(argv[arg][2]));
        else if ((++arg<argc) && isdigit(argv[arg][0]))
          session->remote_port = atoi(argv[arg]);
        else {
          fprintf(stderr,"Need port number after -p flag.\n");
          usage();
          exit(1);
        }
        break;

      case 't':
        if (isdigit(argv[arg][2]))
          session->timeout = atoi(&(argv[arg][2])) * 1000000L;
        else if ((++arg<argc) && isdigit(argv[arg][0]))
          session->timeout = atoi(argv[arg]) * 1000000L;
        else {
          fprintf(stderr,"Need time in seconds after -t flag.\n");
          usage();
          exit(1);
        }
        break;

      case 'r':
        if (isdigit(argv[arg][2]))
          session->retries = atoi(&(argv[arg][2]));
        else if ((++arg<argc) && isdigit(argv[arg][0]))
          session->retries = atoi(argv[arg]);
        else {
          fprintf(stderr,"Need number of retries after -r flag.\n");
          usage();
          exit(1);
        }
        break;

#ifdef USE_V2PARTY_PROTOCOL
      case 'c':
        clock_flag++;
        if (isdigit(argv[arg][2]))
          srcclock = (u_long)(atol(&(argv[arg][2])));
        else if ((++arg<argc) && isdigit(argv[arg][0]))
          srcclock = (u_long)(atol(argv[arg]));
        else {
          fprintf(stderr,"Need source clock value after -c flag.\n");
          usage();
          exit(1);
        }
        if ((++arg<argc) && isdigit(argv[arg][0]))
          dstclock = (u_long)(atol(argv[arg]));
        else {
          fprintf(stderr,"Need destination clock value after -c flag.\n");
          usage();
          exit(1);
        }
        break;
#endif /* USE_V2PARTY_PROTOCOL */

      case 'V':
        fprintf(stderr,"UCD-snmp version: %s\n", VersionInfo);
        exit(0);

      case 'v':
        if (argv[arg][2] != 0)
          psz = &(argv[arg][2]);
        else
          psz = argv[++arg];
        if( psz == NULL) {
          fprintf(stderr,"Need version value after -v flag. \n");
          usage();
          exit(1);
        }
        if (!strcmp(psz,"1")) {
          session->version = SNMP_VERSION_1;
        } else if (!strcasecmp(psz,"2c")) {
          session->version = SNMP_VERSION_2c;
        } else if (!strcasecmp(psz,"2p")) {
          session->version = SNMP_VERSION_2p;
        } else if (!strcasecmp(psz,"3")) {
          session->version = SNMP_VERSION_3;
        } else {
          fprintf(stderr,"Invalid version specified after -v flag: %s\n", psz);
          usage();
          exit(1);
        }
        break;

      case 'e':
        if (argv[arg][2] != 0)
          psz = &(argv[arg][2]);
        else
          psz = argv[++arg];
        if( psz == NULL) {
          fprintf(stderr,"Need engine ID value after -e flag. \n");
          usage();
          exit(1);
        }
	if ((bsize = hex_to_binary(psz,buf)) <= 0) {
          fprintf(stderr,"Need engine ID value after -e flag. \n");
          usage();
          exit(1);
	}
	session->contextEngineID = malloc(bsize);
	memcpy(session->contextEngineID, buf, bsize);
	session->contextEngineIDLen = bsize;
        break;

      case 'n':
        if (argv[arg][2] != 0)
          psz = &(argv[arg][2]);
        else
          psz = argv[++arg];
        if( psz == NULL) {
          fprintf(stderr,"Need context name value after -n flag. \n");
          usage();
          exit(1);
        }
	session->contextName = strdup(psz);
	session->contextNameLen = strlen(psz);
        break;

      case 'u':
        if (argv[arg][2] != 0)
          psz = &(argv[arg][2]);
        else
          psz = argv[++arg];
        if( psz == NULL) {
          fprintf(stderr,"Need security user name value after -u flag. \n");
          usage();
          exit(1);
        }
	session->securityName = strdup(psz);
	session->securityNameLen = strlen(psz);
        break;

      case 'l':
        if (argv[arg][2] != 0)
          psz = &(argv[arg][2]);
        else
          psz = argv[++arg];
        if( psz == NULL) {
          fprintf(stderr,"Need security level value after -l flag. \n");
          usage();
          exit(1);
        }
        if (!strcmp(psz,"noAuthNoPriv") || !strcmp(psz,"1")) {
          session->securityLevel = SNMP_SEC_LEVEL_NOAUTH;
        } else if (!strcmp(psz,"authNoPriv") || !strcmp(psz,"2")) {
          session->securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
        } else if (!strcmp(psz,"authPriv") || !strcmp(psz,"3")) {
          session->securityLevel = SNMP_SEC_LEVEL_AUTHPRIV;
        } else {
          fprintf(stderr,"Invalid security level specified after -l flag: %s\n", psz);
          usage();
          exit(1);
        }
	
        break;

      case 'a':
        if (argv[arg][2] != 0)
          psz = &(argv[arg][2]);
        else
          psz = argv[++arg];
        if( psz == NULL) {
          fprintf(stderr,"Need authentication protocol value after -a flag. \n");
          usage();
          exit(1);
        }
        if (!strcmp(psz,"MD5")) {
          session->securityAuthProto = usmHMACMD5AuthProtocol;
          session->securityAuthProtoLen = USM_AUTH_PROTO_MD5_LEN;
        } else if (!strcmp(psz,"SHA")) {
          session->securityAuthProto = usmHMACSHA1AuthProtocol;
          session->securityAuthProtoLen = USM_AUTH_PROTO_SHA_LEN;
        } else {
          fprintf(stderr,"Invalid authentication protocol specified after -a flag: %s\n", psz);
          usage();
          exit(1);
        }
        break;

      case 'x':
        if (argv[arg][2] != 0)
          psz = &(argv[arg][2]);
        else
          psz = argv[++arg];
        if( psz == NULL) {
          fprintf(stderr,"Need privacy protocol value after -x flag. \n");
          usage();
          exit(1);
        }
        if (!strcmp(psz,"DES")) {
          session->securityPrivProto = usmDESPrivProtocol;
          session->securityPrivProtoLen = USM_PRIV_PROTO_DES_LEN;
        } else {
          fprintf(stderr,"Invalid privacy protocol specified after -x flag: %s\n", psz);
          usage();
          exit(1);
        }
        break;

      case 'A':
        if (argv[arg][2] != 0)
          psz = &(argv[arg][2]);
        else
          psz = argv[++arg];
        if( psz == NULL) {
          fprintf(stderr,"Need authentication pass phrase value after -A flag. \n");
          usage();
          exit(1);
        }
	session->securityAuthKeyLen = USM_AUTH_KU_LEN;
	if (generate_Ku(session->securityAuthProto, 
			session->securityAuthProtoLen,
			psz, strlen(psz), 
			session->securityAuthKey, 
			&session->securityAuthKeyLen) != SNMPERR_SUCCESS) {
          fprintf(stderr,"Error generating Ku from authentication pass phrase. \n");
          usage();
          exit(1);
        }
        break;

      case 'X':
        if (argv[arg][2] != 0)
          psz = &(argv[arg][2]);
        else
          psz = argv[++arg];
        if( psz == NULL) {
          fprintf(stderr,"Need privacy pass phrase value after -X flag. \n");
          usage();
          exit(1);
        }
	session->securityPrivKeyLen = USM_PRIV_KU_LEN;
	if (generate_Ku(session->securityPrivProto, 
			session->securityPrivProtoLen,
			psz, strlen(psz), 
			session->securityPrivKey, 
			&session->securityPrivKeyLen) != SNMPERR_SUCCESS) {
          fprintf(stderr,"Error generating Ku from privacy pass phrase. \n");
          usage();
          exit(1);
        }

        break;

      case 'h':
        usage();
        exit(0);
        break;
          
      default:
        /* This should be removed to support options in clients that
           have more parameters than the defaults above! */
        fprintf(stderr, "invalid option: -%c\n", argv[arg][1]);
        usage();
        exit(1);
        break;
    }
  }

  /* read in MIB database and initialize the snmp library*/
  init_snmp(type);

  /* get the hostname */
  if (arg == argc) {
    fprintf(stderr,"No hostname specified.\n");
    usage();
    exit(1);
  }
  session->peername = argv[arg++];     /* hostname */

  /* get community or party */
  if ((session->version == SNMP_VERSION_1) ||
      (session->version == SNMP_VERSION_2c)) {
    /* v1 and v2c - so get community string */
    if (arg == argc) {
      fprintf(stderr,"No community name specified.\n");
      usage();
      exit(1);
    }
    session->community = (unsigned char *)argv[arg];
    session->community_len = strlen((char *)argv[arg]);
    arg++;
#ifdef USE_V2PARTY_PROTOCOL
  } else if (session->version == SNMP_VERSION_2p) {
    /* v2p - so get party info */
    if (arg == argc) {
      fprintf(stderr,"Neither a source party nor noAuth was specified.\n");
      usage();
      exit(1);
    }

    session->srcParty = src;
    session->dstParty = dst;
    session->context = context;

    if (!strcasecmp(argv[arg], "noauth")){
      if ((destAddr = inet_addr(session->peername)) == -1){
        hp = gethostbyname(session->peername);
        if (hp == NULL){
          fprintf(stderr, "unknown host: %s\n", session->peername);
          exit(1);
        } else {
          memmove(&destAddr, hp->h_addr, hp->h_length);
        }
      }
      session->srcPartyLen = session->dstPartyLen =
                    session->contextLen = MAX_NAME_LEN;
      ms_party_init(destAddr, session->srcParty, &(session->srcPartyLen),
                    session->dstParty, &(session->dstPartyLen),
                    session->context, &(session->contextLen));
      arg++;
    } else {
      sprintf(ctmp,"%s/party.conf",SNMPSHAREPATH);
      if (read_party_database(ctmp) != 0){
	snmp_perror(argv[0]);
        exit(1);
      }
      sprintf(ctmp,"%s/context.conf",SNMPSHAREPATH);
      if (read_context_database(ctmp) != 0){
	snmp_perror(argv[0]);
        exit(1);
      }
      sprintf(ctmp,"%s/acl.conf",SNMPSHAREPATH);
      if (read_acl_database(ctmp) != 0){
	snmp_perror(argv[0]);
        exit(1);
      }

      /* source party */
      
      party_scanInit();
      session->srcPartyLen = MAX_NAME_LEN;
      for(pp = party_scanNext(); pp; pp = party_scanNext()){
        if (!strcasecmp(pp->partyName, argv[arg])){
          session->srcPartyLen = pp->partyIdentityLen;
          memmove(session->srcParty, pp->partyIdentity,
                  session->srcPartyLen * sizeof(oid));
          break;
        }
      }
      if (!pp){
        session->srcPartyLen = MAX_NAME_LEN;
        if (!read_objid(argv[arg], session->srcParty, &(session->srcPartyLen))){
          fprintf(stderr,"Invalid source party: %s.\n", argv[arg]);
          session->srcPartyLen = 0;
          usage();
          exit(1);
        }
      }
      arg++;

      if (arg == argc) {
        fprintf(stderr,"No destination party specified.\n");
        usage();
        exit(1);
      }

      /* destination party */
      
      session->dstPartyLen = MAX_NAME_LEN;
      party_scanInit();
      for(pp = party_scanNext(); pp; pp = party_scanNext()){
        if (!strcasecmp(pp->partyName, argv[arg])){
          session->dstPartyLen = pp->partyIdentityLen;
          memmove(session->dstParty, pp->partyIdentity,
                  session->dstPartyLen * sizeof(oid));
          break;
        }
      }
      if (!pp){
        if (!read_objid(argv[arg], session->dstParty, &(session->dstPartyLen))){
          fprintf(stderr,"Invalid destination party: %s.\n", argv[arg]);
          session->dstPartyLen = 0;
          usage();
          exit(1);
        }
      }
      arg++;

      /* context */

      if (arg == argc) {
        fprintf(stderr,"No context specified.\n");
        usage();
        exit(1);
      }

      session->contextLen = MAX_NAME_LEN;
      context_scanInit();
      for(cxp = context_scanNext(); cxp; cxp = context_scanNext()){
        if (!strcasecmp(cxp->contextName, argv[arg])){
          session->contextLen = cxp->contextIdentityLen;
          memmove(session->context, cxp->contextIdentity,
                  session->contextLen * sizeof(oid));
          break;
        }
      }
      if (!cxp){
        if (!read_objid(argv[arg], session->context, &(session->contextLen))){
          fprintf(stderr,"Invalid context: %s.\n", argv[arg]);
          session->contextLen = 0;
          usage();
          exit(1);
        }
      }
      arg++;

      if (clock_flag){
        pp = party_getEntry(session->srcParty, session->srcPartyLen);
        if (pp){
            pp->partyAuthClock = srcclock;
            gettimeofday(&pp->tv, (struct timezone *)0);
            pp->tv.tv_sec -= pp->partyAuthClock;
        }
        pp = party_getEntry(session->dstParty, session->dstPartyLen);
        if (pp){
            pp->partyAuthClock = dstclock;
            gettimeofday(&pp->tv, (struct timezone *)0);
            pp->tv.tv_sec -= pp->partyAuthClock;
        }
      }
    }
#endif /* USE_V2PARTY_PROTOCOL */
  }
  return arg;
}

oid
*snmp_parse_oid(argv,root,rootlen) 
  char *argv;
  oid *root;
  int *rootlen;
{
  if (random_access) {
    if (get_node(argv,root,rootlen)) {
      return root;
    }
  } else {
    if (read_objid(argv,root,rootlen)) {
      return root;
    }
  }
  return NULL;
}

