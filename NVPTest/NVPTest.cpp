// NVPTest.cpp : Defines the entry po int for the console application.
//

#include "stdafx.h"
#include "NVPCybersource.h"
#include "util.h"
#include "Windows.h"
#pragma comment(lib, "NVPClient.lib")
#pragma comment(lib, "BASECLIENT.lib")

const char CYBS_INI_FILE[]   = "../cybs.ini";

void printContent(CybsMap *map);
char *runAuth(CybsMap *cfgMap);
void runCapture( CybsMap* config, char* authRequestID );
void handleFault(INVPTransactionProcessorProxy proxy);

void handleError( int status, CybsMap *store )
{
	printf( "RunTransaction Status: %d\n", status );
	switch( status ) {
		case CYBS_S_PRE_SEND_ERROR:
			printf( "The following error occurred before the request could be sent:\n" );
			printf( "%s\n", cybs_get(store, CYBS_SK_ERROR_INFO));
		break;
		case CYBS_S_SEND_ERROR:
			printf( "The following error occurred:\n" );
			printf( "%s\n", cybs_get(store, CYBS_SK_ERROR_INFO));
		break;
		default:
			printf( "The following error occurred:\n" );
			printf( "%s\n", cybs_get(store, CYBS_SK_ERROR_INFO));
		break;
	}
}

int main(void)
{
	CybsMap *cfgMap = cybs_create_map();
	char *requestID;
	cybs_load_config(CYBS_INI_FILE, cfgMap);

	printf( "\nCONFIGURATION (%s): \n", CYBS_INI_FILE);

	if ( !cfgMap ) {
		printf( "Error: could not find the config file [%s]\n", CYBS_INI_FILE );
		return(1);
	}

	printContent(cfgMap);
	//int i = 0;
	//while (i<2) {
	requestID = runAuth(cfgMap);
	//i++;

	if ( requestID ) {
		runCapture( cfgMap, requestID );
		free( requestID );
	}
	//}

	cybs_destroy_map(cfgMap);
	return (0);
}

char *runAuth(CybsMap *cfgMap) {
	const char *decision, *requestID;
	char *ret = NULL;
	INVPTransactionProcessorProxy proxy = INVPTransactionProcessorProxy ();
	//proxy.soap = soap_new();
	//proxy.soap_own = true;
	//proxy.INVPTransactionProcessorProxy_init(SOAP_IO_DEFAULT, SOAP_IO_DEFAULT);
	soap_mode(proxy.soap, (soap_mode)SOAP_XML_CANONICAL);
	CybsMap *requestMap = cybs_create_map();
	CybsMap *responseMap = cybs_create_map();
	
	// cybs_add(requestMap, "merchantID", "visadctest_ashish");
	cybs_add(requestMap, "merchantReferenceCode", "your_merchant_reference_code");
	cybs_add(requestMap, "billTo_firstName", "John");
	cybs_add(requestMap, "billTo_lastName", "Doe");
	cybs_add(requestMap, "billTo_street1", "1295 Charleston Road");
	cybs_add(requestMap, "billTo_city", "Mountain View");
	cybs_add(requestMap, "billTo_state", "CA");
	cybs_add(requestMap, "billTo_postalCode", "94043");
	cybs_add(requestMap, "billTo_country", "US");
	cybs_add(requestMap, "billTo_email", "nobody@cybersource.com");
	cybs_add(requestMap, "billTo_ipAddress", "10.7.7.7");
	cybs_add(requestMap, "billTo_phoneNumber", "650-965-6000");
	cybs_add(requestMap, "shipTo_firstName", "Jane");
	cybs_add(requestMap, "shipTo_lastName", "Jane");
	//cybs_add(requestMap, "shipTo_firstName", "Doe");
	cybs_add(requestMap, "shipTo_street1", "123 toile d�lav�e noir");
	//cybs_add(requestMap, "shipTo_city", "San Mateo");
	cybs_add(requestMap, "shipTo_state", "CA");
	cybs_add(requestMap, "shipTo_city", "CA");
	cybs_add(requestMap, "shipTo_postalCode", "94401");
	cybs_add(requestMap, "shipTo_country", "US");
	cybs_add(requestMap, "card_accountNumber", "4111111111111111");
	cybs_add(requestMap, "card_expirationMonth", "12");
	cybs_add(requestMap, "card_expirationYear", "2020");
	cybs_add(requestMap, "purchaseTotals_currency", "USD");
	cybs_add(requestMap, "item_0_unitPrice", "12.34");
	cybs_add(requestMap, "ccAuthService_run", "true");

	printf( "CREDIT CARD AUTHORIZATION REQUEST: \n" );
	printContent(requestMap);

	int status = runTransaction(&proxy, cfgMap, requestMap, responseMap);

	switch (status)
	{
		case SOAP_OK:
			printf( "CREDIT CARD AUTHORIZATION REPLY: \n" );
			printContent(responseMap);
			decision = (const char *)cybs_get(responseMap, "decision");
			if( strcmp(decision, "ACCEPT") == 0 ) {
				requestID = (const char *)cybs_get( responseMap, "requestID" );
				ret = (char *) malloc( (strlen(requestID)+1) * sizeof(char) );
				strcpy( ret, requestID );
			}
		break;

		case SOAP_FAULT:
			handleFault(proxy);
		break;
		
		case CYBS_S_PRE_SEND_ERROR:
			if (cybs_get(cfgMap, CYBS_SK_ERROR_INFO)) {
				printf( "%s\n", cybs_get(cfgMap, CYBS_SK_ERROR_INFO));
			}
		break;

		default:
			if (cybs_get(cfgMap, CYBS_SK_ERROR_INFO)) {
				printf( "%s\n", cybs_get(cfgMap, CYBS_SK_ERROR_INFO));
			}
			printf("faultstring = %s\n", proxy.soap_fault_string());
	}
	cybs_destroy_map(requestMap);
	cybs_destroy_map(responseMap);

	proxy.destroy();
	soap_delete(proxy.soap, NULL);
	soap_dealloc(proxy.soap, NULL);
	soap_destroy(proxy.soap);
	soap_end(proxy.soap);
	soap_done(proxy.soap);

	return( ret );
}

