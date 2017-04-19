#ifndef GPRS_FLUX_H_
#define GPRS_FLUX_H_

#include <sqlite3.h>
///#include <sqlite3ext.h>

int add_byte_via_date(int flux, char *date);
void sim_card_flux_database_init(void);
int get_flux_in_sum(void);

#endif /* gprs_flux.h */
