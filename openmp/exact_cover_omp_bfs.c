/**
 * Version OpenMP avec BFS
 * 
 * Quentin Deschamps, 2021
 */
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <getopt.h>
#include <sys/time.h>

#include <omp.h>

/* Nombre minimum de tâches */
#define MIN 1000

double start = 0.0;

char *in_filename = NULL;              // nom du fichier contenant la matrice
bool print_solutions = false;          // affiche chaque solution
long long report_delta = 1e6;          // affiche un rapport tous les ... noeuds
long long next_report;                 // prochain rapport affiché au noeud...
long long max_solutions = 0x7fffffffffffffff;        // stop après ... solutions

/* Variables de file */
struct context_t **queue;               // file de contextes
int queue_front = 0;                    // tête de la file
int queue_rear = -1;                    // queue de la file
int queue_size = 0;                     // nombre de contextes dans la file

struct instance_t {
        int n_items;
        int n_primary;
        int n_options;
        char **item_name;   // potentiellement NULL, sinon de taille n_items
        int *options;       // l'option i contient les objets options[ptr[i]:ptr[i+1]]
        int *ptr;           // taille n_options + 1
};

struct sparse_array_t {
        int len;           // nombre d'éléments stockés
        int capacity;      // taille maximale
        int *p;            // contenu de l'ensemble = p[0:len] 
        int *q;            // taille capacity (tout comme p)
};

struct context_t {
        struct sparse_array_t *active_items;      // objets actifs
        struct sparse_array_t **active_options;   // options actives contenant l'objet i
        int *chosen_options;                      // options choisies à ce stade
        int *child_num;                           // numéro du fils exploré
        int *num_children;                        // nombre de fils à explorer
        int level;                                // nombre d'options choisies
        long long nodes;                          // nombre de noeuds explorés
        long long solutions;                      // nombre de solutions trouvées 
};

static const char DIGITS[62] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 
                                'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
                                'u', 'v', 'w', 'x', 'y', 'z',
                                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 
                                'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
                                'U', 'V', 'W', 'X', 'Y', 'Z'};


double wtime()
{
        struct timeval ts;
        gettimeofday(&ts, NULL);
        return (double) ts.tv_sec + ts.tv_usec / 1e6;
}


void usage(char **argv)
{
        printf("%s --in FILENAME [OPTIONS]\n\n", argv[0]);
        printf("Options:\n");
        printf("--progress-report N   display a message every N nodes (0 to disable)\n");
        printf("--print-solutions     display solutions when they are found\n");
        printf("--stop-after N        stop the search once N solutions are found\n");
        exit(0);
}


bool item_is_primary(const struct instance_t *instance, int item)
{
        return item < instance->n_primary;
}


void print_option(const struct instance_t *instance, int option)
{
        if (instance->item_name == NULL)
                errx(1, "tentative d'affichage sans noms d'objet");
        for (int p = instance->ptr[option]; p < instance->ptr[option + 1]; p++) {
                int item = instance->options[p];
                printf("%s ", instance->item_name[item]);
        }
        printf("\n");
}



struct sparse_array_t * sparse_array_init(int n)
{
        struct sparse_array_t *S = malloc(sizeof(*S));
        if (S == NULL)
                err(1, "impossible d'allouer un tableau creux");
        S->len = 0;
        S->capacity = n;
        S->p = malloc(n * sizeof(int));
        S->q = malloc(n * sizeof(int));
        if (S->p == NULL || S->q == NULL)
                err(1, "Impossible d'allouer p/q dans un tableau creux");
        for (int i = 0; i < n; i++)
                S->q[i] = n;           // initialement vide
        return S;
}

bool sparse_array_membership(const struct sparse_array_t *S, int x)
{
        return (S->q[x] < S->len);
}

bool sparse_array_empty(const struct sparse_array_t *S)
{
        return (S->len == 0);
}

void sparse_array_add(struct sparse_array_t *S, int x)
{
        int i = S->len;
        S->p[i] = x;
        S->q[x] = i;
        S->len = i + 1;
}

void sparse_array_remove(struct sparse_array_t *S, int x)
{
        int j = S->q[x];
        int n = S->len - 1;
        // échange p[j] et p[n] 
        int y = S->p[n];
        S->p[n] = x;
        S->p[j] = y;
        // met q à jour
        S->q[x] = n;
        S->q[y] = j;
        S->len = n;
}

void sparse_array_unremove(struct sparse_array_t *S)
{
        S->len++;
}

