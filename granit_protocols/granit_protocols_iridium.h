#ifndef _GRANIT_PROTOCOLS_IRIDIUM_H_
#define _GRANIT_PROTOCOLS_IRIDIUM_H_

#ifndef bool
  #define bool int
  #define false 0
  #define true 1
  #define __bool_true_false_are_defined 1
#endif

//compacted size
#define MAIN_DATA_LENGTH_IRIDIUM          (15)
#define GPS_DATA_LENGTH_IRIDIUM           (16)
#define SAT_DATA_LENGTH_IRIDIUM           (1)
#define GRANIT_DATA_LENGTH_IRIDIUM        (23)

#define PACKET_LENGTH_IRIDIUM             (MAIN_DATA_LENGTH_IRIDIUM + \
                                           GPS_DATA_LENGTH_IRIDIUM + \
                                           SAT_DATA_LENGTH_IRIDIUM + \
                                           GRANIT_DATA_LENGTH_IRIDIUM)

#define IRIDIUM_SBD_TX_LENGTH_MAX         (340)
#define HEADER_LENGTH_IRIDIUM             (3)
#define IRIDIUM_PACKET_VERSION            (01)

//insert point in compact form for Iridium into buffer and return number of processed bytes
int granit_iridium_compact_point(full_point_struct *point, unsigned char *buffer);

//restore point struct from compacted form for Iridium
//return false if data cannot be restored
bool granit_iridium_restore_point(full_point_struct *point, unsigned char *buffer);

#endif //_GRANIT_PROTOCOLS_IRIDIUM_H_
