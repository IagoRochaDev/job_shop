#include <iostream>
#include <iomanip>
#include <sstream>

#include "instancia.hpp"
#include "solverJSP.hpp"

using namespace std;

int main() {

    double somaGap = 0.0;
    int totalInstancias = 0;

    cout << fixed << setprecision(2);

    for (int i = 1; i <= 40; i++) {

        stringstream ss;
        ss << "../instancias/ta";

        if (i < 10) ss << "0";
        ss << i << "Jsp.psi";

        string caminho = ss.str();

        cout << "\n=============================\n";
        cout << "Instancia: " << caminho << endl;

        Instancia inst;
        inst.ler_arquivo(caminho);

        SolverJSP solver;

        solver.jobs = inst.jobsCount;
        solver.machines = inst.machinesCount;

        int lb = solver.lowerBound(inst);

        Solucao sol = solver.resolver(inst);

        sol.imprimir();

        int makespan = sol.makespan;

        double gap = ((double)(makespan - lb) / lb) * 100.0;

        cout << "\n=============================\n";
        cout << "GAP (%): " << gap << endl;

        somaGap += gap;
        totalInstancias++;
    }

    cout << "\n=============================\n";
    cout << "MEDIA GAP (%): " << (somaGap / totalInstancias) << endl;

    return 0;
}
