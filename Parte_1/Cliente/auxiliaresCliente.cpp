#include "auxiliaresCliente.hpp"
#include <cstddef>
#include <stdio.h>
#include <ctime>

void format_time(char *buffer, time_t time);

/* Verifica se o usuario inseriu a quantidade correta de parametros, para o comando escolhido. */
int verificaParametros(char *comando, int quantidade_parametros)
{
    int numPalavras = 0;
    int dentroPalavra = 0;

    for (int i = 0; comando[i] != '\0'; i++)
    {
        if (comando[i] == ' ' || comando[i] == '\t')
            dentroPalavra = 0;

        else
        {
            if (dentroPalavra == 0)
            {
                numPalavras++;
                dentroPalavra = 1;
            }
        }
    }

    return (numPalavras == quantidade_parametros + 1);
}

/* Retorna um vetor de char * para o inicio de cada string do comando */
std::vector<char *> splitComando(char *comando)
{
    std::vector<char *> strings;
    bool dentroPalavra = false;

    for (size_t i = 0; comando[i] != '\0'; i++)
    {
        if (comando[i] == ' ' || comando[i] == '\t')
        {
            // Estava dentro de uma palavra, \0 para indicar fim
            if (dentroPalavra)
            {
                comando[i] = '\0';
            }

            dentroPalavra = false;
        }
        else
        {
            // Encontrou nova palavra, adiciona ao vetor
            if (!dentroPalavra)
            {
                strings.push_back(&comando[i]);
                dentroPalavra = true;
            }
        }
    }

    return strings;
}

void format_time(char *buffer, time_t time)
{
    auto tmp = localtime(&time);

    if (!tmp)
    {
        return;
    }

    strftime(buffer, sizeof(char) * TIME_FORMATTED_BUFFER, TIME_FORMAT, tmp);
}

void print_files(std::vector<File> files)
{
    char mtime_formatted[TIME_FORMATTED_BUFFER]{};
    char atime_formatted[TIME_FORMATTED_BUFFER]{};
    char ctime_formatted[TIME_FORMATTED_BUFFER]{};

    printf(HEADER_FORMAT, "Nome", "Tamanho", "M time", "A time", "C time");

    for (auto file : files)
    {
        format_time(mtime_formatted, file.mtime);
        format_time(atime_formatted, file.atime);
        format_time(ctime_formatted, file.ctime);
        printf(FILE_FORMAT, file.name, file.size, mtime_formatted, atime_formatted, ctime_formatted);
    }
}