void sparse_array_unadd(struct sparse_array_t *S)
{
        S->len--;
}

/**
 * Affiche un tableau creux.
 * 
 * @param S tableau creu
 */
void sparse_array_print(const struct sparse_array_t *S)
{
        printf("== Sparse array ==\n");
        printf("len: %d\n", S->len);
        printf("capacity: %d\n", S->capacity);
        printf("p:");
        for (int i = 0; i < S->capacity; i++)
                printf(" %d", S->p[i]);
        printf("\nq:");        
        for (int i = 0; i < S->capacity; i++)
                printf(" %d", S->q[i]);
        printf("\n");
}


bool item_is_active(const struct context_t *ctx, int item)
{
        return sparse_array_membership(ctx->active_items, item);
}

void solution_found(const struct instance_t *instance, struct context_t *ctx)
{
        ctx->solutions++;
        if (!print_solutions)
                return;
        printf("Trouvé une nouvelle solution au niveau %d après %lld noeuds\n", 
                        ctx->level, ctx->nodes);
        printf("Options : \n");
        for (int i = 0; i < ctx->level; i++) {
                int option = ctx->chosen_options[i];
                printf("+ %d : ", option);
                print_option(instance, option);
        }
        printf("\n");
        printf("----------------------------------------------------\n");
}

void cover(const struct instance_t *instance, struct context_t *ctx, int item);

void choose_option(const struct instance_t *instance, struct context_t *ctx, 
                        int option, int chosen_item)
{
        ctx->chosen_options[ctx->level] = option;
        ctx->level++;
        for (int p = instance->ptr[option]; p < instance->ptr[option + 1]; p++) {
                int item = instance->options[p];
                if (item == chosen_item)
                        continue;
                cover(instance, ctx, item);
        }
}

void uncover(const struct instance_t *instance, struct context_t *ctx, int item);

void unchoose_option(const struct instance_t *instance, struct context_t *ctx, 
                        int option, int chosen_item)
{
        for (int p = instance->ptr[option + 1] - 1; p >= instance->ptr[option]; p--) {
                int item = instance->options[p];
                if (item == chosen_item)
                        continue;
                uncover(instance, ctx, item);
        }
        ctx->level--;
}


int choose_next_item(struct context_t *ctx)
{
        int best_item = -1;
        int best_options = 0x7fffffff;
        struct sparse_array_t *active_items = ctx->active_items;
        for (int i = 0; i < active_items->len; i++) {
                int item = active_items->p[i];
                struct sparse_array_t *active_options = ctx->active_options[item];
                int k = active_options->len;
                if (k < best_options) {
                        best_item = item;
                        best_options = k;
                }
        }
        return best_item;
}

void progress_report(const struct context_t *ctx)
{
        double now = wtime();
        printf("Exploré %lld noeuds, trouvé %lld solutions, temps écoulé %.1fs. ", 
                        ctx->nodes, ctx->solutions, now - start);
        int i = 0;
        for (int k = 0; k < ctx->level; k++) {
                if (i > 44)
                        break;
                int n = ctx->child_num[k];
                int m = ctx->num_children[k];
                if (m == 1)
                        continue;
                printf("%c%c ", (n < 62) ? DIGITS[n] : '*', (m < 62) ? DIGITS[m] : '*');
                i++;
        }
        printf("\n"),
        next_report += report_delta;
}

void deactivate(const struct instance_t *instance, struct context_t *ctx, 
                        int option, int covered_item);

void cover(const struct instance_t *instance, struct context_t *ctx, int item)
{
        if (item_is_primary(instance, item))
                sparse_array_remove(ctx->active_items, item);
        struct sparse_array_t *active_options = ctx->active_options[item];
        for (int i = 0; i < active_options->len; i++) {
                int option = active_options->p[i];
                deactivate(instance, ctx, option, item);
        }
}


void deactivate(const struct instance_t *instance, struct context_t *ctx, 
                        int option, int covered_item)
{
        for (int k = instance->ptr[option]; k < instance->ptr[option+1]; k++) {
                int item = instance->options[k];
                if (item == covered_item)
                        continue;
                sparse_array_remove(ctx->active_options[item], option);
        }
}


void reactivate(const struct instance_t *instance, struct context_t *ctx, 
                        int option, int uncovered_item);

