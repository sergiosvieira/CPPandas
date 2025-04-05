/**
 * @file cppandas.hpp
 * @brief Definição da classe CPPandas com funcionalidades semelhantes ao pandas
 * @author CPPandas Team
 */

#ifndef CPPANDAS_HPP
#define CPPANDAS_HPP

#include "cppandas/csv.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <memory>
#include <algorithm>
#include <numeric>
#include <typeinfo>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <functional>

namespace CPPandas {

// Add this to your cppandas.hpp file, above the DataFrame class
class ColumnNotFoundException : public std::exception {
    private:
        std::string m_message;
        
    public:
        explicit ColumnNotFoundException(const std::vector<std::string>& columns) {
            std::stringstream ss;
            ss << "KeyError: \"[";
            for (size_t i = 0; i < columns.size(); ++i) {
                ss << "'" << columns[i] << "'";
                if (i < columns.size() - 1) {
                    ss << ", ";
                }
            }
            ss << "]\"";
            m_message = ss.str();
        }
        
        const char* what() const noexcept override {
            return m_message.c_str();
        }
};

class StatisticalSummary {
private:
    std::vector<std::string> m_index;
    std::vector<std::string> m_columns;
    std::map<std::string, std::map<std::string, double>> m_data;

public:
    StatisticalSummary() = default;

    // Adicionar uma linha
    void addRow(const std::string& rowName) {
        if (std::find(m_index.begin(), m_index.end(), rowName) == m_index.end()) {
            m_index.push_back(rowName);
        }
    }

    // Adicionar uma coluna
    void addColumn(const std::string& columnName) {
        if (std::find(m_columns.begin(), m_columns.end(), columnName) == m_columns.end()) {
            m_columns.push_back(columnName);
        }
    }

    // Definir valor
    void setValue(const std::string& rowName, const std::string& columnName, double value) {
        // Garantir que a linha e coluna existam
        addRow(rowName);
        addColumn(columnName);
        // Definir o valor
        m_data[rowName][columnName] = value;
    }

    // Obter valor
    double getValue(const std::string& rowName, const std::string& columnName) const {
        auto rowIt = m_data.find(rowName);
        if (rowIt != m_data.end()) {
            auto colIt = rowIt->second.find(columnName);
            if (colIt != rowIt->second.end()) {
                return colIt->second;
            }
        }
        return std::numeric_limits<double>::quiet_NaN();
    }

    // Acessar linha (similar ao .loc[] do pandas)
    class RowAccessor {
    private:
        StatisticalSummary& m_summary;
        std::string m_rowName;

    public:
        RowAccessor(StatisticalSummary& summary, const std::string& rowName)
            : m_summary(summary), m_rowName(rowName) {
            // Garantir que a linha exista
            m_summary.addRow(rowName);
        }

        // Operador para atribuir valores de outro DataFrame ou lista de valores
        void operator=(const std::vector<double>& values) {
            if (values.size() != m_summary.m_columns.size()) {
                throw std::invalid_argument("Número de valores não corresponde ao número de colunas");
            }

            for (size_t i = 0; i < m_summary.m_columns.size(); ++i) {
                m_summary.setValue(m_rowName, m_summary.m_columns[i], values[i]);
            }
        }
    };

    // Obter acessador de linha
    RowAccessor loc(const std::string& rowName) {
        return RowAccessor(*this, rowName);
    }

    // Obter nomes das linhas
    const std::vector<std::string>& index() const {
        return m_index;
    }

    // Obter nomes das colunas
    const std::vector<std::string>& columns() const {
        return m_columns;
    }

