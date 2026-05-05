#pragma once
#include "instancia.hpp"
#include "solucao.hpp"

#include <vector>
#include <queue>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <iostream>

using namespace std;

class SolverJSP {
public:

    int jobs, machines;

    int getId(int j, int m) {
        return j * machines + m;
    }

    struct Resultado {
        int makespan;
        vector<int> caminho;
        vector<int> topo;
    };

    int lowerBound(Instancia& inst) {

        int lb = 0;

        for (int j = 0; j < jobs; j++) {
            int soma = 0;

            for (int k = 0; k < machines; k++) {
                int m = inst.sequence[j][k];
                soma += inst.costs[j][m];
            }

            lb = max(lb, soma);
        }

        for (int m = 0; m < machines; m++) {
            int soma = 0;

            for (int j = 0; j < jobs; j++) {
                soma += inst.costs[j][m];
            }

            lb = max(lb, soma);
        }

        return lb;
    }

    Resultado avaliar(Instancia& inst, vector<vector<int>>& maquinas) {

        int n = jobs * machines;

        vector<vector<int>> adj(n);
        vector<int> indegree(n, 0);
        vector<int> dp(n, 0);
        vector<int> parent(n, -1);
        vector<int> peso(n, 0);
        vector<int> topo;

        for (int j = 0; j < jobs; j++) {
            for (int k = 0; k < machines; k++) {
                int m = inst.sequence[j][k];
                peso[getId(j, m)] = inst.costs[j][m];
            }
        }

        for (int j = 0; j < jobs; j++) {
            for (int k = 0; k < machines - 1; k++) {

                int u = getId(j, inst.sequence[j][k]);
                int v = getId(j, inst.sequence[j][k + 1]);

                adj[u].push_back(v);
                indegree[v]++;
            }
        }

        for (int m = 0; m < machines; m++) {
            for (int i = 0; i < (int)maquinas[m].size() - 1; i++) {

                int u = maquinas[m][i];
                int v = maquinas[m][i + 1];

                adj[u].push_back(v);
                indegree[v]++;
            }
        }

        queue<int> q;

        for (int i = 0; i < n; i++) {
            if (indegree[i] == 0) {
                dp[i] = peso[i];
                q.push(i);
            }
        }

        int last = -1;
        int best = 0;

        while (!q.empty()) {

            int u = q.front();
            q.pop();

            topo.push_back(u);

            if (dp[u] > best) {
                best = dp[u];
                last = u;
            }

            for (int v : adj[u]) {

                if (dp[v] < dp[u] + peso[v]) {
                    dp[v] = dp[u] + peso[v];
                    parent[v] = u;
                }

                indegree[v]--;

                if (indegree[v] == 0)
                    q.push(v);
            }
        }

        if ((int)topo.size() < n)
            return {1000000000, {}, {}};

        vector<int> caminho;

        while (last != -1) {
            caminho.push_back(last);
            last = parent[last];
        }

        reverse(caminho.begin(), caminho.end());

        return {best, caminho, topo};
    }

    void construir(Instancia& inst, vector<vector<int>>& maquinas) {

        maquinas.assign(machines, {});

        vector<int> prox(jobs, 0);
        vector<int> restante(jobs, 0);

        for (int j = 0; j < jobs; j++) {
            for (int k = 0; k < machines; k++) {
                int m = inst.sequence[j][k];
                restante[j] += inst.costs[j][m];
            }
        }

        int total = jobs * machines;

        for (int step = 0; step < total; step++) {

            vector<int> candidatos;

            for (int j = 0; j < jobs; j++) {
                if (prox[j] < machines)
                    candidatos.push_back(j);
            }

            sort(candidatos.begin(), candidatos.end(),
                 [&](int a, int b) {
                     return restante[a] > restante[b];
                 });

            int rcl = min(4, (int)candidatos.size());
            int escolhido = candidatos[rand() % rcl];

            int m = inst.sequence[escolhido][prox[escolhido]];

            maquinas[m].push_back(getId(escolhido, m));

            restante[escolhido] -= inst.costs[escolhido][m];
            prox[escolhido]++;
        }
    }

