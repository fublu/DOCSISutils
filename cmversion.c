#include <stdio.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>


/* cmversion (c): Marcin Jurczuk <marcin@jurczuk.eu>
 * simple hack of snmpdemo rewriten to use SNMP_2c
 * just to get sysDescr.0 from CM. Very usefull tool for daily work
 */

struct snmp_session session, *ss;
struct snmp_pdu *pdu;
struct snmp_pdu *response;
oid anOID[MAX_OID_LEN];
size_t anOID_len = MAX_OID_LEN;
               
struct variable_list *vars;
int status;
int main(int argc, char **argv[]) {
    if ( argc < 2) {
        printf("Missing CM IP\nUSAGE: %s <CM_IP>\n", argv[0]);
        exit(1);
    }
    init_snmp("snmpapp");
    snmp_sess_init( &session );
    session.peername = (char*) argv[1];
    session.version=SNMP_VERSION_2c;
    session.community="public";
    session.community_len = strlen(session.community);
    ss = snmp_open(&session); 
    if (!ss) {
       snmp_perror("ack");
       snmp_log(LOG_ERR, "Unable to open snmp session!!\n");
       exit(2);
   }
    
    pdu = snmp_pdu_create(SNMP_MSG_GET);
    read_objid(".1.3.6.1.2.1.1.1.0", anOID, &anOID_len);
    snmp_add_null_var(pdu, anOID, anOID_len);
    status = snmp_synch_response(ss, pdu, &response);
    if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
     /*
      * SUCCESS: Print the result variables
      */
    //for(vars = response->variables; vars; vars = vars->next_variable)
     //  print_variable(vars->name,vars->name_length, vars);
      /* manipulate the information ourselves */
     for(vars = response->variables; vars; vars = vars->next_variable) {
       int count=1;
       if (vars->type == ASN_OCTET_STR) {
         char *sp = malloc(1 + vars->val_len);
         memcpy(sp, vars->val.string, vars->val_len);
         sp[vars->val_len] = '\0';
         //printf("%d: %s\n", count++, sp);
         printf("%s\n", sp);
         free(sp);
       }
       else
         printf("value #%d is NOT a string! Ack!\n", count++);
     }
        } else {
     /*
      * FAILURE: print what went wrong!
      */
    
     if (status == STAT_SUCCESS)
       fprintf(stderr, "Error in packet\nReason: %s\n",
               snmp_errstring(response->errstat));
     else
       snmp_sess_perror("snmpget", ss);
    
   }
   if (response)
     snmp_free_pdu(response);
   snmp_close(ss);
    
   /* windows32 specific cleanup (is a noop on unix) */
   SOCK_CLEANUP;



}
