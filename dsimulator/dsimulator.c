#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define PROTOCOL_VERSION 0x0
#define MAX_PAYLOAD_SIZE 64

struct Rfc1006Header {
  uint8_t version;
  uint8_t reserved;
  uint16_t length;
} __attribute__((packed));

struct PayloadHeader {
  uint16_t deviceId;
  uint16_t measurementTag;
  uint32_t timestamp;
  uint16_t measurementType;
  uint16_t dataLength;
} __attribute__((packed));

struct config {
  uint16_t deviceId;
  uint16_t port;
  in_addr_t serverIp;
  uint8_t intensity;
};

ssize_t sendPacket(int sockFd, uint16_t devId) {
#define PREFIX_SIZE \
  (sizeof(struct Rfc1006Header) + sizeof(struct PayloadHeader))

  static char newPacket[PREFIX_SIZE + MAX_PAYLOAD_SIZE];
  const size_t payloadSize = rand() % MAX_PAYLOAD_SIZE;
  const size_t packetSize = PREFIX_SIZE + payloadSize;

  struct Rfc1006Header rfcHeader;
  struct PayloadHeader payloadHeader;

  rfcHeader.version = PROTOCOL_VERSION;
  rfcHeader.length = sizeof(struct PayloadHeader) + payloadSize;
  payloadHeader.deviceId = devId;
  payloadHeader.timestamp = time(NULL);
  payloadHeader.dataLength = payloadSize;

  memcpy(newPacket, &rfcHeader, sizeof(struct Rfc1006Header));
  memcpy(newPacket + sizeof(struct Rfc1006Header), &payloadHeader,
         sizeof(struct PayloadHeader));

  ssize_t writeResult = write(sockFd, newPacket, packetSize);
  bzero(newPacket, sizeof(newPacket));
  return writeResult;
}

void startSending(int sockFd, uint16_t devId, uint8_t intensity) {
  srand(time(NULL));
  while (1) {
    if (sendPacket(sockFd, devId) > 0)
      usleep((rand() % 10000) * 100 / intensity);
    else {
      fprintf(stderr, "Error: %s\n", strerror(errno));
      exit(errno);
    }
  }
}

int parseConfig(int argc, char **argv, struct config *conf) {
#define REQUIRED_ARGS_COUNT 4

  if (argc < REQUIRED_ARGS_COUNT + 1) {
    fprintf(stderr, "Not all required arguments are present! \n");
    fprintf(stderr,
            "Usage: ./simulator <server_ip> <server_port> <device_id> "
            "<intensity_multiplier> \n");
    return 2;
  }

  conf->deviceId = atoi(argv[3]);
  conf->port = atoi(argv[2]);
  if (conf->port == 0) {
    fprintf(stderr, "Wrong port number! \n");
    return 2;
  }
  char *serverIp = argv[1];
  conf->serverIp = inet_addr(serverIp);
  if (conf->serverIp == INADDR_NONE) {
    fprintf(stderr, "Wrong server IP address: %s \n", serverIp);
    return 2;
  }

  conf->intensity = atoi(argv[4]);
  if (conf->intensity == 0 || conf->intensity > 100) {
    fprintf(stderr, "Wrong intensity multiplier, must be [1..100]! \n");
    return 2;
  }
  return 0;
}

int main(int argc, char **argv) {
  struct config conf;
  int confErr = parseConfig(argc, argv, &conf);
  if (confErr) {
    return confErr;
  }
  int sockFd;
  struct sockaddr_in servaddr;

  sockFd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFd == -1) {
    printf("Failed to create socket!\n");
    exit(1);
  }
  bzero(&servaddr, sizeof(servaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = conf.serverIp;
  servaddr.sin_port = htons(conf.port);

  if (connect(sockFd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) {
    printf("Failed to connect to the server %s \n", argv[1]);
    return 1;
  } else
    printf("Connected to the server, starting sending, deviceId is %d...\n",
           conf.deviceId);

  signal(SIGPIPE, SIG_IGN);
  startSending(sockFd, conf.deviceId, conf.intensity);
  return 0;
}
