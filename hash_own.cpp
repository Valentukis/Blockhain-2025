#include <iostream>
#include <fstream>
#include <bitset>
#include <vector>
#include <cstdint>
#include <algorithm>

using namespace std;

int main() {

    int ivesties_tipas;
    string ivestis, ivesties_failas, ivestis_bin;
    vector <bitset<8>> seperate_bytes;
    uint64_t seed = 371928463890165017; 

    cout << "Kaip norėsite įvesti tekstą? [0 - rankinis įvedimas, 1 - įvedimas iš failo]" << endl;
    cin >> ivesties_tipas;

    if (ivesties_tipas == 0) {
        cout << "Iveskite hash'uojamą string'ą: ";
        cin >> ivestis;
    }

    else if (ivesties_tipas == 1) {
        cout << "Iveskite failo pavadinimą formatu [pavadinimas.txt]: ";
        cin >> ivesties_failas;
        
        ifstream input(ivesties_failas);
        input >> ivestis;
    }  


    for (char c: ivestis) {
        bitset<8> bits(c);
        
        seperate_bytes.push_back(bits);

    }

    for (auto n: seperate_bytes) {
        cout << n << " ";
    }
    cout << endl;
    int stumimas = (ivestis.size() * 4683726410483) % seperate_bytes[0].count();
    rotate(seperate_bytes.begin(), seperate_bytes.begin() + stumimas, seperate_bytes.end());

    for (auto n: seperate_bytes) {
        cout << n << " ";
    }

}




