#!/usr/bin/python3
import struct

TABLE_N_SAMPLES = 2048
TABLE_BITS_PER_SAMPLE = 16

def read_wav(filename,channel=0):
    """
    Reads a WAV file
    Parameters
    ----------
    filename : file name
    channel : the channel to be return, default is 0 (left)
    Returns
    -------
    sound : list of int
    """
    # WAV is little endian, so struct format is '<'
    f = open(filename, 'rb')
    # 'RIFF' block
    assert 'RIFF'==struct.unpack('<4s',f.read(4))[0].decode('UTF-8')
    struct.unpack('<L',f.read(4))[0]
    assert 'WAVE'==struct.unpack('<4s',f.read(4))[0].decode('UTF-8')
    # 'fmt ' block
    assert 'fmt '==struct.unpack('<4s',f.read(4))[0].decode('UTF-8')
    fmt_size_n = struct.unpack('<L',f.read(4))[0]      # block size = 16
    assert 1==struct.unpack('<H',f.read(2))[0]         # PCM format
    n_channels = struct.unpack('<H',f.read(2))[0]      # number of channels
    sampling_rate = struct.unpack('<L',f.read(4))[0]   # number of samples per second
    byte_rate = struct.unpack('<L',f.read(4))[0]       # number of bytes per second
    alignment = struct.unpack('<H',f.read(2))[0]       # block alignment
    bits_per_sample = struct.unpack('<H',f.read(2))[0] # number of bits per sample
    # check ' fmt' header
    assert bits_per_sample in [16,24,32], f'Unimplemented sample size {bits_per_sample}'
    if ((bits_per_sample//8*n_channels)!=alignment
        or byte_rate!=sampling_rate*alignment):
        print('/!\\Warning /!\\ Inconsistent header')
    if sampling_rate!=44100:
        print('/!\\Warning /!\\ Sampling frequency is not 44100')
    # extra 'fmt ' data
    for i in range(fmt_size_n-16):
        struct.unpack('<B',f.read(1))[0]
    # 'data' block
    assert 'data'==struct.unpack('<4s',f.read(4))[0].decode('UTF-8')
    n_bytes = struct.unpack('<L',f.read(4))[0] # total number of bytes
    n_samples = n_bytes//(n_channels*bits_per_sample//8)
    sound = [0]*n_samples
    for i in range(n_samples):
        for i_channel in range(n_channels):
            if bits_per_sample==16:
                sample = struct.unpack('<h',f.read(2))[0]
            elif bits_per_sample==24:
                b = f.read(3)
                sample = struct.unpack('<i',b+(b'\0' if b[2]<128 else b'\xff'))[0]
            elif bits_per_sample==32:
                sample = struct.unpack('<l',f.read(4))[0]
            if i_channel==channel:
                sound[i] = sample
    f.close()
    return sound

def write_wav(filename,table):
    """
    Writes a mono WAV file
    Parameters
    ----------
    filename : file name
    table : list of 16 bit ints
    Returns
    -------
    None
    """
    assert len(table)==TABLE_N_SAMPLES and all(isinstance(e,int) 
        and -2**(TABLE_BITS_PER_SAMPLE-1)<=e<2**(TABLE_BITS_PER_SAMPLE-1) for e in table
    )
    # prepare writing :
    n_channels = 1
    sampling_rate = 44100
    bytes_per_sample = 2 # 16bit PCM
    byte_rate = n_channels*sampling_rate*bytes_per_sample
    n_bytes = n_channels*TABLE_N_SAMPLES*bytes_per_sample
    # WAV is little endian, so struct format is '<'
    f = open(filename, 'wb')
    # 'RIFF' block
    f.write(struct.pack('<4s','RIFF'.encode('UTF-8')))
    f.write(struct.pack('<L',36+n_bytes)) # header_size-8 = 36
    f.write(struct.pack('<4s','WAVE'.encode('UTF-8')))
    # 'fmt ' block
    f.write(struct.pack('<4s','fmt '.encode('UTF-8')))
    f.write(struct.pack('<L',16))                          # block size = 16
    f.write(struct.pack('<H',1))                           # PCM format = 1
    f.write(struct.pack('<H',n_channels))                  # number of channel
    f.write(struct.pack('<L',sampling_rate))               # number of samples per second
    f.write(struct.pack('<L',byte_rate))                   # number of bytes per second
    f.write(struct.pack('<H',bytes_per_sample*n_channels)) # block alignment
    f.write(struct.pack('<H',8*bytes_per_sample))          # number of bits per sample
    # 'data' block
    f.write(struct.pack('<4s','data'.encode('UTF-8')))
    f.write(struct.pack('<L',n_bytes)) # total number of bytes
    #
    for sample in table:
        f.write(struct.pack('<h',sample))
    f.close()


H_FILE_HEADER = """
#ifndef EK_TABLES_H_
#define EK_TABLES_H_

#include "ek_config.h"

#define N_TABLES {}
#define TABLE_N_SAMPLES {}
#define TABLE_PHASE_SHIFT {}
#define CHANGE_TABLE_OFFSET 256 // due to max bluetooth packet

"""

H_FILE_FOOTER = """
static int16_t *tables[N_TABLES] = {{
    {}
}};

#endif /* EK_TABLES_H_ */
"""

TABLE_TEMPLATE = """
static {}int16_t {}[TABLE_N_SAMPLES]  =
{{{}
}};
"""

def table_to_string(name,table,is_const):
    values_by_line = 32
    data = ''
    for i in range(len(table)):
        if i%values_by_line==0:
            data += '\n    '
        data += '{:4d}, '.format(table[i])
    return TABLE_TEMPLATE.format('const ' if is_const else '',name,data)

def write_tables_for_c(tables):
    n_tables = len(tables)
    f = open('ek_tables.h','wt')
    print(H_FILE_HEADER.format(n_tables,TABLE_N_SAMPLES,32-len(f'{TABLE_N_SAMPLES:b}')+1),file=f)
    for i in range(n_tables):
        name,table = tables[i]
        print(f"// {name}",file=f)
        print(table_to_string(f'table_{i:04d}',table,i!=0),file=f)
    print(H_FILE_FOOTER.format(',\n    '.join(f'table_{i:04d}' for i in range(n_tables))),file=f)
    f.close()