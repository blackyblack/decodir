#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "granit_protocols.h"
#include "granit_protocols_iridium.h"

//-------------------------------------------- defines

#define PACKET_LENGTH               (PACKET_LENGTH_IRIDIUM)

//-------------------------------------------- data

//-------------------------------------------- prototypes

static int gps_restore_data(unsigned char *from, gps_data_raw_struct *to);
static int sat_restore_data(unsigned char *from, sat_data_struct *to);
static int granit_restore_data(unsigned char *from, granit_data_struct *to);

//-------------------------------------------- modules

//restore point struct from compacted form for Iridium
//return false if data cannot be restored
bool granit_iridium_restore_point(full_point_struct *point, unsigned char *buffer)
{
  int num = 0;
  memset(point, 0, sizeof(full_point_struct));

  point->id = 0;
  point->id |= ((CPU_INT64U)buffer[num++]) << (0);
  point->id |= ((CPU_INT64U)buffer[num++]) << (8);
  point->id |= ((CPU_INT64U)buffer[num++]) << (16);
  point->id |= ((CPU_INT64U)buffer[num++]) << (24);
  point->id |= ((CPU_INT64U)buffer[num++]) << (32);
  point->id |= ((CPU_INT64U)buffer[num++]) << (40);
  point->id |= ((CPU_INT64U)buffer[num++]) << (48);
  point->id |= ((CPU_INT64U)buffer[num++]) << (56);

  point->date_time.year = 0;
  point->date_time.year |= ((unsigned int)buffer[num++]) << (0);
  point->date_time.year |= ((unsigned int)buffer[num++]) << (8);
  point->date_time.month = buffer[num++];
  point->date_time.day = buffer[num++];
  point->date_time.hour = buffer[num++];
  point->date_time.minute = buffer[num++];
  point->date_time.second = buffer[num++];
    
  //GPS data
  num += gps_restore_data((char *)buffer + num, &point->gps_data);
  //SAT (Iridium)
  num += sat_restore_data((char *)buffer + num, &point->sat_data);
  //Granit data
  num += granit_restore_data((char *)buffer + num, &point->granit_data);
    
  return true;
}

//utility conversion from digital values with dividers to long integer
static long gps_atol(char *from)
{
  int i, j;
  char tmp[17];
  bool minus = false;
  unsigned long value = 0;

  if(from == NULL)
    return 0;

  if(from[0] == '-')
   minus = true;
  
  j = 0;
  for(i = 0; i < 16; i++)
  {
    if(from[i] == 0)
      break;
    
    if(isdigit(from[i]))
      tmp[j++] = from[i];
  }
  
  tmp[j] = 0;
  
  value = strtoul(tmp, NULL, 10);

  if(minus)
    return value * (-1);
      
  return value;
}

//utility conversion from digital values with dividers to unsigned long integer
static unsigned long gps_atoul(unsigned char *from)
{
  int i, j;
  char tmp[17];
  unsigned long value = 0;

  if(from == NULL)
    return 0;
  
  j = 0;
  for(i = 0; i < 16; i++)
  {
    if(from[i] == 0)
      break;
    
    if(isdigit(from[i]))
      tmp[j++] = from[i];
  }
  
  tmp[j] = 0;
  
  value = strtoul(tmp, NULL, 10);

  return value;
}

//insert string 'insert' into 'from' string at position 'position'
//and copy result into 'where' buffer
static char *gps_strinsert(char *from, char *where, char *insert, int position)
{
  int length = strlen(from);
  
  where[0] = 0;
  strncat(where, from, position);
  strcat(where, insert);
  strncat(where, from + position, length - position);
  
  return where;
}

//convert compact format to GPS data (to read from flash)
//return read bytes
static int gps_restore_data(unsigned char *from, gps_data_raw_struct *to)
{
  int num = 0;
  unsigned long data;
  long signed_data;
  char temp[32];

  //'A' or 'V'
  to->status[0] = from[num++];
  to->status[1] = 0;
  
  //read latitude
  data = 0;
  data |= ((unsigned long)from[num++]) << (0);
  data |= ((unsigned long)from[num++]) << (8);
  data |= ((unsigned long)from[num++]) << (16);
  data |= ((unsigned long)from[num++]) << (24);
  
  //convert latitude value to string
  sprintf(temp, "%08lu", data);
  gps_strinsert(temp, to->latitude, ".", strlen(temp) - 4);
  //terminate for safety
  to->latitude[11] = 0;
  
  to->latitude_i[0] = from[num++];
  to->latitude_i[1] = 0;
  
  //read longitude
  data = 0;
  data |= ((unsigned long)from[num++]) << (0);
  data |= ((unsigned long)from[num++]) << (8);
  data |= ((unsigned long)from[num++]) << (16);
  data |= ((unsigned long)from[num++]) << (24);
  
  //convert longitude value to string
  sprintf(temp, "%09lu", data);
  gps_strinsert(temp, to->longitude, ".", strlen(temp) - 4);
  to->longitude[11] = 0;
  
  to->longitude_i[0] = from[num++];
  to->longitude_i[1] = 0;
  
  //write sattelites
  data = from[num++];
  
  sprintf(temp, "%lu", data);
  strncpy(to->sat_used, temp, 2);
  to->sat_used[2] = 0;
  
  //read altitude
  //altitude has sign
  signed_data = 0;
  signed_data |= ((unsigned long)from[num++]) << (0);
  signed_data |= ((unsigned long)from[num++]) << (8);
  signed_data |= ((unsigned long)from[num++]) << (16);
  signed_data |= ((unsigned long)from[num++]) << (24);
  
  //convert altitude value to string
  sprintf(temp, "%02ld", signed_data);
  gps_strinsert(temp, to->altitude, ".", strlen(temp) - 1);
  to->altitude[7] = 0;
  
  return num;
}

//convert compact format to SAT data (to read from flash)
//return read bytes
static int sat_restore_data(unsigned char *from, sat_data_struct *to)
{
  int num = 0;

  //read sat level
  to->level = from[num++];
  
  return num;
}

//convert compact format to Granit data (to read from flash)
//return read bytes
static int granit_restore_data(unsigned char *from, granit_data_struct *to)
{
  int num = 0;
  unsigned long data;
  int i;

  to->turnon_flag = from[num++];
  to->ctrl_valve_flag = from[num++];
  to->sens_gear_flag = from[num++];
  to->sens_direction_flag = from[num++];
  
  //read sens_temp
  data = 0;
  data |= ((unsigned long)from[num++]) << (0);
  data |= ((unsigned long)from[num++]) << (8);
  
  to->sens_temp = data;
  
  //read sens_press_in
  data = 0;
  data |= ((unsigned long)from[num++]) << (0);
  data |= ((unsigned long)from[num++]) << (8);
  
  to->sens_press_in = data;
  
  //read sens_press_out
  data = 0;
  data |= ((unsigned long)from[num++]) << (0);
  data |= ((unsigned long)from[num++]) << (8);
  
  to->sens_press_out = data;
  
  //read gen_volts
  data = 0;
  data |= ((unsigned long)from[num++]) << (0);
  data |= ((unsigned long)from[num++]) << (8);
  
  to->gen_volts = data;
  
  to->alert_flag = from[num++];
  
  //write extra buttons
  to->button1 = from[num++];
  to->button2 = from[num++];
  
  for(i = 0; i < 8; i++)
  {
    to->extra[i] = from[num++];
  }
  
  return num;
}
