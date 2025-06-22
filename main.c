#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include <math.h>
#include <ctype.h>
// Inclui a biblioteca específica do Windows para funções como Sleep() e system("cls")
#include <windows.h>


// Defines: são como apelidos para valores fixos. Fica mais fácil de mudar depois se precisar.
#define CAPACIDADE_DINAMICA 10   // Número inicial de espaços quando a gente cria um array dinâmico.
#define MINIMO_CAIXA_TROCO 50.0f // Regra de negócio: o caixa não pode ficar com menos de 50 reais pra ter troco.


// Ficha de cadastro para um Usuário do sistema (quem vai operar o programa).
typedef struct {
    char login[13]; // Guarda o nome de usuário (user).
    char senha[9];  // Guarda a senha (password).
    int tipo;       // Define se é Admin (tipo 1) ou Usuário comum (tipo 2).
} Usuario;

// Ficha para uma Categoria de produto (ex: "Alimentos", "Limpeza").
typedef struct {
    int codigo;         // Código único pra identificar a categoria.
    char descricao[50]; // O nome da categoria.
} Categoria;

// Ficha de cadastro para um Cliente da loja.
typedef struct {
    int codigo;                 // Código único do cliente.
    char nome_completo[100];    // Nome completo do cliente.
    char nome_social[50];       // Nome social, para atender a legislação.
    char cpf[12];               // CPF do cliente (só números).
    char rua[100];
    char bairro[50];
    char numero[20];
    char celular[12];           // Celular para contato.
} Cliente;

// Ficha de cadastro para um Produto.
typedef struct {
    int codigo;             // Código único do produto (tipo código de barras).
    char descricao[100];    // Nome do produto.
    int codigo_categoria;   // Categoria pelo código.
    float preco_compra;     // Quanto a loja pagou pelo produto.
    float margem_lucro;     // A porcentagem de lucro em cima do preço de compra.
    float preco_venda;      // O preço final para o cliente (calculado automaticamente).
    int estoque;            // Quantidade atual do produto na prateleira.
    int estoque_minimo;     // Quantidade que dispara o alerta de "comprar mais".
} Produto;

// Ficha para cada item DENTRO de uma única venda. Uma venda pode ter vários itens.
typedef struct {
    int codigo_produto;     // Qual produto foi vendido.
    char descricao[100];    // Nome do produto.
    float preco_venda_item; // Preço na hora da venda.
    int quantidade;         // Quantos foram comprados.
    float total_item;       // Preço * Quantidade.
} ItemVenda;

// Estrutura de Pagamento refatorada. guarda os valores separados
typedef struct {
    int num_venda;              // A qual venda este pagamento pertence.
    float valor_pago_dinheiro;  // Parte paga em dinheiro.
    float valor_pago_cartao;    // Parte paga em cartão.
    float total_pago;           // Soma dos dois valores acima, pra conferir.
} Pagamento;

// Ficha principal que agrupa todas as informações de uma Venda.
typedef struct {
    int num_venda;          // Número único da venda.
    int codigo_cliente;     // Quem comprou.
    int dia;                // Data da venda.
    int mes;
    int ano;
    ItemVenda *itens;       // Ponteiro para a lista de itens comprados nessa venda. É um array dinâmico.
    int quantidade_itens;   // Quantos tipos de produtos diferentes foram comprados.
    float total_venda;      // A soma total da compra.
    char status_pagamento;  // Se a venda está 'a' (Aguardando pagamento) ou 'p' (Paga).
} Venda;

// Ponteiros para os arrays dinâmicos. Eles começam como NULL (vazios).
// A memória para eles será alocada depois com malloc/realloc.
Usuario *usuarios = NULL;
Cliente *clientes = NULL;
Produto *produtos = NULL;
Categoria *categorias = NULL; // Adicionamos o de categorias
Venda *vendas = NULL;
Pagamento *pagamentos = NULL;

// Contadores para saber quantos itens temos em cada array e qual a capacidade atual da memória.
int num_usuarios = 0, capacidade_usuarios = 0;
int num_clientes = 0, capacidade_clientes = 0;
int num_produtos = 0, capacidade_produtos = 0;
int num_categorias = 0, capacidade_categorias = 0;
int num_vendas = 0, capacidade_vendas = 0;
int num_pagamentos = 0, capacidade_pagamentos = 0;

// Ponteiro para guardar as informações do usuário que fez o login.
Usuario *usuario_logado = NULL;

// Variáveis de controle do caixa.
float abertura_caixa = -1.0f;     // Começa com -1.0f pra indicar que o caixa está fechado.
float total_sangrias_dia = 0.0f;  // Acumula as retiradas (sangrias) do dia.

// Limpa a tela do console usando o comando específico do Windows.
void limpar_tela() {
    system("cls");
}

// Limpa o buffer do teclado, importante depois de usar scanf.
// Se não limpar, o "Enter" (\n) que a gente digita fica preso e bagunça o próximo scanf ou fgets.
void limpar_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Mostra uma telinha de boas-vindas com uma arte ASCII de gatinho. WIWIWI WAWAWA WIWIWI WAWAWA
void tela_de_boas_vindas() {
    limpar_tela();
    printf("\n\n\n");
    printf("      /\\_/\\  \n");
    printf("     ( o.o ) \n");
    printf("      > ^ <  \n");
    printf("\n\n\n");
    printf("                   Carregando o sistema...");
    Sleep(2500); //só pra ser tipo um "loading"
}

// Função para mostrar um cabeçalho bonitinho e padronizado em todos os menus.
// Centraliza o título
void exibir_cabecalho_melhorado(const char *titulo_menu) {
    limpar_tela();
    printf("\n");
    // Esses números (201, 187, etc.) são códigos da tabela ASCII para desenhar as bordas
    printf("  %c%s%c\n", 201, "=======================================================", 187);
    printf("  %c%55s%c\n", 186, " ", 186);
    printf("  %c%23s/\\_/\\%24s%c\n", 186, " ", " ", 186);
    printf("  %c%22s( o.o )%24s%c\n", 186, " ", " ", 186);
    printf("  %c%23s> ^ <%25s%c\n", 186, " ", " ", 186);
    printf("  %c%55s%c\n", 186, " ", 186);
    printf("  %c%s%c\n", 204, "=======================================================", 185);

    // Este bloco calcula o espaçamento para centralizar o título do menu.
    int padding = (55 - strlen(titulo_menu)) / 2;
    printf("  %c", 186);
    for(int i = 0; i < padding; i++) printf(" ");
    printf("%s", titulo_menu);
    for(int i = 0; i < 55 - padding - strlen(titulo_menu); i++) printf(" ");
    printf("%c\n", 186);
    printf("  %c%s%c\n", 200, "=======================================================", 188);
    printf("\n");
}

// --- FUNÇÕES DE ALOCAÇÃO DINÂMICA ---

// Ela serve para qualquer um dos nossos arrays dinâmicos (clientes, produtos, etc.).
// Ela verifica se o array está cheio. Se estiver, dobra o tamanho dele.
void configura_capacidade(void **array, int *capacidade, int *contador, size_t tamanho_struct) {
    // Se o número de itens (contador) for igual ou maior que a capacidade atual...
    if (*contador >= *capacidade) {
        // a gente dobra a capacidade. Se era 0, começa com CAPACIDADE_DINAMICA (10).
        *capacidade = (*capacidade == 0) ? CAPACIDADE_DINAMICA : *capacidade * 2;
        // realloc() tenta "esticar" a memória já alocada.
        void *temp = realloc(*array, *capacidade * tamanho_struct);
        if (temp == NULL) { // Se o realloc falhar (muito raro), o programa para.
            printf("Erro crítico: Falha ao alocar memória.\n");
            exit(1); // Encerra o programa com um código de erro.
        }
        *array = temp; // Atualiza o ponteiro original para o novo endereço de memória.
    }
}

// --- FUNÇÕES DE BUSCA ---
// Funções "detetives" que procuram por um item específico em nossos arrays.

