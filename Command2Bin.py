import struct

cmd_conv = [6, 6, 3, 3, 1, 0, 4, 4, 1, 1, 1, 1];
cmd_addr = [2147483904, 36, 288, 0, 2148270080, 16, 2147491840, 72, 576];
cmd_mode = [0, 0];
cmd_pool = [4, 4, 0, 0, 0, 0, 0, 0];
cmd_rsvd = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0];

binFile = open('BIN/Command.bin', 'wb')
for i in range(len(cmd_conv)):
    binFile.write(struct.pack('>h', cmd_conv[i])) # h for short 2 bytes size data
for i in range(len(cmd_addr)):
    binFile.write(struct.pack('>L', cmd_addr[i])) # L for short 4 bytes size data
for i in range(len(cmd_mode)):
    binFile.write(struct.pack('>h', cmd_mode[i])) # h for short 2 bytes size data
for i in range(len(cmd_pool)):
    binFile.write(struct.pack('>h', cmd_pool[i])) # h for short 2 bytes size data
for i in range(len(cmd_rsvd)):
    binFile.write(struct.pack('>L', cmd_rsvd[i])) # L for short 4 bytes size data