void uncover(const struct instance_t *instance, struct context_t *ctx, int item)
{
        struct sparse_array_t *active_options = ctx->active_options[item];
        for (int i = active_options->len - 1; i >= 0; i--) {
                int option = active_options->p[i];
                reactivate(instance, ctx, option, item);
        }
        if (item_is_primary(instance, item))
                sparse_array_unremove(ctx->active_items);
}


void reactivate(const struct instance_t *instance, struct context_t *ctx, 
                        int option, int uncovered_item)
{
        for (int k = instance->ptr[option + 1] - 1; k >= instance->ptr[option]; k--) {
                int item = instance->options[k];
                if (item == uncovered_item)
                        continue;
                sparse_array_unremove(ctx->active_options[item]);
        }
}


struct instance_t * load_matrix(const char *filename)
{
        struct instance_t *instance = malloc(sizeof(*instance));
        if (instance == NULL)
                err(1, "Impossible d'allouer l'instance");
        FILE *in = fopen(filename, "r");
        if (in == NULL)
                err(1, "Impossible d'ouvrir %s en lecture", filename);
        int n_it, n_op;
        if (fscanf(in, "%d %d\n", &n_it, &n_op) != 2)
                errx(1, "Erreur de lecture de la taille du problème\n");
        if (n_it == 0 || n_op == 0)
                errx(1, "Impossible d'avoir 0 objets ou 0 options");
        instance->n_items = n_it;
        instance->n_primary = 0;
        instance->n_options = n_op;
        instance->item_name = malloc(n_it * sizeof(char *));
        instance->ptr = malloc((n_op + 1) * sizeof(int));
        instance->options = malloc(n_it * n_op *sizeof(int));         // surallocation massive
        if (instance->item_name == NULL || instance->ptr == NULL || instance->options == NULL)
                err(1, "Impossible d'allouer la mémoire pour stocker la matrice");


        enum state_t {START, ID, WHITESPACE, BAR, ENDLINE, ENDFILE};
        enum state_t state = START;

        char buffer[256];
        int i = 0;     // prochain octet disponible du buffer
        int n = 0;     // dernier octet disponible du buffer

        char id[65];
        id[64] = 0;    // sentinelle à la fin, quoi qu'il arrive
        int j = 0;     // longueur de l'identifiant en cours de lecture

        int current_item = 0;
        while (state != ENDLINE) {
                enum state_t prev_state = state;
                if (i >= n) {
                        n = fread(buffer, 1, 256, in);
                        if (n == 0) {
                                if (feof(in)) {
                                        state = ENDFILE;
                                }
                                if (ferror(in))
                                        err(1, "erreur lors de la lecture de %s", in_filename);
                        }
                        i = 0;

                }
                if (state == ENDFILE) {
                        // don't examine buffer[i]
                } else if (buffer[i] == '\n') {
                        state = ENDLINE;
                } else if (buffer[i] == '|') {
                        state = BAR;
                } else if (isspace(buffer[i])) {
                        state = WHITESPACE;
                } else {
                        state = ID;
                }

                // traite le caractère lu
                if (state == ID) {
                        if (j == 64)
                                errx(1, "nom d'objet trop long : %s", id);
                        id[j] = buffer[i];
                        j++;
                }
                if (prev_state == ID && state != ID) {
                        id[j] = '\0';
                        if (current_item == instance->n_items)
                                errx(1, "Objet excedentaire : %s", id);
                        for (int k = 0; k < current_item; k++)
                                if (strcmp(id, instance->item_name[k]) == 0)
                                        errx(1, "Nom d'objets dupliqué : %s", id);
                        instance->item_name[current_item] = malloc(j+1);
                        strcpy(instance->item_name[current_item], id);
                        current_item++;
                        j = 0;


                }
                if (state == BAR)
                        instance->n_primary = current_item;
                if (state == ENDFILE)
                        errx(1, "Fin de fichier prématurée");
                // passe au prochain caractère
                i++;
        }
        if (current_item != instance->n_items)
                errx(1, "Incohérence : %d objets attendus mais seulement %d fournis\n", 
                                instance->n_items, current_item);
        if (instance->n_primary == 0)
                instance->n_primary = instance->n_items;