void runCapture( CybsMap* cfgMap, char* authRequestID ) {
	INVPTransactionProcessorProxy proxy = INVPTransactionProcessorProxy ();
	//proxy.soap = soap_new();
	//proxy.soap_own = true;
	//proxy.INVPTransactionProcessorProxy_init(SOAP_IO_DEFAULT, SOAP_IO_DEFAULT);
	soap_mode(proxy.soap, (soap_mode)SOAP_XML_CANONICAL);

	CybsMap *requestMap = cybs_create_map();
	CybsMap *responseMap = cybs_create_map();
	
	cybs_add( requestMap, "ccCaptureService_run", "true" );
	cybs_add( requestMap, "merchantReferenceCode", "your_merchant_reference_code" );
	cybs_add( requestMap, "ccCaptureService_authRequestID", authRequestID );
	cybs_add( requestMap, "purchaseTotals_currency", "USD" );
	cybs_add( requestMap, "item_0_unitPrice", "12.34" );

	printf( "FOLLOW-ON CAPTURE REQUEST: \n" );
	printContent(requestMap);
	int status = runTransaction(&proxy, cfgMap, requestMap, responseMap );

	switch (status)
	{
		case SOAP_OK:
			printf( "FOLLOW-ON CAPTURE REPLY: \n" );
			printContent(responseMap);
		break;
		
		case SOAP_FAULT:
			handleFault(proxy);
		break;
		
		case CYBS_S_PRE_SEND_ERROR:
			if (cybs_get(cfgMap, CYBS_SK_ERROR_INFO)) {
				printf( "%s\n", cybs_get(cfgMap, CYBS_SK_ERROR_INFO));
			}
		break;

		default:
			if (cybs_get(cfgMap, CYBS_SK_ERROR_INFO)) {
				printf( "%s\n", cybs_get(cfgMap, CYBS_SK_ERROR_INFO));
			}
			//soap_print_fault(proxy.soap, stdout);
			printf("faultstring = %s\n", proxy.soap_fault_string());
	}

	cybs_destroy_map(requestMap);
	cybs_destroy_map(responseMap);

	proxy.destroy();
	soap_delete(proxy.soap, NULL);
	soap_dealloc(proxy.soap, NULL);
	soap_destroy(proxy.soap);
	soap_end(proxy.soap);
	soap_done(proxy.soap);
}

void handleFault(INVPTransactionProcessorProxy proxy)
{
	//soap_print_fault(proxy.soap, stdout );
	printf("faultcode = %s\n", proxy.soap->fault->faultcode);
	printf("faultstring = %s\n", proxy.soap_fault_string());
	SOAP_ENV__Detail *detail  = proxy.soap->fault->detail;
	if (detail != NULL) {
		printf("detail = %s\n", detail->__any);
	}
}
void printContent(CybsMap *map) {
	int i = 0;
	while (i < map->length) {
		CybsTable table = map->pairs[i];
		printf(" %s ==> %s\n", table.key, table.value);
		i = i +1;
	}
	printf("\n");
}

