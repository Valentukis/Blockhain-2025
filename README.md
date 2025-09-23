# Nuosavo hash'o idėja

Buvo atlikta pradinė versija v1.0 visiškai be AI įrankių, pasinaudojus jais padaryta maksimaliai patobulinta versija, naudojanti tą pačią pagrindinę idėją

---

##  Pseudo-kodas
    function hash_own(input_string):
    SEED = constant_1
    bytes = EMPTY LIST

    for each character ch IN input_string:
        bytes.APPEND( ch_to_bits() )

    k = ( LENGTH(input_string) * constant_2 + Count_ones(bytes[0]) ) MOD LENGTH(bytes)
    IF k = 0 AND LENGTH(bytes) > 1:
        k = 1
    ROTATE_LEFT(bytes, k)

    hash = 64_bits(SEED)
    index = 0
    FOR each byte IN bytes:
        FOR bit FROM 0 TO 7:
            position = bit_index MOD 64
            hash[pos] = hash[pos] XOR byte[b]
            bit_index = bit_index + 1

    h = h * constant_3
    h = h XOR (h shift >> 29)
    h = h * constant_4

    return h

# Eksperimentinis tyrimas


