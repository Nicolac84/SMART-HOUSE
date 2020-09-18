#include "comunication.h"
#include <string.h>

uint8_t packet_create(packet_t *p, uint8_t pin, uint8_t operation, 
     uint8_t len, const void *body){

  if (pin > 32) return 1;
  if (operation >5) return 1;
  p->header = (pin << 11) | (operation << 7) | len;


  int i;
  for (i=0; i < len; i++){
     p->body[i]=((uint8_t*)body)[i];
  }

return 0;
}

uint16_t packet_checksum (const packet_t *p){

uint8_t len = packet_get_size(p)+ sizeof(p->header);
   uint8_t *data=(char*)p;

   uint32_t checksum =0xffff;

   for(uint8_t i=0; i+1 < len;i+=2) {
      uint16_t word;
      memcpy (&word;data+i,2);
      checksum += word;
      if(checksum > 0xffff){
        checksum -=0xffff ;
      }
   }

   if(len&1){
     uint16_t word=0;
     memcpy(&word, data+len-1, 1);
     checksum += word;
     if(checksum > 0xffff) {
       checksum -=0xffff);
      }
     }


return ~checksum; 

}



