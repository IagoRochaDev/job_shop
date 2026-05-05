// #pragma once
// #include <iostream>
// using namespace std;

// class Solucao {
// public:
//     int makespan = 1e9;

//     void imprimir() {
//         cout << "\n===== MAKESPAN =====\n";
//         cout << makespan << endl;
//     }
// };

#pragma once
#include <iostream>
#include <vector>

using namespace std;

struct Solucao {
    int makespan;
    vector<int> topoOrder;
    vector<int> caminhoCritico;

    void imprimir() {
        cout << "===== ORDEM TOPOLOGICA =====\n";
        for (int v : topoOrder)
            cout << v << " ";

        cout << "\n\n===== MAKESPAN =====\n";
        cout << makespan << endl;

        cout << "\n===== CAMINHO CRITICO =====\n";
        for (int v : caminhoCritico)
            cout << v << " ";

        cout << endl;
    }
};