// Procura um produto pelo código.
Produto* encontra_produto(int codigo) {
    for (int i = 0; i < num_produtos; i++) { // Percorre todo o array de produtos.
        if (produtos[i].codigo == codigo) { // Se encontrar
            return &produtos[i]; // retorna o endereço de memória do produto encontrado.
        }
    }
    return NULL; // Se não encontrar, retorna NULL (nulo/vazio).
}

// Procura um cliente pelo código. A lógica é a mesma.
Cliente* encontra_cliente(int codigo) {
    for (int i = 0; i < num_clientes; i++) {
        if (clientes[i].codigo == codigo) {
            return &clientes[i];
        }
    }
    return NULL;
}

// Procura uma categoria pelo código.
Categoria* encontra_categoria(int codigo) {
    for (int i = 0; i < num_categorias; i++) {
        if (categorias[i].codigo == codigo) {
            return &categorias[i];
        }
    }
    return NULL;
}

// --- FUNÇÕES DE PERSISTÊNCIA (SALVAR/CARREGAR DADOS) ---
// Essas funções salvam os dados em arquivos no disco e carregam de volta quando o programa inicia.
// Isso evita que a gente perca tudo ao fechar o sistema.
// Usamos arquivos binários (".dat") porque são mais rápidos e ocupam menos espaço que arquivos de texto

// Função genérica para carregar dados de qualquer um dos nossos arquivos.
// Usamos ponteiros para ponteiros (void**) pra poder modificar o ponteiro original (ex: 'usuarios')
void carregar_dados(const char* nome_arquivo, void** array, int* contador, int* capacidade, size_t tamanho_struct) {
    FILE *arquivo = fopen(nome_arquivo, "rb"); // "rb" = read binary (ler arquivo binário).
    if (arquivo) { // Se o arquivo existir.
        void *buffer = malloc(tamanho_struct); // Aloca um "recipiente" temporário.
        if(buffer == NULL) {
            fclose(arquivo);
            return; // Falha na alocação, melhor parar.
        }

        // fread() lê os dados do arquivo e coloca no buffer.
        // O loop continua enquanto houver dados para ler (fread retorna 1).
        while (fread(buffer, tamanho_struct, 1, arquivo) == 1) {
            configura_capacidade(array, capacidade, contador, tamanho_struct); // Garante que há espaço no array.
            // Copia os dados do buffer para a posição correta no array principal.
            memcpy((char*)(*array) + (*contador * tamanho_struct), buffer, tamanho_struct);
            (*contador)++; // Incrementa o contador de itens.
        }
        free(buffer); // Libera o recipiente temporário.
        fclose(arquivo); // Fecha o arquivo.
    }
}

// Função genérica para salvar dados em um arquivo.
void salvar_dados(const char* nome_arquivo, const void* array, int contador, size_t tamanho_struct) {
    FILE *arquivo = fopen(nome_arquivo, "wb"); // "wb" = write binary (escrever arquivo binário).
    if (arquivo) {
        // fwrite() escreve o array inteiro no arquivo de uma vez só.
        fwrite(array, tamanho_struct, contador, arquivo);
        fclose(arquivo);
    }
}

// Função principal do carregamento de todos os dados.
void carregar_todos_os_dados() {
    // Chama a função genérica para cada tipo de dado.
    carregar_dados("usuarios.dat", (void**)&usuarios, &num_usuarios, &capacidade_usuarios, sizeof(Usuario));
    carregar_dados("clientes.dat", (void**)&clientes, &num_clientes, &capacidade_clientes, sizeof(Cliente));
    carregar_dados("produtos.dat", (void**)&produtos, &num_produtos, &capacidade_produtos, sizeof(Produto));
    carregar_dados("categorias.dat", (void**)&categorias, &num_categorias, &capacidade_categorias, sizeof(Categoria));
    carregar_dados("pagamentos.dat", (void**)&pagamentos, &num_pagamentos, &capacidade_pagamentos, sizeof(Pagamento));

    // Carregar vendas é um caso especial porque a struct Venda tem um ponteiro (*itens).
    // Não podemos simplesmente copiar o bloco de memória. Temos que ler a venda, alocar memória para os itens, e depois ler os itens.
    FILE *arquivo_vendas = fopen("vendas.dat", "rb");
    if(arquivo_vendas) {
        Venda v_temp;
        // Lê a parte principal da struct Venda.
        while(fread(&v_temp, sizeof(Venda), 1, arquivo_vendas)) {
            configura_capacidade((void**)&vendas, &capacidade_vendas, &num_vendas, sizeof(Venda));
            vendas[num_vendas] = v_temp; // Copia os dados básicos da venda.
            if (v_temp.quantidade_itens > 0) { // Se essa venda tiver itens
                // alocamos memória para eles
                vendas[num_vendas].itens = malloc(v_temp.quantidade_itens * sizeof(ItemVenda));
                // e lemos os dados dos itens do arquivo, direto para a memória que acabamos de alocar.
                fread(vendas[num_vendas].itens, sizeof(ItemVenda), v_temp.quantidade_itens, arquivo_vendas);
            } else {
                vendas[num_vendas].itens = NULL; // Se não tiver itens, o ponteiro é nulo.
            }
            num_vendas++;
        }
        fclose(arquivo_vendas);
    }
}

// Função do salvamento de todos os dados.
void salvar_todos_os_dados() {
    salvar_dados("usuarios.dat", usuarios, num_usuarios, sizeof(Usuario));
    salvar_dados("clientes.dat", clientes, num_clientes, sizeof(Cliente));
    salvar_dados("produtos.dat", produtos, num_produtos, sizeof(Produto));
    salvar_dados("categorias.dat", categorias, num_categorias, sizeof(Categoria));
    salvar_dados("pagamentos.dat", pagamentos, num_pagamentos, sizeof(Pagamento));

    // Salvar vendas também é especial, pelo mesmo motivo do carregamento.
    FILE *arquivo_vendas = fopen("vendas.dat", "wb");
    if(arquivo_vendas) {
        for(int i = 0; i < num_vendas; i++) {
            // Primeiro, escrevemos os dados da "ficha" da venda.
            fwrite(&vendas[i], sizeof(Venda), 1, arquivo_vendas);
            if (vendas[i].quantidade_itens > 0) {
                // Depois, escrevemos o array de itens associado a essa venda.
                fwrite(vendas[i].itens, sizeof(ItemVenda), vendas[i].quantidade_itens, arquivo_vendas);
            }
        }
        fclose(arquivo_vendas);
    }
}


// --- FUNÇÕES DE AUTENTICAÇÃO E VALIDAÇÃO ---

// Verifica se o usuário tem permissão de Administrador para operações críticas.
int validar_admin() {
    // Se o usuário já logado for admin (tipo 1), libera o acesso direto.
    if (usuario_logado != NULL && usuario_logado->tipo == 1) {
        return 1; // 1 significa "verdadeiro" ou "permitido".
    }

    // Se não, pede as credenciais de um admin na hora.
    char login_tentativa[13];
    char senha_tentativa[9];

    exibir_cabecalho_melhorado("ACESSO RESTRITO");
    printf("  Esta operacao exige privilegios de Administrador.\n");
    printf("  Por favor, insira as credenciais de um Administrador.\n\n");
    printf("  Login de Admin: ");
    scanf("%12s", login_tentativa);
    limpar_buffer();
    printf("  Senha de Admin: ");
    scanf("%8s", senha_tentativa);
    limpar_buffer();

    // Procura por um usuário que seja admin E tenha as credenciais corretas.
    for (int i = 0; i < num_usuarios; i++) {
        // strcmp() compara duas strings. Retorna 0 se forem iguais.
        if (usuarios[i].tipo == 1 && strcmp(usuarios[i].login, login_tentativa) == 0 && strcmp(usuarios[i].senha, senha_tentativa) == 0) {
            printf("\n  Validacao de Administrador bem-sucedida.\n");
            Sleep(1000);
            return 1; // Acesso permitido.
        }
    }

    printf("\n  Credenciais de Administrador invalidas. Operacao cancelada.\n");
    Sleep(1500);
    return 0; // 0 significa "falso" ou "negado".
}

