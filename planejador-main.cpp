#include <iostream>
#include <chrono>
#include "planejador.h"

using namespace std;

int main()
{
  // O planejador de caminhos
  Planejador G;
  // O caminho a ser calculado:
  // Os pontos do caminho
  Caminho C;
  // O numero de nohs gerados no calculo do caminho
  int NA(-1),NF(-1);
  // O comprimento do caminho calculado
  double compr(-1.0);
  // O tempo de calculo do caminho
  double deltaT;

  if (!G.ler("pontos.txt", "rotas.txt"))
  {
    cerr << "Erro na leitura dos arquivos do mapa\n";
    return -1;
  }

  // Variaveis auxiliares
  IDPonto id_origem, id_destino;
  Rota R;
  Ponto P;
  string S;

  int opcao;
  do
  {
    cout << endl;
    cout << "1 - Imprimir pontos\n";
    cout << "2 - Imprimir rotas\n";
    cout << "3 - Calcular e imprimir caminho\n";
    cout << "0 - Sair\n";
    do
    {
      cout << "OPCAO: ";
      cin >> opcao;
    }
    while (opcao<0 || opcao>3);
    switch(opcao)
    {
    case 1:
      cout << "PONTOS:\n";
      G.imprimirPontos();
      break;
    case 2:
      cout << "ROTAS\n";
      G.imprimirRotas();
      break;
    case 3:
      do
      {
        cout << "ID do ponto de origem: ";
        cin >> S;
        id_origem.set(move(S));
      } while (!id_origem.valid());
      do
      {
        cout << "ID do ponto de destino: ";
        cin >> S;
        id_destino.set(move(S));
      } while (!id_destino.valid());

      // Calcula o tempo de execucao do calculo do caminho
      {
        using namespace chrono;

        // Relogio antes da execucao
        steady_clock::time_point t1 = steady_clock::now();
        // Calcula o caminho
        compr = G.calculaCaminho(id_origem,id_destino,C,NA,NF);
        // Relogio depois da execucao
        steady_clock::time_point t2 = steady_clock::now();
        // Diferenca entre os dois instantes de tempo
        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
        // Converte para milessegundos
        deltaT = 1000*time_span.count();
      }

      // Imprime os dados sobre o calculo do caminho
      cout << "Tempo: " << deltaT << "ms\t"
           << "Nohs em aberto: " << NA << " fechado: " << NF << endl;

      // Imprime o comprimento total (-1 se erro ou se nao existe caminho)
      cout << "TOTAL: " << compr << "km\n";

      // Imprime o resultado do calculo
      if (NA<0 || NF<0)
      {
        cout << "Erro no calculo do caminho\n";
      }
      else if (compr<0.0)
      {
        cout << "Algoritmo concluido. Nenhum caminho foi encontrado\n";
      }
      else
      {
        // Imprime as etapas do caminho
        cout << "==========\n";
        for (auto par : C)
        {
          if (par.first == IDRota())
          {
            cout << "De ";
          }
          else
          {
            R = G.getRota(par.first);
            cout << "Por " << R.nome << " ateh ";
          }
          P = G.getPonto(par.second);
          cout << P.nome;
          if (par.first != IDRota()) cout << " (" << R.comprimento << "km)";
          cout << endl;
        }
      }


      break;
    case 0:
    default:
      break;
    }
  }
  while (opcao!=0);

  return 0;
}
