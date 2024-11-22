#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <vector>
#include <stack>

#include "planejador.h"

using namespace std;

/* *************************
   * CLASSE IDPONTO        *
   ************************* */

/// Atribuicao de string
void IDPonto::set(string&& S)
{
    t=move(S);
    if (!valid()) t.clear();
}

/* *************************
   * CLASSE IDROTA         *
   ************************* */

/// Atribuicao de string
void IDRota::set(string&& S)
{
    t=move(S);
    if (!valid()) t.clear();
}

/* *************************
   * CLASSE PONTO          *
   ************************* */

/// Distancia entre 2 pontos (formula de haversine)
double haversine(const Ponto& P1, const Ponto& P2)
{
    // Tratar logo pontos identicos
    if (P1.id == P2.id) return 0.0;

    static const double MY_PI = 3.14159265358979323846;
    static const double R_EARTH = 6371.0;
    // Conversao para radianos
    double lat1 = MY_PI*P1.latitude/180.0;
    double lat2 = MY_PI*P2.latitude/180.0;
    double lon1 = MY_PI*P1.longitude/180.0;
    double lon2 = MY_PI*P2.longitude/180.0;

    double cosseno = sin(lat1)*sin(lat2) + cos(lat1)*cos(lat2)*cos(lon1-lon2);
    // Para evitar eventuais erros na funcao acos por imprecisao numerica
    // nas operacoes com double: acos(1.0000001) eh NAN
    if ( cosseno > 1.0 ) cosseno = 1.0;
    if ( cosseno < -1.0 ) cosseno = -1.0;
    // Distancia entre os pontos
    return R_EARTH*acos(cosseno);
}

/* *************************
   * CLASSE PLANEJADOR     *
   ************************* */

/// Torna o mapa vazio
void Planejador::clear()
{
    pontos.clear();
    rotas.clear();
}

/// Retorna um Ponto do mapa, passando a id como parametro.
/// Se a id for inexistente, retorna um Ponto vazio.
Ponto Planejador::getPonto(const IDPonto& Id) const // Eu preciso testar se ela é válida?
{
    // Procura um ponto que corresponde aa Id do parametro

    // Testa se é uma ID válida
    if(Id.valid())
    {
        // Cria um interator para uma lista de pontos
        list<Ponto>::const_iterator itr;
        // Procura o ID na lista de pontos
        itr = find(pontos.begin(), pontos.end(), Id); // Em uma lista de pontos, procura uma ID (Ponto == ID)

        // Em caso de sucesso, retorna o ponto encontrado

        if(itr!=pontos.end())  // Se itr != pontos.end() significa que algum ponto correspondente foi encontrado
        {
            return *itr; // Retorna a desreferenciação do iterator
        }
    }
    // Se nao encontrou, retorna um ponto vazio
    return Ponto();
}

/// Retorna um Rota do mapa, passando a id como parametro.
/// Se a id for inexistente, retorna um Rota vazio.
Rota Planejador::getRota(const IDRota& Id) const
{
    // Procura uma rota que corresponde aa Id do parametro

    // Testa se é uma ID válida
    if(Id.valid())
    {
        // Cria um interator para uma lista de rotas
        list<Rota>::const_iterator itr;
        // Procura o ID na lista de rotas
        itr = find(rotas.begin(), rotas.end(), Id); // Em uma lista de rotas, procura uma ID (Rota == ID)

        // Em caso de sucesso, retorna a rota encontrada
        if(itr!=rotas.end())  // Se itr != rotas.end() significa que alguma rota correspondente foi encontrado
        {
            return *itr; // Retorna a desreferenciação do iterator
        }
    }

    // Se nao encontrou, retorna uma rota vazia
    return Rota();
}

/// Imprime os pontos do mapa no console
void Planejador::imprimirPontos() const
{
    for (const auto& P : pontos)
    {
        cout << P.id << '\t' << P.nome
             << " (" <<P.latitude << ',' << P.longitude << ")\n";
    }
}

/// Imprime as rotas do mapa no console
void Planejador::imprimirRotas() const
{
    for (const auto& R : rotas)
    {
        cout << R.id << '\t' << R.nome << '\t' << R.comprimento << "km"
             << " [" << R.extremidade[0] << ',' << R.extremidade[1] << "]\n";
    }
}

