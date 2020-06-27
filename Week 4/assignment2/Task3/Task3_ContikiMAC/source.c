#include "contiki.h"
#include "project-conf.h"
#include "net/rime.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "sys/energest.h"
#include "random.h"
#include <math.h>
#include <stdio.h>
#include <clock.h>

#define Voltage 3.6
#define I_cpu 0.0018
#define I_lpm 0.0000545
#define I_transmit 0.0195
#define I_listen 0.0218

/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Example unicast");
AUTOSTART_PROCESSES(&example_unicast_process);
/*---------------------------------------------------------------------------*/
static void
recv_uc(struct unicast_conn *c, const rimeaddr_t *from)
{
  printf("unicast message received from %d.%d: '%s'\n",
         from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}
static const struct unicast_callbacks unicast_callbacks = {recv_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data)
{
  static struct etimer et;
  rimeaddr_t addr;
  static unsigned int all_cpu, all_lpm, all_transmit, all_listen;
  static unsigned int last_cpu, last_lpm, last_transmit, last_listen = 0;
  float E_cpu, E_lpm, E_transmit, E_listen;

  PROCESS_EXITHANDLER(unicast_close(&uc);)
  PROCESS_BEGIN();
  
  unicast_open(&uc, 129, &unicast_callbacks);

  while(1) {
    /* Delay 2-4 seconds */
    etimer_set(&et, CLOCK_SECOND * 4 + random_rand() % (CLOCK_SECOND * 4));
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    // Read the total on time of the components
    energest_flush();
    all_cpu = energest_type_time(ENERGEST_TYPE_CPU);
    all_lpm = energest_type_time(ENERGEST_TYPE_LPM);
    all_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
    all_listen = energest_type_time(ENERGEST_TYPE_LISTEN);

    // Calculate the energy estimations
    E_cpu      = (all_cpu - last_cpu)           / CLOCK_SECOND * I_cpu * Voltage;
    E_lpm      = (all_lpm - last_lpm)           / CLOCK_SECOND * I_lpm * Voltage;
    E_transmit = (all_transmit - last_transmit) / CLOCK_SECOND * I_transmit * Voltage;
    E_listen   = (all_listen - last_listen)     / CLOCK_SECOND * I_listen * Voltage;

    // Read the last on time of the components
    last_cpu = energest_type_time(ENERGEST_TYPE_CPU);
    last_lpm = energest_type_time(ENERGEST_TYPE_LPM);
    last_transmit = energest_type_time(ENERGEST_TYPE_TRANSMIT);
    last_listen = energest_type_time(ENERGEST_TYPE_LISTEN);

    // Print the floating point values with a small trick
    printf("Energy cpu: %d.%d\n",      (int)E_cpu,      (int)((E_cpu - (int)E_cpu) * 1000));
    printf("Energy lpm: %d.%d\n",      (int)E_lpm,      (int)((E_lpm - (int)E_lpm) * 1000));
    printf("Energy transmit: %d.%d\n", (int)E_transmit, (int)((E_transmit - (int)E_transmit) * 1000));
    printf("Energy listen: %d.%d\n",   (int)E_listen,   (int)((E_listen - (int)E_listen) * 1000));

    packetbuf_copyfrom("Hello", 6);
    addr.u8[0] = 1;
    addr.u8[1] = 0;
    
    if(!rimeaddr_cmp(&addr, &rimeaddr_node_addr)) {
      unicast_send(&uc, &addr);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