        int current_option = 0;
        int p = 0;       // pointeur courant dans instance->options
        instance->ptr[0] = p;
        bool has_primary = false;
        while (state != ENDFILE) {
                enum state_t prev_state = state;
                if (i >= n) {
                        n = fread(buffer, 1, 256, in);
                        if (n == 0) {
                                if (feof(in)) {
                                        state = ENDFILE;
                                }
                                if (ferror(in))
                                        err(1, "erreur lors de la lecture de %s", in_filename);
                        }
                        i = 0;

                }
                if (state == ENDFILE) {
                        // don't examine buffer[i]
                } else if (buffer[i] == '\n') {
                        state = ENDLINE;
                } else if (buffer[i] == '|') {
                        state = BAR;
                } else if (isspace(buffer[i])) {
                        state = WHITESPACE;
                } else {
                        state = ID;
                }

                // traite le caractère lu
                if (state == ID) {
                        if (j == 64)
                                errx(1, "nom d'objet trop long : %s", id);
                        id[j] = buffer[i];
                        j++;
                }
                if (prev_state == ID && state != ID) {
                        id[j] = '\0';
                        // identifie le numéro de l'objet en question
                        int item_number = -1;
                        for (int k = 0; k < instance->n_items; k++)
                                if (strcmp(id, instance->item_name[k]) == 0) {
                                        item_number = k;
                                        break;
                                }
                        if (item_number == -1)
                                errx(1, "Objet %s inconnu dans l'option #%d", id, current_option);
                        // détecte les objets répétés
                        for (int k = instance->ptr[current_option]; k < p; k++)
                                if (item_number == instance->options[k])
                                        errx(1, "Objet %s répété dans l'option %d\n", 
                                                        instance->item_name[item_number], current_option);
                        instance->options[p] = item_number;
                        p++;
                        has_primary |= item_is_primary(instance, item_number);
                        j = 0;


                }
                if (state == BAR) {
                        errx(1, "Trouvé | dans une option.");
                }
                if ((state == ENDLINE || state == ENDFILE)) {
                        // esquive les lignes vides
                        if (p > instance->ptr[current_option]) {
                                if (current_option == instance->n_options)
                                        errx(1, "Option excédentaire");
                                if (!has_primary)
                                        errx(1, "Option %d sans objet primaire\n", current_option);
                                current_option++;
                                instance->ptr[current_option] = p;
                                has_primary = false;


                        }
                }
                // passe au prochain caractère
                i++;
        }
        if (current_option != instance->n_options)
                errx(1, "Incohérence : %d options attendues mais seulement %d fournies\n", 
                                instance->n_options, current_option);


        fclose(in);
        fprintf(stderr, "Lu %d objets (%d principaux) et %d options\n", 
                instance->n_items, instance->n_primary, instance->n_options);
        return instance;
}


struct context_t * backtracking_setup(const struct instance_t *instance)
{
        struct context_t *ctx = malloc(sizeof(*ctx));
        if (ctx == NULL)
                err(1, "impossible d'allouer un contexte");
        ctx->level = 0;
        ctx->nodes = 0;
        ctx->solutions = 0;
        int n = instance->n_items;
        int m = instance->n_options;
        ctx->active_options = malloc(n * sizeof(*ctx->active_options));
        ctx->chosen_options = malloc(n * sizeof(*ctx->chosen_options));
        ctx->child_num = malloc(n * sizeof(*ctx->child_num));
        ctx->num_children = malloc(n * sizeof(*ctx->num_children));
        if (ctx->active_options == NULL || ctx->chosen_options == NULL
                || ctx->child_num == NULL || ctx->num_children == NULL)
                err(1, "impossible d'allouer le contexte");
        ctx->active_items = sparse_array_init(n);
        for (int item = 0; item < instance->n_primary; item++)
                sparse_array_add(ctx->active_items, item);

        for (int item = 0; item < n; item++)
                ctx->active_options[item] = sparse_array_init(m);
        for (int option = 0; option < m; option++)
                for (int k = instance->ptr[option]; k < instance->ptr[option + 1]; k++) {
                        int item = instance->options[k];
                        sparse_array_add(ctx->active_options[item], option);
                }



        return ctx;
}


/**
 * Copie un tableau d'entiers.
 * 
 * @param a tableau d'entiers
 * @param n taille du tableau
 * @return copie de a
 */
int *array_copy(const int *a, int n)
{
        int *A = malloc(n * sizeof(int));
        if (A == NULL)
                err(1, "impossible d'allouer un tableau");
        for (int i = 0; i < n; i++)
        {
                A[i] = a[i];
        }
        return A;
}

/**
 * Copie un tableau creux.
 * 
 * @param s tableau creux
 * @return copie de s
 */