/// Leh um mapa dos arquivos arq_pontos e arq_rotas.
/// Caso nao consiga ler dos arquivos, deixa o mapa inalterado e retorna false.
/// Retorna true em caso de leitura bem sucedida
bool Planejador::ler(const std::string& arq_pontos,
                     const std::string& arq_rotas)
{
    // Listas temporarias para armazenamento dos dados lidos
    list<Ponto> listP;
    list<Rota> listR;
    // Variaveis auxiliares para buscas nas listas
    list<Ponto>::iterator itr_ponto;
    list<Rota>::iterator itr_rota;
    // Variaveis auxiliares para leitura de dados
    Ponto P;
    Rota R;
    string prov;

    // Leh os pontos do arquivo
    try
    {
        // Abre o arquivo de pontos
        ifstream arq(arq_pontos);
        if (!arq.is_open()) throw 1;

        // Leh o cabecalho
        getline(arq,prov);
        if (arq.fail() ||
                prov != "ID;Nome;Latitude;Longitude") throw 2;

        // Leh os pontos
        do
        {
            // Leh a ID
            getline(arq,prov,';');
            if (arq.fail()) throw 3;
            P.id.set(move(prov));
            if (!P.valid()) throw 4;

            // Leh o nome
            getline(arq,prov,';');
            if (arq.fail() || prov.size()<2) throw 5;
            P.nome = move(prov);

            // Leh a latitude
            arq >> P.latitude;
            if (arq.fail()) throw 6;
            arq.ignore(1,';');

            // Leh a longitude
            arq >> P.longitude;
            if (arq.fail()) throw 7;
            arq >> ws;

            // Verifica se jah existe ponto com a mesma ID no conteiner de pontos lidos (listP) - FIND
            // Caso exista, throw 8

            // Se o find não retornar um iterador para listP.end() significa que a ID já existe (ERRO)
            if(find(listP.begin(), listP.end(), P)!=listP.end()) throw 8; // Ponto sobrecarregado == com outro ponto

            // Inclui o ponto na lista de pontos
            listP.push_back(move(P));
        }
        while (!arq.eof());

        // Fecha o arquivo de pontos
        arq.close();
    }
    catch (int i)
    {
        cerr << "Erro " << i << " na leitura do arquivo de pontos "
             << arq_pontos << endl;
        return false;
    }

    // Leh as rotas do arquivo
    try
    {
        // Abre o arquivo de rotas
        ifstream arq(arq_rotas);
        if (!arq.is_open()) throw 1;

        // Leh o cabecalho
        getline(arq,prov);
        if (arq.fail() ||
                prov != "ID;Nome;Extremidade 1;Extremidade 2;Comprimento") throw 2;

        // Leh as rotas
        do
        {
            // Leh a ID
            getline(arq,prov,';');
            if (arq.fail()) throw 3;
            R.id.set(move(prov));
            if (!R.valid()) throw 4;

            // Leh o nome
            getline(arq,prov,';');
            if (arq.fail() || prov.size()<2) throw 4;
            R.nome = move(prov);

            // Leh a id da extremidade[0]
            getline(arq,prov,';');
            if (arq.fail()) throw 6;
            R.extremidade[0].set(move(prov));
            if (!R.extremidade[0].valid()) throw 7;

            // Verifica se a Id corresponde a um ponto no conteiner de pontos lidos (listP)
            // Caso ponto nao exista, throw 8

            // Se o find retorna um iterador para listP.end() significa que a ID não foi achada (ERRO)
            if(find(listP.begin(), listP.end(), R.extremidade[0])==listP.end()) throw 8; //sobrecarga == no ponto em relação a uma id

            // Leh a id da extremidade[1]
            getline(arq,prov,';');
            if (arq.fail()) throw 9;
            R.extremidade[1].set(move(prov));
            if (!R.extremidade[1].valid()) throw 10;

            // Verifica se a Id corresponde a um ponto no conteiner de pontos lidos (listP)
            // Caso ponto nao exista, throw 11

            // Se o find retorna um iterador para listP.end() significa que a ID não foi achada (ERRO)
            if(find(listP.begin(), listP.end(), R.extremidade[1])==listP.end()) throw 11; //sobrecarga == no ponto em relação a uma id


            // Leh o comprimento
            arq >> R.comprimento;
            if (arq.fail()) throw 12;
            arq >> ws;

            // Verifica se jah existe rota com a mesma ID no conteiner de rotas lidas (listR)
            // Caso exista, throw 13

            // Se o find não retornar um iterador para listP.end() significa que a ID já existe (ERRO)
            if(find(listR.begin(), listR.end(), R)!=listR.end()) throw 13; // Rota sobrecarregado == com outra rota

            // Inclui a rota na lista de rotas
            listR.push_back(move(R));
        }
        while (!arq.eof());

        // Fecha o arquivo de rotas
        arq.close();
    }
    catch (int i)
    {
        cerr << "Erro " << i << " na leitura do arquivo de rotas "
             << arq_rotas << endl;
        return false;
    }

    // Soh chega aqui se nao entrou no catch, jah que ele termina com return.
    // Move as listas de pontos e rotas para o planejador.
    pontos = move(listP);
    rotas = move(listR);

    return true;
}