// Controla a tela de login no início do programa.
void login() {
    char login_tentativa[13];
    char senha_tentativa[9];
    int tentativas = 0;

    // Se não houver usuários cadastrados (ex: primeira vez rodando), o sistema não pode iniciar.
    if (num_usuarios == 0) {
        exibir_cabecalho_melhorado("ERRO CRITICO");
        printf("  Nao ha usuarios cadastrados para fazer login.\n");
        exit(1);
    }

    // O usuário tem 3 chances para acertar o login e a senha.
    while (tentativas < 3) {
        exibir_cabecalho_melhorado("TELA DE LOGIN");
        printf("  Login: ");
        scanf("%12s", login_tentativa);
        limpar_buffer();
        printf("  Senha: ");
        scanf("%8s", senha_tentativa);
        limpar_buffer();

        // Procura por um usuário com as credenciais fornecidas.
        for (int i = 0; i < num_usuarios; i++) {
            if (strcmp(usuarios[i].login, login_tentativa) == 0 && strcmp(usuarios[i].senha, senha_tentativa) == 0) {
                // Se achou, guarda as informações do usuário na variável global 'usuario_logado'.
                usuario_logado = &usuarios[i];
                printf("\n  Login bem-sucedido! Bem-vindo, %s.\n", usuario_logado->login);
                Sleep(1500);
                return; // Sai da função de login e continua para o menu principal.
            }
        }
        tentativas++;
        printf("\n  Login ou senha invalidos. Tentativas restantes: %d\n", 3 - tentativas);
        Sleep(1500);
    }

    // Se errar 3 vezes, o programa fecha por segurança.
    printf("\n  Numero maximo de tentativas excedido. Encerrando o sistema.\n");
    exit(0);
}

// --- FUNÇÕES DE CADASTRO ---

// Função especial para criar o primeiro usuário (que tem que ser Admin), quando o sistema roda pela primeira vez.
void criar_primeiro_admin() {
    configura_capacidade((void**)&usuarios, &capacidade_usuarios, &num_usuarios, sizeof(Usuario));
    Usuario *u = &usuarios[num_usuarios]; // Pega o próximo espaço livre no array.

    // Pede o login e valida o tamanho.
    do {
        printf("  Login do novo Administrador (8 a 12 caracteres): ");
        scanf("%12s", u->login);
        limpar_buffer();
        if (strlen(u->login) < 8 || strlen(u->login) > 12) {
            printf("  Erro: O login deve ter entre 8 e 12 caracteres.\n");
        }
    } while (strlen(u->login) < 8 || strlen(u->login) > 12);

    // Pede a senha e valida o tamanho.
    do {
        printf("  Senha (6 a 8 caracteres): ");
        scanf("%8s", u->senha);
        limpar_buffer();
        if (strlen(u->senha) < 6 || strlen(u->senha) > 8) {
            printf("  Erro: A senha deve ter entre 6 e 8 caracteres.\n");
        }
    } while (strlen(u->senha) < 6 || strlen(u->senha) > 8);

    u->tipo = 1; // Define como Administrador.
    num_usuarios++; // Incrementa o contador de usuários.
}

// Cadastra novos usuários (requer permissão de Admin).
void cadastro_usuarios() {
    if (!validar_admin()) return; // Se a validação do admin falhar, a função para aqui.

    configura_capacidade((void**)&usuarios, &capacidade_usuarios, &num_usuarios, sizeof(Usuario));
    Usuario *u = &usuarios[num_usuarios];

    exibir_cabecalho_melhorado("CADASTRO DE USUARIO");
    // Validações de login, senha e tipo.
    do {
        printf("  Login (8 a 12 caracteres): ");
        scanf("%12s", u->login);
        limpar_buffer();
        if (strlen(u->login) < 8 || strlen(u->login) > 12) {
            printf("  Erro: O login deve ter entre 8 e 12 caracteres.\n");
        }
    } while (strlen(u->login) < 8 || strlen(u->login) > 12);

    do {
        printf("  Senha (6 a 8 caracteres): ");
        scanf("%8s", u->senha);
        limpar_buffer();
        if (strlen(u->senha) < 6 || strlen(u->senha) > 8) {
            printf("  Erro: A senha deve ter entre 6 e 8 caracteres.\n");
        }
    } while (strlen(u->senha) < 6 || strlen(u->senha) > 8);

    do {
        printf("  Tipo de usuario (1: Admin, 2: Usuario): ");
        if (scanf("%d", &u->tipo) != 1) u->tipo = 0; // Se não for número, força erro.
        limpar_buffer();
        if (u->tipo != 1 && u->tipo != 2) {
            printf("  Erro: Tipo invalido. Por favor, digite 1 ou 2.\n");
        }
    } while (u->tipo != 1 && u->tipo != 2);

    num_usuarios++;
    printf("\n  Usuario '%s' cadastrado com sucesso!\n", u->login);
    Sleep(1500);
}

// Cadastra novos clientes.
void cadastro_clientes() {
    char continuar;
    do {
        configura_capacidade((void**)&clientes, &capacidade_clientes, &num_clientes, sizeof(Cliente));
        Cliente *c = &clientes[num_clientes];
        c->codigo = num_clientes + 1; // Gera um código sequencial simples.

        exibir_cabecalho_melhorado("CADASTRO DE CLIENTE");
        printf("  Codigo: %d\n\n", c->codigo);

        // Para ler textos com espaços (como nome completo), usamos fgets().
        printf("  Nome Completo: ");
        fgets(c->nome_completo, 100, stdin); c->nome_completo[strcspn(c->nome_completo, "\n")] = 0; // Remove o Enter
        printf("  Nome Social: ");
        fgets(c->nome_social, 50, stdin); c->nome_social[strcspn(c->nome_social, "\n")] = 0;
        printf("  CPF (so numeros): ");
        fgets(c->cpf, 12, stdin); c->cpf[strcspn(c->cpf, "\n")] = 0;
        printf("  Rua: ");
        fgets(c->rua, 100, stdin); c->rua[strcspn(c->rua, "\n")] = 0;
        printf("  Numero: ");
        fgets(c->numero, 20, stdin); c->numero[strcspn(c->numero, "\n")] = 0;
        printf("  Bairro: ");
        fgets(c->bairro, 50, stdin); c->bairro[strcspn(c->bairro, "\n")] = 0;
        printf("  Celular/WhatsApp (so numeros): ");
        fgets(c->celular, 12, stdin); c->celular[strcspn(c->celular, "\n")] = 0;

        num_clientes++;
        printf("\n  Cliente '%s' cadastrado com sucesso!\n", c->nome_completo);

        // Pergunta se deseja cadastrar outro para agilizar.
        printf("\n  Deseja cadastrar outro cliente? (s/n): ");
        scanf(" %c", &continuar);
        limpar_buffer();
    } while (continuar == 's' || continuar == 'S');
}

// Cadastra novas categorias de produtos.
void cadastro_categorias() {
    char continuar;
    do {
        configura_capacidade((void**)&categorias, &capacidade_categorias, &num_categorias, sizeof(Categoria));
        Categoria *cat = &categorias[num_categorias];
        cat->codigo = num_categorias + 1; // Código sequencial.

        exibir_cabecalho_melhorado("CADASTRO DE CATEGORIA");
        printf("  Codigo: %d\n\n", cat->codigo);
        printf("  Descricao da Categoria: ");
        fgets(cat->descricao, 50, stdin); cat->descricao[strcspn(cat->descricao, "\n")] = 0;

        num_categorias++;
        printf("\n  Categoria '%s' cadastrada com sucesso!\n", cat->descricao);

        printf("\n  Deseja cadastrar outra categoria? (s/n): ");
        scanf(" %c", &continuar);
        limpar_buffer();
    } while (continuar == 's' || continuar == 'S');
}