    // Método para exibir o resumo estatístico (similar ao print em Python)
    void print() const {
        // Calcular largura das colunas
        size_t indexWidth = 10;  // Largura mínima para a coluna de índice
        for (const auto& idx : m_index) {
            indexWidth = std::max(indexWidth, idx.size() + 2);
        }

        std::vector<size_t> columnWidths(m_columns.size(), 12);  // Largura padrão para colunas de dados

        // Imprimir cabeçalho
        std::cout << std::setw(indexWidth) << "";
        for (size_t i = 0; i < m_columns.size(); ++i) {
            std::cout << std::setw(columnWidths[i]) << m_columns[i];
        }
        std::cout << std::endl;

        // Imprimir linhas
        for (const auto& rowName : m_index) {
            std::cout << std::setw(indexWidth) << rowName;

            for (size_t i = 0; i < m_columns.size(); ++i) {
                const std::string& colName = m_columns[i];
                auto rowIt = m_data.find(rowName);

                if (rowIt != m_data.end()) {
                    auto colIt = rowIt->second.find(colName);
                    if (colIt != rowIt->second.end()) {
                        double value = colIt->second;
                        if (std::isnan(value)) {
                            std::cout << std::setw(columnWidths[i]) << "NaN";
                        } else {
                            std::cout << std::fixed << std::setprecision(4) << std::setw(columnWidths[i]) << value;
                        }
                    } else {
                        std::cout << std::setw(columnWidths[i]) << "NaN";
                    }
                } else {
                    std::cout << std::setw(columnWidths[i]) << "NaN";
                }
            }
            std::cout << std::endl;
        }
    }
};

/**
 * @brief Classe para retornar o resultado da função mode()
 */
class ModeResult {
private:
    std::vector<double> m_values;

public:
    explicit ModeResult(const std::vector<double>& values) : m_values(values) {}

    double iloc(size_t index) const {
        if (index >= m_values.size()) {
            throw std::out_of_range("Index out of range");
        }
        return m_values[index];
    }
};

class DataFrame {
private:
    CSV m_csv;
    std::vector<std::string> m_activeColumns; // Para rastrear quais colunas estão ativas

public:
    DataFrame() = default;
    explicit DataFrame(const CSV& csv) : m_csv(csv) {
        // Inicialmente, todas as colunas estão ativas
        m_activeColumns = m_csv.headers();
    }
    
    // Acesso aos dados do CSV
    size_t rowCount() const { return m_csv.rowCount(); }
    
    size_t columnCount() const { 
        return m_activeColumns.size(); 
    }
    
    const std::vector<std::string>& headers() const { 
        return m_activeColumns;
    }
    
    ModeResult mode() const {
        std::vector<double> result;
        result.reserve(m_activeColumns.size());

        for (const auto& colName : m_activeColumns) {
            result.push_back(mode(colName));
        }

        return ModeResult(result);
    }

    // Acesso às linhas
    CSV::Row getRow(size_t rowIndex) const { 
        if (m_activeColumns.size() == m_csv.headers().size()) {
            // Se todas as colunas estão ativas, retorna a linha completa
            return m_csv.getRow(rowIndex);
        } else {
            // Senão, filtra a linha para incluir apenas as colunas ativas
            CSV::Row fullRow = m_csv.getRow(rowIndex);
            CSV::Row filteredRow;
            
            for (const auto& colName : m_activeColumns) {
                auto it = std::find(m_csv.headers().begin(), m_csv.headers().end(), colName);
                if (it != m_csv.headers().end()) {
                    size_t colIndex = std::distance(m_csv.headers().begin(), it);
                    if (colIndex < fullRow.size()) {
                        filteredRow.push_back(fullRow[colIndex]);
                    }
                }
            }
            
            return filteredRow;
        }
    }
    
    // Acesso às colunas
    CSV::Column getColumn(const std::string& columnName) const { 
        // Verifica se a coluna está ativa
        auto it = std::find(m_activeColumns.begin(), m_activeColumns.end(), columnName);
        if (it == m_activeColumns.end()) {
            throw std::out_of_range("Column not in active columns");
        }
        return m_csv.getColumn(columnName);
    }
    
    CSV::Column getColumn(size_t columnIndex) const {
        if (columnIndex >= m_activeColumns.size()) {
            throw std::out_of_range("Column index out of range");
        }
        return m_csv.getColumn(m_activeColumns[columnIndex]);
    }
    
