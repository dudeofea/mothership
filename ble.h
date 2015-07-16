#ifndef ble_h__
#define ble_h__
 
extern int ble_char_read(int handle, unsigned char* buf);
extern void ble_connect(const char *mac);
 
#endif  // ble_h__