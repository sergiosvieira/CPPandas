/**
 * @file csv.cpp
 * @brief Implementação de alta performance da classe CSV sem dependências de threading
 */

 #include "cppandas/csv.hpp"
 #include <fstream>
 #include <stdexcept>
 #include <algorithm>
 #include <memory>
 #include <vector>
 #include <cstring>
 #include <sstream>
 
 namespace CPPandas {
 
 CSV::CSV() : m_hasHeader(false), m_delimiter(',') {}
 
 CSV::CSV(const std::string& filename, bool hasHeader, char delimiter) 
     : m_hasHeader(hasHeader), m_delimiter(delimiter) {
     load(filename, hasHeader, delimiter);
 }
 
 CSV::~CSV() {}
 
 bool CSV::load(const std::string& filename, bool hasHeader, char delimiter) {
     // Usar técnica de buffer otimizado para leitura mais rápida
     std::ifstream file(filename, std::ios::binary | std::ios::ate);
     if (!file.is_open()) {
         return false;
     }
 
     // Configurar atributos da classe
     m_hasHeader = hasHeader;
     m_delimiter = delimiter;
     m_data.clear();
     m_headers.clear();
     m_headerMap.clear();
 
     // Determinar tamanho do arquivo e reservar memória
     std::streamsize fileSize = file.tellg();
     file.seekg(0, std::ios::beg);
 
     // Alocar buffer de memória
     std::unique_ptr<char[]> buffer(new char[fileSize + 1]);
     
     // Ler o arquivo inteiro de uma vez
     if (!file.read(buffer.get(), fileSize)) {
         return false;
     }
     buffer[fileSize] = '\0'; // Garantir terminação apropriada
 
     // Reservar capacidade estimada para reduzir realocações
     size_t estimatedLines = std::count(buffer.get(), buffer.get() + fileSize, '\n') + 1;
     m_data.reserve(estimatedLines);
 
     // Processar o buffer de forma eficiente
     char* lineStart = buffer.get();
     char* current = buffer.get();
     char* end = buffer.get() + fileSize;
 
     // Ler cabeçalho se existir
     if (m_hasHeader) {
         // Encontrar final da primeira linha
         while (current < end && *current != '\n' && *current != '\r') {
             current++;
         }
         
         // Processar cabeçalho
         std::string headerLine(lineStart, current - lineStart);
         m_headers = parseLine(headerLine, m_delimiter);
         
         // Criar mapeamento de nomes para índices
         m_headerMap.reserve(m_headers.size()); // Pré-alocar para evitar rehashing
         for (size_t i = 0; i < m_headers.size(); ++i) {
             m_headerMap[m_headers[i]] = i;
         }
 
         // Avançar para além dos caracteres de nova linha
         if (current < end && *current == '\r') current++;
         if (current < end && *current == '\n') current++;
         
         lineStart = current;
     }
 
     // Processar todas as linhas de uma vez de forma otimizada
     while (lineStart < end) {
         current = lineStart;
         
         // Encontrar fim da linha
         while (current < end && *current != '\n' && *current != '\r') {
             current++;
         }
         
         // Processar linha (pular linhas vazias)
         if (current > lineStart) {
             std::string line(lineStart, current - lineStart);
             Row row = parseLine(line, m_delimiter);
             m_data.push_back(std::move(row));
         }
         
         // Avançar para a próxima linha
         if (current < end && *current == '\r') current++;
         if (current < end && *current == '\n') current++;
         
         lineStart = current;
     }
 
     return true;
 }
 
 size_t CSV::rowCount() const {
     return m_data.size();
 }
 
 size_t CSV::columnCount() const {
     if (m_data.empty()) {
         return 0;
     }
     return m_data[0].size();
 }
 
 const std::vector<std::string>& CSV::headers() const {
     return m_headers;
 }
 
 CSV::Row CSV::getRow(size_t rowIndex) const {
     if (rowIndex >= m_data.size()) {
         throw std::out_of_range("Row index out of range");
     }
     return m_data[rowIndex];
 }
 
 CSV::Column CSV::getColumn(const std::string& columnName) const {
     auto it = m_headerMap.find(columnName);
     if (it == m_headerMap.end()) {
         throw std::out_of_range("Column name not found");
     }
     return getColumn(it->second);
 }
 
 CSV::Column CSV::getColumn(size_t columnIndex) const {
     if (m_data.empty() || columnIndex >= m_data[0].size()) {
         throw std::out_of_range("Column index out of range");
     }
 
     Column column;
     column.reserve(m_data.size());  // Pré-alocar para evitar realocações
 
     for (const auto& row : m_data) {
         column.push_back(row[columnIndex]);
     }
 
     return column;
 }
 
 const CSV::DataFrame& CSV::data() const {
     return m_data;
 }
 
 bool CSV::save(const std::string& filename, char delimiter) const {
     std::ofstream file(filename, std::ios::binary);  // Modo binário para melhor desempenho
     if (!file.is_open()) {
         return false;
     }
 
     // Estimar tamanho do buffer necessário
     size_t estimatedSize = 0;
     
     // Cabeçalho
     if (!m_headers.empty()) {
         for (const auto& header : m_headers) {
             estimatedSize += header.size() + 1; // +1 para o delimitador
         }
         estimatedSize += 1; // Para o \n
     }
     
     // Dados
     for (const auto& row : m_data) {
         for (const auto& cell : row) {
             estimatedSize += cell.size() + 1; // +1 para o delimitador
         }
         estimatedSize += 1; // Para o \n
     }
     
     // Criar buffer
     std::string buffer;
     buffer.reserve(estimatedSize);
     
     // Escrever cabeçalho se existir
     if (!m_headers.empty()) {
         for (size_t i = 0; i < m_headers.size(); ++i) {
             if (i > 0) {
                 buffer += delimiter;
             }
             buffer += m_headers[i];
         }
         buffer += '\n';
     }
 
     // Escrever dados
     for (const auto& row : m_data) {
         for (size_t i = 0; i < row.size(); ++i) {
             if (i > 0) {
                 buffer += delimiter;
             }
             buffer += row[i];
         }
         buffer += '\n';
     }
     
     // Escrever tudo de uma vez
     file.write(buffer.data(), buffer.size());
 
     return true;
 }
 
 CSV::Row CSV::parseLine(const std::string& line, char delimiter) const {
     Row result;
     
     // Evitar realocações estimando número de campos
     size_t estimatedFields = std::count(line.begin(), line.end(), delimiter) + 1;
     result.reserve(estimatedFields);
     
     // Implementação otimizada sem stringstream
     size_t pos = 0;
     size_t nextDelimiter = 0;
     
     while ((nextDelimiter = line.find(delimiter, pos)) != std::string::npos) {
         std::string field = line.substr(pos, nextDelimiter - pos);
         
         // Remover caracteres de nova linha
         field.erase(std::remove_if(field.begin(), field.end(), [](unsigned char c) {
             return c == '\r' || c == '\n';
         }), field.end());
         
         result.push_back(std::move(field));
         pos = nextDelimiter + 1;
     }
     
     // Adicionar o último campo
     std::string lastField = line.substr(pos);
     lastField.erase(std::remove_if(lastField.begin(), lastField.end(), [](unsigned char c) {
         return c == '\r' || c == '\n';
     }), lastField.end());
     
     result.push_back(std::move(lastField));
     
     return result;
 }
 
 } // namespace CPPandas