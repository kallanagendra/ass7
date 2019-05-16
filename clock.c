#include "clock.h"

/* compare two clocks */
enum check_res check_time(struct clock *c1, struct clock * c2){

  if (c1->sec > c2->sec) {        return AFTER; /* c1 is after c2 */

  }else if (c1->sec < c2->sec) {  return BEFORE;  /* c1 is before c2*/

  }else if (c1->sec == c2->sec){
    if(c1->nsec > c2->nsec)       return AFTER;
    else if(c1->nsec < c2->nsec)  return BEFORE;
    else                          return EQUAL;

  } else {                        return EQUAL;
  }
}

/* increment clock wit hthe specified nanosecond interval */
int add_time(struct clock *c, const unsigned int nsec){

  c->nsec += nsec;
	if(c->nsec >= 1000000000){
    c->nsec = 0;
		c->sec++;
	}
  return 0;
}
