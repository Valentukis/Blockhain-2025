#include <iostream>
#include <fstream>
#include <bitset>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <limits>
#include <iomanip>
using namespace std;
// 0x055D3F9EC866DB6C
// 0x054C33BBC571D86D
// 0x604016D6C571D86D
int main() {
    int ivesties_tipas;
    string ivestis, ivesties_failas;
    vector<bitset<8>> separate_bytes;
    uint64_t seed = 371928463890165017ull; 

    cout << "Kaip norėsite įvesti tekstą? [0 - rankinis įvedimas, 1 - įvedimas iš failo]" << endl;
    cin >> ivesties_tipas;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (ivesties_tipas == 0) {
        cout << "Iveskite hash'uojamą string'ą: " << endl;
        getline(cin, ivestis);
    }

    else if (ivesties_tipas == 1) {
        cout << "Iveskite failo pavadinimą formatu [pavadinimas.txt]: " << endl;
        getline(cin, ivesties_failas);
        ifstream input(ivesties_failas);
        getline(input, ivestis);
    }
     
    else {
        cout << "Neteisingas pasirinkimas.\n";
        return 0;
    }

    for (unsigned char c : ivestis) { 
        separate_bytes.push_back(bitset<8>(c));
    }

    //Permaisyt byetus
    if (!separate_bytes.empty()) {
        int k = (ivestis.size() * 781928401873 + separate_bytes.front().count()) % separate_bytes.size();
        if (k == 0 && separate_bytes.size() > 1) k = 1;     // Butinas persukimas
        rotate(separate_bytes.begin(), separate_bytes.begin() + k, separate_bytes.end());
    }

    std::bitset<64> hash(seed); //xor kiekviena bit'a is seed ir input
    int index = 0;
    for (const auto& byte : separate_bytes) {

        for (int b = 0; b < 8; ++b, ++index) {
        int pos = index % 64; 
        hash[pos] = hash[pos] ^ byte[b];
        }
    }

    uint64_t h = hash.to_ullong();
    cout  << "Hash (64-bit): 0x" << setw(16) << setfill('0') << hex << h << endl;

}