#ifndef _GRANIT_PROTOCOLS_H_
#define _GRANIT_PROTOCOLS_H_

#include <stdint.h>

#define CPU_INT64U    uint64_t

//�������� ���� � �������
typedef struct datetime_struct_tag
{
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
} datetime_struct;

//������, ���������� �� NMEA ��� ����
typedef struct
{
  char status[2]; // ���������������� ������ [N]
  char latitude[12]; // ������ [GGMM.MMMM]
  char latitude_i[2]; // �������� ��� ����� [N]
  char longitude[12]; // ������� [GGGMM.MMMM]
  char longitude_i[2]; // �������� ��� ��������� [N]
  char sat_used[3]; // ���������� ������������ ��������� [N[M]]
  char hdop[8]; // �������������� ���������� [N.MM]
  char altitude[8]; // ������ ��� ������� ���� [N.M]
  char course[7]; // ���� � �������� �� ������ [N.MM]
  char speed[8]; // �������� � knots [N.MM]
} gps_data_raw_struct;

/*
Compact GPS data is:

status[1]
latitude[4] //as unsigned long, divide on 10000 to get real value
latitude_i[1]
longitude[4]  //as unsigned long, divide on 10000 to get real value
longitude_i[1]
sat_used[1]
hdop[2] //as unsigned int, divide on 100 to get real value
altitude[4] //as unsigned long, divide on 10 to get real value, has sign
course[2] //as unsigned int, divide on 100 to get real value
speed[4] //as unsigned long, divide on 100 to get real value
*/

//�������� GSM ����
typedef struct
{
  int operator_code;
  int level;
  int sim_number;
} gsm_data_struct;

/*
Compact GSM data is:

operator_code[2]
level[1]
sim_number[1]
*/

//�������� SAT ����
typedef struct
{
  int level;
} sat_data_struct;

/*
Compact SAT data is:

level[1]
*/

//�������� �������� ������
typedef struct
{
  char turnon_flag;
  char ctrl_valve_flag;
  char sens_gear_flag;
  char sens_gear_event;
  char sens_direction_flag;
  char sens_direction_event;
  int sens_temp;
  int sens_press_in;
  int sens_press_out;
  int gen_volts;
  char alert_flag;
  char button1;
  char button2;
  char extra[8];
} granit_data_struct;

typedef struct
{
  CPU_INT64U id;
  char sent;
  datetime_struct date_time;
  gps_data_raw_struct gps_data;
  gsm_data_struct gsm_data;
  sat_data_struct sat_data;
  granit_data_struct granit_data;
} full_point_struct;

#include "granit_protocols_iridium.h"

#endif  //_GRANIT_PROTOCOLS_H_
