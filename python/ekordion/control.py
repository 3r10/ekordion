import socket, ipywidgets

TABLE_N_SAMPLES = 2048
TABLE_BITS_PER_SAMPLE = 16
CUSTOM_TABLE_CODE = 0

client_socket = None

def bt_connect(mac_address):
    global client_socket
    client_socket = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
    try:
        client_socket.connect((mac_address,1))
        print('Connected')
    except:
        print('Unable to connect')

def bt_disconnect():
    global client_socket
    try:
        client_socket.close()
        print('Disconnected')
    except:
        print('Unable to disconnect')

def bt_send_message(data):
    # if len(data)>990, message is split in 2 parts by the underlying layer
    assert len(data)<=990 and all(isinstance(e,int) and 0<=e<256 for e in data)
    global client_socket
    client_socket.send(bytes(data))

def bt_send_custom_table(table):
    assert len(table)==TABLE_N_SAMPLES and all(isinstance(e,int) 
        and -2**(TABLE_BITS_PER_SAMPLE-1)<=e<2**(TABLE_BITS_PER_SAMPLE-1) for e in table
    )
    global client_socket
    table_bytes = []
    for e in table:
        table_bytes.append(e&255)
        table_bytes.append((e>>8)%256)
    for i in range(8):
        client_socket.send(bytes([CUSTOM_TABLE_CODE,i]+table_bytes[i*512:(i+1)*512]))
