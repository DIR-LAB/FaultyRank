import os
import math

# change number of clients
NUMBER_OF_CLIENTS = 10
TOTAL_LINES = 112020366
LINES_PER_PARTITION  = math.ceil(TOTAL_LINES/NUMBER_OF_CLIENTS)

input_file = "anon-archive-fs1.txt"
input_log = open(input_file, 'r')
print ("reading lanl log")
content = input_log.readlines()
print ("complete reading lanl log")



# todo: make this into one for loop for the number of clients.

lanl_log_client1 = "dataset_client1.txt"
lanl_log_client2 = "lanl_log_client2.txt"
lanl_log_client3 = "/client/faultyrank/multiple_clients/lanl_log_client3.txt"
lanl_log_client4 = "/client/faultyrank/multiple_clients/lanl_log_client4.txt"
lanl_log_client5 = "/client/faultyrank/multiple_clients/lanl_log_client5.txt"
lanl_log_client6 = "/client/faultyrank/multiple_clients/lanl_log_client6.txt"
lanl_log_client7 = "/client/faultyrank/multiple_clients/lanl_log_client7.txt"
lanl_log_client8 = "/client/faultyrank/multiple_clients/lanl_log_client8.txt"
lanl_log_client9 = "/client/faultyrank/multiple_clients/lanl_log_client9.txt"
lanl_log_client10 = "/client/faultyrank/multiple_clients/lanl_log_client10.txt"

output_1 = open(lanl_log_client1, 'w')
output_2 = open(lanl_log_client2, 'w')
output_3 = open(lanl_log_client3, 'w')
output_4 = open(lanl_log_client4, 'w')
output_5 = open(lanl_log_client5, 'w')
output_6 = open(lanl_log_client6, 'w')
output_7 = open(lanl_log_client7, 'w')
output_8 = open(lanl_log_client8, 'w')
output_9 = open(lanl_log_client9, 'w')
output_10 = open(lanl_log_client10, 'w')

# first portion of lanl logs for client 1
print("writing first portion")
start1 = 0
end1 = int(LINES_PER_PARTITION)
portion_1 = ''.join(content[start1:end1])
output_1.write(portion_1)
print("completed first portion")

# second portion of lanl logs for client 2
print("writing second portion")
start2 = int(end1)
end2 = int(start2 + LINES_PER_PARTITION)
portion_2 = ''.join(content[start2:end2])
output_2.write(portion_2)
print("completed second portion")

# third portion of lanl logs for client 3
print("writing third portion")
start3 = int(end2)
end3 = int(start3 + LINES_PER_PARTITION)
portion_3 = ''.join(content[start3:end3])
output_3.write(portion_3)
print("completed third portion")

# fourth portion of lanl logs for client 4
print("writing fourth portion")
start4 = int(end3)
end4 = int(start4 + LINES_PER_PARTITION)
portion_4 = ''.join(content[start4:end4])
output_4.write(portion_4)
print("completed fourth portion")

# fifth portion of lanl logs for client 5
print("writing fifth portion")
start5 = int(end4)
end5 = int(start5 + LINES_PER_PARTITION)
portion_5 = ''.join(content[start5:end5])
output_5.write(portion_5)
print("completed fifth portion")

# sixth portion of lanl logs for client 6
print("writing sixth portion")
start6 = int(end5)
end6 = int(start6 + LINES_PER_PARTITION)
portion_6 = ''.join(content[start6:end6])
output_6.write(portion_6)
print("completed sixth portion")

# seventh portion of lanl logs for client 7
print("writing seventh portion")
start7 = int(end6)
end7 = int(start7 + LINES_PER_PARTITION)
portion_7 = ''.join(content[start7:end7])
output_7.write(portion_7)
print("completed seventh portion")

# eight portion of lanl logs for client 8
print("writing eight portion")
start8 = int(end7)
end8 = int(start8 + LINES_PER_PARTITION)
portion_8 = ''.join(content[start8:end8])
output_8.write(portion_8)
print("completed eight portion")

# ninth portion of lanl logs for client 9
print("writing ninth portion")
start9 = int(end8)
end9 = int(start9 + LINES_PER_PARTITION)
portion_9 = ''.join(content[start9:end9])
output_9.write(portion_9)
print("completed ninth portion")

# tenth portion of lanl logs for client 10
print("writing tenth portion")
start10 = int(end9)
end10 = int(start10 + LINES_PER_PARTITION)
portion_10 = ''.join(content[start10:end10])
output_10.write(portion_10)
print("completed tenth portion")



