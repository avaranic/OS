// e simboluri ASCII. Cifrele din șir formează numere, scrise în baza 10. Să se
// determine suma acestor numere./
#include <iostream>
#include <ctype.h>
#include <string.h>
using namespace std;

int main()
{
    char sir[100];
    int nr = 0;
    fgets(sir, 100, stdin);
    int s = 0;
    cout << "len= " << strlen(sir);
    for (int i = 0; i < strlen(sir); i++)
    {
        if (isdigit(sir[i]) != 0)
        {
            nr = nr * 10 + (sir[i] - '0');
        }
        else
        {
            // cout << nr << endl;
            s = s + nr;
            nr = 0;
        }
    }
    cout << "suma este:" << s << endl;
    return 0;
}
