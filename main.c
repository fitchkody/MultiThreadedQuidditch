#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

// Helpers
void random_sleep(int);

// Thread functions
void *bludger_quaffle_beater_function(void *);
void *snitch_function(void *);
void *chaser_function(void *);
void *keeper_function(void *);
void *seeker_function(void *);
void *goal_function(void *);

// Signal handlers
void hit_by_bludger_or_goal_attempt(int);
void saved_by_beater_or_goal_blocked(int);
void caught_quaffle(int);

// Global variables
const int MAX_BLUDGER_SLEEP_TIME = 20;
const int MAX_SNITCH_SLEEP_TIME = 5;
const int MAX_SEEKER_SLEEP_TIME = 20;
const int MAX_KEEPER_SLEEP_TIME = 20;
const int MIN_SLEEP_TIME = 1;
const int NUMBER_OF_PLAYERS = 14;
const int NUMBER_OF_CHASERS = 6;
static volatile sig_atomic_t Caught_Snitch = 0;
static volatile sig_atomic_t team_a_score = 0;
static volatile sig_atomic_t team_b_score = 0;

// Player representation
struct Player
{
    pthread_t *thread;
    char team;
    char name[11];
    int id;
    int alive;
};

// Team A
pthread_t seeker_a;
pthread_t keeper_a;
pthread_t beater_a_1, beater_a_2;
pthread_t chaser_a_1, chaser_a_2, chaser_a_3;

// Team B
pthread_t seeker_b;
pthread_t keeper_b;
pthread_t beater_b_1, beater_b_2;
pthread_t chaser_b_1, chaser_b_2, chaser_b_3;

// Player structs
struct Player seeker_a_struct = {.thread = &seeker_a, .team = 'A', .name = "seeker_a", .id = 1, .alive = 1};
struct Player seeker_b_struct = {.thread = &seeker_b, .team = 'B', .name = "seeker_b", .id = 2, .alive = 1};
struct Player keeper_a_struct = {.thread = &keeper_a, .team = 'A', .name = "keeper_a", .id = 3, .alive = 1};
struct Player keeper_b_struct = {.thread = &keeper_b, .team = 'B', .name = "keeper_b", .id = 4, .alive = 1};
struct Player beater_a_1_struct = {.thread = &beater_a_1, .team = 'A', .name = "beater_a_1", .id = 5, .alive = 1};
struct Player beater_a_2_struct = {.thread = &beater_a_2, .team = 'A', .name = "beater_a_2", .id = 6, .alive = 1};
struct Player beater_b_1_struct = {.thread = &beater_b_1, .team = 'B', .name = "beater_b_1", .id = 7, .alive = 1};
struct Player beater_b_2_struct = {.thread = &beater_b_2, .team = 'B', .name = "beater_b_2", .id = 8, .alive = 1};
struct Player chaser_a_1_struct = {.thread = &chaser_a_1, .team = 'A', .name = "chaser_a_1", .id = 9, .alive = 1};
struct Player chaser_a_2_struct = {.thread = &chaser_a_2, .team = 'A', .name = "chaser_a_2", .id = 10, .alive = 1};
struct Player chaser_a_3_struct = {.thread = &chaser_a_3, .team = 'A', .name = "chaser_a_3", .id = 11, .alive = 1};
struct Player chaser_b_1_struct = {.thread = &chaser_b_1, .team = 'B', .name = "chaser_b_1", .id = 12, .alive = 1};
struct Player chaser_b_2_struct = {.thread = &chaser_b_2, .team = 'B', .name = "chaser_b_2", .id = 13, .alive = 1};
struct Player chaser_b_3_struct = {.thread = &chaser_b_3, .team = 'B', .name = "chaser_b_3", .id = 14, .alive = 1};

// Array of pointers to all of the player structs
struct Player *player_pointers[14] = {&seeker_a_struct, &seeker_b_struct,
                                      &keeper_a_struct, &keeper_b_struct,
                                      &beater_a_1_struct, &beater_a_2_struct, &beater_b_1_struct, &beater_b_2_struct,
                                      &chaser_a_1_struct, &chaser_a_2_struct, &chaser_a_3_struct, &chaser_b_1_struct, &chaser_b_2_struct, &chaser_b_3_struct};

// Balls
pthread_t snitch;
pthread_t quaffle;
pthread_t bludger_1, bludger_2;

