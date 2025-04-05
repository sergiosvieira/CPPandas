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

        // Gerar estatísticas descritivas com percentis personalizados
        std::vector<double> percentiles = {0.2, 0.5, 0.7};
        auto desc = df_selection.describe(percentiles);

        // Adicionar variância
        std::vector<double> varValues;
        for (const auto& colName : df_selection.headers()) {
            varValues.push_back(df_selection.var(colName));
        }
        desc.loc("var") = varValues;

        // Adicionar moda
        std::vector<double> modeValues;
        auto modes = df_selection.mode();
        for (size_t i = 0; i < df_selection.headers().size(); ++i) {
            modeValues.push_back(modes.iloc(0));
        }
        desc.loc("mode") = modeValues;

        // Adicionar quartis (já está incluído no describe padrão, mas vamos demonstrar como adicionar manualmente)
        desc.loc("q1") = df_selection.quantile(0.25);
        desc.loc("q2") = df_selection.quantile(0.5);
        desc.loc("q3") = df_selection.quantile(0.75);

        // Exibir estatísticas descritivas
        std::cout << "Estatísticas descritivas:" << std::endl;
        desc.print();

    } catch (const CPPandas::ColumnNotFoundException& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
