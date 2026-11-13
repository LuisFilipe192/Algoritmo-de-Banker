# Algoritmo do Banqueiro

## Descrição do projeto

O Algoritmo do Banqueiro é um projeto desenvolvido em C que tem como objetivo evitar a ocorrência de deadlock no gerenciamento de recursos de um sistema operacional, com base no algoritmo apresentado no livro *Operating System Concepts* (Silberschatz et al., 10ª edição).

O sistema simula um "banco" que possui uma quantidade limitada de instâncias de cada tipo de recurso e vários clientes que solicitam e liberam esses recursos. Cada cliente declara previamente a demanda máxima de cada recurso que poderá utilizar.

Antes de atender uma solicitação, o programa verifica se, após a alocação, o sistema continuará em um estado seguro, ou seja, se existe alguma ordem em que todos os clientes conseguem terminar. Caso a alocação leve a um estado inseguro, a solicitação é negada.

A quantidade de clientes e de tipos de recurso não é fixa: ela é definida a partir dos argumentos da linha de comando e do arquivo `customer.txt`. Os comandos são lidos do arquivo `commands.txt`, e o resultado de cada um é gravado no arquivo `result.txt`.

O projeto foi desenvolvido em C para ambiente Linux, utilizando o Ubuntu por meio do WSL.

## Estrutura dos arquivos

O projeto está organizado da seguinte forma:

- `banker.c` — contém a implementação do algoritmo do banqueiro, a leitura e validação dos arquivos de entrada e a geração do arquivo de saída.
- `Makefile` — utilizado para compilar o programa e remover os arquivos gerados.
- `README.md` — documentação do projeto.
- `evidencias.log` — registro dos testes executados durante o desenvolvimento.

Os arquivos `customer.txt` e `commands.txt` são fornecidos pelo professor e devem estar no mesmo diretório do executável.

## Como compilar

Para compilar o projeto, basta executar o comando:

```bash
make
```

O comando gera o executável:

```
banker
```

Para remover o executável e o arquivo de saída gerado, utilize:

```bash
make clean
```

## Como executar

Após compilar o projeto, execute o programa informando a quantidade de instâncias de cada tipo de recurso:

```bash
./banker 10 5 7
```

Nesse exemplo, o sistema possui três tipos de recurso, com 10, 5 e 7 instâncias, respectivamente.

O programa lê os arquivos `customer.txt` e `commands.txt` e gera o arquivo `result.txt` com o resultado de cada comando.

## Arquivos de entrada

### customer.txt

Cada linha representa um cliente e contém a demanda máxima de cada recurso, separada por vírgulas. O número de linhas define a quantidade de clientes:

```
7,5,3
3,2,2
9,0,2
2,2,2
4,3,3
```

### commands.txt

Cada linha contém um comando:

- `RQ <cliente> <valores>` — o cliente solicita recursos.
- `RL <cliente> <valores>` — o cliente libera recursos.
- `*` — imprime as matrizes MAXIMUM, ALLOCATION e NEED e o vetor AVAILABLE.

## Funcionamento do algoritmo

### Estruturas de dados

O programa utiliza as seguintes estruturas, alocadas dinamicamente:

- `available` — quantidade de instâncias livres de cada recurso.
- `maximum` — demanda máxima de cada cliente.
- `allocation` — quantidade de recursos alocada a cada cliente.
- `need` — quantidade que cada cliente ainda pode solicitar, calculada como `maximum − allocation`.

### Leitura das entradas

A função `parse_args()` lê os argumentos da linha de comando e define a quantidade de tipos de recurso e o vetor `available`.

A função `load_customers()` lê o arquivo `customer.txt` em duas passadas. Na primeira, valida o formato e conta os clientes. Na segunda, após a alocação das matrizes, preenche as matrizes `maximum` e `need`.

A função `validate_commands()` verifica todas as linhas do arquivo `commands.txt` antes que qualquer comando seja executado.

### Solicitação de recursos

A função `request_resources()` trata o comando `RQ` e segue a seguinte ordem:

1. Se a solicitação ultrapassa o `need` do cliente, ela é negada por exceder a demanda máxima.
2. Se a solicitação ultrapassa o `available`, ela é negada por falta de recursos.
3. Caso contrário, a alocação é simulada, atualizando `available`, `allocation` e `need`.
4. Se o estado resultante for inseguro, a simulação é desfeita e a solicitação é negada. Caso contrário, a alocação é confirmada.

### Verificação de estado seguro

A função `is_safe()` utiliza um vetor `work`, cópia de `available`, e um vetor `finish`. Ela procura repetidamente um cliente ainda não finalizado cujo `need` possa ser atendido com o `work` atual. Quando encontra, considera que esse cliente termina e devolve seus recursos, somando a sua alocação ao `work`.

O estado é considerado seguro somente se todos os clientes puderem ser finalizados.

### Liberação de recursos

A função `release_resources()` trata o comando `RL`. Ela verifica se o cliente não está liberando mais do que possui e, em seguida, devolve os recursos ao `available`, atualizando `allocation` e `need`.

### Impressão do estado

A função `print_state()` trata o comando `*`, imprimindo as matrizes MAXIMUM, ALLOCATION e NEED de cada cliente e o vetor AVAILABLE.

## Tratamento de erros

O programa valida todas as entradas antes de executar os comandos. Quando encontra um erro, exibe uma mensagem no terminal e encerra com código de saída 1, sem gerar um `result.txt` incompleto.

São tratados os seguintes casos:

- Ausência de argumentos ou argumentos que não são inteiros não negativos.
- Arquivo `customer.txt` ou `commands.txt` inexistente ou que não pode ser aberto.
- Linhas com formatação inválida, como letras, campos vazios ou comandos desconhecidos, indicando o número da linha.
- Quantidade de recursos nos arquivos diferente da quantidade de argumentos.
- Comandos que referenciam um cliente inexistente.

Linhas em branco são ignoradas, e quebras de linha no padrão do Windows (`\r\n`) são aceitas. Uma liberação maior do que o alocado ao cliente é negada com a mensagem `was denied because exceed its allocation`, já que esse caso não é definido no enunciado.

## Testes realizados

Durante o desenvolvimento, o programa foi compilado e executado utilizando os arquivos fornecidos pelo professor.

Foram realizados testes para verificar:

- A alocação de recursos quando a solicitação é válida e o estado resultante é seguro.
- A negação de solicitações que excedem a demanda máxima do cliente.
- A negação de solicitações quando não há recursos disponíveis suficientes.
- A negação de solicitações que levariam o sistema a um estado inseguro.
- A liberação de recursos e a impressão do estado do sistema.
- O tratamento de argumentos inválidos, arquivos inexistentes e linhas mal formatadas.

O arquivo `result.txt` gerado com as entradas do professor foi comparado com a saída esperada utilizando o comando `diff`, sem apresentar diferenças.

## Sistema operacional utilizado

O projeto foi desenvolvido e testado no seguinte ambiente:

- Sistema operacional: Ubuntu Linux via WSL.
- Linguagem: C.
- Compilador: GCC.
- Ferramenta de compilação: Make.