struct sparse_array_t *sparse_array_copy(const struct sparse_array_t *s)
{
        struct sparse_array_t *S = malloc(sizeof(*S));
        if (S == NULL)
                err(1, "impossible d'allouer un tableau creux");
        S->len = s->len;
        S->capacity = s->capacity;
        S->p = array_copy(s->p, s->capacity);
        S->q = array_copy(s->q, s->capacity);
        return S;
}

/**
 * Crée une copie du contexte donné en argument.
 * 
 * @param ctx contexte
 * @param n nombre d'items
 * @return copie de ctx
 */
struct context_t * copy_ctx(const struct context_t *ctx, int n)
{
        struct context_t *ctx_copy = malloc(sizeof(*ctx_copy));
        if (ctx_copy == NULL)
                err(1, "impossible d'allouer un contexte");

        /* Copie de level, nodes et solutions */
        ctx_copy->level = ctx->level;
        ctx_copy->nodes = ctx->nodes;
        ctx_copy->solutions = ctx->solutions;

        /* Copie de chosen_options */
        ctx_copy->chosen_options = array_copy(ctx->chosen_options, n);

        /* Copie de child_num */
        ctx_copy->child_num = array_copy(ctx->child_num, n);

        /* Copie de num_children */
        ctx_copy->num_children = array_copy(ctx->num_children, n);

        /* Copie de active_items */
        ctx_copy->active_items = sparse_array_copy(ctx->active_items);

        /* Copie de active_options */
        ctx_copy->active_options = malloc(n * sizeof(*ctx_copy->active_options));
        for (int item = 0; item < n; item++)
                ctx_copy->active_options[item] = sparse_array_copy(ctx->active_options[item]);

        return ctx_copy;
}

/**
 * Nettoie la mémoire pour un tableau creux.
 * 
 * @param S tableau creux
 */
void sparse_array_free(struct sparse_array_t *S)
{
        free(S->p);
        free(S->q);
        free(S);
}

/**
 * Nettoie la mémoire pour un contexte.
 * 
 * @param ctx contexte
 * @param n nombre d'items
 */
void free_ctx(struct context_t *ctx, int n)
{
        sparse_array_free(ctx->active_items);
        for (int item = 0; item < n; item++)
                sparse_array_free(ctx->active_options[item]);
        free(ctx->active_options);
        free(ctx->chosen_options);
        free(ctx->child_num);
        free(ctx->num_children);
        free(ctx);
}

/**
 * Nettoie la mémoire pour une instance.
 * 
 * @param instance instance
 */
void free_instance(struct instance_t *instance)
{
        if (instance->item_name != NULL)
        {
                for (int i = 0; i < instance->n_items; i++)
                {
                        free(instance->item_name[i]);
                }
                free(instance->item_name);
        }
        free(instance->options);
        free(instance->ptr);
        free(instance);
}


/**
 * Retourne true si la file est vide.
 * 
 * @return file vide
 */
bool queue_is_empty()
{
        return queue_size == 0;
}

/**
 * Enfile un contexte.
 * 
 * @param ctx contexte
 */
void enqueue(struct context_t *ctx)
{
        queue[++queue_rear] = ctx;
        queue_size++;
}

/**
 * Défile un contexte.
 * 
 * @return contexte
 */
struct context_t *dequeue()
{
        queue_size--;
        return queue[queue_front++];
}

/**
 * Libère la mémoire occupée par la file.
 * 
 * @param n nombre d'items
 */
void free_queue(int n)
{
        int size = queue_size;
        for (int i = 0; i < size; i++)
        {
                free_ctx(dequeue(), n);
        }
        free(queue);
}


void solve(const struct instance_t *instance, struct context_t *ctx)
{
        ctx->nodes++;
        if (sparse_array_empty(ctx->active_items)) {
                solution_found(instance, ctx);
                return;                         /* succès : plus d'objet actif */
        }
        int chosen_item = choose_next_item(ctx);
        struct sparse_array_t *active_options = ctx->active_options[chosen_item];
        if (sparse_array_empty(active_options))
                return;           /* échec : impossible de couvrir chosen_item */
        cover(instance, ctx, chosen_item);
        ctx->num_children[ctx->level] = active_options->len;
        for (int k = 0; k < active_options->len; k++) {
                int option = active_options->p[k];
                ctx->child_num[ctx->level] = k;
                choose_option(instance, ctx, option, chosen_item);
                solve(instance, ctx);
                if (ctx->solutions >= max_solutions)
                        return;
                unchoose_option(instance, ctx, option, chosen_item);
        }

        uncover(instance, ctx, chosen_item);                      /* backtrack */
}