    // Seleção de colunas múltiplas (estilo pandas)
    DataFrame operator[](const std::vector<std::string>& columns) const {
        DataFrame result(*this);
        
        // Verificar se todas as colunas solicitadas existem
        std::vector<std::string> missingColumns;
        for (const auto& col : columns) {
            if (std::find(m_csv.headers().begin(), m_csv.headers().end(), col) == m_csv.headers().end()) {
                missingColumns.push_back(col);
            }
        }
        
        // Se alguma coluna não existir, lançar exceção no estilo pandas
        if (!missingColumns.empty()) {
            throw ColumnNotFoundException(missingColumns);
        }
        
        // Atualizar colunas ativas
        result.m_activeColumns = columns;
        
        return result;
    }

    StatisticalSummary describe(const std::vector<double>& percentiles = {0.25, 0.5, 0.75}) const {
        StatisticalSummary summary;

        // Adicionar linhas padrão
        summary.addRow("count");
        summary.addRow("mean");
        summary.addRow("std");
        summary.addRow("min");
        summary.addRow("max");

        // Adicionar linhas para percentis
        for (double p : percentiles) {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << (p * 100) << "%";
            summary.addRow(ss.str());
        }

        // Determinar colunas numéricas
        std::vector<std::string> numericColumns;

        for (const auto& colName : m_activeColumns) {
            auto column = getColumn(colName);

            // Verificar se a coluna é numérica (pelo menos 70% dos valores não vazios são numéricos)
            size_t numericCount = 0;
            size_t nonEmptyCount = 0;

            for (const auto& value : column) {
                if (!value.empty()) {
                    nonEmptyCount++;
                    try {
                        std::stod(value);
                        numericCount++;
                    } catch (...) {
                        // Não é numérico
                    }
                }
            }

            double numericRatio = nonEmptyCount > 0 ? static_cast<double>(numericCount) / nonEmptyCount : 0.0;

            if (numericRatio >= 0.7) {
                numericColumns.push_back(colName);
                summary.addColumn(colName);
            }
        }

        // Calcular estatísticas para cada coluna numérica
        for (const auto& colName : numericColumns) {
            auto column = getColumn(colName);
            auto numericValues = columnToNumeric(column);

            // Contar valores não-NaN
            size_t count = 0;
            for (double value : numericValues) {
                if (!std::isnan(value)) {
                    count++;
                }
            }

            // Estatísticas básicas
            summary.setValue("count", colName, count);
            summary.setValue("mean", colName, mean(colName));
            summary.setValue("std", colName, std(colName));
            summary.setValue("min", colName, min(colName));
            summary.setValue("max", colName, max(colName));

            // Percentis
            for (double p : percentiles) {
                std::stringstream ss;
                ss << std::fixed << std::setprecision(1) << (p * 100) << "%";
                summary.setValue(ss.str(), colName, quantile(colName, p));
            }
        }

        return summary;
    }


