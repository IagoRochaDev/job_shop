#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>

class Instancia {
public:
    std::vector<std::vector<int>> costs;
    std::vector<std::vector<int>> jobs;

    Instancia(const std::string& arquivo){
       ler_arquivo(arquivo); 
    }
    
private:
    void ler_arquivo(const std::string& arquivo) {
        costs.clear();
        jobs.clear();

        std::ifstream input(arquivo);
        if (!input.is_open()) {
            throw std::runtime_error("Nao foi possivel abrir o arquivo: " + arquivo);
        }

        std::string line;
        int totalJobs = 0;
        int totalMachines = 0;

        while (std::getline(input, line)) {
            if (line.find("TotalJobs:") != std::string::npos) {
                std::istringstream header(line);
                std::string token;
                while (header >> token) {
                    if (token == "TotalJobs:") {
                        header >> totalJobs;
                    } else if (token == "TotalMachines:") {
                        header >> totalMachines;
                    }
                }
                break;
            }
        }

        if (totalJobs <= 0 || totalMachines <= 0) {
            throw std::runtime_error("Formato invalido: TotalJobs ou TotalMachines faltando ou invalido.");
        }

        while (std::getline(input, line)) {
            if (line.find("Costs:") != std::string::npos) {
                break;
            }
        }

        if (input.fail()) {
            throw std::runtime_error("Formato invalido: secao Costs nao encontrada.");
        }

        costs.reserve(totalJobs);
        for (int i = 0; i < totalJobs; ++i) {
            if (!std::getline(input, line)) {
                throw std::runtime_error("Formato invalido: custos insuficientes no arquivo.");
            }

            if (line.empty()) {
                --i;
                continue;
            }

            std::istringstream row(line);
            std::vector<int> costRow;
            int value;
            while (row >> value) {
                costRow.push_back(value);
            }

            if ((int)costRow.size() != totalMachines) {
                throw std::runtime_error("Formato invalido: numero de custos diferente de TotalMachines.");
            }

            costs.push_back(std::move(costRow));
        }

        jobs.assign(totalJobs, std::vector<int>(totalMachines, -1));
        int jobIndex = 0;

        while (std::getline(input, line)) {
            if (line.empty()) {
                continue;
            }

            if (line.find("Job:") != std::string::npos) {
                if (jobIndex >= totalJobs) {
                    break;
                }

                while (std::getline(input, line)) {
                    if (line.empty()) {
                        break;
                    }

                    std::istringstream mapping(line);
                    int source;
                    char arrow1;
                    char arrow2;
                    int target;

                    mapping >> source >> arrow1 >> arrow2 >> target;
                    if (mapping.fail() || arrow1 != '-' || arrow2 != '>') {
                        continue;
                    }

                    if (source < 0 || source >= totalMachines) {
                        throw std::runtime_error("Valor de job invalido na secao Job.");
                    }
                    jobs[jobIndex][source] = target;
                }

                ++jobIndex;
            }
        }

        if (jobIndex != totalJobs) {
            throw std::runtime_error("Formato invalido: numero de blocos Job diferente de TotalJobs.");
        }
    }
};