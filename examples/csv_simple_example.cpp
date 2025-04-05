/**
 * @file csv_example.cpp
 * @brief Exemplo de uso da classe CSV
 */

#include <iostream>
#include "cppandas/csv.hpp"

int main() {
    // Criar um arquivo CSV de teste
    {
        std::ofstream testFile("test.csv");
        testFile << "Name,Age,City\n";
        testFile << "John,30,New York\n";
        testFile << "Alice,25,London\n";
        testFile << "Bob,35,Tokyo\n";
    }
    
    // Carregar e processar o arquivo CSV
    CPPandas::CSV csv("test.csv");
    
    // Mostrar informações básicas
    std::cout << "Loaded CSV with " << csv.rowCount() << " rows and " 
            << csv.columnCount() << " columns." << std::endl;
    
    // Mostrar cabeçalhos
    std::cout << "Headers: ";
    for (const auto& header : csv.headers()) {
        std::cout << header << " ";
    }
    std::cout << std::endl;
    
    // Mostrar todos os dados
    std::cout << "Data:" << std::endl;
    for (size_t i = 0; i < csv.rowCount(); ++i) {
        auto row = csv.getRow(i);
        for (const auto& item : row) {
            std::cout << item << "\t";
        }
        std::cout << std::endl;
    }
    
    // Obter uma coluna específica
    std::cout << "Names: ";
    auto names = csv.getColumn("Name");
    for (const auto& name : names) {
        std::cout << name << " ";
    }
    std::cout << std::endl;
    
    // Salvar em um novo arquivo
    csv.save("output.csv");
    std::cout << "Saved to output.csv" << std::endl;
    
    return 0;
}