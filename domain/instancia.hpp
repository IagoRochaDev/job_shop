#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <cctype>

class Instancia {
public:
    std::vector<std::vector<int>> costs;
    std::vector<std::vector<int>> jobs;

    Instancia(const std::string& arquivo){
       ler_arquivo(arquivo); 
    }
    
private:
    void ler_arquivo(const std::string& arquivo) {
        auto trim = [](std::string& s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
            s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), s.end());
        };

        costs.clear();
        jobs.clear();

        std::ifstream input(arquivo);
        if (!input.is_open()) {
            throw std::runtime_error("Nao foi possivel abrir o arquivo: " + arquivo);
        }

        std::string line;
        int totalJobs = 0;
        int totalMachines = 0;

        bool headerFound = false;
        while (std::getline(input, line)) {
            trim(line);
            if (line.empty() || line[0] == '#') {
                continue;
            }
            if (line.find("TotalJobs:") != std::string::npos) {
                headerFound = true;
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

        if (!headerFound || totalJobs <= 0 || totalMachines <= 0) {
            throw std::runtime_error("Formato invalido: TotalJobs ou TotalMachines faltando ou invalido.");
        }

        bool costsFound = false;
        while (std::getline(input, line)) {
            trim(line);
            if (line.empty()) {
                continue;
            }
            if (line.find("Costs:") != std::string::npos) {
                costsFound = true;
                break;
            }
        }

        if (!costsFound) {
            throw std::runtime_error("Formato invalido: secao Costs nao encontrada.");
        }

        costs.reserve(totalJobs);
        while ((int)costs.size() < totalJobs && std::getline(input, line)) {
            trim(line);
            if (line.empty()) {
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

        if ((int)costs.size() != totalJobs) {
            throw std::runtime_error("Formato invalido: custos insuficientes no arquivo.");
        }

        jobs.assign(totalJobs, std::vector<int>(totalMachines, -1));
        int jobIndex = -1;
        int expectedMappings = -1;
        int currentMappings = 0;

        auto validate_previous_job = [&]() {
            if (jobIndex < 0 || expectedMappings < 0) {
                return;
            }
            if (currentMappings != expectedMappings) {
                std::ostringstream message;
                message << "Formato invalido: Job " << jobIndex << " tem " << currentMappings << " mappings, esperado " << expectedMappings << ".";
                throw std::runtime_error(message.str());
            }
        };

        while (std::getline(input, line)) {
            trim(line);
            if (line.empty()) {
                continue;
            }

            if (line.compare(0, 4, "Job:") == 0) {
                if (jobIndex >= 0) {
                    validate_previous_job();
                }
                ++jobIndex;
                if (jobIndex >= totalJobs) {
                    throw std::runtime_error("Formato invalido: numero de blocos Job maior do que TotalJobs.");
                }

                std::istringstream header(line);
                std::string token;
                header >> token;
                header >> expectedMappings;
                if (expectedMappings < 0) {
                    throw std::runtime_error("Formato invalido: numero de operacoes no Job invalido.");
                }
                currentMappings = 0;
                jobs[jobIndex].assign(totalMachines, -1);
                continue;
            }

            if (jobIndex < 0) {
                continue;
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

            if (source < 0 || source >= totalMachines || target < 0 || target >= totalMachines) {
                throw std::runtime_error("Valor de job invalido na secao Job.");
            }
            jobs[jobIndex][source] = target;
            ++currentMappings;
        }

        validate_previous_job();

        if (jobIndex + 1 != totalJobs) {
            throw std::runtime_error("Formato invalido: numero de blocos Job diferente de TotalJobs.");
        }
    }
};