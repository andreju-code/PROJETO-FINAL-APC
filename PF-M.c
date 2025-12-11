#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

#ifdef _WIN32
  #include <conio.h>
  #define CLEAR_CMD "cls"
#endif

#define MAX_CLIENTES 3
#define MAX_INVESTIMENTOS 20
#define CPF_DIGITOS 11

typedef struct {
    char nome[100];
    int dia;
    int mes;
    int ano;
} Data;

typedef struct {
    char nome[60];
    double valorInvestido;
    double taxaAnual; 
} Investimento;

typedef struct {
    char nome[100];
    Data nascimento;
    char cpf[CPF_DIGITOS + 1]; 
    char senha[30]; 
    int numeroConta; 
    double saldo;
    double totalInvestido;
    Investimento investimentos[MAX_INVESTIMENTOS];
    int qtdInvestimentos;
} Cliente;

Cliente clientes[MAX_CLIENTES];
int totalClientes = 0;

void pauseConsole();
void limparTela();
void lerString(const char *prompt, char *buffer, int tamanho);
int lerInteiro(const char *prompt);
double lerDouble(const char *prompt);
char *somenteDigitos(const char *s, char *out);

int calcularIdade(Data nasc);
Data lerData(const char *prompt);
bool validarCPF(const char *cpf);
bool cpfJaCadastrado(const char *cpf);

int gerarNumeroConta();
void cadastrarConta();
int autenticarCliente();
void menuInicial();
void homeCliente(int idx);

void depositar(int idx);
void sacar(int idx);
void investir(int idx);
void simularInvestimento();
double calcularMontante(double capital, double taxaAnual, int meses);

void carregarProdutosPadrao();
double taxaAleatoriaPorCategoria(int categoria); 


void mostrarTelaInicial();

#define NUM_RENDA_FIXA 5
#define NUM_FUNDOS 5
#define NUM_BOLSA 8
#define NUM_TESOURO 4
#define NUM_PREVIDENCIA 3

const char *rendaFixa[NUM_RENDA_FIXA];
const char *fundos[NUM_FUNDOS];
const char *bolsa[NUM_BOLSA];
const char *tesouro[NUM_TESOURO];
const char *previdencia[NUM_PREVIDENCIA];

void pauseConsole() {
    printf("\nPressione qualquer tecla para continuar...");
    #ifdef _WIN32
      getch();
    #else
      getchar();
    #endif
}

void limparTela() {
    system(CLEAR_CMD);
}

void lerString(const char *prompt, char *buffer, int tamanho) {
    printf("%s", prompt);
    if (fgets(buffer, tamanho, stdin) != NULL) {
        size_t ln = strlen(buffer);
        if (ln > 0 && buffer[ln - 1] == '\n') buffer[ln - 1] = '\0';
    } else {
        buffer[0] = '\0';
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {}
    }
}

int lerInteiro(const char *prompt) {
    char tmp[64];
    while (1) {
        lerString(prompt, tmp, sizeof(tmp));
        if (tmp[0] == '\0') continue;
        char *p;
        long val = strtol(tmp, &p, 10);
        if (*p == '\0') return (int)val;
        printf("Entrada inválida. Digite um número inteiro.\n");
    }
}

double lerDouble(const char *prompt) {
    char tmp[128];
    while (1) {
        lerString(prompt, tmp, sizeof(tmp));
        if (tmp[0] == '\0') continue;
        char *p;
        double val = strtod(tmp, &p);
        if (*p == '\0') return val;
        printf("Entrada inválida. Digite um número válido (ex: 1500.50).\n");
    }
}

char* somenteDigitos(const char *s, char *out) {
    int j = 0;
    for (int i = 0; s[i] != '\0'; i++) {
        if (isdigit((unsigned char)s[i])) out[j++] = s[i];
    }
    out[j] = '\0';
    return out;
}

