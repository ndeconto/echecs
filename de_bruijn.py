#genere des suites de de bruijn

NB_BITS_MAX = 8
CONNU = [False] * 2**NB_BITS_MAX

def value(suite, dep, size):
    s = 0
    for i in range(dep, dep + size):
        s = (s << 1) + suite[i]
    return s

def de_bruijn(suite, n, nb_bits, result, juste_un=False):

    if n == (1 << nb_bits):
        if suite[n - 1]:
            result.append(suite[:])
            return True
        return False

    for i in [0, 1]:
        suite[n] = i
        temp = value(suite, n - nb_bits + 1, nb_bits)
        if CONNU[temp] == False:
            CONNU[temp] = True
            if de_bruijn(suite, n + 1, nb_bits, result, juste_un) and juste_un:
                return True
            CONNU[temp] = False
        

    
def compute(nb_bits, juste_un=False):
    for i in range(2**NB_BITS_MAX): CONNU[i] = False
    s = [0] * 2**nb_bits
    a = []
    CONNU[0] = True
    de_bruijn(s, nb_bits, nb_bits, a, juste_un)
    return a
    #for i in a : print i

def find_index(suite, nb_bits):
    P = 1 << nb_bits
    r = [0] * P
    for i in range(P - nb_bits + 1):
        r[value(suite, i, nb_bits)] = i
    for i in range(1, nb_bits):
        r[value(suite[-nb_bits + i:] + suite[:i], 0, nb_bits)] = P - nb_bits + i
        
    return r
        
    
def bit_scan(nb, key, table, nb_bits):
    return table[(((nb & (-nb)) * key) & ((1 << (1 << nb_bits)) - 1))
                  >> ((1 << nb_bits) - nb_bits)]

#test
#for i in compute(4): print i

#utile
suite = compute(6, True)[0]
key = value(suite, 0, 64)
T = find_index(suite, 6)
print "key =", key, "\nhashtable =", T