// Cadastra novos produtos.
void cadastro_produtos() {
    // Regra de negócio: não pode cadastrar produto se não tiver nenhuma categoria criada.
    if (num_categorias == 0) {
        printf("  ERRO: Nenhuma categoria cadastrada.\n");
        printf("  Por favor, cadastre uma categoria antes de cadastrar um produto.\n");
        Sleep(2500);
        return;
    }

    configura_capacidade((void**)&produtos, &capacidade_produtos, &num_produtos, sizeof(Produto));
    Produto *p = &produtos[num_produtos];
    p->codigo = (num_produtos == 0) ? 1000 : produtos[num_produtos - 1].codigo + 1;

    exibir_cabecalho_melhorado("CADASTRO DE PRODUTO");
    printf("  Codigo: %d\n\n", p->codigo);

    printf("  Descricao: ");
    fgets(p->descricao, 100, stdin); p->descricao[strcspn(p->descricao, "\n")] = 0;

    // Mostra as categorias existentes para o usuário escolher.
    printf("\n  Categorias Disponiveis:\n");
    for (int i = 0; i < num_categorias; i++) {
        printf("    [%d] %s\n", categorias[i].codigo, categorias[i].descricao);
    }
    do {
        printf("  Escolha o codigo da categoria: ");
        if(scanf("%d", &p->codigo_categoria) != 1) p->codigo_categoria = 0;
        limpar_buffer();
        // Valida se a categoria escolhida existe.
        if (encontra_categoria(p->codigo_categoria) == NULL) {
            printf("  Codigo de categoria invalido. Tente novamente.\n");
            p->codigo_categoria = 0; // Força a repetição do loop.
        }
    } while (p->codigo_categoria == 0);

    // Loops para garantir que um valor numérico válido seja digitado.
    printf("  Preco de Compra: R$ ");
    while (scanf("%f", &p->preco_compra) != 1) { printf("  Entrada invalida: R$ "); limpar_buffer(); }
    limpar_buffer();

    printf("  Margem de Lucro (%%): ");
    while (scanf("%f", &p->margem_lucro) != 1) { printf("  Entrada invalida: "); limpar_buffer(); }
    limpar_buffer();

    // O preço de venda é calculado automaticamente.
    p->preco_venda = p->preco_compra * (1 + p->margem_lucro / 100);
    printf("  --> Preco de Venda calculado: R$%.2f\n", p->preco_venda);

    printf("  Quantidade em Estoque: ");
    while (scanf("%d", &p->estoque) != 1) { printf("  Entrada invalida: "); limpar_buffer(); }
    limpar_buffer();

    printf("  Estoque Minimo: ");
    while (scanf("%d", &p->estoque_minimo) != 1) { printf("  Entrada invalida: "); limpar_buffer(); }
    limpar_buffer();

    num_produtos++;
    printf("\n  Produto '%s' cadastrado com sucesso!\n", p->descricao);
    Sleep(1500);
}


// --- FUNÇÕES DE VENDA E PAGAMENTO ---

// Processa o pagamento de uma venda. Essa função foi bem melhorada.
void registrar_pagamento(Venda *venda, float total_a_pagar) {
    int opcao = 0;
    float valor_dinheiro = 0, valor_cartao = 0;

    exibir_cabecalho_melhorado("FORMA DE PAGAMENTO");
    printf("  Valor Total: R$ %.2f\n\n", total_a_pagar);
    printf("  [1] Dinheiro\n");
    printf("  [2] Cartao\n");
    printf("  [3] Misto (Dinheiro + Cartao)\n\n");
    printf("  Opcao: ");
    if(scanf("%d", &opcao) != 1) opcao = 0;
    limpar_buffer();

    configura_capacidade((void**)&pagamentos, &capacidade_pagamentos, &num_pagamentos, sizeof(Pagamento));
    Pagamento *pag = &pagamentos[num_pagamentos];
    pag->num_venda = venda->num_venda;
    pag->valor_pago_dinheiro = 0; // Zera os valores pra não ter lixo de memória
    pag->valor_pago_cartao = 0;

    switch(opcao) {
        case 1: // Dinheiro
            printf("\n  Valor recebido: R$ ");
            if (scanf("%f", &valor_dinheiro) != 1 || valor_dinheiro < total_a_pagar) {
                limpar_buffer();
                printf("\n  Valor invalido ou insuficiente. Pagamento cancelado.\n");
                Sleep(1500);
                return; // Volta sem registrar o pagamento.
            }
            limpar_buffer();
            printf("  Troco: R$%.2f\n", valor_dinheiro - total_a_pagar);
            pag->valor_pago_dinheiro = total_a_pagar; // Registra o valor total da compra como pago em dinheiro.
            break;

        case 2: // Cartão
            printf("\n  Processando pagamento no cartao... (1 para OK, 0 para Falha): ");
            int status_cartao;
            if (scanf("%d", &status_cartao) != 1 || status_cartao != 1) {
                limpar_buffer();
                printf("\n  Pagamento no cartao falhou. Tente novamente.\n");
                Sleep(1500);
                return;
            }
            limpar_buffer();
            pag->valor_pago_cartao = total_a_pagar; // Registra o valor total como pago em cartão.
            break;

        case 3: // Misto
            printf("\n  Valor a ser pago em dinheiro: R$ ");
            // Valida se o valor em dinheiro é válido (maior que zero e menor que o total).
            if (scanf("%f", &valor_dinheiro) != 1 || valor_dinheiro <= 0 || valor_dinheiro >= total_a_pagar) {
                limpar_buffer();
                printf("\n  Valor invalido. Pagamento cancelado.\n");
                Sleep(1500);
                return;
            }
            limpar_buffer();

            valor_cartao = total_a_pagar - valor_dinheiro; // O resto vai pro cartão.
            printf("  Restante de R$%.2f sera pago no cartao.\n", valor_cartao);
            printf("  Processando pagamento no cartao... (1 para OK, 0 para Falha): ");

            if (scanf("%d", &status_cartao) != 1 || status_cartao != 1) {
                limpar_buffer();
                printf("\n  Pagamento no cartao falhou. Pagamento misto cancelado.\n");
                Sleep(1500);
                return;
            }
            limpar_buffer();
            // Agora sim, registra as duas partes separadamente.
            pag->valor_pago_dinheiro = valor_dinheiro;
            pag->valor_pago_cartao = valor_cartao;
            break;

        default:
            printf("\n  Opcao de pagamento invalida.\n");
            Sleep(1500);
            return; // Sai sem fazer nada.
    }

    // No final, atualiza o status da venda e salva o pagamento.
    pag->total_pago = pag->valor_pago_dinheiro + pag->valor_pago_cartao;
    num_pagamentos++;
    venda->status_pagamento = 'p'; // Atualiza o status da venda para "Paga".
    printf("\n  Pagamento da Venda %d registrado com sucesso!\n", venda->num_venda);
    Sleep(2000);
}

