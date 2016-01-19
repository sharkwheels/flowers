### IMPORTS ###############################################

from __future__ import print_function
import myo as libmyo; libmyo.init()
import time
import sys
import serial
import logging
import struct
import inspect

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


def parse_data(playerData):
	"""
	Player Data is sent through as a list of Dicts. This function will parse desired data out of the structure
	and send commands down to the pass to function. Currently just using poses, but can parse all kinds of things.
	"""
	#print(playerData)
	#print(" ")

	player1 = None
	player2 = None

	### dirty AF but it will work for now...
	try:
		player1 = playerData[0]
	except IndexError:
		pass
	try:
		player2 = playerData[1]
	except IndexError:
		pass

	### This is dirty but it works for now.

	if player1:
		if player1['pose'] == 'fist':
			pass_to_arduino("P1close")
		elif player1['pose'] == 'wave_out':
			pass_to_arduino("P1open")

	if player2:
		if player2['pose'] == 'fist':
			pass_to_arduino("P2close")
		elif player2['pose'] == 'wave_out':
			pass_to_arduino("P2open")


### ARDUINO OUT #########################################

def pass_to_arduino(command):
	"""
	Use passed command string to trigger command number out to serial port.  
	"""
	#print(command)
	if ser:
		#print("open port")
		if command == "P1open":
			ser.write(struct.pack('>B', 11))
		elif command == "P1close":
			ser.write(struct.pack('>B', 12))
		elif command == "P2open":
			ser.write(struct.pack('>B',21))
		elif command == "P2close":
			ser.write(struct.pack('>B',22))
	else:
		print("The serial port has closed")
	



### RUNTIME ###############################################

def main():
	"""
	This is the main feed run loop. It initiates the myo hub and sets things up to assign the right data to each myo.
	Still to do:
		A disconnect function of sorts. 
		Clean-up
	"""
	
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
		
		if not myo:
			print("No myo connected after 5 seconds")
			return

		rawMyos = feed.get_devices() 
		namedMyos = []

		for idx, val in enumerate(rawMyos,start=1):
			print(idx, val)

			## if the myo is connected add the idx and myo to a list
			if val.connected:
				namedMyos.append([idx,val])	
			
			## buzz them diff depending on which player they are
			if idx == 1 and val.connected:
				val.vibrate('long')
				print("player1 -long-")
				print(val.emg)
				
			if idx == 2 and val.connected:
				val.vibrate('short')
				val.vibrate('short')
				print("player2 -shortx2-")
			
		print(namedMyos)

		while hub.running and myo.connected:

			playerData = []

			### dirty but it will work for now.

			try:
				p1 = namedMyos[0][1]
				player1 = {'name': 'player1','pose': p1.pose, 'orientation':p1.orientation, 'acceleration':p1.acceleration, 'arm':p1.arm, 'gyro':p1.gyroscope}
				#print(player1)
				playerData.append(player1)
			except IndexError:
				pass

			try:
				p2 = namedMyos[1][1]
				player2 = {'name': 'player2','pose': p2.pose, 'orientation':p2.orientation,'acceleration':p2.acceleration, 'arm':p2.arm, 'gyro':p2.gyroscope}
				#print(player2)
				playerData.append(player2)
			except IndexError:
				pass

			#print("TOSEND",sendToHelper)
			#print(" ")
			
			parse_data(playerData)
			time.sleep(0.1) ## this is just here to slow it down while debugging
			
		print("Disconnecting")

	except KeyboardInterrupt:
		print("KeyboardInterrupt")
	finally:
		print("Shutting Down Hub")
		hub.shutdown()


if __name__ == "__main__":
	main()
