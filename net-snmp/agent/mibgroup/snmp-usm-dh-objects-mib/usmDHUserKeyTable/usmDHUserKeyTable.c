/*
 * Note: this file originally auto-generated by mib2c using
 *       version : 1.25 $ of : mfd-top.m2c,v $ 
 *
 * $Id$
 */
/** \mainpage MFD helper for usmDHUserKeyTable
 *
 * \section intro Introduction
 * Introductory text.
 *
 */
/*
 * standard Net-SNMP includes 
 */
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

/*
 * include our parent header 
 */
#include "usmDHUserKeyTable.h"

#include <net-snmp/agent/mib_modules.h>

#include "usmDHUserKeyTable_interface.h"

oid             usmDHUserKeyTable_oid[] = { USMDHUSERKEYTABLE_OID };
int             usmDHUserKeyTable_oid_size =
OID_LENGTH(usmDHUserKeyTable_oid);

void            initialize_table_usmDHUserKeyTable(void);


/**
 * Initializes the usmDHUserKeyTable module
 */
void
init_usmDHUserKeyTable(void)
{
    DEBUGMSGTL(("verbose:usmDHUserKeyTable:init_usmDHUserKeyTable",
                "called\n"));

    /*
     * here we initialize all the tables we're planning on supporting 
     */
    if (should_init("usmDHUserKeyTable")) {
        initialize_table_usmDHUserKeyTable();
    }
}

/**
 * Initialize the table usmDHUserKeyTable 
 *    (Define its contents and how it's structured)
 */
void
initialize_table_usmDHUserKeyTable(void)
{
    usmDHUserKeyTable_registration_ptr user_context;
    u_long          flags;

    DEBUGMSGTL(("verbose:usmDHUserKeyTable:initialize_table_usmDHUserKeyTable", "called\n"));

    /*
     * if you'd like to pass in a pointer to some data for this
     * table, allocate or set it up here.
     */
    user_context = NULL;

    /*
     * No support for any flags yet, but in the future you would
     * set any flags here.
     */
    flags = 0;

    /*
     * call interface initialization code
     */
    _usmDHUserKeyTable_initialize_interface(user_context, flags);
}

/**
 * pre-request callback
 *
 *
 * @retval MFD_SUCCESS              : success.
 * @retval MFD_ERROR                : other error
 */
int
usmDHUserKeyTable_pre_request(usmDHUserKeyTable_registration_ptr
                              user_context)
{
    DEBUGMSGTL(("verbose:usmDHUserKeyTable_pre_request", "called\n"));

    /*
     * TODO:
     * pre-request setup
     */

    return MFD_SUCCESS;
}

/**
 * post-request callback
 *
 *
 * @retval MFD_SUCCESS : success.
 * @retval MFD_ERROR   : other error (ignored)
 */
int
usmDHUserKeyTable_post_request(usmDHUserKeyTable_registration_ptr
                               user_context)
{
    DEBUGMSGTL(("verbose:usmDHUserKeyTable_post_request", "called\n"));

    /*
     * TODO:
     * post-request cleanup
     */

    return MFD_SUCCESS;
}


/** @{ */
