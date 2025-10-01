# Savo sugalvotas hash'as


Buvo atlikta pradinė versija v1.0 visiškai be DI įrankių, pasinaudojus jais padaryta maksimaliai patobulinta versija v2.0, išlaikant tą pačią unikalią idėją

---

## Idėja (sugalvota be DI)
Pagrindinę idėją galima aprašyti šitaip:  
1. Paimti 3-4 dideles konstantas, kurios padės išmaišime. Pačia pirmą priskiriame kaip seed'ą
2. Paimame iš vartotojo įvestį
3. Padalijame į byte'us
4. Persukame byte'us per k (k gauname per kintantį input'o ilgį ir konstantą ), k != 0
5. Gaminame hash'ą - paimame pradinį seed'ą, ir kiekvieną jo bit'ą lyginame su atitinkamu pramaišyto input'o bitu taikant XOR 
6. Gautą hash'ą d dar padauginame porą kartu iš konstantų ir perslenkam per kažkiek vienetų (pasirinkta 29, bet nebūtinai tiek)
7. Grąžiname 64bit hash'ą!

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
Buvo atlikti eskperimentiniai tyrimai pagal duotus reikalavimus. 

## 1. Hash funkcijos savybės

| Savybė                     | Rezultatas (Taip/Ne) |
|-----------------------------|-------------|
| Išvedimo dydis fiksuotas    | Taip / Ne   |
| Deterministiškumas          | Taip / Ne   |

---

## 2. Efektyvumas (konstitucija.txt)

| Eilučių skaičius | Vidutinis laikas (ms) |
|------------------|------------------------|
| 1                | … |
| 2                | … |
| 4                | … |
| 8                | … |
| ...              | … |
| 512              | … |


### Grafikas
čia įterpti grafiką*

---


## 3. Kolizijų paieška

Sugeneruota po **100 000 atsitiktinių string porų**, skirtingo ilgio (10, 100, 500, 1000 simbolių).  
Patikrinta, kiek jų hash’ai sutapo.

| String ilgis | Kolizijų skaičius | Kolizijų dažnumas (proc) |
|--------------|-------------------|--------------------------|
| 10           | ...               |  %                       |
| 100          | ...               |  %                       |
| 500          | ...               |  %                       |
| 1000         | ...               |  %                       | 

---

## 4. Lavinos efektas

Sugeneruota **100 000 porų**, kurios skiriasi tik vienu simboliu.  
Skirtumai matuoti **bitų** ir **hex** lygmeniu.

### Bitų lygmuo

| Min skirtumas (%) | Max skirtumas (%) | Vidutinis (%) |
|-------------------|-------------------|---------------|
| …                 | …                 | …             |

### Hex lygmuo

| Min skirtumas (%) | Max skirtumas (%) | Vidutinis (%) |
|-------------------|-------------------|---------------|
| …                 | …                 | …             |

---

## 5. Negrįžtamumo demonstracija

Metodas: `prideti`  
Pademonstruota, kad neįmanoma atkurti pradinio input vien iš hash’o.

| Įvestis | Naudotas salt | Hash rezultatas | Ar įmanoma atkurti input? |
|-----------------|---------------|-----------------|---------------------------|
| "test"          | abc123        | …               | Ne                        |
| "slaptas"       | xyz789        | …               | Ne                        |

---

## 6. Išvados

- **Stiprybės**: prideti
- **Trūkumai**: prideti

