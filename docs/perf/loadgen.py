import json
import matplotlib
import matplotlib.pyplot as plot
import subprocess
import pandas as pd
import re
import time
import random
import statistics

# load_url = "http://localhost:8080/arithmetic/prime?num=5000003"
#load_url = "http://localhost:8080/arithmetic/fibonacci?num=100"
#load_url = "http://localhost:8080/hello"

#load_url = "/arithmetic/fibonacci?num=100"

load_url = "http://127.0.0.1:4096/set"

def run_load_ab(num_requests: int, num_concurrent: int, url: str, method = 'GET', postdata = 'a.json'):
	throughput_regex = re.compile(
		r"^Requests per second:\s*([0-9]+\.[0-9]+)\s*\[\#\/sec]\s*\(mean\)\s*$",
		re.MULTILINE)

	text = ""

	if method == 'GET':
		result = subprocess.run(
			['ab',
				'-n', str(num_requests),
				'-c', str(num_concurrent),
				url
			],

			stdout = subprocess.PIPE,
			stderr = subprocess.STDOUT
		)
		text = result.stdout.decode()

	elif method == 'POST':
		result = subprocess.run(
			['ab',
				'-n', str(num_requests),
				'-c', str(num_concurrent),
				'-T', 'application/json',
				'-p', postdata,
				url
			],

			stdout = subprocess.PIPE,
			stderr = subprocess.STDOUT
		)
		text = result.stdout.decode()

	matches = throughput_regex.search(text)

	if not matches:
		print(text)
		raise Exception("Could not extract information");

	return float(matches[1])



server_num_threads = [ 1, 10, 20, 50 ]
client_num_concurrency =  list(range(1, 10, 1)) + list(range(10, 110, 10))

client_num_concurrency_httperf =  [ 10, 20, 50, 100, 200, 300, 400, 500, 600, 700 ]
column_names = ["num_threads", "num_concurrency", "throughput"]

def create_config(threads: int):
	filename = f"/tmp/test-{threads}.json"
	with open(filename, "w") as file:
		file.write(json.dumps({
			"client_listener_port": 4096,
			"max_client_connections": 128,
			"max_concurrency": threads,
			"max_server_connections": 20,
			"max_peers": 10,
			"merge_attributes": [
				{ "key": "ul", "attr": "arr_union" }
			],
			"peer_retention": 0.5,
			"peers": [
				{ "address": "127.0.0.1", "peer_port": 5095, "client_port": 5096 }
			],
			"server_listener_port": 4095,
			"sync_mode": "broadcast"
		}))
		file.close()
	return filename

def run_experiment():
	result = []

	for num_threads in server_num_threads:
		thread_result = pd.DataFrame(columns=column_names)
		filename = create_config(num_threads)
		p = subprocess.Popen(
			["../../build/server", filename],
			stderr=subprocess.DEVNULL
		)

		for num_concurrency in client_num_concurrency:
			print(f"Running load: num_threads: {num_threads} num_concurr: {num_concurrency}")
			throughput = statistics.fmean([
				run_load_ab(1000, num_concurrency, load_url, method='POST') for _ in range(3)
			])
			thread_result.loc[len(thread_result)] = {
				"num_threads": num_threads,
				"num_concurrency": num_concurrency,
				"throughput": throughput
			}

		p.kill()
		p.wait()

		result.append(thread_result)
		plot.plot(
			list(thread_result["num_concurrency"]),
			list(thread_result["throughput"]),
			label = f"Thread pool size: {num_threads}")
	plot.legend()
	plot.xlabel("Concurrent Requests")
	plot.ylabel("Throughput (Req/sec)")
	plot.show()

# run_experiment()
run_experiment()