/**
 * @author      : alex (alexbora@gmail.com)
 * @file        : test
 * @created     : Luni Sep 04, 2023 18:07:29 EEST
 */

#include <ao/ao.h>
#include <curl/curl.h>
#include <mpg123.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __WIN32
#include <io.h>
#include <stdint.h>
#endif

/* #ifdef __APPLE__ */
/* #include <AudioToolbox/AudioQueue.h> */
/* #include <CoreAudio/CoreAudio.h> */
/* #endif */

#ident "mpg123 test program"

#ifdef DNDEBUG
#define DEBUG 0
#else
#define DEBUG 1
#define GREEN "\033[0;32m"
#define RED "\033[0;31m"
#define RESET "\033[0m"
#endif

#ifdef RELEASE
#define DEBUG 0
#else
#define DEBUG 1
#endif

#ifdef DEBUG
#define debug(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#else
#define debug(fmt, ...) /* Don't do anything in release builds */
#endif

#define BITS 8

size_t jump(char *arr, size_t interval, size_t size, char **icy);

mpg123_handle *mh;
ao_device *dev;
CURL *curl;
mpg123_id3v1 *v1;
mpg123_id3v2 *v2;
volatile unsigned icy_metaint = 0;  // = 16000;

typedef struct {
  int bits;        /* bits per sample */
  int rate;        /* samples per second (in a single channel) */
  int channels;    /* number of audio channels */
  int byte_format; /* Byte ordering in sample, see constants below */
  char *matrix;    /* input channel location/ordering */
} Format;

Format format;

char *icy;

size_t play_stream(void *buffer, size_t size, size_t nmemb, void *userp) {
  int err = MPG123_OK;
  off_t frame_offset = 0;
  unsigned char *audio = NULL;
  size_t done = 0;
  int channels, encoding = channels = 0;
  long rate = 0;

  jump((char *)buffer, 16000, size * nmemb, &icy);
  /* printf("\nICY: %s\n", icy); */

  return size * nmemb;

  mpg123_feed(mh, (const unsigned char *)buffer, size * nmemb);

  do {
    err = mpg123_decode_frame(mh, &frame_offset, &audio, &done);

    fprintf(stderr, "\rframe_offset: %lld %d", frame_offset, icy_metaint);
    fflush(stderr);

    switch (err) {
      case MPG123_NEW_FORMAT:
        mpg123_getformat(mh, &rate, &channels, &encoding);
        format.bits = mpg123_encsize(encoding) * BITS;
        format.rate = rate;
        format.channels = channels;
        format.byte_format = AO_FMT_NATIVE;
        format.matrix = 0;
        dev = ao_open_live(ao_default_driver_id(), (ao_sample_format *)&format,
                           NULL);

        printf("bits: %d rate: %d charnnels: %d byte_format: %d\n", format.bits,
               format.rate, format.channels, format.byte_format);
        break;

      case MPG123_OK:
        if (done > 0) ao_play(dev, (char *)audio, done);
        break;

      case MPG123_NEED_MORE:
        break;

      default:
        break;
    }
  } while (err == MPG123_OK);

  return size * nmemb;
}
void clean(CURL *curl, mpg123_handle *mh, ao_device *dev) {
  curl_easy_cleanup(curl);

  mpg123_close(mh);
  mpg123_delete(mh);
  mpg123_exit();

  ao_close(dev);
  ao_shutdown();
}

int in(const char *arg) {
  int err = MPG123_OK;
  char *url;
  ao_initialize();

#if MPG123_API_VERSION < 46
  mpg123_init();
#endif

  mh = mpg123_new(NULL, &err);

  mpg123_param(mh, MPG123_VERBOSE, 0,
               0);  // Set verbosity level to 2 (maximum verbosity)
  mpg123_param(mh, MPG123_RESYNC_LIMIT, -1, 0);

  mpg123_open_feed(mh);

  curl = curl_easy_init();

  struct curl_slist *header1 = NULL, *header2 = NULL;
  header1 = curl_slist_append(header1, "Icy-MetaData:1");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header1);
  /* header2 = curl_slist_append(header2, "Upgrade: websocket"); */

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, play_stream);

  /* curl_easy_setopt(curl, CURLOPT_URL, */
  /* "s1-webradio.antenne.de/chillout/stream/mp3"); */

  curl_easy_setopt(curl, CURLOPT_URL, arg);

// Set the URL to fetch data from
#ifdef _WIN32
  curl_easy_setopt(curl, CURLOPT_PROXY, "pxgot1-onprem.srv.volvo.com:8080");
  curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, "a049689:SummicronSummilux-50");
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
#endif

  format = (Format){.bits = 16, .rate = 44100, .channels = 2, .byte_format = 4};

#if 0 
  printf("default driver: %d\n", ao_default_driver_id());
  printf("name: %s\n", ao_driver_info(0)->name);
  printf("short name: %s\n", ao_driver_info(0)->short_name);

  while (*ao_driver_info(0)->options++)
    printf("Options: %s\n", *ao_driver_info(0)->options);

  printf("type: %d\n", ao_driver_info(0)->type);
  printf("byte_format: %d\n", ao_driver_info(0)->preferred_byte_format);
  printf("options count: %d\n", ao_driver_info(0)->option_count);

#endif
  dev = ao_open_live(ao_default_driver_id(), (ao_sample_format *)&format, NULL);

  return err;
}

int main(int argc, char *argv[]) {
  in(argv[1]);

  curl_easy_perform(curl);

  clean(curl, mh, dev);

  return 0;
}
