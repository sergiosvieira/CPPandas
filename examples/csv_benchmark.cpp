/**
 * @file csv_benchmark.cpp
 * @brief Exemplo para medir o tempo de carregamento de arquivos CSV
 */

#include <iostream>
#include <chrono>
#include <iomanip>
#include <string>
#include "cppandas/csv.hpp"

// Função para medir o tempo com precisão de nanossegundos
template<typename Func>
auto measureExecutionTime(Func&& func) {
    // Usar high_resolution_clock para máxima precisão
    auto start = std::chrono::high_resolution_clock::now();
    
    // Executar a função
    func();
    
    auto end = std::chrono::high_resolution_clock::now();
    return end - start;
}

// Função para formatar a duração em unidades apropriadas
std::string formatDuration(std::chrono::nanoseconds ns) {
    double duration_ns = ns.count();
    
    // Converter para a unidade mais apropriada
    if (duration_ns < 1000.0) {
        return std::to_string(duration_ns) + " ns";
    } else if (duration_ns < 1000000.0) {
        return std::to_string(duration_ns / 1000.0) + " µs";
    } else if (duration_ns < 1000000000.0) {
        return std::to_string(duration_ns / 1000000.0) + " ms";
    } else {
        return std::to_string(duration_ns / 1000000000.0) + " s";
    }
}

int main(int argc, char* argv[]) {
    // Verificar se o caminho do arquivo foi fornecido
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <caminho_do_arquivo_csv>" << std::endl;
        return 1;
    }
    
    std::string filepath = argv[1];
    std::cout << "Carregando arquivo: " << filepath << std::endl;
    
    CPPandas::CSV csv;
    
    // Medir o tempo de carregamento com precisão de nanossegundos
    auto duration = measureExecutionTime([&]() {
        if (!csv.load(filepath)) {
            std::cerr << "Erro ao carregar o arquivo CSV." << std::endl;
            exit(1);
        }
    });
    
    // Exibir estatísticas
    std::cout << "\n=== Estatísticas de Carregamento ===" << std::endl;
    std::cout << "Tempo de carregamento: " << formatDuration(std::chrono::duration_cast<std::chrono::nanoseconds>(duration)) << std::endl;
    std::cout << "Número de linhas: " << csv.rowCount() << std::endl;
    std::cout << "Número de colunas: " << csv.columnCount() << std::endl;
    
    // Exibir cabeçalhos se existirem
    if (!csv.headers().empty()) {
        std::cout << "\n=== Cabeçalhos ===" << std::endl;
        for (const auto& header : csv.headers()) {
            std::cout << header << " | ";
        }
        std::cout << std::endl;
    }
    
    // Exibir algumas linhas de amostra (primeiras 5 linhas)
    const size_t sampleSize = std::min(size_t(5), csv.rowCount());
    if (sampleSize > 0) {
        std::cout << "\n=== Amostra de Dados (primeiras " << sampleSize << " linhas) ===" << std::endl;
        
        for (size_t i = 0; i < sampleSize; ++i) {
            auto row = csv.getRow(i);
            for (const auto& item : row) {
                std::cout << item << " | ";
            }
            std::cout << std::endl;
        }
    }
    
    // Exibir tempos de acesso para operações comuns
    if (csv.rowCount() > 0 && csv.columnCount() > 0) {
        std::cout << "\n=== Tempos de Acesso ===" << std::endl;
        
        // Medir tempo para acessar uma linha
        auto rowAccessTime = measureExecutionTime([&]() {
            auto row = csv.getRow(0);
        });
        std::cout << "Tempo para acessar uma linha: " 
                << formatDuration(std::chrono::duration_cast<std::chrono::nanoseconds>(rowAccessTime)) 
                << std::endl;
        
        // Medir tempo para acessar uma coluna por índice
        auto colIndexAccessTime = measureExecutionTime([&]() {
            auto col = csv.getColumn(0);
        });
        std::cout << "Tempo para acessar uma coluna por índice: " 
                << formatDuration(std::chrono::duration_cast<std::chrono::nanoseconds>(colIndexAccessTime)) 
                << std::endl;
        
        // Medir tempo para acessar uma coluna por nome (se houver cabeçalhos)
        if (!csv.headers().empty()) {
            auto colNameAccessTime = measureExecutionTime([&]() {
                try {
                    auto col = csv.getColumn(csv.headers()[0]);
                } catch (const std::exception& e) {
                    // Ignorar erros
                }
            });
            std::cout << "Tempo para acessar uma coluna por nome: " 
                    << formatDuration(std::chrono::duration_cast<std::chrono::nanoseconds>(colNameAccessTime)) 
                    << std::endl;
        }
    }
    
    return 0;
}