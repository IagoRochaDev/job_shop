#pragma once

#include <vector>
#include <list>
#include <queue>
#include <algorithm>
#include "solucao.hpp"

class Digrafo {
private:
    int numVertices;
    std::vector<std::list<int>> adj;
    std::vector<float> pesos;

public:
    explicit Digrafo(int v)
        : numVertices(v), adj(v), pesos(v, 0.0f) {}

    int tamanho() const {
        return numVertices;
    }

    void set_peso_vertice(int v, float peso) {
        if (v >= 0 && v < numVertices) {
            pesos[v] = peso;
        }
    }

    void adicionar_aresta(int u, int v) {
        if (u >= 0 && u < numVertices && v >= 0 && v < numVertices) {
            adj[u].push_back(v);
        }
    }

    std::vector<int> caminhada_topologica() const {
        std::vector<int> grauEntrada(numVertices, 0);
        for (int u = 0; u < numVertices; ++u) {
            for (int v : adj[u]) {
                grauEntrada[v]++;
            }
        }

        std::queue<int> fila;
        for (int i = 0; i < numVertices; ++i) {
            if (grauEntrada[i] == 0) {
                fila.push(i);
            }
        }

        std::vector<int> ordem;
        while (!fila.empty()) {
            int u = fila.front();
            fila.pop();
            ordem.push_back(u);

            for (int v : adj[u]) {
                if (--grauEntrada[v] == 0) {
                    fila.push(v);
                }
            }
        }

        return ordem;
    }

    Solucao encontrar_caminho_maximo() const {
        Solucao resultado;
        resultado.ordem_topologica = caminhada_topologica();

        if ((int)resultado.ordem_topologica.size() != numVertices) {
            resultado.makespan_valor = -1.0f;
            return resultado;
        }

        std::vector<float> dist(numVertices, 0.0f);
        std::vector<int> predecessor(numVertices, -1);

        for (int i = 0; i < numVertices; ++i) {
            dist[i] = pesos[i];
        }

        for (int u : resultado.ordem_topologica) {
            for (int v : adj[u]) {
                float caminhoValor = dist[u] + pesos[v];
                if (caminhoValor > dist[v]) {
                    dist[v] = caminhoValor;
                    predecessor[v] = u;
                }
            }
        }

        float maxMakespan = dist[0];
        int verticeFinal = 0;
        for (int i = 1; i < numVertices; ++i) {
            if (dist[i] > maxMakespan) {
                maxMakespan = dist[i];
                verticeFinal = i;
            }
        }

        resultado.makespan_valor = maxMakespan;
        for (int atual = verticeFinal; atual != -1; atual = predecessor[atual]) {
            resultado.caminho_maximo_vertices.push_back(atual);
        }
        std::reverse(resultado.caminho_maximo_vertices.begin(), resultado.caminho_maximo_vertices.end());

        return resultado;
    }
};
