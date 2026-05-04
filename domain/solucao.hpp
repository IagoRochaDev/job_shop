#pragma once

#include <vector>
#include <string>

class Solucao {
public:
    std::vector<int> ordem_topologica;
    std::vector<int> caminho_maximo_vertices;
    float makespan_valor = 0.0f;

    Solucao() = default;
    explicit Solucao(float makespan)
        : makespan_valor(makespan) {}
};
