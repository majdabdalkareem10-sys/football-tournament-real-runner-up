#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_INPUT_LINE 70000

/*
 * Football Tournament - Real Runner-Up
 *
 * Queue: circular array
 * Stack: linked list
 *
 * Input ends with -9999.
 * Both spaces and commas are accepted as separators.
 */

typedef struct {
    int *data;
    int capacity;
    int front;
    int size;
} Queue;

typedef struct StackNode {
    int value;
    struct StackNode *next;
} StackNode;

typedef struct {
    StackNode *top;
    int size;
} Stack;

static void fail(const char *message) {
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}

static void *safe_malloc(size_t bytes) {
    void *ptr = malloc(bytes);

    if (ptr == NULL) {
        fail("Fatal error: memory allocation failed.");
    }

    return ptr;
}

/* ========================= Queue ========================= */

static Queue *queue_create(int capacity) {
    Queue *queue = safe_malloc(sizeof(*queue));

    if (capacity < 1) {
        capacity = 1;
    }

    queue->data = safe_malloc((size_t)capacity * sizeof(*queue->data));
    queue->capacity = capacity;
    queue->front = 0;
    queue->size = 0;

    return queue;
}

static void queue_destroy(Queue *queue) {
    if (queue != NULL) {
        free(queue->data);
        free(queue);
    }
}

static bool queue_is_empty(const Queue *queue) {
    return queue->size == 0;
}

static void queue_enqueue(Queue *queue, int value) {
    if (queue->size == queue->capacity) {
        fail("Fatal error: queue capacity exceeded.");
    }

    int rear = (queue->front + queue->size) % queue->capacity;
    queue->data[rear] = value;
    ++queue->size;
}

