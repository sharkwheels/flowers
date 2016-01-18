### IMPORTS ###############################################

from __future__ import print_function
import myo as libmyo; libmyo.init()
import time
import sys
import serial
import logging
import struct

### Logging ###################################################

logging.basicConfig(level=logging.WARNING,
                    format='%(asctime)s %(levelname)s %(message)s',
                    filename='flowers.log',
                    filemode='w')

### PYTHON SERIAL OUT #########################################

try:
	ser = serial.Serial('/dev/cu.usbmodem14221', 9600)
	time.sleep(2)
	print("Connection to " + ser + " established succesfully!\n")
except Exception as e:
	print(e)


### MYO DATA HELPERS ############################################

def parse_data(myoData):
	"""
	Just parsing a list containg all the data, dunno if this is the best way, but it feels right. 
	"""
	myoID = myoData[0]
	orientation = [myoData[1][0],myoData[1][1],myoData[1][2]]
	pose = myoData[2]
	accel = [myoData[1][0],myoData[1][1],myoData[1][2]]
	#print(myoID,accel,pose,orientation)
	
	pass_to_fc(myoID,pose)
	## eventually use some orientation data in here as well, pose just to test

### ARDUINO OUT #########################################

def pass_to_fc(myoID, pose):
	print(pose)
	##print(type(myoID))
	### find a way to make the class thing into a straight up raw string
	### then parse the 0x1243 whatever from that string
	### then use that to diff between myos
	
	if pose == 'fist':
		print("F1 closed")
		## send a serial command to close (2)
		ser.write(struct.pack('>B', 12))
	elif pose == 'wave_out':
		## send a serial command to open (1)
		print("F1 open")
		ser.write(struct.pack('>B', 11))
	elif pose == 'fingers_spread':
		print("F2 open")
		ser.write(struct.pack('>B',21))
	elif pose == 'wave_in':
		print("F2")
		ser.write(struct.pack('>B',22))
	



### RUNTIME ###############################################

def main():

	connectedMyos = []
	
	try:
		hub = libmyo.Hub()
	except MemoryError:
		print("Myo Hub could not be created. Make sure Myo Connect is running.")
		return
	feed = libmyo.device_listener.Feed()
	hub.run(1000, feed)
	
	try:
		print("Waiting to connect...")
		myo = feed.wait_for_single_device(5) ## pass the seconds to wait
		if myo:
			myo.vibrate('short')
			connectedMyos.append(myo)

		if not myo:
			print("No myo connected after 5 seconds")
			return

		#connectedMyos = feed.get_devices() ## not sure i actually need this in any way
		while hub.running and myo.connected:
			
			### this might not be right or be too slow, but we'll try it for now
			## mostly I'm just concerned w/ the myos getting mixed up. 
			for m in connectedMyos:
				myoData = [m,m.orientation,m.pose,m.acceleration]
				parse_data(myoData)
				time.sleep(0.1)
			
		print("Disconnecting")

	except KeyboardInterrupt:
		print("KeyboardInterrupt")
	finally:
		print("Shutting Down Hub")
		hub.shutdown()


if __name__ == "__main__":
	main()