    // Método info() similar ao pandas
    void info() const {
        try {
            std::cout << "<class 'CPPandas.DataFrame'>" << std::endl;
            std::cout << "RangeIndex: " << rowCount() << " entries, 0 to " 
                    << (rowCount() > 0 ? rowCount() - 1 : 0) << std::endl;
            
            std::cout << "Data columns (total " << columnCount() << " columns):" << std::endl;
            
            // Cabeçalho da tabela
            std::cout << std::setw(5) << "#" << std::setw(25) << "Column" 
                    << std::setw(15) << "Non-Null Count" << std::setw(15) << "Dtype" << std::endl;
            std::cout << std::string(60, '-') << std::endl;
            
            // Pré-computar e armazenar as colunas para evitar chamadas repetidas a getColumn()
            std::vector<CSV::Column> cachedColumns;
            cachedColumns.reserve(m_activeColumns.size());
            
            for (const auto& colName : m_activeColumns) {
                try {
                    cachedColumns.push_back(m_csv.getColumn(colName));
                } catch (const std::exception& e) {
                    std::cerr << "Erro ao acessar coluna '" << colName << "': " << e.what() << std::endl;
                    // Adicionar uma coluna vazia como fallback
                    cachedColumns.push_back(CSV::Column());
                }
            }
            
            // Dados das colunas
            for (size_t i = 0; i < m_activeColumns.size(); ++i) {
                const std::string& colName = m_activeColumns[i];
                const CSV::Column& column = cachedColumns[i];
                
                // Contar valores não nulos com limite de verificação
                size_t nonNullCount = 0;
                size_t checkLimit = std::min(column.size(), size_t(1000)); // Limitar verificação
                
                for (size_t j = 0; j < checkLimit; j++) {
                    if (!column[j].empty()) {
                        nonNullCount++;
                    }
                }
                
                if (checkLimit < column.size()) {
                    // Estimar para o resto da coluna
                    nonNullCount = (nonNullCount * column.size()) / checkLimit;
                }
                
                // Determinar o tipo de dados (simplificado e protegido)
                std::string dtype = "string";
                
                // Verificar apenas as primeiras 100 linhas não vazias para determinar o tipo
                bool allNumeric = true;
                size_t checkedRows = 0;
                
                for (const auto& val : column) {
                    if (val.empty()) continue;
                    
                    if (checkedRows >= 100) break; // Limite para evitar loop infinito
                    
                    checkedRows++;
                    
                    if (!std::all_of(val.begin(), val.end(), [](char c) {
                        return std::isdigit(c) || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E';
                    })) {
                        allNumeric = false;
                        break;
                    }
                }
                
                if (allNumeric && checkedRows > 0) {
                    // Verificar se tem ponto decimal (apenas nas primeiras linhas)
                    bool hasDecimal = false;
                    checkedRows = 0;
                    
                    for (const auto& val : column) {
                        if (val.empty()) continue;
                        
                        if (checkedRows >= 50) break; // Limite menor aqui
                        
                        checkedRows++;
                        
                        if (val.find('.') != std::string::npos) {
                            hasDecimal = true;
                            break;
                        }
                    }
                    
                    dtype = hasDecimal ? "float64" : "int64";
                }
                
                // Impressão da linha
                std::cout << std::setw(5) << i 
                        << std::setw(25) << colName 
                        << std::setw(15) << nonNullCount << " non-null" 
                        << std::setw(15) << dtype << std::endl;
            }
            
            // Resumo de memória (simplificado e com limite de segurança)
            std::cout << std::endl;
            std::cout << "dtypes: mixed" << std::endl;
            
            // Estimativa simplificada baseada apenas no número de elementos
            size_t memoryUsage = m_activeColumns.size() * rowCount() * sizeof(std::string);
            memoryUsage += m_activeColumns.size() * sizeof(std::string); // cabeçalhos
            
            std::cout << "memory usage: ~" << (memoryUsage / 1024) << " KB" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Erro ao exibir informações do DataFrame: " << e.what() << std::endl;
        }
    }    
    // Método para visualizar as primeiras n linhas (como o método head() do pandas)
    void head(size_t n = 5) const {
        n = std::min(n, rowCount());
        
        // Imprimir o cabeçalho
        for (size_t i = 0; i < m_activeColumns.size(); ++i) {
            std::cout << std::setw(20) << m_activeColumns[i];
        }
        std::cout << std::endl;
        
        // Imprimir a linha de separação
        for (size_t i = 0; i < m_activeColumns.size(); ++i) {
            std::cout << std::string(20, '-');
        }
        std::cout << std::endl;
        
        // Imprimir as primeiras n linhas
        for (size_t rowIdx = 0; rowIdx < n; ++rowIdx) {
            auto row = getRow(rowIdx);
            for (size_t colIdx = 0; colIdx < row.size(); ++colIdx) {
                std::cout << std::setw(20) << row[colIdx];
            }
            std::cout << std::endl;
        }
    }
    
