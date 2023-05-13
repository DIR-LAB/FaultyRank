import os
import math
from timeit import default_timer as timer

# todo: make this an argument input
# Path to the input file
file_size_map_file = "FaultyRank/client/lanl_log_client.txt"

# Lustre filesystem mount point
lustre_mount_point = "/mnt/skfs"

# Configure according to the number of OSTs in Lustre Cluster
SCALE_SIZE = 8

# Calculate the stripe size
STRIPE_SIZE_PER_OST = 65536 # 64 KB
STRIP_SIZE = SCALE_SIZE * STRIPE_SIZE_PER_OST
#STRIP_SIZE = 524288         # (0.5 MB) => 8 OST * 64 KB Stripe Size per OST
MAX_FILE_SIZE = 4194304     # 4 MB

LOG_PROGRESS_INTERVAL = 10000
FILE_PROGRESS_INTERVAL = 10000
ONE_MILLION = 1000000.0

num_dirs = 0
num_empty_dirs = 0
num_empty_files = 0
total_space = 0
num_lines_processed = 0
num_lines_processed_prev = 0 # this line needs to be changed
MAX_LINES_TO_READ = 30000
file_created = 0

input_log = open(file_size_map_file, 'r')

start = timer()
for line in input_log:
    if num_lines_processed == MAX_LINES_TO_READ:
        break

    if num_lines_processed <= num_lines_processed_prev:
        num_lines_processed += 1
        continue

    # if file_created > MAX_FILES_TO_CREATE:
    #     break

    data = line.split("|")

    permission = data[1]
    file_size = int(data[4])

    is_dir = permission[0]
    if is_dir == 'd':
        num_dirs += 1
        if file_size == 0:
            num_empty_dirs += 1

    else:
        if file_size == 0:
            num_empty_files += 1
            continue

        lanl_path = data[8].strip()
        log_path = lanl_path[1:]
        parsed_log_path = log_path.rsplit('/', 1)[0]
        dir_path = os.path.join(lustre_mount_point, parsed_log_path)

        if not os.path.exists(dir_path):
            os.makedirs(dir_path)
            # print ("dir path donot exist\n")

        file_path = os.path.join(lustre_mount_point, log_path)
        # print(file_path)
        # print("dir_path: {}, file_path: {}".format(dir_path, file_path))

        if file_size > STRIP_SIZE:
            if file_size > MAX_FILE_SIZE:
                file_size = MAX_FILE_SIZE                       # file size larger than 4MB is scaled down to 4MB
            file_size = int(math.ceil(file_size / SCALE_SIZE))

        if file_size == 0:
            file_size = 128

        total_space += file_size
        command = "dd if=/dev/zero of=" + file_path + " bs=" + str(file_size) + " count=1 status=none"
        os.system(command)
        file_created += 1

    num_lines_processed += 1
    if (file_created % FILE_PROGRESS_INTERVAL) == 0:
        print("created {}M files from the log.".format(float(file_created / ONE_MILLION)))

end = timer()
print("total time to finish the processing: {} sec.".format(end - start))
print("number of directory entries in the log: {}".format(num_dirs))
print("number of empty directory entries in the log: {}".format(num_empty_dirs))
print("number of empty files in the log: {}".format(num_empty_files))
print("previously line courser was in: {}".format(num_lines_processed_prev))
print("currently line courser is in: {}".format(num_lines_processed))
print("number of line processed in this run: {}".format(num_lines_processed - num_lines_processed_prev))
print("number of files created in this run: {}".format(file_created))
print("total space required: {}".format(total_space))
