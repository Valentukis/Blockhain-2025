#include <iostream>
#include <fstream>
#include <bitset>
using namespace std;

int main() {

    int ivesties_tipas;
    string ivestis, ivesties_failas, ivestis_bin;

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
        ivestis_bin += bits.to_string();

    }

    cout << ivestis_bin;


}




