#include <iostream>
#include <climits>
using namespace std;

class Elicopter
{
private:
    string model;
    string firma;
    int vitezaMax;
    int inaltimeMax;
    int pret;
    int raport;

public:
    Elicopter(string mod = "NULL", string firm = "NULL", int vit = 0, int inalt = 0, int prt = 0)
    {
        model = mod;
        firma = firm;
        vitezaMax = vit;
        inaltimeMax = inalt;
        pret = prt;
         raport = (vitezaMax != 0) ? pret / vitezaMax : 0;

        // (vitezaMax != 0) ? raport = pret / vitezaMax : raport = 0;

        // if(vitezaMax != 0)
        //     raport = pret/vitezaMax;
        // else
        //     raport = 0;
    }

    void afisare()
    {
        cout << "Modelul elicopterului: " << model << endl;
        cout << "Firma: " << firma << endl;
        cout << "Viteza maxima a elicopterlui: " << vitezaMax << endl;
        cout << "Altitudinea maxima: " << inaltimeMax << endl;
        cout << "Pretul: " << pret << endl<< endl;
    }

    int getRaport(){
        return raport;
    }
};
Elicopter cautareRaport(Elicopter *tablou , int len){
    int maxim = INT_MIN;
    int maxim_ind = 0;
    for(int i=0; i<len; i++){
        if(tablou[i].getRaport() > maxim){
            maxim = tablou[i].getRaport();
            maxim_ind = i;
        }
    }
    return tablou[maxim_ind];
}

void adaugareTablou(Elicopter *tablou, int &len, Elicopter aux)
{
    tablou[len] = aux;
    len++;
}

void afisare2(Elicopter *tablou, int len)
{
    for (int i = 0; i < len; i++)
    {
        tablou[i].afisare();
    }
}
int main()
{
    int len = 0;
    Elicopter tablou[50];
    Elicopter e1("smurd", "Dacia", 200, 1000, 34000);
    Elicopter e2("blindat", "Aro", 400, 500, 70000);
    Elicopter e3("simplu", "Baza", 100, 200, 10000);
    // e2.afisare();
    adaugareTablou(tablou, len, e1);
    adaugareTablou(tablou, len, e2);
    adaugareTablou(tablou, len, e3);
    afisare2(tablou, len);
    Elicopter raportmax =cautareRaport(tablou, len);
    raportmax.afisare();
}