    void buscaLocal(Instancia& inst, vector<vector<int>>& maquinas) {

        bool melhorou = true;

        while (melhorou) {

            melhorou = false;

            Resultado atual = avaliar(inst, maquinas);
            int melhor = atual.makespan;

            vector<bool> critico(jobs * machines, false);

            for (int v : atual.caminho)
                critico[v] = true;

            for (int m = 0; m < machines && !melhorou; m++) {

                for (int i = 0; i < (int)maquinas[m].size() - 1 && !melhorou; i++) {

                    int a = maquinas[m][i];
                    int b = maquinas[m][i + 1];

                    if (!critico[a] && !critico[b]) continue;

                    swap(maquinas[m][i], maquinas[m][i + 1]);

                    Resultado novo = avaliar(inst, maquinas);

                    if (novo.makespan < melhor) {
                        melhor = novo.makespan;
                        melhorou = true;
                    } else {
                        swap(maquinas[m][i], maquinas[m][i + 1]);
                    }
                }
            }

            for (int m = 0; m < machines && !melhorou; m++) {

                for (int i = 0; i < (int)maquinas[m].size() && !melhorou; i++) {

                    int op = maquinas[m][i];

                    if (!critico[op]) continue;

                    maquinas[m].erase(maquinas[m].begin() + i);

                    for (int j = 0; j <= (int)maquinas[m].size() && !melhorou; j++) {

                        maquinas[m].insert(maquinas[m].begin() + j, op);

                        Resultado novo = avaliar(inst, maquinas);

                        if (novo.makespan < melhor) {
                            melhor = novo.makespan;
                            melhorou = true;
                        } else {
                            maquinas[m].erase(maquinas[m].begin() + j);
                        }
                    }

                    if (!melhorou)
                        maquinas[m].insert(maquinas[m].begin() + i, op);
                }
            }
        }
    }

    Solucao resolver(Instancia& inst) {

        jobs = inst.jobsCount;
        machines = inst.machinesCount;

        srand(time(NULL));

        int lb = lowerBound(inst);

        vector<vector<int>> melhorMaquinas;
        int melhorGlobal = 1000000000;

        clock_t inicio = clock();
        double limite = 10.0;

        while ((clock() - inicio) / (double)CLOCKS_PER_SEC < limite) {

            vector<vector<int>> maquinas;

            construir(inst, maquinas);

            Resultado r = avaliar(inst, maquinas);
            if (r.makespan == 1000000000) continue;

            buscaLocal(inst, maquinas);

            r = avaliar(inst, maquinas);

            if (r.makespan < melhorGlobal) {
                melhorGlobal = r.makespan;
                melhorMaquinas = maquinas;
            }
        }

        Resultado finalRes = avaliar(inst, melhorMaquinas);
    
        cout << "\n=============================\n";
        cout << "Resultado";
        cout << "\n=============================\n";
        // cout << "Lower Bound: " << lb << endl;
        // cout << "Makespan: " << finalRes.makespan << endl;

        // cout << "\nOrdem Topologica:\n";
        // for (int v : finalRes.topo) {
        //     cout << v << " ";
        // }
        // cout << endl;

        // cout << "\nCaminho Critico:\n";
        // for (int v : finalRes.caminho) {
        //     int job = v / machines;
        //     int maquina = v % machines;
        //     cout << "(J" << job << ",M" << maquina << ") ";
        // }
        // cout << endl;
        // cout << "\n=============================\n";
        
        cout << "Lower Bound: " << lb << endl;
        cout << "Makespan: " << finalRes.makespan << endl;

        Solucao sol;
        sol.makespan = finalRes.makespan;
        sol.topoOrder = finalRes.topo;
        sol.caminhoCritico = finalRes.caminho;

        return sol;
    }
};
