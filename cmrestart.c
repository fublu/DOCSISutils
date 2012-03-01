#include <stdio.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>


/* cmversion (c): Marcin Jurczuk <marcin@jurczuk.eu>
 * simple hack of snmpset rewriten to use SNMP_2c
 * just to set reset OID on CM. Very usefull tool for daily work
 */
#define COMMUNITY "public"
#define RESETOID ".1.3.6.1.2.1.69.1.1.3.0"

struct snmp_session session, *ss;
struct snmp_pdu *pdu;
struct snmp_pdu *response;
oid anOID[MAX_OID_LEN];
size_t anOID_len = MAX_OID_LEN;
const char setValue[] = "1";
struct variable_list *vars;
int status;
int count;
int failures = 0;
static int exitval = 0;
static int quiet = 0;
int main(int argc, char **argv[]) {
    if ( argc < 2) {
        printf("Missing CM IP\nUSAGE: %s <CM_IP>\n", argv[0]);
        exit(1);
    }
    init_snmp("snmpapp");
    snmp_sess_init( &session );
    session.peername = (char*) argv[1];
    session.version=SNMP_VERSION_2c;
    session.community=COMMUNITY;
    session.community_len = strlen(session.community);
    /*
     * create PDU for SET request and add object names and values to request
     */
    pdu = snmp_pdu_create(SNMP_MSG_SET);
    read_objid(RESETOID, anOID, &anOID_len);
            if (snmp_add_var(pdu, anOID, anOID_len, 'i',setValue)) {
            //snmp_perror();
            failures++;
        }

    if (failures) {
        snmp_close(ss);
        SOCK_CLEANUP;
        exit(1);
    }

    ss = snmp_open(&session); 
    if (!ss) {
       snmp_perror("ack");
       snmp_log(LOG_ERR, "Unable to open snmp session!!\n");
       exit(2);
   }
    
      status = snmp_synch_response(ss, pdu, &response);
    if (status == STAT_SUCCESS) {
        if (response->errstat == SNMP_ERR_NOERROR) {
            if (!quiet) {
     for(vars = response->variables; vars; vars = vars->next_variable) {
       int count=1;
//       if (vars->type == ASN_OCTET_STR) {
       if (vars->type == ASN_INTEGER) {
         //char *sp = malloc(1 + vars->val_len);
         //memcpy(sp, vars->val.string, vars->val_len);
         //sp[vars->val_len] = '\0';
         //printf("%d: %s\n", count++, sp);
         //printf("%d\n", vars->val.integer);
         printf("%s: Restart in progress..\n",argv[1]);
//         free(sp);
       }
       else
         printf("value #%d is NOT a string! Ack!\n", count++);
     }

            }
        } else {
            fprintf(stderr, "Error in packet.\nReason: %s\n",
                    snmp_errstring(response->errstat));
            if (response->errindex != 0) {
                fprintf(stderr, "Failed object: ");
                for (count = 1, vars = response->variables;
                     vars && (count != response->errindex);
                     vars = vars->next_variable, count++);
                if (vars)
                    fprint_objid(stderr, vars->name, vars->name_length);
                fprintf(stderr, "\n");
            }
            exitval = 2;
        }
    } else if (status == STAT_TIMEOUT) {
        fprintf(stderr, "Timeout: No Response from %s\n",
                session.peername);
        exitval = 1;
    } else {                    /* status == STAT_ERROR */
        snmp_sess_perror("snmpset", ss);
        exitval = 1;
    }

    if (response)
        snmp_free_pdu(response);
    snmp_close(ss);
    SOCK_CLEANUP;
  
    return exitval;

}