    // Acesso aos dados brutos (considerando apenas colunas ativas)
    CSV::DataFrame data() const { 
        if (m_activeColumns.size() == m_csv.headers().size()) {
            // Se todas as colunas estão ativas, retorna todos os dados
            return m_csv.data();
        } else {
            // Senão, filtra os dados para incluir apenas as colunas ativas
            CSV::DataFrame filteredData;
            filteredData.reserve(m_csv.rowCount());
            
            for (size_t i = 0; i < m_csv.rowCount(); ++i) {
                filteredData.push_back(getRow(i));
            }
            
            return filteredData;
        }
    }
    
    // Salvar dados
    bool save(const std::string& filename, char delimiter = ',') const { 
        if (m_activeColumns.size() == m_csv.headers().size()) {
            // Se todas as colunas estão ativas, salva diretamente
            return m_csv.save(filename, delimiter); 
        } else {
            // Senão, cria um CSV temporário com apenas as colunas ativas
            CSV tempCsv;
            
            // Configurar cabeçalho
            std::vector<std::string> headers = m_activeColumns;
            
            // Configurar dados
            CSV::DataFrame data;
            for (size_t i = 0; i < m_csv.rowCount(); ++i) {
                data.push_back(getRow(i));
            }
            
            // Criar CSV temporário manualmente e salvar
            std::ofstream file(filename);
            if (!file.is_open()) {
                return false;
            }
            
            // Escrever cabeçalho
            for (size_t i = 0; i < headers.size(); ++i) {
                if (i > 0) file << delimiter;
                file << headers[i];
            }
            file << '\n';
            
            // Escrever dados
            for (const auto& row : data) {
                for (size_t i = 0; i < row.size(); ++i) {
                    if (i > 0) file << delimiter;
                    file << row[i];
                }
                file << '\n';
            }
            
            return true;
        }
    }

    // Add this method to your DataFrame class in cppandas.hpp

    /**
 * @brief Remove rows with missing values (similar to pandas dropna())
 * @param subset Vector of column names to consider for NA values. If empty, all columns are used.
 * @param how String specifying how to drop rows. "any" drops rows with any NA value, "all" drops only if all values are NA.
 * @return A new DataFrame with NA rows removed
 */
    DataFrame dropna(const std::vector<std::string>& subset = {}, const std::string& how = "any") const {
        DataFrame result(*this);

        // Determine which columns to check for NA values
        std::vector<std::string> columnsToCheck;
        if (subset.empty()) {
            // If no subset specified, use all active columns
            columnsToCheck = m_activeColumns;
        } else {
            // Verify all columns in subset exist
            for (const auto& col : subset) {
                if (std::find(m_activeColumns.begin(), m_activeColumns.end(), col) == m_activeColumns.end()) {
                    throw ColumnNotFoundException({col});
                }
            }
            columnsToCheck = subset;
        }

        // Get indices of rows to keep
        std::vector<size_t> rowsToKeep;

        for (size_t rowIndex = 0; rowIndex < m_csv.rowCount(); ++rowIndex) {
            bool keepRow = true;

            if (how == "any") {
                // Drop row if ANY specified column has NA/empty value
                for (const auto& colName : columnsToCheck) {
                    try {
                        const std::string& cellValue = m_csv.getColumn(colName)[rowIndex];
                        if (cellValue.empty()) {
                            keepRow = false;
                            break;
                        }
                    } catch (const std::exception& e) {
                        keepRow = false;
                        break;
                    }
                }
            } else if (how == "all") {
                // Drop row only if ALL specified columns have NA/empty values
                keepRow = false;
                for (const auto& colName : columnsToCheck) {
                    try {
                        const std::string& cellValue = m_csv.getColumn(colName)[rowIndex];
                        if (!cellValue.empty()) {
                            keepRow = true;
                            break;
                        }
                    } catch (const std::exception& e) {
                        // Continue checking other columns
                    }
                }
            } else {
                throw std::invalid_argument("Invalid 'how' parameter: must be 'any' or 'all'");
            }

            if (keepRow) {
                rowsToKeep.push_back(rowIndex);
            }
        }

        // Create new data with only rows to keep
        CSV::DataFrame newData;
        newData.reserve(rowsToKeep.size());

        for (size_t index : rowsToKeep) {
            newData.push_back(m_csv.getRow(index));
        }

        // Create a new CSV with the same headers but filtered data
        CSV newCSV;

        // Set up a temporary CSV file to load from
        std::string tempFilename = "temp_dropna_" + std::to_string(reinterpret_cast<uintptr_t>(this)) + ".csv";
        std::ofstream tempFile(tempFilename);

        // Write headers
        for (size_t i = 0; i < m_csv.headers().size(); ++i) {
            if (i > 0) tempFile << m_csv.getDelimiter();
            tempFile << m_csv.headers()[i];
        }
        tempFile << "\n";

        // Write filtered data
        for (const auto& row : newData) {
            for (size_t i = 0; i < row.size(); ++i) {
                if (i > 0) tempFile << m_csv.getDelimiter();
                tempFile << row[i];
            }
            tempFile << "\n";
        }

        tempFile.close();

        // Load the CSV file to properly initialize all CSV internals
        newCSV.load(tempFilename, true, m_csv.getDelimiter());

        // Clean up the temporary file
        std::remove(tempFilename.c_str());

        // Create result DataFrame
        DataFrame filteredDF(newCSV);
        filteredDF.m_activeColumns = m_activeColumns;

        return filteredDF;
    }

