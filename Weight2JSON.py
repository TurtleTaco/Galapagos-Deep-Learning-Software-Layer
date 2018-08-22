import sys
import json
import struct

def binary(num):
        return ''.join(bin(ord(c)).replace('0b', '').rjust(8, '0') for c in struct.pack('!f', num))

def ieee754(data):
    # convert input IEEE-754 float data to binary represention
    # convert binary representation to hex, then store corresponding hex as decimal 
    # eg. float: 8.0 -> 
    #     binary represenation in IEEE-754 (default in python): 01000001000000000000000000000000 ->
    #     above binary in hex: 0x41000000 ->
    #     above hex in decimal: 1090519040
    hex_representation = hex(int(binary(data), 2))
    return int(hex_representation, 16)

__META__ = sys.argv[1]
__TXTFILE__ = sys.argv[2]

with open(__TXTFILE__, 'r') as myFile:
    content = myFile.readlines()

content = [x.strip() for x in content]

weights_json = {}
packets_json = []
packets_detail = {}
payload_json = []

for item in content:
    curr_payload = {}
    curr_payload['last'] = 15
    if (int(__META__) == 1):
        curr_payload['data'] = ieee754(float(item))
    else:
        curr_payload['data'] = int(item)
    curr_payload['keep'] = 255
    payload_json.append(curr_payload)

packets_detail['interface'] = 'axis_net'
packets_detail['width'] = 32
packets_detail['type'] = "flit"
packets_detail["payload"] = payload_json
packets_json.append(packets_detail)

weights_json["packets"] = packets_json

output_file = __TXTFILE__.replace(__TXTFILE__, __TXTFILE__ + '.json')
output_file = output_file.replace('TXT', 'JSON')
with open(output_file, 'w') as output_json:
    json.dump(weights_json, output_json)


