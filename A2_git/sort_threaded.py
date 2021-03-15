import sys
import operator
import heapq
from heapq import heapify, heappush, heappop
import threading
import time

sort_col_index = []
#####################################################################################

def readmeta():
	f = open("metadata.txt", "r")
	tuple_size = 0
	col_names = []
	col_sizes = []

	for x in f:
	   temp = x.split(',')
	   tuple_size += int(temp[1])
	   col_names.append(temp[0])
	   col_sizes.append(int(temp[1]))

	return tuple_size , col_names , col_sizes

#####################################################################################
#convert file lines to list
def line_to_list(col_sizes,row):
	temp = []
	i = 0
	for size in col_sizes:
	  	temp.append(row[i:i+size])
	  	i += size + 2
	return temp

#####################################################################################
def sortandwrite(file_name , array  ,rev):
	array.sort(key = operator.itemgetter(*sort_col_index),reverse=rev)

	with open("temp_files/" + str(file_name), "w") as f:
		for row in array:
			for i in range(len(row)-1):
				f.write(str(row[i]) + "  ")	#store in space seaprated format
			f.write(str(row[len(row)-1]) + "\n")	#write last columne without 2 eextra spaces

#####################################################################################

def createlists(mm_tuples , col_sizes , col_names , thread_count):
	#print(sys.argv)
	f = open(sys.argv[1], "r")

	list_n = 0
	row_count = 0
	array = []
	threads = []

	#decide sorting order
	rev = False
	if sys.argv[5].lower() == "desc":
		rev=True

	#store indices of cols parameters for sorting
			
	for i in range(6,len(sys.argv)):		
		sort_col_index.append(col_names.index(sys.argv[i]))

	file_name = 0	#decides name for sorted lists
	
	print("max tuples allowed in  M/M: ",mm_tuples)
	thread_tuple = int(mm_tuples / thread_count)
	print("tuples in 1 thread: ",thread_tuple)

	cur_batch = []			#store current batch of sublists

	for row in f:
		#M/M has free space
		if len(array) <	 thread_tuple:	
			temp = line_to_list(col_sizes,row)	
			array.append(temp)

		#memory is full so store sorted list
		else:
			#create thread for sorting and writing
			if len(cur_batch) == thread_count:
				threads = []

				for arr in cur_batch:
					file_name += 1			
					t = threading.Thread(target = sortandwrite, args = [file_name , arr ,rev])
					t.start()
					threads.append(t)
					print("sorting list ", file_name, "of size ", len(arr))
					
				#move ahead after completing threads
				for thread in threads:
					thread.join()

				cur_batch = [array]
				
			else:
				cur_batch.append(array)

			row_count=0
			#print("Sorted list ",file_name, " with number of rows ",len(array))			
			array = []			#empty RAM					
			array.append(temp)
	
	#sort remaining elements in batch
	threads = []
	for arr in cur_batch:
		file_name += 1			
		t = threading.Thread(target = sortandwrite, args = [file_name , arr ,rev])
		t.start()
		threads.append(t)
		print("sorting list ", file_name, "of size ", len(arr))
		
	#move ahead after completing threads
	for thread in threads:
		thread.join()

	#sort remaining elements in array not appended to batch
	file_name += 1
	t = threading.Thread(target = sortandwrite, args = [file_name , array ,rev])
	t.start()
	t.join()
	print("sorting list ", file_name, "of size ", len(array))
	
	f.close()

	return file_name

#####################################################################################
#class for heap objects
class heapObj(object):
    def __init__(self, val):
    	self.val = val

    def __lt__(self, other):

    	if sys.argv[5].lower() == "desc":
    		for i in sort_col_index:
	    		if(self.val[0][i] != other.val[0][i]):
	                    return self.val[0][i] > other.val[0][i]

    	else:
	    	for i in sort_col_index:
	    		if(self.val[0][i] != other.val[0][i]):
	                    return self.val[0][i] < other.val[0][i]

#####################################################################################	

def mergelists(mm_tuples , col_sizes , col_names , list_count):
	file_dict = {}			#file name-->file pointer
	open_file_count=0

	#create file dictionary
	for file in range(1,list_count+1):
		f = open("temp_files/" + str(file), "r")
		file_dict[str(file)] = f
		open_file_count += 1

	heap = []
	heapify(heap)

	#push 1 element from each file
	for file in range(1,list_count+1):
		file = str(file)
		key = file_dict[file].readline()
		#file has no entries left (not necessary to check)
		if key:
			key = line_to_list(col_sizes,key)
			heappush(heap, heapObj([key,file]))
		else:
			file_dict[file].close()
			open_file_count -= 1		#if a file is empty close it and decrement count
	

	op = open(str(sys.argv[2]), "w")		#output file

	n = 0
	while open_file_count > 0 and len(heap) > 0:
		 value = heappop(heap).val 			#[[row][fname]]

		 for i in range(len(value[0])-1):
		 	op.write(str(value[0][i]) + "  ")	#store in space seaprated format
		 op.write(str(value[0][len(value[0])-1]) + "\r\n")

		 next_row = file_dict[value[1]].readline()

		 if next_row:	#file has more elements
		 	next_row = line_to_list(col_sizes,next_row)
		 	heappush(heap, heapObj([next_row,value[1]]))

		 else:
		 	open_file_count -= 1
		 	file_dict[value[1]].close()
		 	print("file merged completely ",value[1])
		 n +=1
	
	print("number of tuples written: ",n)


#####################################################################################

def main():
	#./sort input.txt output.txt 50 asc C1 C2
	print("start execution")

	tuple_size , col_names , col_sizes = readmeta()
	#print(tuple_size,"\t",col_names)

	mm_size = int(sys.argv[3])*1024*1024			#MB to BYTES
	mm_tuples = int(mm_size/tuple_size)		#number of tuples possible in Main Memory
	
	thread_count = int(sys.argv[4])
	print("threads : ",thread_count)
	
	print("start Phase 1")
	list_count = createlists(mm_tuples,col_sizes,col_names,thread_count)
	print("End Phase 1\n")
	
	if((list_count * tuple_size) > mm_tuples):		#if heap cannot be formed for all sorted lists
		print("insufficient Main memory size")
		return

	print("Start Phase 2")
	mergelists(mm_tuples , col_sizes , col_names , list_count)
	print("End Phase 2")

	print("end execution")
	
#####################################################################################
if __name__ == "__main__":

	try:
		start = time.time()

		main()

		end= time.time()
		print("\ntime taken: ",(end-start)," seconds")
	except:
		print("Invalid input(s)")
		sys.exit()