    /**
     * @brief Converte uma string em double
     * @param str String a ser convertida
     * @return Valor double ou NaN se a conversão falhar
     */
    static double toDouble(const std::string& str) {
        try {
            if (str.empty()) {
                return std::numeric_limits<double>::quiet_NaN();
            }
            return std::stod(str);
        } catch (...) {
            return std::numeric_limits<double>::quiet_NaN();
        }
    }

    /**
     * @brief Converte uma coluna em valores numéricos
     * @param column Coluna a ser convertida
     * @return Vetor de valores numéricos
     */
    std::vector<double> columnToNumeric(const CSV::Column& column) const {
        std::vector<double> numericValues;
        numericValues.reserve(column.size());

        for (const auto& value : column) {
            numericValues.push_back(toDouble(value));
        }

        return numericValues;
    }

    /**
     * @brief Calcula a média de uma coluna numérica
     * @param columnName Nome da coluna
     * @return Média dos valores
     */
    double mean(const std::string& columnName) const {
        auto column = getColumn(columnName);
        auto numericValues = columnToNumeric(column);

        // Contar valores não-NaN
        size_t count = 0;
        double sum = 0.0;

        for (double value : numericValues) {
            if (!std::isnan(value)) {
                sum += value;
                count++;
            }
        }

        return count > 0 ? sum / count : std::numeric_limits<double>::quiet_NaN();
    }

    /**
     * @brief Calcula a variância de uma coluna numérica
     * @param columnName Nome da coluna
     * @return Variância dos valores
     */
    double var(const std::string& columnName) const {
        auto column = getColumn(columnName);
        auto numericValues = columnToNumeric(column);

        double meanValue = mean(columnName);
        if (std::isnan(meanValue)) {
            return std::numeric_limits<double>::quiet_NaN();
        }

        // Calcular variância
        size_t count = 0;
        double sumSquares = 0.0;

        for (double value : numericValues) {
            if (!std::isnan(value)) {
                double diff = value - meanValue;
                sumSquares += diff * diff;
                count++;
            }
        }

        return count > 1 ? sumSquares / (count - 1) : std::numeric_limits<double>::quiet_NaN();
    }

    /**
     * @brief Calcula o desvio padrão de uma coluna numérica
     * @param columnName Nome da coluna
     * @return Desvio padrão dos valores
     */
    double std(const std::string& columnName) const {
        double variance = var(columnName);
        return std::isnan(variance) ? variance : std::sqrt(variance);
    }

