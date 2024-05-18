import math

TABLE_N_SAMPLES = 2048
TABLE_BITS_PER_SAMPLE = 16

# FILTERS
# Designs : http://shepazu.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html

def design_low_pass(f0,Q):
    # pulsation :
    sampling_rate = 44100
    omega0 = 2*math.pi*f0/sampling_rate
    alpha = math.sin(omega0)/2/Q
    b = [(1-math.cos(omega0))/2,1-math.cos(omega0),(1-math.cos(omega0))/2]
    a = [1+alpha,-2*math.cos(omega0),1-alpha]
    return b,a

def design_high_pass(f0,Q):
    # pulsation :
    sampling_rate = 44100
    omega0 = 2*math.pi*f0/sampling_rate
    alpha = math.sin(omega0)/2/Q
    b = [(1+math.cos(omega0))/2,-(1+math.cos(omega0)),(1+math.cos(omega0))/2]
    a = [1+alpha,-2*math.cos(omega0),1-alpha]
    return b,a

def design_band_pass(f0,Q):
    # pulsation :
    sampling_rate = 44100
    omega0 = 2*math.pi*f0/sampling_rate
    alpha = math.sin(omega0)/2/Q
    b = [alpha,0,-alpha]
    a = [1+alpha,-2*math.cos(omega0),1-alpha]
    return b,a

def design_low_shelf(f0,dB_gain):
    # pulsation :
    sampling_rate = 44100
    omega0 = 2*math.pi*f0/sampling_rate
    cos_omega0 = math.cos(omega0)
    S = 1
    A = 10**(dB_gain/40)
    alpha = math.sin(omega0)/2*math.sqrt((A+1/A)*(1/S-1)+2)
    b = [
        A*((A+1)-(A-1)*cos_omega0+2*(A**0.5)*alpha),
        2*A*((A-1)-(A+1)*cos_omega0),
        A*((A+1)-(A-1)*cos_omega0-2*(A**0.5)*alpha)
    ]
    a = [
        (A+1)+(A-1)*cos_omega0+2*(A**0.5)*alpha,
        -2*((A-1)+(A+1)*cos_omega0),
        (A+1)+(A-1)*cos_omega0-2*(A**0.5)*alpha
    ]
    return b,a

def design_high_shelf(f0,dB_gain):
    # pulsation :
    sampling_rate = 44100
    omega0 = 2*math.pi*f0/sampling_rate
    cos_omega0 = math.cos(omega0)
    S = 1
    A = 10**(dB_gain/40)
    alpha = math.sin(omega0)/2*math.sqrt((A+1/A)*(1/S-1)+2)
    b = [
        A*((A+1)+(A-1)*cos_omega0+2*(A**0.5)*alpha),
        -2*A*((A-1)+(A+1)*cos_omega0),
        A*((A+1)+(A-1)*cos_omega0-2*(A**0.5)*alpha)
    ]
    a = [
        (A+1)-(A-1)*cos_omega0+2*(A**0.5)*alpha,
        2*((A-1)-(A+1)*cos_omega0),
        (A+1)-(A-1)*cos_omega0-2*(A**0.5)*alpha
    ]
    return b,a

def apply_iir_filter(table,numerator,denominator=[1]):
    """
    https://en.wikipedia.org/wiki/Infinite_impulse_response
    numerator = [b0,b1,...,bP]
    denominator = [a0,a1,...,aQ]
    """
    sound_in = table*10    
    n_samples = len(sound_in)
    sound_out = [0]*n_samples
    for n in range(n_samples):
        for i in range(len(numerator)):
            i_in = n-i
            if i_in>=0:
                sound_out[n] += numerator[i]*sound_in[i_in]
        for i in range(1,len(denominator)):
            i_out = n-i
            if i_out>=0:
                sound_out[n] -= denominator[i]*sound_out[i_out]
        sound_out[n] /= denominator[0]
    return sound_out[-TABLE_N_SAMPLES:]

# NON LINEAR

def apply_overdrive(table,factor):
    maxi = max(abs(x) for x in table)
    return [math.atan(factor*x/maxi)*2/math.pi for x in table]

# FINAL QUANTIZATION

def final_quantization(table):
    maxi = max(abs(x) for x in table)
    return [int((2**(TABLE_BITS_PER_SAMPLE-1)-1)*x/maxi) for x in table]