//---------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "granit_protocols\granit_protocols.h"
#pragma hdrstop

//---------------------------------------------------------------------------
#define EI 11  /* typically 10..13 */
#define EJ  4  /* typically 4..5 */
#define P   1  /* If match length <= P then output one character */
#define N (1 << EI)  /* buffer size */
#define F ((1 << EJ) + P)  /* lookahead buffer size */

typedef struct my_file
{
  char *data;
  int position;
  int size;
  int max_size;
} my_file_t;

static int bit_buffer = 0, bit_mask = 128;
static unsigned long codecount = 0, textcount = 0;
static unsigned char buffer[N * 2];

static int getbit(my_file_t *in, int n);

static int decode(my_file_t *in, my_file_t *out)
{
  int i, j, k, r, c;
  bit_buffer = 0; bit_mask = 0;

  for (i = 0; i < N - F; i++) buffer[i] = ' ';
  r = N - F;
  while ((c = getbit(in, 1)) >= 0)
  {
    if (c)
    {
      c = getbit(in, 8);
      if (c < 0)
        break;

      if(out->position >= out->size)
        return (-1);

      out->data[out->position++] = c;
      buffer[r++] = c;  r &= (N - 1);
    }
    else
    {
      i = getbit(in, EI);
      j = getbit(in, EJ);
      if (i < 0)
        break;
      if (j < 0)
        break;
      for (k = 0; k <= j + 1; k++)
      {
        c = buffer[(i + k) & (N - 1)];
        if(out->position >= out->size)
          return (-1);

        out->data[out->position++] = c;
        buffer[r++] = c;  r &= (N - 1);
      }
    }
  }
  return 0;
}

// get n bits
static int getbit(my_file_t *in, int n)
{
  int i, x;

  x = 0;
  for (i = 0; i < n; i++)
  {
    if (bit_mask == 0)
    {
      if(in->position >= in->size) return (-1);
      bit_buffer = in->data[in->position++];
      bit_mask = 128;
    }
    x <<= 1;
    if (bit_buffer & bit_mask) x++;
    bit_mask >>= 1;
  }
  return x;
}

static my_file_t input;
static my_file_t output;

char input_data[500];
char output_data[3000];

char json_buffer[50000];
char point_buffer[1000];

full_point_struct points[50];
int points_count = 0;

//Длина заголовка Iridium
#define HEADER_LENGTH_IRIDIUM             (3)

static void longlongtoa(CPU_INT64U from, char *where);
static void granit_point_to_json(full_point_struct *point, char *json_buffer);

