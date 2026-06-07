#include "contiki.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#include "sys/node-id.h"
#include <string.h>

#define LOG_MODULE "OTA-Client"
#define LOG_LEVEL  LOG_LEVEL_INFO
#define UDP_PORT   8765
#define BLOCK_SIZE 64
#define TOTAL_BLOCKS 2028 // 129760 / 64

struct ota_packet {
  uint16_t seq_num;
  uint16_t len;
  uint8_t  data[BLOCK_SIZE];
};

static struct simple_udp_connection udp_conn;
static uip_ipaddr_t dest_ipaddr;

PROCESS(udp_client_process, "OTA Sender");
AUTOSTART_PROCESSES(&udp_client_process);

PROCESS_THREAD(udp_client_process, ev, data) {
  static struct etimer timer;
  static struct ota_packet pkt;
  static uint16_t g_seq = 0;

  PROCESS_BEGIN();
  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, NULL);

  if(node_id == 2) {
    LOG_INFO("Node 2: Agin kurulmasi bekleniyor (15sn)...\n");
    etimer_set(&timer, CLOCK_SECOND * 15);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    // ADRES: Senin loglarindaki Node 1 adresi
    uip_ip6addr(&dest_ipaddr, 0xfd00, 0, 0, 0, 0xc30c, 0, 0, 1);
    
    LOG_INFO("OTA AKTARIMI BASLIYOR (2028 Blok)...\n");

    for(g_seq = 0; g_seq < TOTAL_BLOCKS; g_seq++) {
      pkt.seq_num = g_seq;
      pkt.len = 64;
      memset(pkt.data, 0xAB, 64); // Örnek veri

      simple_udp_sendto(&udp_conn, &pkt, sizeof(pkt), &dest_ipaddr);
      LOG_INFO("Gonderildi: Blok %u / 2028\n", g_seq);

      
      etimer_set(&timer, CLOCK_SECOND / 2);
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    }
    LOG_INFO("\n==========================================\n");
    LOG_INFO("=== TRANSFER BASARIYLA TAMAMLANDI ===\n");
    LOG_INFO("==========================================\n");
  } else {
    LOG_INFO("Node %u: Sadece iletici modunda.\n", node_id);
  }
  PROCESS_END();
}