#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

bool IsEmpty(BENSCHILLIBOWL* bcb);
bool IsFull(BENSCHILLIBOWL* bcb);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BENSCHILLIBOWLMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BENSCHILLIBOWLMenuLength = 10;

/* Select a random item from the Menu and return it */
MenuItem PickRandomMenuItem() {
  int random_num = rand() % BENSCHILLIBOWLMenuLength;
  return BENSCHILLIBOWLMenu[random_num];
}

/* Allocate memory for the Restaurant, then create the mutex and condition variables needed to instantiate the Restaurant */

BENSCHILLIBOWL* OpenRestaurant(int max_size, int expected_num_orders) {
  BENSCHILLIBOWL *restaurant = (BENSCHILLIBOWL*)malloc(sizeof(BENSCHILLIBOWL));
  restaurant->orders = NULL;
  restaurant->current_size = 0;
	restaurant->next_order_number=1;
  restaurant->orders_handled=0;
  restaurant->max_size = max_size;
  restaurant->expected_num_orders=expected_num_orders;
  
  //initialize mutex
  pthread_mutex_init(&(restaurant->mutex), NULL);
	
  printf("Restaurant is open!\n");
  return restaurant;
}


/* check that the number of orders received is equal to the number handled (ie.fullfilled). Remember to deallocate your resources */

void CloseRestaurant(BENSCHILLIBOWL* bcb) {
  if (bcb->orders_handled != bcb->expected_num_orders) {    // ensure all the orders were handled
        fprintf(stderr, "Not all the orders were handled.\n");
        exit(0);
    }
    pthread_mutex_destroy(&(bcb->mutex));  // delete the synchronization variables
    free(bcb);   // free space used by restaurant
    printf("Restaurant is closed!\n");
}

/* add an order to the back of queue */
int AddOrder(BENSCHILLIBOWL* bcb, Order* order) {
  pthread_mutex_lock(&(bcb->mutex)); //order queue is in critical section. grab the lock
  
    while (bcb->current_size == bcb->max_size) { // wait until restaurant is not full
        pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
    }
  
    order->order_number = bcb->next_order_number;
    AddOrderToBack(&(bcb->orders), order);
  
    bcb->next_order_number++; //update next order number 
    bcb->current_size++;   //update current size
    
    pthread_cond_broadcast(&(bcb->can_get_orders)); // sending signal to indicate an order had been added.
        
    pthread_mutex_unlock(&(bcb->mutex)); // sending signal to indicate an order had been added.
    
    return order->order_number;
}

/* remove an order from the queue */
Order *GetOrder(BENSCHILLIBOWL* bcb) {
  pthread_mutex_lock(&(bcb->mutex)); // grabbing the lock as order is in critical section
       
    while(bcb->current_size == 0) {  // waiting until restaurant is not empty
        
        // If all the orders have been already fulfilled notify cook by unlocking the mutex.
        if (bcb->orders_handled >= bcb->expected_num_orders) {
            pthread_mutex_unlock(&(bcb->mutex));
            return NULL;
        }
        pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
    }
    
    // Get the order from the front.
    Order *order = bcb->orders;
    bcb->orders = bcb->orders->next;
    
    bcb->current_size--; // update the current order size
    bcb->orders_handled++; // update the orders handled
    
    pthread_cond_broadcast(&(bcb->can_add_orders));
        
    // Release the lock.
    pthread_mutex_unlock(&(bcb->mutex));   
    return order;
}

// Optional helper functions (you can implement if you think they would be useful)
bool IsEmpty(BENSCHILLIBOWL* bcb) {
  if (bcb->current_size = 0)
		return true;
	else
		return false;
}

bool IsFull(BENSCHILLIBOWL* bcb) {
	if (bcb->expected_num_orders >= bcb->max_size)
		return true;
	else
		return false;
}

/* this methods adds order to rear of queue */
void AddOrderToBack(Order **orders, Order *order) {
  if (*orders == NULL) {
    *orders = order;
  }
  else {
    Order *new_order = *orders;
    while (new_order->next) {
      new_order = new_order->next;
    }
    new_order->next = order;
  }
}