#pragma argsused
int main(int argc, char* argv[])
{
  char *input_file;
  char *output_file;
  FILE *h = NULL;
  FILE *out = NULL;
  int i = 0;
  int k = 0;
  int protocol = 0;
  int length = 0;
  int result = 0;

  if(argc < 3)
  {
    fprintf(stderr, "Usage: decodir input_file output_file\r\n");
    return 1;
  }

  input_file = argv[1];
  output_file = argv[2];

  h = fopen(input_file, "rb");

  if(h == NULL)
  {
    fprintf(stderr, "Input file read failed\r\n");
    return 2;
  }

  input.data = input_data;
  input.max_size = 500;

  i = 0;
  while(1)
  {
    int a = fgetc(h);
    if(a == EOF)
      break;

    input.data[i++] = (char) a;
  }

  fclose(h);
  h = NULL;

  input.size = i;

  if(input.size < HEADER_LENGTH_IRIDIUM)
  {
    fprintf(stderr, "File size too small\r\n");
    return 3;
  }

  protocol = input.data[0];
  length = (input.data[1] << 0) + (input.data[2] << 8);

  if(protocol != 0x01)
  {
    fprintf(stderr, "Protocol not supported [%02X]\r\n", protocol);
    return 3;
  }

  if((length + HEADER_LENGTH_IRIDIUM) < input.size)
  {
    input.size = length + HEADER_LENGTH_IRIDIUM;
  }

  output.data = output_data;
  output.position = 0;
  output.size = 3000;
  output.max_size = 3000;

  input.position = HEADER_LENGTH_IRIDIUM;

  result = decode(&input, &output);

  if(result < 0)
  {
    fprintf(stderr, "Decode failed\r\n");
    return 3;
  }

  points_count = output.position / PACKET_LENGTH_IRIDIUM;

  k = 0;
  for(i = 0; i < points_count; i++)
  {
    bool point_result = granit_iridium_restore_point(&points[k], output.data + (i * PACKET_LENGTH_IRIDIUM));

    if(point_result)
    {
      k++;
    }
    else
    {
      fprintf(stderr, "Point restore failed at %d\r\n", i);
      continue;
    }
  }

  length = 0;
  memset(json_buffer, 0, 50000);
  strcat(json_buffer, "[");
  for(i = 0; i < k; i++)
  {
    granit_point_to_json(&points[i], point_buffer);
    strcat(json_buffer, point_buffer);

    if((i + 1) < k)
    {
      strcat(json_buffer, ",\r\n");
    }
  }
  strcat(json_buffer, "]");
  length = strlen(json_buffer);

  //now inflate to json data

  out = fopen(output_file, "w+");

  if(out == NULL)
  {
    fprintf(stderr, "Output file write failed\r\n");
    return 2;
  }

  i = 0;
  while(1)
  {
    if(i >= length)
      break;
    fputc(json_buffer[i++], out);
  }

  fclose(out);
  out = NULL;

  fprintf(stdout, "decodir %s %s: %d bytes written\r\n", input_file, output_file, i);
  return 0;
}

// Reverse a string in place.
static void strReverse(char *str)
{
  int i, j;
  char t;
  int n = strlen(str);
  
  for(i = 0, j = n-1; i < j; ++i, --j) {
    t = str[i];
    str[i] = str[j];
    str[j] = t;
  }
}

//64bit unsigned integer to string conversion
static void longlongtoa(CPU_INT64U from, char *where)
{
  int i = 0;
  int digit = 0;
  
  while(1)
  {
    digit = from % 10;
    from = from / 10;
  
    where[i++] = digit + '0';
    
    if(from == 0)
      break;
  }
  
  where[i] = 0;
  
  strReverse(where);
}

//Convert point to JSON
static void granit_point_to_json(full_point_struct *point, char *json_buffer)
{
  char imei[18];
  char id_string[28];

  longlongtoa(point->id, id_string);
  
  sprintf(json_buffer, "{\"id\":%s,\"datetime\":\"%04d.%02d.%02d %02d:%02d:%02d\",\
\"gps_status\":\"%s\",\"gps_lat\":\"%s\",\"gps_lat_i\":\"%s\",\"gps_lon\":\"%s\",\
\"gps_lon_i\":\"%s\",\"gps_sat\":\"%s\",\"gps_alt\":\"%s\",\
\"sat_level\":%d,\
\"turnon\":%d,\"sens_gear\":%d,\"sens_direction\":%d,\"sens_temp\":%d,\"sens_press_in\":%d,\
\"sens_press_out\":%d,\"gen_volts\":%d,\"alert\":%d}",
          id_string,
          point->date_time.year, point->date_time.month, point->date_time.day,
          point->date_time.hour, point->date_time.minute, point->date_time.second,
          point->gps_data.status, point->gps_data.latitude, point->gps_data.latitude_i,
          point->gps_data.longitude, point->gps_data.longitude_i, point->gps_data.sat_used,
          point->gps_data.altitude,
          point->sat_data.level,
          point->granit_data.turnon_flag,
          point->granit_data.sens_gear_flag, point->granit_data.sens_direction_flag,
          point->granit_data.sens_temp,
          point->granit_data.sens_press_in, point->granit_data.sens_press_out,
          point->granit_data.gen_volts,
          point->granit_data.alert_flag);
}
//---------------------------------------------------------------------------
