#!/usr/bin/python
import gevent
from gevent.monkey import patch_all
patch_all()
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

	def close(self):
		self.sock.close()

if __name__ == '__main__':
	num = 1000
	def fetch_cities():
		cs = Cityslicker()
		start = time.time()
		
		for x in xrange(num):
			cs.get_ids_by_bounding_box(
				59.7826690674, 60.9674186707, 
				17.5203819275, 17.7878952026
			)

		elapsed = (time.time() - start) * 1000
		print "Elapsed time for %d calls: %.2f ms (%.2f ms per req)" % (num, elapsed, elapsed / num)

		cs.close()

	tot_start = time.time()
	num_jobs = 10
	jobs = [gevent.spawn(fetch_cities) for x in xrange(num_jobs)]
	gevent.joinall(jobs)
	tot_elapsed = (time.time() - tot_start) * 1000
	print "Total: %.2f ms (%.2f ms per job, %.2f ms per req)" % (tot_elapsed, tot_elapsed / num_jobs, tot_elapsed / (num_jobs * num))


