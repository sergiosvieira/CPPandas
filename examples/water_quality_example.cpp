#include <iostream>
#include "cppandas/cppandas.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <caminho_do_arquivo_csv>" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    auto df = CPPandas::CPPandas::read_csv(filename);

    // Selecionar colunas específicas
    std::vector<std::string> columns = {
        "Salinity (ppt)",
        "Dissolved Oxygen (mg/L)",
        "pH (standard units)",
        "Air Temp (?F)"
    };

    try {
        // Selecionar colunas e remover linhas com valores NaN
        auto df_selection = df[columns];
        df_selection = df_selection.dropna();

        // Exibir informações do DataFrame
        std::cout << "DataFrame info:" << std::endl;
        df_selection.info();
        std::cout << std::endl;

        // Criar um StandardScaler para normalizar os dados
        CPPandas::StandardScaler scaler;
        auto df_selection_scaled = scaler.fit_transform(df_selection);

        // Visualizar os primeiros registros dos dados normalizados
        std::cout << "Dados normalizados (primeiras 5 linhas):" << std::endl;
        df_selection_scaled.head(5);
        std::cout << std::endl;

        // Mostrar estatísticas dos dados normalizados
        auto desc_scaled = df_selection_scaled.describe();
        std::cout << "Estatísticas dos dados normalizados:" << std::endl;
        desc_scaled.print();
        std::cout << std::endl;

        // Criar um boxplot dos dados normalizados
        CPPandas::BoxPlot::plot(df_selection_scaled,
                                "Boxplot das Features Selecionadas (StandardScaler)",
                                "Features",
                                "boxplot_features.html");

        std::cout << "Boxplot gerado em 'boxplot_features.html'" << std::endl;
        std::cout << "Por favor, abra este arquivo em um navegador para visualizar o gráfico." << std::endl;

        df_selection.hist(30, "histograma_features.html");

        // Método 2: Alternativa usando a classe Histogram (similar à interface de BoxPlot)
        CPPandas::Histogram::plot(df_selection, 30, "histograma_features2.html");

        std::cout << "Histogramas gerados em 'histograma_features.html' e 'histograma_features2.html'" << std::endl;
        std::cout << "Por favor, abra estes arquivos em um navegador para visualizar os gráficos." << std::endl;


    } catch (const CPPandas::ColumnNotFoundException& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
