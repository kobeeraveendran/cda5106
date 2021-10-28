from genericpath import isfile
import matplotlib.pyplot as plt
import pandas as pd
import math
import os
import subprocess

def calculate_aat(reads, read_misses, writes, write_misses, access_time, miss_latency = 100):
	return access_time + ((read_misses + write_misses) / (reads + writes)) * miss_latency

def cacti_search(df, cache_size, block_size, assoc):
	row = df[(df["Cache Size(kb)"] == cache_size // 1024) \
		  & (df["Block Size(bytes)"] == block_size) \
		  & (df["Associativity"] == assoc)]
	
	try:
		return row.iloc[0]["Access Time(ns)"]
	except IndexError:
		return None

	

if __name__ == "__main__":
	os.system("make clean && make")
	
	# lookup the CACTI table results for each configuration and make a dict of them here
	xls = pd.ExcelFile("../cacti_table.xls")
	df = xls.parse(xls.sheet_names[0])
	
	l1_size = [2 ** i for i in range (10, 21)]
	
	# group by assoc
	assocs = {
		1: [],
		2: [],
		4: [],
		8: [],
		'fa': []
	}

	g1_count = 0
	
	# graph 1
	for size in l1_size:
		for assoc in [1, 2, 4, 8, size // 32]:
			miss_rate = subprocess.run(
				["./sim_cache", '32', str(size), str(assoc), '0', '0', '0', '0', "gcc_trace.txt", '1'], 
				capture_output = True
			).stdout
			
			miss_rate = float(miss_rate) * 100

			print("(graph 1) size: {}\t | assoc: {}\t | miss rate (%): {}".format(size, assoc, miss_rate))
			
			assocs['fa' if assoc == size // 32 else assoc].append(miss_rate)
			
			g1_count += 1

		print("----------------------------------------------------------------------")

	print("\nGRAPH1 SIMULATIONS PLOTTED:", g1_count, '\n')
			
	# plot graph 1 experiment results
	plt.title("Graph 1: Size and Assoc. vs. Miss Rate")
	plt.xlabel("log2(Cache Size)")
	plt.ylabel("L1 Miss Rate (%)")
	
	l1_plot_size = [math.log(size, 2) for size in l1_size]

	for a in [1, 2, 4, 8, 'fa']:
		plt.plot(l1_plot_size, assocs[a], label = str(a))
		plt.legend(title = "Associativity", loc = "upper right")
		

	plt.savefig("graph_logs/graph1.png")
			
	# clear results dict and plots for next graph
	for key in assocs.keys():
		assocs[key] = []

	plt.clf()

	# graph 2
	
	# uses the same sizes and assocs from graph 1

	g2_count = 0

	for size in l1_size:
		for assoc in [1, 2, 4, 8, size // 32]:
			ht = cacti_search(df, size, 32, assoc if assoc != size // 32 else "FA")

			# if this entry isn't in the CACTI table, skip it as we can't compute the AAT
			if ht == None:
				continue
			line = subprocess.run(
				["./sim_cache", '32', str(size), str(assoc), '0', '0', '0', '0', "gcc_trace.txt", '2'], 
				capture_output = True
			).stdout.decode("utf-8")

			reads, read_misses, writes, write_misses = [int(arg) for arg in line.split(',')]

			aat = calculate_aat(reads, read_misses, writes, write_misses, ht)
			
			assocs['fa' if assoc == size // 32 else assoc].append((size, aat))
			print("(graph 2) size: {}\t | assoc: {}\t | AAT (ns): {}".format(size, assoc, aat))
			g2_count += 1

		print("----------------------------------------------------------------------")

	print("\nGRAPH2 SIMULATIONS PLOTTED:", g2_count, '\n')

	plt.title("Graph 2: Size and Assoc. vs. AAT")
	plt.xlabel("log2(Cache Size)")
	plt.ylabel("Average Access Time (ns)")

	for a in [1, 2, 4, 8, 'fa']:
		x = [math.log2(assocs[a][i][0]) for i in range(len(assocs[a]))]
		y = [assocs[a][i][1] for i in range(len(assocs[a]))]
		plt.plot(x, y, label = str(a))
		plt.legend(title = "Associativity", loc = "upper right")
		
	plt.savefig("graph_logs/graph2.png")
	plt.clf()
	
	# graph 3
	l1_size = [2 ** i for i in range(10, 19)]
	g3_count = 0

	rep_pols = {
		0: [], 
		1: [], 
		2: []
	}
	
	for size in l1_size:
		for rep_pol in [0, 1, 2]:
			ht = cacti_search(df, size, 32, 4)
			line = subprocess.run(
				["./sim_cache", '32', str(size), '4', '0', '0', str(rep_pol), '0', "gcc_trace.txt", '3'], 
				capture_output = True
			).stdout.decode("utf-8")

			reads, read_misses, writes, write_misses = [int(arg) for arg in line.split(',')]

			aat = calculate_aat(reads, read_misses, writes, write_misses, ht)

			rep_pols[rep_pol].append((size, aat))
			print("(graph 3) size: {}\t | rep_pol: {}\t | AAT (ns): {}".format(size, rep_pol, aat))
			g3_count += 1

		print("----------------------------------------------------------------------")

	print("\nGRAPH3 SIMULATIONS PLOTTED:", g3_count, '\n')

	plt.title("Graph 3: Size and Replacement Policy vs. AAT")
	plt.xlabel("log2(Cache Size)")
	plt.ylabel("Average Access Time (ns)")

	policy_mapping = {
		0: "LRU", 
		1: "PseudoLRU", 
		2: "Optimal"
	}

	for r in [0, 1, 2]:
		x = [math.log2(rep_pols[r][i][0]) for i in range(len(rep_pols[r]))]
		y = [rep_pols[r][i][1] for i in range(len(rep_pols[r]))]
		plt.plot(x, y, label = policy_mapping[r])
		plt.legend(title = "Replacement Policy", loc = "upper right")

	plt.savefig("graph_logs/graph3.png")
	plt.clf()
			
	# graph 4
	l2_size = [2 ** i for i in range(11, 17)]
	g4_count = 0

	inc_prop = {
		0: [], 
		1: []
	}
	
	for size in l2_size:
		for inclusion in [0, 1]:
			ht_l1 = cacti_search(df, 1024, 32, 4)
			ht_l2 = cacti_search(df, size, 32, 8)

			line = subprocess.run(
				["./sim_cache", '32', '1024', '4', str(size), '8', '0', str(inclusion), "gcc_trace.txt", '4'], 
				capture_output = True
			).stdout.decode("utf-8")

			line = line.split('\n')

			l1_reads, l1_readmisses, l1_writes, l1_writemisses = [int(arg) for arg in line[0].split(',')]
			l2_reads, l2_readmisses, l2_writes, l2_writemisses = [int(arg) for arg in line[1].split(',')]

			aat = ht_l1 + (l1_readmisses + l1_writemisses) / (l1_reads + l1_writes) * ht_l2 + \
				  (l2_readmisses / (l1_reads + l1_writes)) * 100

			inc_prop[inclusion].append((size, aat))
			print("(graph 3) size: {}\t | rep_pol: {}\t | AAT (ns): {}".format(size, rep_pol, aat))
			g4_count += 1

		print("----------------------------------------------------------------------")

	for inc in [0, 1]:
		x = [math.log2(inc_prop[inc][i][0]) for i in range(len(inc_prop[inc]))]
		y = [inc_prop[inc][i][1] for i in range(len(inc_prop[inc]))]
		plt.plot(x, y, label = "inclusive" if inc else "non-inclusive")
		plt.legend(title = "Inclusion Property", loc = "upper right")

	plt.savefig("graph_logs/graph4.png")
	plt.clf()