// Goals
pthread_t goal_a, goal_b; // goal_a is defended by Team A

int main()
{
    srand((unsigned int)time(NULL));

    // Setup signal handlers
    struct sigaction sigint_act;
    sigint_act.sa_handler = hit_by_bludger_or_goal_attempt;
    sigint_act.sa_flags = 0;
    sigemptyset(&sigint_act.sa_mask);
    sigaction(SIGINT, &sigint_act, NULL);

    struct sigaction sigusr1_act;
    sigusr1_act.sa_handler = saved_by_beater_or_goal_blocked;
    sigusr1_act.sa_flags = 0;
    sigemptyset(&sigusr1_act.sa_mask);
    sigaction(SIGUSR1, &sigusr1_act, NULL);

    struct sigaction sigusr2_act;
    sigusr2_act.sa_handler = caught_quaffle;
    sigusr2_act.sa_flags = 0;
    sigemptyset(&sigusr2_act.sa_mask);
    sigaction(SIGUSR2, &sigusr2_act, NULL);

    // Create bludgers
    char b = 'b';
    pthread_create(&bludger_1, NULL, &bludger_quaffle_beater_function, (void *)&b);
    pthread_create(&bludger_2, NULL, &bludger_quaffle_beater_function, (void *)&b);

    // Create quaffles
    char q = 'q';
    pthread_create(&quaffle, NULL, &bludger_quaffle_beater_function, (void *)&q);

    // Create snitch
    pthread_create(&snitch, NULL, &snitch_function, NULL);

    // Create chasers
    pthread_create(&chaser_a_1, NULL, &chaser_function, NULL);
    pthread_create(&chaser_a_2, NULL, &chaser_function, NULL);
    pthread_create(&chaser_a_3, NULL, &chaser_function, NULL);
    pthread_create(&chaser_b_1, NULL, &chaser_function, NULL);
    pthread_create(&chaser_b_2, NULL, &chaser_function, NULL);
    pthread_create(&chaser_b_3, NULL, &chaser_function, NULL);

    // Create keepers
    pthread_create(&keeper_a, NULL, &keeper_function, NULL);
    pthread_create(&keeper_b, NULL, &keeper_function, NULL);

    // Create beaters
    char t = 't';
    pthread_create(&beater_a_1, NULL, &bludger_quaffle_beater_function, (void *)&t);
    pthread_create(&beater_a_2, NULL, &bludger_quaffle_beater_function, (void *)&t);
    pthread_create(&beater_b_1, NULL, &bludger_quaffle_beater_function, (void *)&t);
    pthread_create(&beater_b_2, NULL, &bludger_quaffle_beater_function, (void *)&t);

    // Create seekers
    pthread_create(&seeker_a, NULL, &seeker_function, NULL);
    pthread_create(&seeker_b, NULL, &seeker_function, NULL);

    // Create goals
    pthread_create(&goal_a, NULL, &goal_function, NULL);
    pthread_create(&goal_b, NULL, &goal_function, NULL);

    // Join all players, so the game exits if they all fall off their brooms
    for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
    {
        pthread_join(*player_pointers[i]->thread, NULL);
    }

    printf("All players have been bludgend. GAME OVER...\n");
    exit(0);
}

// Thread functions
void *bludger_quaffle_beater_function(void *p)
{
    int num = NUMBER_OF_PLAYERS;
    int offset = 0;
    int signum = SIGINT;

    // Determine which type of entity that this thread represents
    if (*((char *)p) == 'b')
    {
        // case for bludger
        // variables already apply
    }
    else if (*((char *)p) == 'q')
    {
        // case for quaffle
        num = NUMBER_OF_CHASERS;
        offset = 8;
        signum = SIGUSR2;
    }
    else if (*((char *)p) == 't')
    {
        // case for beater
        signum = SIGUSR1;
    }

    // Sleep, pick random player, send signal according to this threads type, repeat
    for (;;)
    {
        random_sleep(MAX_BLUDGER_SLEEP_TIME);
        int random_player_index;
        do
            random_player_index = rand() % num + offset;
        while (player_pointers[random_player_index]->alive == 0);

        pthread_t *pRandom_player = player_pointers[random_player_index]->thread;
        pthread_kill(*pRandom_player, signum);
    }
}