// Inicia uma nova venda. Essa função é o coração do caixa.
void nova_venda() {
    configura_capacidade((void**)&vendas, &capacidade_vendas, &num_vendas, sizeof(Venda));
    Venda *venda = &vendas[num_vendas];
    venda->num_venda = (num_vendas == 0) ? 1 : vendas[num_vendas - 1].num_venda + 1;

    exibir_cabecalho_melhorado("NOVA VENDA");
    printf("  Venda No. %d\n\n", venda->num_venda);

    printf("  Digite o codigo do cliente: ");
    if (scanf("%d", &venda->codigo_cliente) != 1) {
        limpar_buffer(); printf("\n  Codigo invalido. Venda cancelada.\n"); Sleep(1500); return;
    }
    limpar_buffer();

    // Verifica se o cliente existe antes de continuar.
    if (encontra_cliente(venda->codigo_cliente) == NULL) {
        printf("\n  Cliente nao encontrado. Venda cancelada.\n"); Sleep(1500); return;
    }

    // Registra a data da venda.
    time_t t = time(NULL); struct tm tm = *localtime(&t);
    venda->dia = tm.tm_mday;
    venda->mes = tm.tm_mon + 1;      // tm_mon vai de 0 a 11, por isso somamos 1.
    venda->ano = tm.tm_year + 1900;  // tm_year conta os anos desde 1900.

    // Inicializa os dados da venda.
    venda->itens = NULL;
    venda->quantidade_itens = 0;
    venda->total_venda = 0;
    venda->status_pagamento = 'a'; // 'a' de "Aberto" ou "Aguardando".

    // Loop do carrinho de compras.
    char continuar = 's';
    while (continuar == 's' || continuar == 'S') {
        exibir_cabecalho_melhorado("CARRINHO DE COMPRAS");
        printf("  Venda No. %d | Total Parcial: R$%.2f\n", venda->num_venda, venda->total_venda);
        printf("  --------------------------------------\n");

        int cod_prod;
        printf("  Codigo do produto: ");
        if(scanf("%d", &cod_prod) != 1) {
            limpar_buffer(); printf("  ERRO: Codigo invalido.\n"); Sleep(1000); continue;
        }
        limpar_buffer();

        // Validações do produto (se existe, se tem estoque).
        Produto *produto = encontra_produto(cod_prod);
        if (!produto) { printf("  ERRO: Produto nao encontrado.\n"); Sleep(1000); continue; }
        if (produto->estoque == 0) { printf("  ALERTA: Produto com estoque ZERADO.\n"); Sleep(1500); continue; }

        int qtd;
        printf("  Quantidade desejada: ");
        if(scanf("%d", &qtd) != 1 || qtd <= 0) {
            limpar_buffer(); printf("  ERRO: Quantidade invalida.\n"); Sleep(1000); continue;
        }
        limpar_buffer();

        // Se o cliente pedir mais do que tem no estoque.
        if (qtd > produto->estoque) {
            printf("\n  ALERTA: Solicitado (%d) > estoque (%d).\n", qtd, produto->estoque);
            printf("  Vender quantidade disponivel? (s/n): ");
            char resp; scanf(" %c", &resp); limpar_buffer();
            if (resp == 'n' || resp == 'N') continue; // Pula pro próximo item.
            qtd = produto->estoque; // Ajusta a quantidade para o máximo disponível.
        }

        // Adiciona o item ao carrinho.
        venda->quantidade_itens++;
        venda->itens = realloc(venda->itens, venda->quantidade_itens * sizeof(ItemVenda));
        ItemVenda *item = &venda->itens[venda->quantidade_itens - 1];

        // Copia os dados do produto para o item da venda.
        item->codigo_produto = produto->codigo;
        strcpy(item->descricao, produto->descricao);
        item->preco_venda_item = produto->preco_venda;
        item->quantidade = qtd;
        item->total_item = produto->preco_venda * qtd;
        venda->total_venda += item->total_item; // Atualiza o total da venda.
        produto->estoque -= qtd;                // Dá baixa no estoque.

        printf("\n  Item '%s' adicionado. Total item: R$%.2f\n", item->descricao, item->total_item);
        // Alerta de estoque baixo.
        if (produto->estoque <= produto->estoque_minimo) {
            printf("  ALERTA: Estoque do produto '%s' atingiu o minimo!\n", produto->descricao);
        }

        printf("\n  Adicionar novo item? (s/n): ");
        scanf(" %c", &continuar);
        limpar_buffer();
    }

    // Se o carrinho estiver vazio no final, cancela a venda.
    if (venda->quantidade_itens == 0) {
        printf("  Nenhum item adicionado. Venda cancelada.\n"); Sleep(1500); return;
    }

    // Mostra o resumo final da venda.
    exibir_cabecalho_melhorado("RESUMO DA VENDA");
    for(int i = 0; i < venda->quantidade_itens; i++) {
        ItemVenda *item = &venda->itens[i];
        printf("  %-20s (Qtd: %-3d) - R$ %.2f\n", item->descricao, item->quantidade, item->total_item);
    }
    printf("  --------------------------------------\n");
    printf("  Total da Venda: R$%.2f\n\n", venda->total_venda);

    // Aplica um desconto, se necessário.
    float desconto;
    printf("  Aplicar desconto (informe 0 ou a %%): ");
    if (scanf("%f", &desconto) != 1) desconto = 0;
    limpar_buffer();

    float total_final = venda->total_venda * (1 - desconto / 100);
    printf("\n  Total Final com Desconto: R$%.2f\n\n", total_final);
    Sleep(2000);

    // Chama a função para processar o pagamento.
    registrar_pagamento(venda, total_final);
    // Só incrementa o número de vendas se o pagamento foi concluído com sucesso.
    num_vendas++;
}

// Permite pagar uma venda que ficou "em aberto".
void pagamento_em_aberto() {
    exibir_cabecalho_melhorado("PAGAR VENDA EM ABERTO");
    int encontrou = 0;
    // Procura por todas as vendas com status 'a'.
    for(int i = 0; i < num_vendas; i++) {
        if(vendas[i].status_pagamento == 'a') {
            encontrou = 1;
            printf("  Venda No. %d | Cliente %d | Total R$%.2f\n", vendas[i].num_venda, vendas[i].codigo_cliente, vendas[i].total_venda);
            printf("  Deseja pagar esta venda agora? (s/n): ");
            char resp;
            scanf(" %c", &resp);
            limpar_buffer();
            if(resp == 's' || resp == 'S') {
                // ...a lógica de desconto e pagamento é a mesma da nova venda...
                float desconto;
                printf("  Aplicar desconto (0 para nao ou a %%): ");
                if(scanf("%f", &desconto) != 1) desconto = 0;
                limpar_buffer();
                float total_final = vendas[i].total_venda * (1 - desconto / 100);
                printf("  Total a pagar: R$%.2f\n", total_final);
                Sleep(1500);
                registrar_pagamento(&vendas[i], total_final);
                exibir_cabecalho_melhorado("PAGAR VENDA EM ABERTO"); // Redesenha a tela
            }
        }
    }
    if(!encontrou) {
        printf("  Nenhuma venda com pagamento em aberto foi encontrada.\n");
    }
    printf("\n  Pressione Enter para retornar...");
    getchar();
}

// --- FUNÇÕES DO CAIXA ---

// Abre o caixa no início do expediente.
void abrir_caixa() {
    if (!validar_admin()) return;

    exibir_cabecalho_melhorado("ABERTURA DE CAIXA");
    // Verifica se o caixa já não foi aberto.
    if (abertura_caixa >= 0) {
        printf("  O caixa ja foi aberto hoje com o valor de R$%.2f.\n", abertura_caixa);
        Sleep(2000);
        return;
    }

    float valor;
    do {
        printf("  Informe o valor inicial do caixa (maior ou igual a 0): R$ ");
        if (scanf("%f", &valor) != 1) valor = -1;
        limpar_buffer();
        if (valor < 0) printf("  Valor invalido. Tente novamente.\n");
    } while (valor < 0);

    abertura_caixa = valor;
    total_sangrias_dia = 0; // Zera o contador de sangrias do dia anterior.
    printf("\n  Caixa aberto com R$%.2f\n", abertura_caixa);
    Sleep(1500);
}

// Retira dinheiro do caixa (sangria) por segurança.
void sangria() {
    if (!validar_admin()) return;

    exibir_cabecalho_melhorado("RETIRADA DE CAIXA (SANGRIA)");

    // Calcula o total de dinheiro físico que DEVERIA estar no caixa.
    float total_dinheiro_caixa = abertura_caixa - total_sangrias_dia;
    // Soma apenas os pagamentos em dinheiro recebidos.
    for (int i = 0; i < num_pagamentos; i++) {
        total_dinheiro_caixa += pagamentos[i].valor_pago_dinheiro;
    }

    printf("  Total em dinheiro disponivel no caixa: R$%.2f\n\n", total_dinheiro_caixa);

    float valor_retirada;
    printf("  Informe o valor da retirada: R$ ");
    if(scanf("%f", &valor_retirada) != 1 || valor_retirada <= 0) {
        limpar_buffer();
        printf("\n  Valor de retirada invalido.\n"); Sleep(1500); return;
    }
    limpar_buffer();

    // Regra de negócio: não permite que a sangria deixe o caixa com menos que o mínimo para troco.
    if ((total_dinheiro_caixa - valor_retirada) < MINIMO_CAIXA_TROCO) {
        printf("\n  Retirada nao permitida. O caixa deve manter um saldo minimo de R$%.2f.\n", MINIMO_CAIXA_TROCO);
        Sleep(2000); return;
    }

    total_sangrias_dia += valor_retirada; // Adiciona a retirada ao total de sangrias do dia.
    printf("\n  Retirada de R$%.2f realizada com sucesso.\n", valor_retirada);
    printf("  Total de sangrias no dia: R$%.2f\n", total_sangrias_dia);
    Sleep(2000);
}