    /**
     * @brief Calcula o valor mínimo de uma coluna numérica
     * @param columnName Nome da coluna
     * @return Valor mínimo
     */
    double min(const std::string& columnName) const {
        auto column = getColumn(columnName);
        auto numericValues = columnToNumeric(column);

        double minValue = std::numeric_limits<double>::infinity();
        bool hasValidValue = false;

        for (double value : numericValues) {
            if (!std::isnan(value)) {
                minValue = std::min(minValue, value);
                hasValidValue = true;
            }
        }

        return hasValidValue ? minValue : std::numeric_limits<double>::quiet_NaN();
    }

    /**
     * @brief Calcula o valor máximo de uma coluna numérica
     * @param columnName Nome da coluna
     * @return Valor máximo
     */
    double max(const std::string& columnName) const {
        auto column = getColumn(columnName);
        auto numericValues = columnToNumeric(column);

        double maxValue = -std::numeric_limits<double>::infinity();
        bool hasValidValue = false;

        for (double value : numericValues) {
            if (!std::isnan(value)) {
                maxValue = std::max(maxValue, value);
                hasValidValue = true;
            }
        }

        return hasValidValue ? maxValue : std::numeric_limits<double>::quiet_NaN();
    }

    /**
     * @brief Calcula o quantil de uma coluna numérica
     * @param columnName Nome da coluna
     * @param q Valor do quantil (entre 0 e 1)
     * @return Valor do quantil
     */
    double quantile(const std::string& columnName, double q) const {
        if (q < 0.0 || q > 1.0) {
            throw std::invalid_argument("Quantile value must be between 0 and 1");
        }

        auto column = getColumn(columnName);
        auto numericValues = columnToNumeric(column);

        // Filtrar valores NaN
        std::vector<double> validValues;
        validValues.reserve(numericValues.size());

        for (double value : numericValues) {
            if (!std::isnan(value)) {
                validValues.push_back(value);
            }
        }

        if (validValues.empty()) {
            return std::numeric_limits<double>::quiet_NaN();
        }

        // Ordenar valores
        std::sort(validValues.begin(), validValues.end());

        // Calcular índice
        double index = q * (validValues.size() - 1);
        size_t lowerIndex = static_cast<size_t>(index);
        size_t upperIndex = std::min(lowerIndex + 1, validValues.size() - 1);
        double fraction = index - lowerIndex;

        // Interpolar
        return validValues[lowerIndex] + fraction * (validValues[upperIndex] - validValues[lowerIndex]);
    }

    /**
     * @brief Calcula os quantis para todas as colunas
     * @param q Valor do quantil (entre 0 e 1)
     * @return Vetor de valores de quantil para cada coluna ativa
     */
    std::vector<double> quantile(double q) const {
        std::vector<double> result;
        result.reserve(m_activeColumns.size());

        for (const auto& colName : m_activeColumns) {
            result.push_back(quantile(colName, q));
        }

        return result;
    }

    /**
     * @brief Encontra a moda (valor mais frequente) de uma coluna
     * @param columnName Nome da coluna
     * @return Moda da coluna
     */
    double mode(const std::string& columnName) const {
        auto column = getColumn(columnName);
        auto numericValues = columnToNumeric(column);

        // Contar frequências
        std::map<double, size_t> frequencies;

        for (double value : numericValues) {
            if (!std::isnan(value)) {
                frequencies[value]++;
            }
        }

        if (frequencies.empty()) {
            return std::numeric_limits<double>::quiet_NaN();
        }

        // Encontrar valor mais frequente
        double modeValue = frequencies.begin()->first;
        size_t maxFrequency = frequencies.begin()->second;

        for (const auto& pair : frequencies) {
            if (pair.second > maxFrequency) {
                modeValue = pair.first;
                maxFrequency = pair.second;
            }
        }

        return modeValue;
    }
};



class CPPandas {
public:
    static DataFrame read_csv(const std::string& filename, bool hasHeader = true, char delimiter = ',') {
        CSV csv(filename, hasHeader, delimiter);
        return DataFrame(csv);
    }
};

} // namespace CPPandas

#endif // CPPANDAS_HPP
