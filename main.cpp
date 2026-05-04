#include <iostream>
#include <exception>
#include <string>
#include "domain/instancia.hpp"
#include "solvers/solver_JPS.hpp"

int main(int argc, char* argv[]) {
    try {
        const std::string caminho = (argc > 1 ? argv[1] : "res/instancias/JSP/ta01Jsp.psi");
        Instancia instancia(caminho);

        Solver_JPS solver(instancia);
        if (!solver.solver()) {
            std::cerr << "Falha ao resolver o problema." << std::endl;
            return 1;
        }

        const auto& solucao = solver.get_solucao();
        std::cout << "Arquivo lido: " << caminho << "\n";
        std::cout << "Total de jobs: " << instancia.costs.size() << "\n";
        if (!instancia.costs.empty()) {
            std::cout << "Total de maquinas: " << instancia.costs[0].size() << "\n";
        }
        std::cout << "Makespan calculado: " << solucao.makespan_valor << "\n";
        std::cout << "Caminho critico (vertices): ";
        for (size_t i = 0; i < solucao.caminho_maximo_vertices.size(); ++i) {
            std::cout << solucao.caminho_maximo_vertices[i];
            if (i + 1 < solucao.caminho_maximo_vertices.size()) {
                std::cout << " -> ";
            }
        }
        std::cout << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Erro: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}