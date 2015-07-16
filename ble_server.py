#!/usr/bin/python
#
#	BLE Module Controller Server
#
#	Connects to clients	ands routes commands
#	to and from mothership engine to BLE Clients
#
import pexpect, time

#defines
mac_addr = 'D0:39:72:C3:AC:AA'

def str2hex(msg):
	hex_msg = ''.join(["{:02x}".format(ord(m)) for m in msg])
	return hex_msg

class BLEController(object):
	write_port = '0x0016'
	def __init__(self, addr):
		self.gatt = pexpect.spawn('gatttool -I')
		self.gatt.expect('.*\[LE\]> ')
		self.gatt.sendline('connect {0}\n'.format(addr))
		time.sleep(0.6)
	def read(self):
		vals = self.read_hex()
		chars = [str(unichr(int(x, 16))) for x in vals]
		return ''.join(chars)
	def read_hex(self):
		self.gatt.sendline('char-read-hnd 0x0012')
		self.gatt.expect('.*Characteristic value/descriptor: ')
		return self.gatt.readline().replace('\n', '').split()
	def decompress_values(self, hex_msg):
		binstr = [bin(int(x, 16))[2:].zfill(8) for x in hex_msg]
		return ''.join(binstr)[10:20]
	def write(self, msg):
		hex_msg = str2hex(msg)
		self.write_hex(hex_msg)
	def write_hex(self, hex_msg):
		self.gatt.sendline('char-write-cmd 0x0016 '+'{:02x}'.format(len(hex_msg)/2)+hex_msg)

cont = BLEController(mac_addr)
cont.write_hex('01' + '10' + '05' + str2hex('sine generator'))
while True:
	print int("0b"+cont.decompress_values(cont.read_hex()), 2)