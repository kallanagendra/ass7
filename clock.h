/* This is our virtual clock, shared between all processes */
struct clock{
	unsigned int sec;	/* seconds */
	unsigned int nsec;	/* nanoseconds*/
};

//return type for the clock check
enum check_res {	EQUAL=0,	/* c1 == c2 */
									AFTER=1,	/* c1 > c2 */
									BEFORE=2};/* c1 < c2 */

/* compare two clocks */
enum check_res check_time(struct clock *c1, struct clock * c2);
/* increment clock wit hthe specified nanosecond interval */
int add_time(struct clock *c, const unsigned int nsec);
