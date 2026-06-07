#include "contiki.h"
#include "net/ipv6/simple-udp.h"
#include "os/storage/cfs/cfs.h"
#include "sys/log.h"
#include "net/routing/routing.h"

#define LOG_MODULE "OTA-Server"
#define LOG_LEVEL  LOG_LEVEL_INFO
#define UDP_PORT   8765

struct ota_packet {
  uint16_t seq_num;
  uint16_t len;
  uint8_t  data[64];
};

static struct simple_udp_connection udp_conn;

static void
udp_rx_callback(struct simple_udp_connection *c, const uip_ipaddr_t *sender_addr,
                uint16_t sender_port, const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port, const uint8_t *data, uint16_t datalen)
{
  struct ota_packet *pkt = (struct ota_packet *)data;
  
  
  if(pkt->seq_num < 1000) {
    int fd = cfs_open("received_fw.bin", CFS_WRITE | CFS_APPEND);
    if(fd >= 0) {
      cfs_write(fd, pkt->data, pkt->len);
      cfs_close(fd);
    }
  }
  
  LOG_INFO("ALICI: Paket %u alindi ve kaydedildi.\n", pkt->seq_num);

  
  if(pkt->seq_num == 2027) {
    LOG_INFO("\n************************************************\n");
    LOG_INFO("Yuklenmeye hazir yeni firmware alimi tamamlandi.\n");
    LOG_INFO("************************************************\n");
  }
}

PROCESS(udp_server_process, "OTA Receiver");
AUTOSTART_PROCESSES(&udp_server_process);

PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();
  NETSTACK_ROUTING.root_start(); // Node 1 agin merkezi
  cfs_remove("received_fw.bin"); // Temizlik
  simple_udp_register(&udp_conn, UDP_PORT, NULL, UDP_PORT, udp_rx_callback);
  LOG_INFO("ALICI: Node 1 hazir, paket bekleniyor...\n");
  while(1) { PROCESS_WAIT_EVENT(); }
  PROCESS_END();
}