// Fecha o caixa no final do dia e confere os valores. Essa função foi BEM reformulada.
void fechamento_de_caixa() {
    if (!validar_admin()) return;

    exibir_cabecalho_melhorado("FECHAMENTO DE CAIXA");
    if (abertura_caixa < 0) {
        printf("  O caixa nao foi aberto hoje. Nao ha o que fechar.\n");
        Sleep(2000); return;
    }

    // Variáveis para o resumo do dia.
    float faturamento_dia = 0, total_dinheiro = 0, total_cartao = 0;
    int qtd_vendas_dia = 0;

    // Pega a data atual do sistema.
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    // Itera sobre as vendas PAGAS de HOJE para calcular os totais.
    for(int i=0; i < num_vendas; i++) {
        // Verifica se a venda é de hoje e se está paga.
        if(vendas[i].ano == (tm.tm_year + 1900) && vendas[i].mes == (tm.tm_mon + 1) && vendas[i].dia == tm.tm_mday && vendas[i].status_pagamento == 'p') {
            qtd_vendas_dia++;
            // Acha o pagamento correspondente a essa venda.
            for(int j=0; j < num_pagamentos; j++) {
                if(pagamentos[j].num_venda == vendas[i].num_venda) {
                    total_dinheiro += pagamentos[j].valor_pago_dinheiro;
                    total_cartao += pagamentos[j].valor_pago_cartao;
                    faturamento_dia += pagamentos[j].total_pago;
                    break; // Achou o pagamento, pode sair do loop interno.
                }
            }
        }
    }

    // Exibe o resumo do dia.
    printf("  --- RESUMO DO DIA (%02d/%02d/%d) ---\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    printf("  Quantidade de vendas pagas: %d\n", qtd_vendas_dia);
    printf("  Faturamento total: R$%.2f\n\n", faturamento_dia);
    printf("  Valor de abertura: R$%.2f\n", abertura_caixa);
    printf("  (+) Recebido em dinheiro: R$%.2f\n", total_dinheiro);
    printf("  (+) Recebido em cartao: R$%.2f\n", total_cartao);
    printf("  (-) Total de sangrias: R$%.2f\n", total_sangrias_dia);
    printf("  --------------------------------------\n");

    // Lógica correta para o fechamento.
    // O que deveria ter em dinheiro = Abertura + Entradas em dinheiro - Saídas em dinheiro (sangrias).
    float dinheiro_esperado = abertura_caixa + total_dinheiro - total_sangrias_dia;
    printf("  VALOR ESPERADO EM DINHEIRO NO CAIXA: R$%.2f\n\n", dinheiro_esperado);

    // Agora, o operador precisa contar o dinheiro físico e informar.
    float valor_contado;
    printf("  Por favor, informe o valor total contado em dinheiro no caixa: R$ ");
    if(scanf("%f", &valor_contado) != 1){
        limpar_buffer();
        printf("\n  Entrada invalida. Operacao cancelada.\n"); Sleep(1500); return;
    }
    limpar_buffer();

    // Compara o valor contado com o esperado.
    float divergencia = valor_contado - dinheiro_esperado;

    // fabs() calcula o valor absoluto (sem sinal de +/-). Usamos para ver se a diferença é maior que 1 centavo.
    if (fabs(divergencia) > 0.01) {
        printf("\n  ATENCAO: O caixa fechou com divergencia de R$%.2f\n", divergencia);
        if (divergencia > 0) printf("  (SOBRA DE CAIXA)\n");
        else printf("  (FALTA NO CAIXA)\n");
    } else {
        printf("\n  CAIXA FECHADO CORRETAMENTE (Divergencia ZERO).\n");
    }

    // Confirmação final antes de zerar o caixa.
    printf("\n  Deseja confirmar o fechamento do caixa? (s/n): ");
    char confirma;
    scanf(" %c", &confirma);
    limpar_buffer();

    if(confirma == 's' || confirma == 'S') {
        abertura_caixa = -1.0f; // Reseta o caixa para o dia seguinte.
        total_sangrias_dia = 0;
        printf("\n  Caixa oficialmente fechado.\n");
    } else {
        printf("\n  Fechamento de caixa cancelado pelo usuario.\n");
    }

    printf("\n  Pressione Enter para continuar...");
    getchar();
}

// --- FUNÇÕES DE RELATÓRIOS ---

// Funções de comparação para usar com a função qsort (quick sort) do C.
int comparar_clientes_nome(const void *a, const void *b) {
    // Converte os ponteiros genéricos para ponteiros de Cliente e compara os nomes.
    return strcmp(((Cliente *)a)->nome_completo, ((Cliente *)b)->nome_completo);
}
int comparar_produtos_descricao(const void *a, const void *b) {
    return strcmp(((Produto *)a)->descricao, ((Produto *)b)->descricao);
}

// Lista todos os clientes em ordem alfabética.
void listar_clientes_ordenado() {
    exibir_cabecalho_melhorado("LISTA DE CLIENTES");
    if (num_clientes == 0) { printf("  Nenhum cliente cadastrado.\n"); }
    else {
        // Cria uma cópia do array de clientes para não bagunçar o original.
        Cliente *copia = malloc(num_clientes * sizeof(Cliente));
        memcpy(copia, clientes, num_clientes * sizeof(Cliente));
        // Ordena a cópia.
        qsort(copia, num_clientes, sizeof(Cliente), comparar_clientes_nome);

        printf("  %-5s | %-30s | %-15s\n", "Cod.", "Nome Completo", "Celular");
        printf("  --------------------------------------------------------\n");
        for (int i = 0; i < num_clientes; i++) {
            printf("  %-5d | %-30.30s | %-15s\n", copia[i].codigo, copia[i].nome_completo, copia[i].celular);
        }
        free(copia); // Libera a memória da cópia.
    }
    printf("\n  Pressione Enter para retornar...");
    getchar();
}

// Lista todos os produtos em ordem alfabética.
void listar_produtos_ordenado() {
    exibir_cabecalho_melhorado("LISTA DE PRODUTOS");
    if (num_produtos == 0) { printf("  Nenhum produto cadastrado.\n"); }
    else {
        Produto *copia = malloc(num_produtos * sizeof(Produto));
        memcpy(copia, produtos, num_produtos * sizeof(Produto));
        qsort(copia, num_produtos, sizeof(Produto), comparar_produtos_descricao);

        printf("  %-5s|%-30s|%-15s|%-10s|%-8s\n", "Cod.", "Descricao", "Categoria", "Preco V.", "Estoque");
        printf("  --------------------------------------------------------------------------\n");
        for (int i = 0; i < num_produtos; i++) {
            // Acha a descrição da categoria para mostrar no relatório.
            Categoria *cat = encontra_categoria(copia[i].codigo_categoria);
            char *desc_cat = (cat) ? cat->descricao : "N/A";
            printf("  %-5d|%-30.30s|%-15.15s|R$ %-7.2f|%-8d\n",
                   copia[i].codigo, copia[i].descricao, desc_cat,
                   copia[i].preco_venda, copia[i].estoque);
        }
        free(copia);
    }
    printf("\n  Pressione Enter para retornar...");
    getchar();
}

// Lista apenas os produtos que precisam ser comprados.
void listar_produtos_estoque_minimo() {
    exibir_cabecalho_melhorado("PRODUTOS COM ESTOQUE BAIXO");
    int encontrou = 0;
    if (num_produtos == 0) { printf("  Nenhum produto cadastrado.\n"); }
    else {
        printf("  %-5s | %-30s | %-10s | %-10s\n", "Cod.", "Descricao", "Est. Atual", "Est. Minimo");
        printf("  ------------------------------------------------------------\n");
        for (int i = 0; i < num_produtos; i++) {
            if (produtos[i].estoque <= produtos[i].estoque_minimo) {
                encontrou = 1;
                printf("  %-5d | %-30.30s | %-10d | %-10d\n", produtos[i].codigo, produtos[i].descricao, produtos[i].estoque, produtos[i].estoque_minimo);
            }
        }
        if (!encontrou) {
            printf("\n  Nenhum produto com estoque baixo ou zerado encontrado.\n");
        }
    }
    printf("\n  Pressione Enter para retornar...");
    getchar();
}

// Lista as vendas realizadas em um intervalo de datas.
void listar_vendas_periodo() {
    exibir_cabecalho_melhorado("VENDAS POR PERIODO");
    int dia_ini, mes_ini, ano_ini, dia_fim, mes_fim, ano_fim;
    printf("  Digite a data de inicio (dd mm aaaa): ");
    if (scanf("%d %d %d", &dia_ini, &mes_ini, &ano_ini) != 3) { limpar_buffer(); printf("\n  Data invalida.\n"); Sleep(1500); return; }
    limpar_buffer();
    printf("  Digite a data de fim (dd mm aaaa): ");
    if (scanf("%d %d %d", &dia_fim, &mes_fim, &ano_fim) != 3) { limpar_buffer(); printf("\n  Data invalida.\n"); Sleep(1500); return; }
    limpar_buffer();

    printf("\n\n  --- Vendas de %02d/%02d/%d a %02d/%02d/%d ---\n", dia_ini, mes_ini, ano_ini, dia_fim, mes_fim, ano_fim);
    printf("  %-10s|%-10s|%-15s|%-10s| %s\n", "Data", "No. Venda", "Cliente", "Total", "Pgto");
    printf("  ------------------------------------------------------------------\n");

    float faturamento_consolidado = 0;
    int encontrou = 0;
    // Um truque para comparar datas facilmente: transformar aaaa/mm/dd em um número grande (aaaammdd).
    long data_inicio = ano_ini * 10000 + mes_ini * 100 + dia_ini;
    long data_final = ano_fim * 10000 + mes_fim * 100 + dia_fim;

    for (int i = 0; i < num_vendas; i++) {
        long data_venda = vendas[i].ano * 10000 + vendas[i].mes * 100 + vendas[i].dia;
        if (data_venda >= data_inicio && data_venda <= data_final) {
            encontrou = 1;
            Cliente *c = encontra_cliente(vendas[i].codigo_cliente);
            char *nome_cli = (c) ? c->nome_social : "N/A";
            printf("  %02d/%02d/%-5d|%-10d|%-15.15s|R$ %-7.2f| %c\n", vendas[i].dia, vendas[i].mes, vendas[i].ano, vendas[i].num_venda, nome_cli, vendas[i].total_venda, vendas[i].status_pagamento);
            if(vendas[i].status_pagamento == 'p') {
                faturamento_consolidado += vendas[i].total_venda;
            }
        }
    }

    if (!encontrou) { printf("\n  Nenhuma venda encontrada no periodo.\n"); }
    else { printf("\n  Faturamento Consolidado (vendas pagas) no periodo: R$%.2f\n", faturamento_consolidado); }
    printf("\n  Pressione Enter para retornar...");
    getchar();
}

// NOVO RELATÓRIO: Lista os clientes que compraram em um determinado período.
void listar_clientes_compraram_periodo() {
    exibir_cabecalho_melhorado("CLIENTES POR PERIODO DE COMPRA");
    int dia_ini, mes_ini, ano_ini, dia_fim, mes_fim, ano_fim;
    printf("  Digite a data de inicio (dd mm aaaa): ");
    if (scanf("%d %d %d", &dia_ini, &mes_ini, &ano_ini) != 3) { limpar_buffer(); printf("\n  Data invalida.\n"); Sleep(1500); return; }
    limpar_buffer();
    printf("  Digite a data de fim (dd mm aaaa): ");
    if (scanf("%d %d %d", &dia_fim, &mes_fim, &ano_fim) != 3) { limpar_buffer(); printf("\n  Data invalida.\n"); Sleep(1500); return; }
    limpar_buffer();

    // Array para guardar os códigos dos clientes que compraram, sem repetir.
    int *clientes_unicos = NULL;
    int num_clientes_unicos = 0;
    long data_inicio = ano_ini * 10000 + mes_ini * 100 + dia_ini;
    long data_final = ano_fim * 10000 + mes_fim * 100 + dia_fim;

    // Passa por todas as vendas do período.
    for (int i = 0; i < num_vendas; i++) {
        long data_venda = vendas[i].ano * 10000 + vendas[i].mes * 100 + vendas[i].dia;
        if (data_venda >= data_inicio && data_venda <= data_final) {
            int achou = 0;
            // Verifica se o cliente dessa venda já está na nossa lista de clientes únicos.
            for(int j=0; j < num_clientes_unicos; j++){
                if(clientes_unicos[j] == vendas[i].codigo_cliente) {
                    achou = 1;
                    break;
                }
            }
            // Se não achou, adiciona ele na lista.
            if(!achou){
                num_clientes_unicos++;
                clientes_unicos = realloc(clientes_unicos, num_clientes_unicos * sizeof(int));
                clientes_unicos[num_clientes_unicos - 1] = vendas[i].codigo_cliente;
            }
        }
    }

    if(num_clientes_unicos == 0){
        printf("\n  Nenhum cliente realizou compras no periodo especificado.\n");
    } else {
        printf("\n\n  --- Clientes que compraram de %02d/%02d/%d a %02d/%02d/%d ---\n", dia_ini, mes_ini, ano_ini, dia_fim, mes_fim, ano_fim);
        printf("  %-5s | %-30s | %-15s\n", "Cod.", "Nome Completo", "Celular");
        printf("  --------------------------------------------------------\n");
        // Mostra os dados dos clientes que estão na lista.
        for(int i = 0; i < num_clientes_unicos; i++){
            Cliente *c = encontra_cliente(clientes_unicos[i]);
            if(c){
                printf("  %-5d | %-30.30s | %-15s\n", c->codigo, c->nome_completo, c->celular);
            }
        }
    }
    free(clientes_unicos); // Libera a memória do array temporário.
    printf("\n  Pressione Enter para retornar...");
    getchar();
}

// Struct temporária só para ajudar a criar o ranking de produtos.
typedef struct { int codigo_produto; int quantidade_vendida; } ProdutoVendido;
// Função de comparação para ordenar os produtos do mais vendido para o menos vendido.
int comparar_produtos_vendidos(const void *a, const void *b) {
    return ((ProdutoVendido *)b)->quantidade_vendida - ((ProdutoVendido *)a)->quantidade_vendida;
}

// NOVO RELATÓRIO: Lista os produtos mais vendidos em um período.
void listar_produtos_mais_vendidos() {
    exibir_cabecalho_melhorado("PRODUTOS MAIS VENDIDOS");
    int dia_ini, mes_ini, ano_ini, dia_fim, mes_fim, ano_fim;
    printf("  Digite a data de inicio (dd mm aaaa): ");
    if (scanf("%d %d %d", &dia_ini, &mes_ini, &ano_ini) != 3) { limpar_buffer(); printf("\n  Data invalida.\n"); Sleep(1500); return; }
    limpar_buffer();
    printf("  Digite a data de fim (dd mm aaaa): ");
    if (scanf("%d %d %d", &dia_fim, &mes_fim, &ano_fim) != 3) { limpar_buffer(); printf("\n  Data invalida.\n"); Sleep(1500); return; }
    limpar_buffer();

    ProdutoVendido *ranking = NULL;
    int num_ranking = 0;
    long data_inicio = ano_ini * 10000 + mes_ini * 100 + dia_ini;
    long data_final = ano_fim * 10000 + mes_fim * 100 + dia_fim;

    // Passa por todas as vendas e todos os itens de cada venda no período.
    for (int i = 0; i < num_vendas; i++) {
        long data_venda = vendas[i].ano * 10000 + vendas[i].mes * 100 + vendas[i].dia;
        if (data_venda >= data_inicio && data_venda <= data_final) {
            for(int j=0; j < vendas[i].quantidade_itens; j++){
                ItemVenda *item = &vendas[i].itens[j];
                int achou = 0;
                // Procura se o produto já está no nosso ranking.
                for(int k=0; k < num_ranking; k++){
                    if(ranking[k].codigo_produto == item->codigo_produto){
                        // Se achou, só soma a quantidade vendida.
                        ranking[k].quantidade_vendida += item->quantidade;
                        achou = 1;
                        break;
                    }
                }
                // Se não achou, adiciona ele no ranking.
                if(!achou){
                    num_ranking++;
                    ranking = realloc(ranking, num_ranking * sizeof(ProdutoVendido));
                    ranking[num_ranking-1].codigo_produto = item->codigo_produto;
                    ranking[num_ranking-1].quantidade_vendida = item->quantidade;
                }
            }
        }
    }

    if(num_ranking == 0){
        printf("\n  Nenhum produto foi vendido no periodo especificado.\n");
    } else {
        // Ordena o ranking usando a função qsort.
        qsort(ranking, num_ranking, sizeof(ProdutoVendido), comparar_produtos_vendidos);
        printf("\n\n  --- Ranking de Produtos Vendidos de %02d/%02d/%d a %02d/%02d/%d ---\n", dia_ini, mes_ini, ano_ini, dia_fim, mes_fim, ano_fim);
        printf("  %-5s | %-30s | %s\n", "Cod.", "Descricao", "Qtd. Vendida");
        printf("  --------------------------------------------------------\n");
        for(int i=0; i < num_ranking; i++){
            Produto *p = encontra_produto(ranking[i].codigo_produto);
            if(p){
                printf("  %-5d | %-30.30s | %d\n", p->codigo, p->descricao, ranking[i].quantidade_vendida);
            }
        }
    }
    free(ranking);
    printf("\n  Pressione Enter para retornar...");
    getchar();
}

// --- MENUS DE NAVEGAÇÃO ---
// Controlam a navegação do usuário pelo sistema.

// Menu para os cadastros.
void menu_cadastros() {
    int opcao;
    do { // 'do-while(1)' cria um loop infinito que só é quebrado com um 'return'.
        exibir_cabecalho_melhorado("MENU DE CADASTROS");
        printf("  [1] Cadastro de Usuarios\n");
        printf("  [2] Cadastro de Clientes\n");
        printf("  [3] Cadastro de Produtos\n");
        printf("  [4] Cadastro de Categorias\n");
        printf("  [5] Retornar ao Menu Principal\n\n");
        printf("  Escolha uma opcao: ");

        if(scanf("%d", &opcao) != 1) opcao = 0; // Se a entrada não for um número, define uma opção inválida.
        limpar_buffer();

        switch (opcao) {
            case 1: cadastro_usuarios(); break;
            case 2: cadastro_clientes(); break;
            case 3: cadastro_produtos(); break;
            case 4: cadastro_categorias(); break;
            case 5: return; // Retorna para a função que chamou (menu_principal).
            default: printf("\n  Opcao invalida.\n"); Sleep(1000);
        }
    } while (1);
}

// Menu para as vendas e operações de caixa.
void menu_vendas_caixa() {
    // Regra de negócio: não pode vender se o caixa não estiver aberto.
    if (abertura_caixa < 0) {
        exibir_cabecalho_melhorado("ATENCAO");
        printf("  Nao e possivel realizar vendas ou\n");
        printf("  operacoes de caixa sem antes\n");
        printf("  realizar a ABERTURA DO CAIXA!\n");
        Sleep(2500); return;
    }

    int opcao;
    do {
        exibir_cabecalho_melhorado("MENU DE VENDAS E CAIXA");
        printf("  [1] Nova Venda\n");
        printf("  [2] Pagamento de Venda em Aberto\n");
        printf("  [3] Retirada de Caixa (Sangria)\n");
        printf("  [4] Retornar ao Menu Principal\n\n");
        printf("  Escolha uma opcao: ");

        if(scanf("%d", &opcao) != 1) opcao = 0;
        limpar_buffer();

        switch (opcao) {
            case 1: nova_venda(); break;
            case 2: pagamento_em_aberto(); break;
            case 3: sangria(); break;
            case 4: return;
            default: printf("\n  Opcao invalida.\n"); Sleep(1000);
        }
    } while (1);
}

// Menu para os relatórios.
void menu_relatorios() {
    int opcao;
    do {
        exibir_cabecalho_melhorado("MENU DE RELATORIOS");
        printf("  [1] Listagem dos Clientes (Ordem Alfabetica)\n");
        printf("  [2] Listagem de Produtos (Ordem Alfabetica)\n");
        printf("  [3] Listagem de Produtos com Estoque Baixo\n");
        printf("  [4] Listagem das Vendas por Periodo\n");
        printf("  [5] Listagem de Clientes que Compraram por Periodo\n");
        printf("  [6] Listagem de Produtos mais Vendidos por Periodo\n");
        printf("  [7] Retornar ao Menu Principal\n\n");
        printf("  Escolha uma opcao: ");
        if(scanf("%d", &opcao) != 1) opcao = 0;
        limpar_buffer();

        switch (opcao) {
            case 1: listar_clientes_ordenado(); break;
            case 2: listar_produtos_ordenado(); break;
            case 3: listar_produtos_estoque_minimo(); break;
            case 4: listar_vendas_periodo(); break;
            case 5: listar_clientes_compraram_periodo(); break;
            case 6: listar_produtos_mais_vendidos(); break;
            case 7: return;
            default: printf("\n  Opcao invalida.\n"); Sleep(1000);
        }
    } while (1);
}

// Menu principal que centraliza todas as outras opções.
void menu_principal() {
    int opcao;
    do {
        exibir_cabecalho_melhorado("MENU PRINCIPAL");
        printf("  Ola, %s! O que voce gostaria de fazer hoje?\n\n", usuario_logado->login);
        printf("  [1] Cadastros\n");
        printf("  [2] Vendas e Caixa\n");
        printf("  [3] Abertura de Caixa\n");
        printf("  [4] Fechamento de Caixa\n");
        printf("  [5] Relatorios\n");
        printf("  [6] Sair do Sistema\n\n");
        printf("  Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1) opcao = 0;
        limpar_buffer();

        switch (opcao) {
            case 1: menu_cadastros(); break;
            case 2: menu_vendas_caixa(); break;
            case 3: abrir_caixa(); break;
            case 4: fechamento_de_caixa(); break;
            case 5: menu_relatorios(); break;
            case 6: break; // Sai do switch, e o loop while verifica que opcao é 6 e termina.
            default: printf("\n  Opcao invalida. Tente novamente.\n"); Sleep(1000);
        }
    } while (opcao != 6);
}

// --- FUNÇÃO PRINCIPAL ---
// Onde tudo começa!

int main() {
    // Configura o programa para usar o idioma português (para acentos, etc.).
    setlocale(LC_ALL, "Portuguese");

    // 1. Carrega todos os dados dos arquivos para a memória.
    carregar_todos_os_dados();
    // 2. Mostra a tela de boas-vindas.
    tela_de_boas_vindas();

    // 3. Verifica se é a primeira execução do sistema (não há usuários).
    if (num_usuarios == 0) {
        exibir_cabecalho_melhorado("CONFIGURACAO INICIAL");
        printf("  Nenhum usuario foi encontrado no sistema.\n");
        printf("  Vamos criar a primeira conta de Administrador.\n\n");
        criar_primeiro_admin();
        salvar_todos_os_dados(); // Salva o novo admin no arquivo.
        printf("\n  Conta de Administrador criada com sucesso!\n");
        printf("  O sistema ira para a tela de login.\n");
        Sleep(2500);
    }

    // 4. Exige que o usuário faça login.
    login();

    // 5. Se o login for bem-sucedido, mostra o menu principal.
    if (usuario_logado != NULL) {
        menu_principal();
    }

    // 6. Ao sair do menu principal, salva todos os dados antes de fechar.
    salvar_todos_os_dados();

    limpar_tela();
    printf("\n\n\n           Sistema encerrado. Volte sempre!\n\n\n");

    // 7. Libera toda a memória que foi alocada dinamicamente. Isso é MUITO importante para não ter vazamento de memória.
    free(usuarios);
    free(clientes);
    free(produtos);
    free(categorias);
    // Para as vendas, precisamos liberar o array de itens de cada venda primeiro.
    for(int i=0; i<num_vendas; i++) free(vendas[i].itens);
    free(vendas);
    free(pagamentos);

    return 0;
}
