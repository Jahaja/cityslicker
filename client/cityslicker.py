import socket
import array
import time

class Cityslicker(object):
	def __init__(self, host="localhost", port=8082):
		self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.sock.connect((host, port))

	def get_ids_by_bounding_box(self, minlat, maxlat, minlng, maxlng):
		arr = array.array("f", [minlat, maxlat, minlng, maxlng])
		self.sock.sendall(arr.tostring())

		resarr = array.array("i", self.sock.recv(1024))
		return resarr.tolist()

if __name__ == '__main__':
	cs = Cityslicker()
	start = time.time()
	num = 100
	for x in xrange(num):
		print len(cs.get_ids_by_bounding_box(
			59.7826690674, 60.9674186707, 
			17.5203819275, 17.7878952026
		))

	elapsed = (time.time() - start) * 1000
	print "Elapsed time for %d calls: %.2f ms (%.2f ms per req)" % (num, elapsed, elapsed / num)