/**
 * Résout l'instance du problème en effectuant un parcours BFS s'arrêtant à un 
 * certain niveau, puis termine la résolution en parallèle avec la fonction solve. 
 * 
 * La fonction retourne le nombre de solutions trouvées.
 * 
 * @param instance instance
 * @param ctx contexte
 * @return solutions
 */
long long solve_bfs(const struct instance_t *instance, struct context_t *ctx)
{
        /* Variable pour mesurer le temps de chaque étape */
        double t_start;

        /* Compteur de noeuds à un certain niveau de l'arbre */
        int count;

        /* Niveau de l'arbre */
        int level = 0;

        /* Nombre de solutions trouvées */
        long long solutions = 0;

        /* Initialise la file */
        queue = malloc(instance->n_options * instance->n_options * sizeof(struct context_t*));
        enqueue(ctx);

        /* Parcourt BFS */
        printf("START BFS\n");
        t_start = wtime();

        while (!queue_is_empty())
        {
                count = queue_size;
                printf("- Level %d: %d nodes\n", level, count);

                /* Condition d'arrêt */
                if (count > MIN)
                {
                        break;
                }

                while (count > 0)
                {
                        ctx = dequeue();
                        count--;
                        if (sparse_array_empty(ctx->active_items))
                        {
                                solutions++;
                                free_ctx(ctx, instance->n_items);
                                continue;       /* succès : plus d'objet actif */
                        }
                        int chosen_item = choose_next_item(ctx);
                        struct sparse_array_t *active_options = ctx->active_options[chosen_item];
                        if (sparse_array_empty(active_options))
                        {
                                free_ctx(ctx, instance->n_items);
                                continue;       /* échec : impossible de couvrir chosen_item */
                        }
                        cover(instance, ctx, chosen_item);
                        ctx->num_children[ctx->level] = active_options->len;

                        #pragma omp parallel for
                        for (int k = 0; k < active_options->len; k++)
                        {
                                int option = active_options->p[k];

                                /* Copie du contexte */
                                struct context_t *ctx_copy = copy_ctx(ctx, instance->n_items);

                                /* Choix de l'option sur la copie */
                                ctx_copy->child_num[ctx_copy->level] = k;
                                choose_option(instance, ctx_copy, option, chosen_item);

                                /* Ajout du contexte à la file */
                                #pragma omp critical
                                enqueue(ctx_copy);
                        }

                        free_ctx(ctx, instance->n_items);
                }
                level++;
        }
        printf("END   BFS: %1.fs\n", wtime() - t_start);

        /* Solve en parallèle */
        printf("START For loop: %d iterations\n", queue_size);
        t_start = wtime();

        #pragma omp parallel for reduction(+:solutions) schedule(dynamic)
        for (int i = queue_front; i <= queue_rear; i++)
        {
                queue[i]->solutions = 0;
                solve(instance, queue[i]);
                solutions += queue[i]->solutions;
        }
        printf("END   For loop: %1.fs\n", wtime() - t_start);

        /* Libère la mémoire occupée par la file */
        free_queue(instance->n_items);

        return solutions;
}


int main(int argc, char **argv)
{
        struct option longopts[5] = {
                {"in", required_argument, NULL, 'i'},
                {"progress-report", required_argument, NULL, 'v'},
                {"print-solutions", no_argument, NULL, 'p'},
                {"stop-after", required_argument, NULL, 's'},
                {NULL, 0, NULL, 0}
        };
        char ch;
        while ((ch = getopt_long(argc, argv, "", longopts, NULL)) != -1) {
                switch (ch) {
                case 'i':
                        in_filename = optarg;
                        break;
                case 'p':
                        print_solutions = true;
                        break;
                case 's':
                        max_solutions = atoll(optarg);
                        break;
                case 'v':
                        report_delta = atoll(optarg);
                        break;          
                default:
                        errx(1, "Unknown option\n");
                }
        }
        if (in_filename == NULL)
                usage(argv);
        next_report = report_delta;

        /* Load instance */
        struct instance_t * instance = load_matrix(in_filename);

        /* Création d'un contexte */
        struct context_t * ctx = backtracking_setup(instance);

        start = wtime();

        /* Solve */
        long long solutions = solve_bfs(instance, ctx);

        /* Free instance */
        free_instance(instance);

        printf("FINI. Trouvé %lld solutions en %.1fs\n", solutions,
                wtime() - start);
        exit(EXIT_SUCCESS);
}
