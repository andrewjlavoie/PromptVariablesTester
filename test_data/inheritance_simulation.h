#ifndef INHERITANCE_SIMULATION_H
#define INHERITANCE_SIMULATION_H

/**
 * Simulation of OOP-style inheritance in C
 */

/* Base "class" for all animals */
typedef struct Animal {
    const char* species;
    int age;
    
    /* Virtual functions (function pointers) */
    void (*make_sound)(struct Animal* self);
    void (*move)(struct Animal* self);
    void (*eat)(struct Animal* self, const char* food);
} Animal;

/* Initialize a base animal */
void animal_init(Animal* animal, const char* species, int age);

/* Default methods for animals */
void animal_make_sound(Animal* animal);
void animal_move(Animal* animal);
void animal_eat(Animal* animal, const char* food);

/* "Derived class" for Dog */
typedef struct {
    Animal base;  /* Inheritance - first member is the base "class" */
    const char* breed;
    int loyalty;
} Dog;

/* Initialize a dog */
void dog_init(Dog* dog, const char* breed, int age, int loyalty);

/* Override methods for dogs */
void dog_make_sound(Animal* animal);
void dog_move(Animal* animal);
void dog_eat(Animal* animal, const char* food);

/* Dog-specific method */
void dog_fetch(Dog* dog, const char* item);

/* "Derived class" for Bird */
typedef struct {
    Animal base;  /* Inheritance - first member is the base "class" */
    float wingspan;
    int can_fly;
} Bird;

/* Initialize a bird */
void bird_init(Bird* bird, const char* species, int age, float wingspan, int can_fly);

/* Override methods for birds */
void bird_make_sound(Animal* animal);
void bird_move(Animal* animal);
void bird_eat(Animal* animal, const char* food);

/* Bird-specific method */
void bird_fly(Bird* bird, int altitude);

#endif /* INHERITANCE_SIMULATION_H */