void *seeker_function(void *p)
{
    for (;;)
    {
        random_sleep(MAX_SEEKER_SLEEP_TIME);
        printf("Reaching for Golden Snitch...");
        if (Caught_Snitch == 1)
        {
            for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
            {
                if (player_pointers[i]->alive == 0)
                    continue;
                if (*(player_pointers[i]->thread) == pthread_self())
                {
                    if (player_pointers[i]->team == 'A')
                        team_a_score += 150;
                    else
                        team_b_score += 150;
                    printf("%s CAUGHT THE SNITCH!!! GAME OVER!!!\n\n", player_pointers[i]->name);
                    printf("Final Scores - Team A: %i, Team B: %i\n\n", team_a_score, team_b_score);
                    exit(0);
                }
            }
        }
        else
            printf("missed.\n\n");
    }
}

void *keeper_function(void *p)
{
    for (;;)
    {
        random_sleep(MAX_KEEPER_SLEEP_TIME);
        for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
        {
            if (player_pointers[i]->alive == 0)
                continue;
            if (*(player_pointers[i]->thread) == pthread_self())
            {
                pthread_t goal = player_pointers[i]->team == 'A' ? goal_a : goal_b;
                pthread_kill(goal, SIGUSR1);
            }
        }
    }
}

void *snitch_function(void *p)
{
    for (;;)
    {
        random_sleep(MAX_SNITCH_SLEEP_TIME);
        Caught_Snitch = 1;
        sleep(1);
        Caught_Snitch = 0;
    }
}

void *chaser_function(void *p)
{
    // Chaser waits to receive the quaffle, then makes a goal attempt. This is done via an sa_handler
    for (;;)
        ;
}

void *goal_function(void *p)
{
    // Goal waits to receive an attempt on it from a chaser, then adjusts the score if completed. This is done via an sa_handler
    for (;;)
        ;
}

// Signal handlers
void hit_by_bludger_or_goal_attempt(int signum)
{
    // Determine if this SIGINT was sent to a player (representing a bludger) or a goal (representing a quaffle)
    int is_player = 0;
    for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
    {
        if (*(player_pointers[i]->thread) == pthread_self())
        {
            is_player = 1;
            break;
        }
    }

    if (is_player == 0)
    {
        write(STDERR_FILENO, "Attempt on goal!!!\n\n", 20);
        if (pthread_self() == goal_a)
            team_b_score += 10;
        else
            team_a_score += 10;
    }
    else
    {
        char print_string[35];
        char name[13];
        write(STDERR_FILENO, "PLAYERS LEFT: ", 14);

        for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
        {
            if (player_pointers[i]->alive == 0)
                continue;
            if (*(player_pointers[i]->thread) == pthread_self())
            {
                strcpy(print_string, player_pointers[i]->name);
                player_pointers[i]->alive = 0;
            }
            else
            {
                strcpy(name, player_pointers[i]->name);
                strcat(name, ", ");
                write(STDERR_FILENO, name, sizeof(name));
            }
        }

        write(STDERR_FILENO, "\n", 1);
        strcat(print_string, " was HIT by bludger!!!\n\n");
        write(STDERR_FILENO, "\n...after ", 10);
        write(STDERR_FILENO, print_string, 35);
        pthread_exit(0);
    }
}

void saved_by_beater_or_goal_blocked(int signum)
{
    // Determine if this SIGINT was sent to a player (representing a bludger) or a goal (representing a quaffle)
    int is_player = 0;
    for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
    {
        if (*(player_pointers[i]->thread) == pthread_self())
        {
            is_player = 1;
            break;
        }
    }

    if (is_player == 0)
        write(STDERR_FILENO, "KEEPER MAKES THE SAVE!!!\n\n", 26);
    write(STDERR_FILENO, "PLAYER SAVED BY BEATER!!!\n\n", 27);
}

void caught_quaffle(int signum)
{
    write(STDERR_FILENO, "Player in possesion of the quaffle.\n\n", 37);

    for (int i = 0; i < NUMBER_OF_PLAYERS; ++i)
    {
        if (player_pointers[i]->alive == 0)
            continue;
        if (*(player_pointers[i]->thread) == pthread_self())
        {
            pthread_t goal = player_pointers[i]->team == 'A' ? goal_b : goal_a;
            pthread_kill(goal, SIGINT);
        }
    }
}

void random_sleep(int time)
{
    int sleep_time = rand() % time;
    sleep_time += MIN_SLEEP_TIME;
    sleep((unsigned int)sleep_time);
}