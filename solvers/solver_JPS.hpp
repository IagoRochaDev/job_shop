#pragma once

#include <algorithm>
#include <limits>
#include <unordered_set>
#include <vector>

#include "../domain/instancia.hpp"
#include "../domain/solucao.hpp"
#include "../domain/digrafo.hpp"

class Solver_JPS {
private:
    Instancia instancia;
    Digrafo digrafo;
    Solucao melhor_solucao;
    std::vector<std::vector<int>> ordem_maquinas;

    int total_jobs() const {
        return static_cast<int>(instancia.costs.size());
    }

    int total_maquinas() const {
        return instancia.costs.empty() ? 0 : static_cast<int>(instancia.costs[0].size());
    }

    int vertex_id(int job, int maquina) const {
        return job * total_maquinas() + maquina;
    }

    void gerar_solucao_inicial() {
        int maquinas = total_maquinas();
        int jobs = total_jobs();

        ordem_maquinas.assign(maquinas, std::vector<int>(jobs));
        for (int m = 0; m < maquinas; ++m) {
            for (int j = 0; j < jobs; ++j) {
                ordem_maquinas[m][j] = j;
            }
        }

        float makespan_inicial = avaliar_solucao(ordem_maquinas);
        if (makespan_inicial < std::numeric_limits<float>::infinity()) {
            melhor_solucao.makespan_valor = makespan_inicial;
        }
    }

    void construir_grafo_disjuntivo(const std::vector<std::vector<int>>& ordem) {
        int maquinas = total_maquinas();
        int jobs = total_jobs();
        int num_vertices = jobs * maquinas;

        digrafo = Digrafo(num_vertices);

        // Construir roteamento de cada job e adicionar pesos
        std::vector<std::vector<int>> roteamento_job(jobs);
        for (int j = 0; j < jobs; ++j) {
            for (int m = 0; m < maquinas; ++m) {
                if (instancia.jobs[j][m] != -1) {
                    roteamento_job[j].push_back(m);
                }
            }
            
            // Definir pesos dos vértices
            for (int m : roteamento_job[j]) {
                float duracao = static_cast<float>(instancia.costs[j][m]);
                digrafo.set_peso_vertice(vertex_id(j, m), duracao);
            }
        }

        // Arestas conjuntivas (precedência dentro do mesmo job)
        for (int j = 0; j < jobs; ++j) {
            for (size_t idx = 0; idx + 1 < roteamento_job[j].size(); ++idx) {
                int maquina_atual = roteamento_job[j][idx];
                int maquina_proxima = roteamento_job[j][idx + 1];
                int u = vertex_id(j, maquina_atual);
                int v = vertex_id(j, maquina_proxima);
                digrafo.adicionar_aresta(u, v);
            }
        }

        // Arestas disjuntivas (baseadas na ordem de máquinas)
        for (int m = 0; m < maquinas; ++m) {
            const std::vector<int>& fila_jobs = ordem[m];
            for (size_t pos = 0; pos + 1 < fila_jobs.size(); ++pos) {
                int job_atual = fila_jobs[pos];
                int job_seguinte = fila_jobs[pos + 1];
                
                // Verificar se ambos os jobs usam esta máquina
                if (std::find(roteamento_job[job_atual].begin(), roteamento_job[job_atual].end(), m) == roteamento_job[job_atual].end()) {
                    continue;
                }
                if (std::find(roteamento_job[job_seguinte].begin(), roteamento_job[job_seguinte].end(), m) == roteamento_job[job_seguinte].end()) {
                    continue;
                }
                
                int u = vertex_id(job_atual, m);
                int v = vertex_id(job_seguinte, m);
                digrafo.adicionar_aresta(u, v);
            }
        }
    }

    float avaliar_solucao(const std::vector<std::vector<int>>& ordem_candidata) {
        construir_grafo_disjuntivo(ordem_candidata);
        Solucao resultado = digrafo.encontrar_caminho_maximo();
        if (resultado.makespan_valor < 0.0f) {
            return std::numeric_limits<float>::infinity();
        }
        return resultado.makespan_valor;
    }

    void busca_local() {
        int maquinas = total_maquinas();
        int jobs = total_jobs();
        if (maquinas == 0 || jobs == 0) {
            return;
        }

        gerar_solucao_inicial();
        float melhor_makespan = melhor_solucao.makespan_valor;
        if (melhor_makespan == 0.0f) {
            melhor_makespan = avaliar_solucao(ordem_maquinas);
            melhor_solucao.makespan_valor = melhor_makespan;
        }

        construir_grafo_disjuntivo(ordem_maquinas);
        melhor_solucao = digrafo.encontrar_caminho_maximo();
        if (melhor_solucao.makespan_valor < 0.0f) {
            melhor_solucao.makespan_valor = std::numeric_limits<float>::infinity();
        }

        bool encontrou_melhora = true;
        int iteracoes = 0;
        const int max_iteracoes = std::max(100, maquinas * jobs * 5);

        while (encontrou_melhora && iteracoes < max_iteracoes) {
            encontrou_melhora = false;
            float melhor_vizinho = melhor_makespan;
            std::vector<std::vector<int>> melhor_ordem = ordem_maquinas;

            std::unordered_set<int> caminho_critico;
            for (int vertice : melhor_solucao.caminho_maximo_vertices) {
                caminho_critico.insert(vertice);
            }

            for (int m = 0; m < maquinas; ++m) {
                for (int pos = 0; pos + 1 < jobs; ++pos) {
                    int job_u = ordem_maquinas[m][pos];
                    int job_v = ordem_maquinas[m][pos + 1];
                    int id_u = vertex_id(job_u, m);
                    int id_v = vertex_id(job_v, m);

                    if (!caminho_critico.empty() &&
                        caminho_critico.find(id_u) == caminho_critico.end() &&
                        caminho_critico.find(id_v) == caminho_critico.end()) {
                        continue;
                    }

                    std::vector<std::vector<int>> candidato = ordem_maquinas;
                    std::swap(candidato[m][pos], candidato[m][pos + 1]);

                    float makespan_candidato = avaliar_solucao(candidato);
                    if (makespan_candidato < melhor_vizinho) {
                        melhor_vizinho = makespan_candidato;
                        melhor_ordem = std::move(candidato);
                        encontrou_melhora = true;
                    }
                }
            }

            if (encontrou_melhora) {
                ordem_maquinas = melhor_ordem;
                melhor_makespan = melhor_vizinho;
                construir_grafo_disjuntivo(ordem_maquinas);
                melhor_solucao = digrafo.encontrar_caminho_maximo();
            }

            ++iteracoes;
        }
    }

public:
    explicit Solver_JPS(const Instancia& inst)
        : instancia(inst), digrafo(inst.costs.size() * (inst.costs.empty() ? 0 : inst.costs[0].size())), melhor_solucao() {
    }

    bool solver() {
        if (total_jobs() == 0 || total_maquinas() == 0) {
            return false;
        }

        gerar_solucao_inicial();
        busca_local();

        return melhor_solucao.makespan_valor < std::numeric_limits<float>::infinity();
    }

    const Solucao& get_solucao() const {
        return melhor_solucao;
    }
};