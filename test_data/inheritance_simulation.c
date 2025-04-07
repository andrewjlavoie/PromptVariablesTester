#include "inheritance_simulation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Base Animal implementation */
void animal_init(Animal* animal, const char* species, int age) {
    animal->species = species;
    animal->age = age;
    
    /* Set up virtual function pointers to default implementations */
    animal->make_sound = animal_make_sound;
    animal->move = animal_move;
    animal->eat = animal_eat;
}

void animal_make_sound(Animal* animal) {
    printf("Generic animal sound from %s\n", animal->species);
}

void animal_move(Animal* animal) {
    printf("%s is moving\n", animal->species);
}

void animal_eat(Animal* animal, const char* food) {
    printf("%s is eating %s\n", animal->species, food);
}

/* Dog implementation */
void dog_init(Dog* dog, const char* breed, int age, int loyalty) {
    /* Initialize base class */
    animal_init((Animal*)dog, "Dog", age);
    
    /* Set dog-specific properties */
    dog->breed = breed;
    dog->loyalty = loyalty;
    
    /* Override virtual methods with dog-specific implementations */
    dog->base.make_sound = dog_make_sound;
    dog->base.move = dog_move;
    dog->base.eat = dog_eat;
}

void dog_make_sound(Animal* animal) {
    Dog* dog = (Dog*)animal;
    printf("Woof! I'm a %s dog, %d years old\n", dog->breed, animal->age);
}

void dog_move(Animal* animal) {
    Dog* dog = (Dog*)animal;
    printf("The %s dog is running\n", dog->breed);
}

void dog_eat(Animal* animal, const char* food) {
    Dog* dog = (Dog*)animal;
    printf("The %s dog is eating %s with enthusiasm\n", dog->breed, food);
}

void dog_fetch(Dog* dog, const char* item) {
    printf("The %s dog fetches the %s and brings it back (loyalty: %d)\n", 
           dog->breed, item, dog->loyalty);
}

/* Bird implementation */
void bird_init(Bird* bird, const char* species, int age, float wingspan, int can_fly) {
    /* Initialize base class */
    animal_init((Animal*)bird, species, age);
    
    /* Set bird-specific properties */
    bird->wingspan = wingspan;
    bird->can_fly = can_fly;
    
    /* Override virtual methods with bird-specific implementations */
    bird->base.make_sound = bird_make_sound;
    bird->base.move = bird_move;
    bird->base.eat = bird_eat;
}

void bird_make_sound(Animal* animal) {
    Bird* bird = (Bird*)animal;
    printf("Chirp! I'm a %s bird with %.1f wingspan\n", animal->species, bird->wingspan);
}

void bird_move(Animal* animal) {
    Bird* bird = (Bird*)animal;
    if (bird->can_fly) {
        printf("The %s bird is flying\n", animal->species);
    } else {
        printf("The %s bird is hopping around\n", animal->species);
    }
}

void bird_eat(Animal* animal, const char* food) {
    printf("The %s bird is pecking at %s\n", animal->species, food);
}

void bird_fly(Bird* bird, int altitude) {
    if (bird->can_fly) {
        printf("The %s bird flies to %d meters altitude\n", 
               bird->base.species, altitude);
    } else {
        printf("The %s bird cannot fly\n", bird->base.species);
    }
}

/* Simple polymorphic function that works with any animal */
void interact_with_animal(Animal* animal) {
    printf("\nInteracting with a %s:\n", animal->species);
    animal->make_sound(animal);
    animal->move(animal);
    animal->eat(animal, "food");
}

/* Test our inheritance simulation */
int main() {
    /* Create and initialize a dog */
    Dog rover;
    dog_init(&rover, "Golden Retriever", 3, 10);
    
    /* Create and initialize birds */
    Bird eagle;
    bird_init(&eagle, "Eagle", 5, 2.1f, 1);
    
    Bird penguin;
    bird_init(&penguin, "Penguin", 7, 0.5f, 0);
    
    /* Use the objects directly */
    printf("Direct usage:\n");
    rover.base.make_sound(&rover.base);
    dog_fetch(&rover, "stick");
    
    eagle.base.make_sound(&eagle.base);
    bird_fly(&eagle, 100);
    
    penguin.base.make_sound(&penguin.base);
    bird_fly(&penguin, 50);
    
    /* Demonstrate polymorphism by using the common Animal interface */
    interact_with_animal((Animal*)&rover);
    interact_with_animal((Animal*)&eagle);
    interact_with_animal((Animal*)&penguin);
    
    return 0;
}