Data lerData(const char *prompt) {
    Data d = {0,0,0};
    char buf[64];
    while (1) {
        lerString(prompt, buf, sizeof(buf));
        if (sscanf(buf, "%d/%d/%d", &d.dia, &d.mes, &d.ano) == 3 ||
            sscanf(buf, "%d-%d-%d", &d.dia, &d.mes, &d.ano) == 3 ||
            sscanf(buf, "%d %d %d", &d.dia, &d.mes, &d.ano) == 3) {
            if (d.ano > 1900 && d.ano <= 2100 && d.mes >= 1 && d.mes <= 12 && d.dia >=1 && d.dia <=31) {
                int diasMes[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
                int bissexto = ( (d.ano%4==0 && d.ano%100!=0) || (d.ano%400==0) );
                if (bissexto) diasMes[2] = 29;
                if (d.dia <= diasMes[d.mes]) return d;
            }
        }
        printf("Data inválida. Formato correto: dd/mm/aaaa\n");
    }
}

int calcularIdade(Data nasc) {
    time_t t = time(NULL);
    struct tm hoje = *localtime(&t);
    int anoAtual = hoje.tm_year + 1900;
    int mesAtual = hoje.tm_mon + 1;
    int diaAtual = hoje.tm_mday;
    int idade = anoAtual - nasc.ano;
    if (mesAtual < nasc.mes || (mesAtual == nasc.mes && diaAtual < nasc.dia)) idade--;
    return idade;
}

bool validarCPF(const char *cpf) {
    if (!cpf) return false;
    if (strlen(cpf) != 11) return false;

    bool todosIguais = true;
    for (int i = 1; i < 11; i++) if (cpf[i] != cpf[0]) { todosIguais = false; break; }
    if (todosIguais) return false;

    int nums[11];
    for (int i = 0; i < 11; i++) nums[i] = cpf[i] - '0';

    int soma = 0;
    for (int i = 0; i < 9; i++) soma += nums[i] * (10 - i);
    int resto = soma % 11;
    int dig1 = (resto < 2) ? 0 : 11 - resto;
    if (dig1 != nums[9]) return false;

    soma = 0;
    for (int i = 0; i < 10; i++) soma += nums[i] * (11 - i);
    resto = soma % 11;
    int dig2 = (resto < 2) ? 0 : 11 - resto;
    if (dig2 != nums[10]) return false;

    return true;
}

bool cpfJaCadastrado(const char *cpf) {
    for (int i = 0; i < totalClientes; i++) if (strcmp(clientes[i].cpf, cpf) == 0) return true;
    return false;
}

int gerarNumeroConta() {
    int min = 10000000, max = 99999999;
    int n = (rand() % (max - min + 1)) + min;
    bool existe = true; int tent = 0;
    while (existe && tent < 50) {
        existe = false;
        for (int i=0;i<totalClientes;i++) if (clientes[i].numeroConta == n) { existe = true; break; }
        if (existe) n = (rand() % (max - min + 1)) + min;
        tent++;
    }
    return n;
}

void cadastrarConta() {
    if (totalClientes >= MAX_CLIENTES) {
        printf("Limite de cadastros atingido (%d clientes).\n", MAX_CLIENTES);
        pauseConsole();
        return;
    }

    Cliente novo;
    memset(&novo, 0, sizeof(Cliente));
    novo.saldo = 0.0;
    novo.totalInvestido = 0.0;
    novo.qtdInvestimentos = 0;

    limparTela();
    printf("=== ABERTURA DE CONTA - STRATTON OAKMONT ===\n\n");
    lerString("Nome completo: ", novo.nome, sizeof(novo.nome));
    novo.nascimento = lerData("Data de nascimento (dd/mm/aaaa): ");
    int idade = calcularIdade(novo.nascimento);
    if (idade < 18) {
        printf("\nVocê tem %d anos. É necessário ter 18 anos ou mais para abrir conta.\n", idade);
        pauseConsole();
        return;
    }
    char cpfRaw[64], cpfLimpo[64];
    while (1) {
        lerString("CPF (apenas números): ", cpfRaw, sizeof(cpfRaw));
        somenteDigitos(cpfRaw, cpfLimpo);
        if (strlen(cpfLimpo) != 11) { printf("CPF deve ter 11 dígitos. Tente novamente.\n"); continue; }
        if (!validarCPF(cpfLimpo)) { printf("CPF inválido segundo algoritmo. Tente novamente.\n"); continue; }
        if (cpfJaCadastrado(cpfLimpo)) { printf("CPF já cadastrado.\n"); pauseConsole(); return; }
        strcpy(novo.cpf, cpfLimpo);
        break;
    }
    char senha[64];
    while (1) {
        lerString("Senha (apenas números, mínimo 6 dígitos): ", senha, sizeof(senha));
        bool allNum = true;
        int len = (int)strlen(senha);
        if (len < 6) { printf("Senha muito curta. Tente novamente.\n"); continue; }
        for (int i=0;i<len;i++) if (!isdigit((unsigned char)senha[i])) { allNum = false; break; }
        if (!allNum) { printf("Senha deve conter apenas dígitos. Tente novamente.\n"); continue; }
        strcpy(novo.senha, senha);
        break;
    }
    novo.numeroConta = gerarNumeroConta();
    clientes[totalClientes++] = novo;
    printf("\nConta criada com sucesso!\nConta nº: %d\n", novo.numeroConta);
    pauseConsole();
}

int autenticarCliente() {
    if (totalClientes == 0) {
        printf("Nenhum cliente cadastrado.\n");
        pauseConsole();
        return -1;
    }
    char cpfRaw[64], cpfLimpo[64], senha[64];
    lerString("CPF: ", cpfRaw, sizeof(cpfRaw));
    somenteDigitos(cpfRaw, cpfLimpo);
    if (strlen(cpfLimpo) != 11) { printf("CPF inválido.\n"); pauseConsole(); return -1; }
    int idx = -1;
    for (int i=0;i<totalClientes;i++) if (strcmp(clientes[i].cpf, cpfLimpo) == 0) { idx = i; break; }
    if (idx == -1) { printf("CPF não encontrado.\n"); pauseConsole(); return -1; }
    lerString("Senha: ", senha, sizeof(senha));
    if (strcmp(senha, clientes[idx].senha) != 0) { printf("Senha incorreta.\n"); pauseConsole(); return -1; }
    return idx;
}

double calcularMontante(double capital, double taxaAnual, int meses) {
    double taxaMensal = taxaAnual / 12.0;
    double fator = pow(1.0 + taxaMensal, meses);
    return capital * fator;
}

double taxaAleatoriaPorCategoria(int categoria) {
    double minv = 0.04, maxv = 0.14;
    switch (categoria) {
        case 1: minv = 0.04; maxv = 0.14; break; // Renda Fixa
        case 2: minv = 0.06; maxv = 0.16; break; // Fundos
        case 3: minv = 0.10; maxv = 0.20; break; // Bolsa
        case 4: minv = 0.04; maxv = 0.14; break; // Tesouro
        case 5: minv = 0.06; maxv = 0.16; break; // Previdência
        default: minv = 0.05; maxv = 0.10; break;
    }
    double r = (double)rand() / RAND_MAX;
    return minv + r * (maxv - minv);
}

void carregarProdutosPadrao() {
    rendaFixa[0] = "CDB Premium - Banco Alfa";
    rendaFixa[1] = "LCI Banco Verde";
    rendaFixa[2] = "Debênture Curto Prazo";
    rendaFixa[3] = "CDB Pós - Banco Azul";
    rendaFixa[4] = "Renda Fixa Plus";

    fundos[0] = "Fundo Multimercado Alpha";
    fundos[1] = "Fundo Renda Fixa Conservador";
    fundos[2] = "Fundo Ações Moderado";
    fundos[3] = "Fundo Cambial";
    fundos[4] = "Fundo Small Caps";

    bolsa[0] = "PETR4";
    bolsa[1] = "VALE3";
    bolsa[2] = "ITUB4";
    bolsa[3] = "BBDC4";
    bolsa[4] = "WEGE3";
    bolsa[5] = "ABEV3";
    bolsa[6] = "BBAS3";
    bolsa[7] = "JBSS3";

    tesouro[0] = "Tesouro Selic 2027";
    tesouro[1] = "Tesouro IPCA+ 2035";
    tesouro[2] = "Tesouro Prefixado 2029";
    tesouro[3] = "Tesouro IPCA+ 2045";

    previdencia[0] = "Previdência Conservadora";
    previdencia[1] = "Previdência Moderada";
    previdencia[2] = "Previdência Agressiva";
}
void mostrarTelaInicial() {
    printf("************************************************************\n");
    printf("********  STRATTON OAKMONT INVESTIMENTOS  *******************\n");
    printf("************************************************************\n");
    printf("Bem-vindo à Stratton Oakmont Investimentos! \n\n");
    printf("A melhor plataforma de investimentos do Brasil! \n\n");
    printf("1 - Abrir Conta\n");
    printf("2 - Entrar em Conta\n");
    printf("3 - Sair\n\n");
}

void mostrarInvestimentosCliente(int idx) {
    Cliente *c = &clientes[idx];
    if (c->qtdInvestimentos == 0) {
        printf("\nNenhum investimento registrado.\n");
        return;
    }
    printf("\n-- Investimentos (%d) --\n", c->qtdInvestimentos);
    for (int i=0;i<c->qtdInvestimentos;i++) {
        Investimento *inv = &c->investimentos[i];
        printf("\n%s\n", inv->nome);
        printf("Valor investido: R$ %.2f\n", inv->valorInvestido);
        printf("Taxa anual utilizada: %.2f%%\n", inv->taxaAnual * 100.0);
        double r1m = calcularMontante(inv->valorInvestido, inv->taxaAnual, 1) - inv->valorInvestido;
        double r6m = calcularMontante(inv->valorInvestido, inv->taxaAnual, 6) - inv->valorInvestido;
        double r1a = calcularMontante(inv->valorInvestido, inv->taxaAnual, 12) - inv->valorInvestido;
        double r5a = calcularMontante(inv->valorInvestido, inv->taxaAnual, 60) - inv->valorInvestido;
        printf("Provável retorno em 1 mês: R$ %.2f\n", r1m);
        printf("Provável retorno em 6 meses: R$ %.2f\n", r6m);
        printf("Provável retorno em 1 ano: R$ %.2f\n", r1a);
        printf("Provável retorno em 5 anos: R$ %.2f\n", r5a);
    }
}

void homeCliente(int idx) {
    Cliente *c = &clientes[idx];
    int opc;
    while (1) {
        limparTela();
        printf("===============================================\n");
        printf("Cliente: %s\n", c->nome);
        printf("Conta: %d\n", c->numeroConta);
        printf("Saldo: R$ %.2f\t Investido: R$ %.2f\n", c->saldo, c->totalInvestido);
        printf("===============================================\n\n");
        printf("1 - Depositar dinheiro para investir\n");
        printf("2 - Sacar dinheiro\n");
        printf("3 - Investir\n");
        printf("4 - Simular investimento\n");
        printf("5 - Sair da conta\n\n");
        mostrarInvestimentosCliente(idx);
        opc = lerInteiro("\nEscolha uma opção: ");
        switch (opc) {
            case 1: depositar(idx); break;
            case 2: sacar(idx); break;
            case 3: investir(idx); break;
            case 4: simularInvestimento(); break;
            case 5: return;
            default: printf("Opção inválida.\n"); pauseConsole(); break;
        }
    }
}

void depositar(int idx) {
    double v = lerDouble("Valor a depositar (R$): ");
    if (v <= 0) { printf("Valor inválido.\n"); pauseConsole(); return; }
    clientes[idx].saldo += v;
    printf("Depósito efetuado. Novo saldo: R$ %.2f\n", clientes[idx].saldo);
    pauseConsole();
}

void sacar(int idx) {
    double v = lerDouble("Valor a sacar (R$): ");
    if (v <= 0) { printf("Valor inválido.\n"); pauseConsole(); return; }
    if (v > clientes[idx].saldo) { printf("Saldo insuficiente. Saldo atual: R$ %.2f\n", clientes[idx].saldo); pauseConsole(); return; }
    clientes[idx].saldo -= v;
    printf("Saque efetuado. Novo saldo: R$ %.2f\n", clientes[idx].saldo);
    pauseConsole();
}

void investir(int idx) {
    Cliente *c = &clientes[idx];
    if (c->qtdInvestimentos >= MAX_INVESTIMENTOS) {
        printf("Número máximo de investimentos atingido.\n");
        pauseConsole();
        return;
    }

    printf("\nCategorias:\n");
    printf("1 - Renda Fixa\n2 - Fundos\n3 - Bolsa de Valores\n4 - Tesouro Direto\n5 - Previdência\n");
    int cat = lerInteiro("Escolha a categoria (1-5): ");
    const char **lista = NULL;
    int tam = 0;
    switch (cat) {
        case 1: lista = rendaFixa; tam = NUM_RENDA_FIXA; break;
        case 2: lista = fundos; tam = NUM_FUNDOS; break;
        case 3: lista = bolsa; tam = NUM_BOLSA; break;
        case 4: lista = tesouro; tam = NUM_TESOURO; break;
        case 5: lista = previdencia; tam = NUM_PREVIDENCIA; break;
        default: printf("Categoria inválida.\n"); pauseConsole(); return;
    }

    printf("\nProdutos disponíveis:\n");
    for (int i=0;i<tam;i++) printf("%d) %s\n", i+1, lista[i]);
    int prod = lerInteiro("Escolha o produto (número): ");
    if (prod < 1 || prod > tam) { printf("Produto inválido.\n"); pauseConsole(); return; }

    double valor = lerDouble("Valor a investir (R$): ");
    if (valor <= 0) { printf("Valor inválido.\n"); pauseConsole(); return; }
    if (valor > c->saldo) { printf("Saldo insuficiente. Saldo atual: R$ %.2f\n", c->saldo); pauseConsole(); return; }

    double taxa = taxaAleatoriaPorCategoria(cat);

    Investimento inv;
    strncpy(inv.nome, lista[prod-1], sizeof(inv.nome)-1);
    inv.nome[sizeof(inv.nome)-1] = '\0';
    inv.valorInvestido = valor;
    inv.taxaAnual = taxa;

    c->investimentos[c->qtdInvestimentos++] = inv;
    c->saldo -= valor;
    c->totalInvestido += valor;

    printf("\nInvestimento realizado com sucesso!\nProduto: %s\nValor: R$ %.2f\nTaxa anual aplicada: %.2f%%\n", inv.nome, inv.valorInvestido, inv.taxaAnual*100.0);

    double m1 = calcularMontante(valor, taxa, 1);
    double m6 = calcularMontante(valor, taxa, 6);
    double m12 = calcularMontante(valor, taxa, 12);
    double m60 = calcularMontante(valor, taxa, 60);
    printf("\nEstimativas de montante:\n1 mês: R$ %.2f\n6 meses: R$ %.2f\n1 ano: R$ %.2f\n5 anos: R$ %.2f\n", m1, m6, m12, m60);

    pauseConsole();
}

void simularInvestimento() {
    printf("\nSIMULADOR DE INVESTIMENTO (não altera saldo)\n");
    double valor = lerDouble("Valor para simular (R$): ");
    if (valor <= 0) { printf("Valor inválido.\n"); pauseConsole(); return; }
    printf("\nCategorias:\n1-Renda Fixa\n2-Fundos\n3-Bolsa\n4-Tesouro\n5-Previdência\n");
    int cat = lerInteiro("Escolha a categoria: ");
    if (cat < 1 || cat > 5) { printf("Categoria inválida.\n"); pauseConsole(); return; }
    double taxa = taxaAleatoriaPorCategoria(cat);
    double m1 = calcularMontante(valor, taxa, 1);
    double m6 = calcularMontante(valor, taxa, 6);
    double m12 = calcularMontante(valor, taxa, 12);
    printf("\nSimulação - Taxa anual utilizada: %.2f%%\n", taxa * 100.0);
    printf("1 mês: R$ %.2f\n6 meses: R$ %.2f\n1 ano: R$ %.2f\n", m1, m6, m12);
    pauseConsole();
}

void menuInicial() {
    int opc;
    while (1) {
        limparTela();
        mostrarTelaInicial();
        opc = lerInteiro("Escolha uma opção: ");
        switch (opc) {
            case 1: cadastrarConta(); break;
            case 2: {
                int idx = autenticarCliente();
                if (idx >= 0) homeCliente(idx);
                break;
            }
            case 3: printf("Saindo... Obrigado.\n"); return;
            default: printf("Opção inválida.\n"); pauseConsole(); break;
        }
    }
}

int main() {
    setlocale(LC_ALL, "portuguese");
    srand((unsigned int)time(NULL));
    carregarProdutosPadrao();
    menuInicial();
    return 0;
}