static int queue_dequeue(Queue *queue) {
    if (queue_is_empty(queue)) {
        fail("Fatal error: attempted to dequeue from an empty queue.");
    }

    int value = queue->data[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    --queue->size;

    return value;
}

static Queue *queue_from_array(const int *values, int count) {
    Queue *queue = queue_create(count);

    for (int i = 0; i < count; ++i) {
        queue_enqueue(queue, values[i]);
    }

    return queue;
}

/* ========================= Stack ========================= */

static void stack_init(Stack *stack) {
    stack->top = NULL;
    stack->size = 0;
}

static bool stack_is_empty(const Stack *stack) {
    return stack->top == NULL;
}

static void stack_push(Stack *stack, int value) {
    StackNode *node = safe_malloc(sizeof(*node));

    node->value = value;
    node->next = stack->top;
    stack->top = node;
    ++stack->size;
}

static int stack_pop(Stack *stack) {
    if (stack_is_empty(stack)) {
        fail("Fatal error: attempted to pop from an empty stack.");
    }

    StackNode *node = stack->top;
    int value = node->value;

    stack->top = node->next;
    --stack->size;
    free(node);

    return value;
}

static void stack_destroy(Stack *stack) {
    while (!stack_is_empty(stack)) {
        (void)stack_pop(stack);
    }
}

/* ==================== Tournament Logic ==================== */

static int match_winner(int first, int second, int round_number) {
    if (round_number % 2 != 0) {
        return first < second ? first : second;
    }

    return first > second ? first : second;
}

/*
 * Plays one round and destroys the old queue.
 * If losses is not NULL, direct losses against champion are pushed.
 */
static Queue *play_round(
    Queue *current,
    int round_number,
    int champion,
    Stack *losses
) {
    int team_count = current->size;
    bool has_bye = team_count % 2 != 0;
    int paired_count = team_count - (has_bye ? 1 : 0);

    Queue *winners = queue_create(paired_count / 2);

    for (int i = 0; i < paired_count; i += 2) {
        int first = queue_dequeue(current);
        int second = queue_dequeue(current);

        int winner = match_winner(first, second, round_number);
        int loser = winner == first ? second : first;

        if (losses != NULL && winner == champion) {
            stack_push(losses, loser);
        }

        queue_enqueue(winners, winner);
    }

    int automatic_qualifier = 0;

    if (has_bye) {
        automatic_qualifier = queue_dequeue(current);
    }

    Queue *next = queue_create((team_count + 1) / 2);

    /*
     * The final unpaired team goes to the head
     * of the next round.
     */
    if (has_bye) {
        queue_enqueue(next, automatic_qualifier);
    }

    while (!queue_is_empty(winners)) {
        queue_enqueue(next, queue_dequeue(winners));
    }

    queue_destroy(winners);
    queue_destroy(current);

    return next;
}

/*
 * Takes ownership of teams and returns
 * the tournament winner.
 */
static int run_tournament(
    Queue *teams,
    int champion,
    Stack *losses
) {
    int round_number = 1;

    while (teams->size > 1) {
        teams = play_round(
            teams,
            round_number,
            champion,
            losses
        );

        ++round_number;
    }

    int winner = queue_dequeue(teams);
    queue_destroy(teams);

    return winner;
}

static int find_real_runner_up(
    const int *team_ids,
    int team_count
) {
    /*
     * First simulation:
     * determine the tournament champion.
     */
    int champion = run_tournament(
        queue_from_array(team_ids, team_count),
        0,
        NULL
    );

    /*
     * Second simulation:
     * collect every team defeated directly
     * by the champion.
     */
    Stack losses;
    stack_init(&losses);

    int repeated_champion = run_tournament(
        queue_from_array(team_ids, team_count),
        champion,
        &losses
    );

    if (
        repeated_champion != champion ||
        stack_is_empty(&losses)
    ) {
        stack_destroy(&losses);
        fail("Fatal error: inconsistent tournament state.");
    }

    /*
     * The stack provides the required reverse order:
     * final-round loser first, then earlier losers.
     */
    Queue *secondary_teams = queue_create(losses.size);

    while (!stack_is_empty(&losses)) {
        queue_enqueue(
            secondary_teams,
            stack_pop(&losses)
        );
    }

    stack_destroy(&losses);

    return run_tournament(
        secondary_teams,
        0,
        NULL
    );
}

/* ========================= Input ========================= */

static bool read_next_int(char **cursor, int *value) {
    char *ptr = *cursor;

    while (
        *ptr != '\0' &&
        (
            isspace((unsigned char)*ptr) ||
            *ptr == ','
        )
    ) {
        ++ptr;
    }

    if (*ptr == '\0') {
        return false;
    }

    char *end;
    long number = strtol(ptr, &end, 10);

    if (
        ptr == end ||
        number < -10000 ||
        number > 10000
    ) {
        return false;
    }

    *value = (int)number;
    *cursor = end;

    return true;
}

/* ========================== Main ========================== */

int main(void) {
    char line[MAX_INPUT_LINE];

    while (fgets(line, sizeof(line), stdin) != NULL) {
        char *cursor = line;
        int number_of_teams;

        if (!read_next_int(
                &cursor,
                &number_of_teams
            )) {
            continue;
        }

        if (number_of_teams == -9999) {
            break;
        }

        if (
            number_of_teams <= 1 ||
            number_of_teams >= 10000
        ) {
            fail(
                "Invalid team count. "
                "Expected 1 < n < 10000."
            );
        }

        int *team_ids = safe_malloc(
            (size_t)number_of_teams *
            sizeof(*team_ids)
        );

        for (
            int i = 0;
            i < number_of_teams;
            ++i
        ) {
            if (!read_next_int(
                    &cursor,
                    &team_ids[i]
                )) {
                free(team_ids);
                fail(
                    "Missing or invalid team ID."
                );
            }

            if (
                team_ids[i] <= -10000 ||
                team_ids[i] >= 10000
            ) {
                free(team_ids);
                fail(
                    "Team ID must satisfy "
                    "-10000 < id < 10000."
                );
            }
        }

        int runner_up = find_real_runner_up(
            team_ids,
            number_of_teams
        );

        printf("%d\n", runner_up);
        free(team_ids);
    }

    return EXIT_SUCCESS;
}