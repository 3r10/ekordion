import math,random

TABLE_N_SAMPLES = 2048

# Float computations in [-1.0,1.0] interval

def generate_white_noise():
    return [random.random()*2-1 for _ in range(TABLE_N_SAMPLES)]

def generate_sine(multiplier):
    return [math.sin(2*math.pi*multiplier*i/TABLE_N_SAMPLES) for i in range(TABLE_N_SAMPLES)]

def generate_triangle(multiplier,increase=0.5):
    return [
        2*((multiplier*i)%TABLE_N_SAMPLES/TABLE_N_SAMPLES/increase)-1
        if ((multiplier*i)%TABLE_N_SAMPLES)<increase*TABLE_N_SAMPLES
        else 1-2*(((multiplier*i)%TABLE_N_SAMPLES-increase*TABLE_N_SAMPLES)/TABLE_N_SAMPLES/(1-increase))
        for i in range(TABLE_N_SAMPLES)
    ]

def generate_sawtooth(multiplier):
    return generate_triangle(multiplier,1)

def generate_pwm(multiplier,duty=0.5):
    # https://en.wikipedia.org/wiki/Pulse-width_modulation
    return [1.0 if ((multiplier*i)%TABLE_N_SAMPLES)<duty*TABLE_N_SAMPLES else -1.0 for i in range(TABLE_N_SAMPLES)]

# Floats may have a modulus greater that 1... 
# Will be normalized and quantized afterward

def generate_additive(amplitudes,generate=generate_sine):
    table = [0.0]*TABLE_N_SAMPLES
    for i_harmonic in range(len(amplitudes)):
        harmonic = generate(i_harmonic+1)
        for i in range(TABLE_N_SAMPLES):
            table[i] += amplitudes[i_harmonic]*harmonic[i]
    return table

def generate_sum(tables,amplitudes):
    assert len(tables)>0 and len(tables)==len(amplitudes) and all(len(table)==len(tables[0]) for table in tables) 
    table = [0]*len(tables[0])
    for i_table in range(len(tables)):
        for i in range(len(table)):
            table[i] += amplitudes[i_table]*tables[i_table][i]
    return table