/// *******************************************************************************
/// Calcula o caminho entre a origem e o destino do planejador usando o algoritmo A*
/// *******************************************************************************

/// Noh: os elementos dos conjuntos de busca do algoritmo A*

struct Noh // Elementos dos contêineres Aberto e Fechado
{
    IDPonto id_pt; // Id do ponto
    IDRota id_rt; // Id da rota do antecessor até o ponto
    double g; // Custo passado
    double h; // Custo futuro
    double f() const // Custo total
    {
        return g+h; // Retorna soma do custo passado com o custo futuro
    }

    // Construtor default
    Noh(): id_pt(), id_rt(), g(0.0), h(0.0) {}

    // Sobrecarga de operadores
    bool operator==(const IDPonto& idponto) const   // Sobrecarga do operador == (Noh == ID)
    {
        return (id_pt == idponto); // Um noh é igual a uma ID, se a IDPonto for igual a ID fornecida
    }
    bool operator<(const Noh& n) const
    {
        return (f() < n.f());
    }
};

/// Calcula o caminho entre a origem e o destino do planejador usando o algoritmo A*
/// Retorna o comprimento do caminho encontrado.
/// (<0 se  parametros invalidos ou nao existe caminho).
/// O parametro C retorna o caminho encontrado
/// (vazio se  parametros invalidos ou nao existe caminho).
/// O parametro NA retorna o numero de nos em aberto ao termino do algoritmo A*
/// (<0 se parametros invalidos, retorna >0 mesmo quando nao existe caminho).
/// O parametro NF retorna o numero de nos em fechado ao termino do algoritmo A*
/// (<0 se parametros invalidos, retorna >0 mesmo quando nao existe caminho).
double Planejador::calculaCaminho(const IDPonto& id_origem,
                                  const IDPonto& id_destino,
                                  Caminho& C, int& NA, int& NF)
{
    // Zera o caminho resultado
    C.clear();

    try
    {
        // Mapa vazio
        if (empty()) throw 1;

        // Calcula o ponto que corresponde a id_origem.
        // Se nao existir, throw 4
        Ponto pt_orig = getPonto(id_origem);
        if (!pt_orig.valid()) throw 4;

        // Calcula o ponto que corresponde a id_destino.
        // Se nao existir, throw 5
        Ponto pt_dest = getPonto(id_destino);
        if (!pt_dest.valid()) throw 5;

        /* *****************************  /
        /  IMPLEMENTACAO DO ALGORITMO A*  /
        /  ***************************** */

        Noh atual; // Noh inicial
        atual.id_pt = id_origem; // Id do nó será a Id do ponto de origem
        // atual.id_rt = vazio - já definido no construtor default
        // atual.g = 0.0 - já definido no construtor default
        atual.h = haversine(pt_orig, pt_dest);

        // Inicializa os conjuntos de Noh's
        list<Noh> Aberto;
        vector<Noh> Fechado; // Lista, deque ou vetor?
        Aberto.push_front(atual); // Incluir atual no Aberto

        // Laço principal do algoritmo
        while( (!Aberto.empty())&&(atual.id_pt != id_destino))
        {

            // Lê e exclui o primeiro Noh de Aberto (o de menor custo)
            atual = Aberto.front();
            Aberto.pop_front();

            // Inclui "atual" em Fechado
            Fechado.push_back(atual);

            // Expande se não é a solução
            if(atual.id_pt != id_destino)
            {

                // Gera sucessor de "atual"
                auto suc_itr = rotas.begin();
                while(suc_itr!= rotas.end())  // Repita  DO WHILE
                {
                    // Busca próxima Rota conectada a "atual"
                    suc_itr = find(suc_itr, rotas.end(), atual.id_pt); // Rota == IDPonto

                    if(suc_itr != rotas.end())  // Se existe rota
                    {
                        Rota rota_suc = *suc_itr; // rota_suc recebe a rota encontrada pelo iterador
                        // Gera Noh sucessor
                        Noh suc;
                        // Suc.id_pt deve ser igual a extremidade da rota encontrada diferente da atual
                        if(rota_suc.extremidade[0] == atual.id_pt)
                        {
                            suc.id_pt = rota_suc.extremidade[1];
                        }
                        else
                        {
                            suc.id_pt = rota_suc.extremidade[0];
                        }

                        // Ponto cuja id == suc.id_pt
                        Ponto pt_suc = getPonto(suc.id_pt);
                        suc.id_rt = rota_suc.id;
                        suc.g = atual.g + rota_suc.comprimento;
                        suc.h = haversine(pt_suc, pt_dest);

                        // Inicialmente assume que não existe Noh igual a "suc" nos contêineres
                        bool eh_inedito = true;
                        // Procura Noh igual a "suc" em fechado
                        auto old_itr = find(Fechado.begin(), Fechado.end(), suc.id_pt); // operador= (Noh = IDPonto)
                        if(old_itr != Fechado.end())
                        {
                            eh_inedito = false;   // Noh já existe
                        }
                        else
                        {
                            // Procura Noh igual a "suc" em aberto
                            auto old_itr = find(Aberto.begin(), Aberto.end(), suc.id_pt);
                            if(old_itr != Aberto.end())
                            {
                                //Menor custo total?
                                Noh old = *old_itr;
                                if(suc.f() < old.f())
                                {
                                    // Exclui anterior
                                    Aberto.erase(old_itr);
                                }
                                else
                                {
                                    // Noh já existe
                                    eh_inedito = false;
                                }
                            }
                        }
                        // Já existe?
                        if(eh_inedito)
                        {
                            // Acha "big", 1º Noh de Aber com custo total f() maior que o custo total f() de "suc"
                            auto big_itr = upper_bound(Aberto.begin(), Aberto.end(), suc); // operador < (Noh < Noh)
                            Aberto.insert(big_itr, suc);
                        }
                        ++suc_itr;
                    }
                }
            }
        }

        // Calcula números de nós da busca
        NA = Aberto.size();
        NF = Fechado.size();

        // Inicialmente, caminho vazio
        //stack<Noh> caminho;
        double compr;
        // Encontrou solução ou não?
        if(atual.id_pt != id_destino) compr = -1.0; // Não existe solução
        else
        {
            // Calcula comprimento do caminho
            compr = atual.g;
            // Refaz o caminho, procurando Nohs antecessores em fechado
            while(atual.id_rt != IDRota())
            {
                // Acrescenta par atual no topo (início) de "caminho"
                C.push_front(pair(atual.id_rt, atual.id_pt));

                // Recupera o antecessor
                // Obtém "rota_ant", Rota que levou até "atual"
                Rota rota_ant = getRota(atual.id_rt);
                // Calcula a id do antecessor
                IDPonto id_pt_ant;
                if(rota_ant.extremidade[0] == atual.id_pt)
                {
                    id_pt_ant = rota_ant.extremidade[1];
                }
                else
                {
                    id_pt_ant = rota_ant.extremidade[0];
                }

                // Procura Noh igual a "id_pt_ant" em Fechado
                auto antecessor = find(Fechado.begin(), Fechado.end(), id_pt_ant); // Noh == IDPonto
                atual = *antecessor;
            }
            // Acrescenta origem no topo (início) de "caminho"
            C.push_front(pair(atual.id_rt, atual.id_pt));
        }

        // O try tem que terminar retornando o comprimento calculado
        return compr;  // SUBSTITUA pelo return do valor correto
    }
    catch(int i)
    {
        cerr << "Erro " << i << " no calculo do caminho\n";
    }

    // Soh chega aqui se executou o catch, jah que o try termina sempre com return.
    // Caminho C permanece vazio.
    NA = NF = -1;
    return -1.0;
}
