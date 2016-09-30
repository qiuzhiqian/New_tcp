#ifndef __EXCH_H
#define __EXCH_H

int strtoint(unsigned char *str);
unsigned char asctohex(unsigned char asc);
void clearbuff(unsigned char *buff,unsigned short len);
unsigned short checksum(unsigned char *buff,unsigned short len);
#endif
