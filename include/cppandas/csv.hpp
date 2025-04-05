/**
 * @file csv.hpp
 * @brief Definição da classe CSV para leitura e manipulação de arquivos CSV
 * @author CPPandas Team
 */

#ifndef CPPANDAS_CSV_HPP
#define CPPANDAS_CSV_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <fstream>

namespace CPPandas {

using VectorStr = std::vector<std::string>;


/**
 * @class CSV
 * @brief Classe para leitura e manipulação de arquivos CSV
 */
class CSV {
public:
    /**
     * @brief Tipo para representar uma linha de dados CSV
     */
    using Row = VectorStr;
    
    /**
     * @brief Tipo para representar uma coluna de dados CSV
     */
    using Column = VectorStr;
    
    /**
     * @brief Tipo para representar o conjunto de dados CSV
     */
    using DataFrame = std::vector<Row>;

    /**
     * @brief Construtor padrão
     */
    CSV();
    
    /**
     * @brief Construtor com arquivo
     * @param filename Nome do arquivo CSV a ser lido
     * @param hasHeader Se o arquivo possui uma linha de cabeçalho
     * @param delimiter Caractere delimitador dos campos
     */
    CSV(const std::string& filename, bool hasHeader = true, char delimiter = ',');
    
    /**
     * @brief Destrutor
     */
    ~CSV();
    
    /**
     * @brief Carrega um arquivo CSV
     * @param filename Nome do arquivo CSV a ser lido
     * @param hasHeader Se o arquivo possui uma linha de cabeçalho
     * @param delimiter Caractere delimitador dos campos
     * @return true se o arquivo foi carregado com sucesso, false caso contrário
     */
    bool load(const std::string& filename, bool hasHeader = true, char delimiter = ',');
    
    /**
     * @brief Obtém o número de linhas
     * @return Número de linhas no arquivo CSV (excluindo cabeçalho)
     */
    size_t rowCount() const;
    
    /**
     * @brief Obtém o número de colunas
     * @return Número de colunas no arquivo CSV
     */
    size_t columnCount() const;
    
    /**
     * @brief Obtém os nomes das colunas
     * @return Vetor com os nomes das colunas
     */
    const VectorStr& headers() const;
    
    /**
     * @brief Obtém uma linha específica
     * @param rowIndex Índice da linha desejada (0-based)
     * @return Linha como um vetor de strings
     */
    Row getRow(size_t rowIndex) const;
    
    /**
     * @brief Obtém uma coluna pelo nome
     * @param columnName Nome da coluna
     * @return Coluna como um vetor de strings
     */
    Column getColumn(const std::string& columnName) const;
    
    /**
     * @brief Obtém uma coluna pelo índice
     * @param columnIndex Índice da coluna (0-based)
     * @return Coluna como um vetor de strings
     */
    Column getColumn(size_t columnIndex) const;
    
    /**
     * @brief Obtém todos os dados
     * @return Matriz com todos os dados
     */
    const DataFrame& data() const;
    
    /**
     * @brief Salva os dados em um novo arquivo CSV
     * @param filename Nome do arquivo para salvar
     * @param delimiter Caractere delimitador dos campos
     * @return true se o arquivo foi salvo com sucesso, false caso contrário
     */
    bool save(const std::string& filename, char delimiter = ',') const;

    char getDelimiter() const {
        return this->m_delimiter;
    }

private:
    DataFrame m_data;                      ///< Dados do arquivo CSV
    VectorStr m_headers;    ///< Nomes das colunas
    std::unordered_map<std::string, size_t> m_headerMap; ///< Mapeamento de nomes para índices
    bool m_hasHeader;                      ///< Se o arquivo tem cabeçalho
    char m_delimiter;                      ///< Delimitador usado no arquivo
    
    /**
     * @brief Processa uma linha do CSV
     * @param line Linha a ser processada
     * @param delimiter Caractere delimitador
     * @return Vetor com os campos da linha
     */
    Row parseLine(const std::string& line, char delimiter) const;
};

} // namespace CPPandas

#endif // CPPANDAS_CSV_HPP
