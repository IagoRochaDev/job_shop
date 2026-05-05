#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdio>

using namespace std;

class Instancia {
public:
    int jobsCount = 0;
    int machinesCount = 0;

    vector<vector<int>> costs;
    vector<vector<int>> sequence;

    void ler_arquivo(string arquivo) {
        ifstream file(arquivo);
        string line;

        if (!file.is_open()) {
            cout << "Erro ao abrir arquivo\n";
            return;
        }

        while (getline(file, line)) {
            if (line.find("TotalJobs") != string::npos) {
                sscanf(line.c_str(),
                       "TotalJobs: %d TotalMachines: %d",
                       &jobsCount,
                       &machinesCount);
                break;
            }
        }

        if (jobsCount == 0 || machinesCount == 0) {
            cout << "Erro ao ler dimensoes\n";
            return;
        }

        while (getline(file, line)) {
            if (line.find("Costs") != string::npos)
                break;
        }

        costs.resize(jobsCount, vector<int>(machinesCount));

        for (int i = 0; i < jobsCount; i++) {
            getline(file, line);
            stringstream ss(line);

            for (int j = 0; j < machinesCount; j++) {
                ss >> costs[i][j];
            }
        }

        sequence.clear();
        sequence.resize(jobsCount);

        int currentJob = -1;
        vector<vector<pair<int,int>>> edges(jobsCount);

        while (getline(file, line)) {

            if (line.find("Job") != string::npos) {
                currentJob++;
                continue;
            }

            if (line.find("->") != string::npos) {
                int a, b;

                if (sscanf(line.c_str(), "%d -> %d", &a, &b) == 2) {
                    edges[currentJob].push_back({a, b});
                }
            }
        }

        for (int j = 0; j < jobsCount; j++) {

            vector<int> indeg(machinesCount, 0);
            vector<int> next(machinesCount, -1);

            for (auto &e : edges[j]) {
                int u = e.first;
                int v = e.second;

                next[u] = v;
                indeg[v]++;
            }

            int start = -1;
            for (int i = 0; i < machinesCount; i++) {
                if (indeg[i] == 0) {
                    start = i;
                    break;
                }
            }

            while (start != -1) {
                sequence[j].push_back(start);
                start = next[start];
            }
        }

        file.close();

        // cout << "\n===== DEBUG INSTANCIA =====\n";
        // cout << "Jobs: " << jobsCount << endl;
        // cout << "Machines: " << machinesCount << endl;
        // cout << "Custo[0][0]: " << costs[0][0] << endl;
        // cout << "Tamanho seq job 0: " << sequence[0].size() << endl;
        // cout << "===========================\n\n";
    }
};
