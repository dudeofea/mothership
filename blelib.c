/*
*
*	BLE C library
*
*	Couldn't find any API for gatttool and felt
*	like having a pure C project, so I wrote this up
*
*	Totally free of course and abides by the same
*	license as the rest of the project
*
*	Lots of things are pretty hardcoded, :(
*
*	Denis Lachance, 2015
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <pthread.h>
#include <sys/time.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

#include "btio/btio.h"
#include "lib/uuid.h"
#include "att.h"
#include "gattrib.h"
#include "gatt.h"
#include "gatttool.h"
#include "ble.h"

static GIOChannel *iochannel = NULL;
static GAttrib *attrib = NULL;
static GMainLoop *loop = NULL;
static GError *connect_err = NULL;
static pthread_t ble_thread = NULL;
static int ble_connected = 0;
static int ble_read = 0;
static int ble_write = 0;
static uint8_t *ble_read_val;
static ssize_t ble_read_len;

static struct timeval t1, t2;
static double elapsedTime;

static const char* name;

//callback function for when we receive a ble message
static void char_read_cb(guint8 status, const guint8 *pdu, guint16 plen,
							gpointer user_data)
{
	uint8_t value[plen];
	ssize_t vlen;

	if (status != 0) {
		printf("Characteristic value/descriptor read failed: %s\n",
							att_ecode2str(status));
		ble_disconnect();
		return;
	}

	vlen = dec_read_resp(pdu, plen, value, sizeof(value));
	if (vlen < 0) {
		printf("Protocol error\n");
		ble_disconnect();
		return;
	}

	ble_read_val = value;
	ble_read_len = vlen;
	ble_read = 1;
}
//callback for when a ble write is complete
static void char_write_cb(guint8 status, const guint8 *pdu, guint16 plen,
							gpointer user_data)
{
	if (status != 0) {
		printf("Characteristic Write Request failed: "
						"%s\n", att_ecode2str(status));
		ble_disconnect();
		return;
	}

	if (!dec_write_resp(pdu, plen) && !dec_exec_write_resp(pdu, plen)) {
		printf("Protocol error\n");
		ble_disconnect();
		return;
	}
	ble_write = 1;
}
//callback function when we get connected
static void connect_cb(GIOChannel *io, GError *err, gpointer user_data)
{
	if (err) {
		printf("%s\n", err->message);
		return;
	}

	if (connect_err) {
		printf("%s\n", connect_err->message);
		return;
	}

	attrib = g_attrib_new(iochannel);
	if(attrib == NULL){
		printf("Connect Error\n");
	}
	ble_connected = 1;
}
//function to actually make the connection
void *new_ble_connection(void *opt_dst_void){
	loop = g_main_loop_new (NULL, FALSE);
	//Connect to address
	char *opt_src = NULL;
	char *opt_dst = (char*)opt_dst_void;
	char *opt_dst_type = "public";
	char *opt_sec_level = "low";
	int opt_psm = 0;
	int opt_mtu = 0;
	iochannel = gatt_connect(opt_src, opt_dst, opt_dst_type, opt_sec_level,
					opt_psm, opt_mtu, connect_cb, &connect_err);
	if(iochannel == NULL){
		printf("iochannel error\n");
	}
	g_main_loop_run(loop);
	printf("done main loop\n");
	return NULL;
}
//reads a certain handle and places value into buf, returns length
int ble_char_read(int handle, unsigned char* buf){
	//auto connect if not connected
	if(!ble_connected){
		ble_connect(name);
	}
	ble_read = 0;
	gettimeofday(&t1, NULL);
	gatt_read_char(attrib, handle, char_read_cb, attrib);
	while(!ble_read){
		usleep(10000);
		gettimeofday(&t2, NULL);
		elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
		elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
		if (elapsedTime > 250)	//timeout of 250ms
		{
			printf("read error: %lf\n", elapsedTime);
			ble_disconnect();
			return 0;
		}
	}
    //printf("(%lfms) len: %d\n", elapsedTime, (int)ble_read_len);
	/*for (i = 0; i < ble_read_len; i++){
		printf("%02x ", ble_read_val[i]);
	}
	printf("\n");*/
	memcpy(buf, ble_read_val, ble_read_len);
	return ble_read_len;
}
//writes to a certain handle the given bytes in buf for the given length
void ble_char_write(int handle, unsigned char* buf, int len){
	//auto connect if not connected
	if(!ble_connected){
		ble_connect(name);
	}
	//ignore 0 length requests
	if(len <= 0){ return; }
	ble_write = 0;
	gettimeofday(&t1, NULL);
	gatt_write_char(attrib, handle, buf,
					len, char_write_cb, NULL);
	//wait for write to be done
	while(!ble_write){
		usleep(10000);
		gettimeofday(&t2, NULL);
		elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
		elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
		if (elapsedTime > 500)	//timeout of 500ms
		{
			printf("write error: %lf\n", elapsedTime);
			ble_disconnect();
			return;
		}
	}
    //printf("write: (%lfms)\n", elapsedTime);
}
//launches a connection on a separate thread, waits for connection.
//this is because the other thread locks upon connecting to run g_main_loop
void ble_connect(const char *mac){
	name = mac;
	pthread_create(&ble_thread, NULL, new_ble_connection, (void*)mac);
	while(!ble_connected){
		//printf("waiting...\n");
		usleep(100000);
	}
}

//this destroys the launched thread and disconnects from the BLE device
void ble_disconnect(){
	//stop the main loop
	g_main_loop_quit(loop);
	pthread_join(ble_thread, NULL);
	usleep(10000);
	ble_connected = 0;
}