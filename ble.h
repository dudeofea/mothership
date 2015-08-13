#ifndef ble_h__
#define ble_h__
 
extern int ble_char_read(int handle, unsigned char* buf);
extern void ble_char_write(int handle, unsigned char* buf, int len);
extern void ble_connect(const char *mac);
extern void ble_disconnect();
 
#endif  // ble_h__