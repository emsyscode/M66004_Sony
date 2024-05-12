
/*
 * This code is not PERFECT and CLEAN, must be looked as a sample
 * wich allow find ideas to use the driver of VFD M66004SP
 * I try let functions as simple possible and without any library
 * or jump to others codes wich don't let use it.
 * Each funtion must be looked as independent block and this
 * make it possible implement it on any solution.
 */
 /*
  *                                     D7  D6  D5   D4  D3  D2  D1  D0
  *Display digit length setting         0   0   0    0   0   *   *   *      -The number of digits to display is fixed. Eight types of setting (from 9 digits to 16 digits) are possible.
  *Dimmer value setting                 0   0   0    0   1   *   *   *      -Eight types of setting (from 1/16 to 14/16) are possible for dimmer value.
  *One-digit display frequency setting  1   1   1    1   0   1   1   *      -Either 128/fOSC or 256/fOSC is selected as onedigit display frequency.
  *Display digit setting                1   1   1    0   *   *   *   *      -The first character code received after executing this command is displayed as designated by this command.
  *Auto increment setting               1   1   1    1   0   1   0   *      -This command is executed to set or cancel the automatic display digit increment function.
  *Cursor ON                            0   0   0    1   *   *   *   *      -This command is executed to make SEG35 ON.
  *Cursor OFF                           1   0   0    0   *   *   *   *      -This command is executed to make SEG35 OFF.
  *All digit display ON/OFF             1   1   1    1   0   0   *   *      -This command is executed to make all-digit display OFF or all-digit/segment display ON.
  *Write to user RAM                    1   1   1    1   1   1   ×   ×      -Character data is written into RAM for user definition.
  *Output port state setting            1   1   1    1   1   0   *   *      -Output ports P0 and P1 are set or reset.
  */

#include <stdio.h>
#include <string.h>

#define VFD_clk     (8)  /* serial clock for both M66004 and '165 */
#define VFD_data    (7)  /* serial data to M66004 */
#define VFD_cs      (9)  /* chip select M66004 */
#define VFD_reset   (10)

void vfd_init(void){
  /* Setup pins */ 
  /* Init the display board */
  digitalWrite(VFD_clk, HIGH);
  digitalWrite(VFD_data, LOW);
  digitalWrite(VFD_cs, HIGH);
  /* set 16 chars length of display */
  send_byte(0x00 | 0x06);  //15 grids, I define it. 
  // of the port 0 and port 1, with pipe work ok. Check it in time.
  delay(4);
  /* Enable auto increment (should never be turned off) */
  send_byte(0xF4 | 0x01); // active autoincrement
  delay(4);
  /* Set dimmer value + freq (128/fOSC) */
  send_byte(0x08 | 0x07);  // dimmer maxi
  delay(4);
  send_byte(0xF0 | 0x06); // Constant display freq
  delay(4);
  send_byte(0xE0); //Position grid to write (have 0xE0)
  delay(1); 
  send_byte(0xF0 | 0x05);// 0xF5 to setting autoincrement, no autoincrement 0xF4
  delay(1);
  send_byte(0xF0 | 0x01);// Normal operation or all on or all off
  //
  write_ram();
  delay(4);
}
/*-------------------send command with the CS signal------------------------*/
static void send_byte(uint8_t b){
  uint8_t i;
  uint8_t bit = 0x80;
  // start condition
  digitalWrite(VFD_cs, LOW);
  // MSB first, shift on SCK lo->hi transition
  //for (i = 0; i < 8; i += 1) {
     for (i = 0; i < 8; i ++) {
    digitalWrite(VFD_clk, LOW);
    if(b & bit) {
      digitalWrite(VFD_data, HIGH);
    } else {
      digitalWrite(VFD_data, LOW);
    }
    digitalWrite(VFD_clk, HIGH);
    bit = bit >> 1;             // shift so we check the next one in order
  }
  // end condition
  digitalWrite(VFD_cs, HIGH);
  delay(1);
}

/*-----------------------send command without CS-----------------------------*/
static void send_byte_without_CS(uint8_t b){
  uint8_t i;
  uint8_t bit = 0x80;
  // MSB first, shift on VFD_clk lo->hi transition
  //for (i = 0; i < 8; i += 1) {
     for (i = 0; i < 8; i ++) {
    digitalWrite(VFD_clk, LOW);
    delayMicroseconds(1);
    if(b & bit) {
      digitalWrite(VFD_data, HIGH);
      delayMicroseconds(1);
    } else {
      digitalWrite(VFD_data, LOW);
      delayMicroseconds(1);
    }
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(1);
    bit = bit >> 1;             // shift so we check the next one in order
  }
  delay(1);
}

/*****************************************************************************************/
void strRevert(char *string){
   // Invert the contents of pointer of string and let it reverted until the next call of
   // function! exp: char letter[16] = "jogos.....tarde"; To do a declaration of pointer:  char* ptr=letter;
   // don't forget the null char "\0", this is a 1 more char presente on the string.
   
   int len = 0;
   while (string[len])
   len++;

   int down = 0;
   int up = len - 1;  //Because the string have the "\0" allways as marker of end string!

         while (up > down)
         {
           char c = string[down];
           string[down++] = string[up];
           string[up--] = c;
         }
 }

static void send_arrayVector(uint8_t *bv, uint8_t len){
  uint8_t i, j, b;
  uint8_t ln = 0x00;
  byte data[16];
  byte tmp[16];
// start condition


strcpy(data, bv);
//strRevert(bv);

ln = strlen(data);
Serial.println(ln, DEC);

for (uint8_t s = 0; s < ln; s++){
  Serial.print(char (data[s]));
}
Serial.println();
  
send_byte(0xF0 | 0x05);// 0xF5 to setting autoincrement, no autoincrement 0xF4
delay(1);
send_byte(0xF0 | 0x01);// Normal operation or all on or all off
delay(1);
send_byte(0xE0); //Position grid to write (have 0xE0)
  
  digitalWrite(VFD_cs, LOW);
  // transmit each byte
  for (j = 0; j < len; j+=1) {
    uint8_t bit = 0x80;
    /* load next byte to transmit */
    b = *(bv + j);
    /* MSB first, shift on SCK lo->hi transition */
    for (i = 0; i < 8; i += 1) {
      digitalWrite(VFD_clk, LOW);
      if(b & bit) {
        digitalWrite(VFD_data, HIGH);
      } else {
        digitalWrite(VFD_data, LOW);
      }
      digitalWrite(VFD_clk, HIGH);
      bit = bit >> 1;             /* shift so we check the next one in order */
    }
    /* small delay here, tBUSY = 6 cycles at 500 kHz =~ 14 µs == 14 cycles at 1 MHz. */
  }
  // end condition
  digitalWrite(VFD_cs, HIGH);
}
/*******************************************************************************************/
void write_ram(){
digitalWrite(VFD_cs, LOW);
delay(1);
send_byte_without_CS(0xFC);// Display digit is set to the  first Digits(first digit is on the left side to this display)
delay(1);
send_byte_without_CS(0x00 ); // Position o RAM address to write
delay(1);
send_byte_without_CS(0b00000000 ); // 1th column of grup 7*5  // the bit corresponding to 0 of byte is not matter the value.
delay(1);
send_byte_without_CS(0b00000000 ); // 2th column of grup 7*5
delay(1);
send_byte_without_CS(0b00000000 ); // 3th column of grup 7*5
delay(1);
send_byte_without_CS(0b00000000 ); // 4th column of grup 7*5
delay(1);
send_byte_without_CS(0b00000000 ); // 5th column of grup 7*5
delay(1);
digitalWrite(VFD_cs, HIGH);
delay(1);
}
/*********************************************************************************/
void fill_grid_11_ram(){
digitalWrite(VFD_cs, LOW);
delay(1);
send_byte_without_CS(0xFC);// Display digit is set to the  first Digits(first digit is on the left side to this display)
delay(1);
send_byte_without_CS(0x00 ); // Position o RAM address to write
delay(1);
for(int n=0; n<5; n++){
  send_byte_without_CS(0x3E); // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
}
digitalWrite(VFD_cs, HIGH);
delay(1);
}
/********************************************************************************/
void grid_11_fill_RAM(){
  uint8_t RAM=0x00;
  uint8_t data=0x00;
  uint8_t column=0;
  // Here I fill the matrix of 5*7 and use the total of 16 position of RAM customer define.
  // you can refill the remaning symbols until from 16 to 24 symbols by active mode.
      for( RAM=0; RAM < 16; RAM ++){
      digitalWrite(VFD_cs, LOW);
      delayMicroseconds(1);
                    send_byte_without_CS(0xFC);// Write to RAM chars of customer.
                    delayMicroseconds(1);
                    send_byte_without_CS(0x00 | RAM ); // Position o RAM address to write
                    delayMicroseconds(1);
 
                  switch (RAM){
                  case 0x00:
                            send_byte_without_CS(0x02);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00); 
                            send_byte_without_CS(0x00);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x01:
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x02);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x02:  
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x02);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x03:  
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x02);
                            send_byte_without_CS(0x00);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x04:  
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x02);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x05:
                            send_byte_without_CS(0x04);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00); 
                            send_byte_without_CS(0x00);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x06:
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x04);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x07:  
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x04);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x08:  
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x04);
                            send_byte_without_CS(0x00);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x09:  
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x04);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x0A:
                            send_byte_without_CS(0x08);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00); 
                            send_byte_without_CS(0x00);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x0B:
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x08);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x0C:  
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x08);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x0D:  
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x08);
                            send_byte_without_CS(0x00);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x0E:  
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x08);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                  case 0x0F:  
                            send_byte_without_CS(0x10);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                            send_byte_without_CS(0x00);
                  break; // the bit 7, 6 & 0, don't matter. Don't have symbols assigned.
                   }
      digitalWrite(VFD_cs, HIGH);
      delayMicroseconds(10);
    } 
} 
/***************************************************************************************/
void test(){
  unsigned long bit32 = 0x00000001;  // pay attention the int is 2 bytes in arduino uno, but is 4 bytes in arduino Duo! Check and take care with this!
  unsigned long var = 0x00;
  unsigned long bites = 0x00000001;
  unsigned long mask;
  uint8_t shift = 0x00;
  char word0, word1, word2, word3, word4, word5;
  bool flag = true;
  uint8_t data = 0x00;

unsigned int x = 5;        // binary: 0000000000000101
unsigned int y = 14;
unsigned int result = x << y;  // binary: 0100000000000000 - the first 1 in 101 was discarded

unsigned short MyShort = 0xEFAB;
unsigned char char1; // lower byte
unsigned char char2; // upper byte
// Split short into two char
char1 = MyShort & 0xFF;
char2 = MyShort >> 8;
// merge two char into short
MyShort = (char2 << 8) | char1;
Serial.print ("char1 : ");Serial.print (char1, HEX);Serial.print (", char2 : ");Serial.print (char2, HEX);
Serial.println();
Serial.print( x , BIN);
Serial.print (" : " );
Serial.println(result, BIN);


uint8_t intSize = sizeof(int);
uint8_t longSize = sizeof(long);
uint8_t shortSize = sizeof(short);
uint8_t byteSize = sizeof(byte);
uint8_t doubleSize = sizeof(double);

Serial.print("int : ");Serial.print(intSize, HEX);
Serial.print(", long : ");Serial.print(longSize, HEX);
Serial.print(", short : ");Serial.print(shortSize, HEX);
Serial.print(", byte : ");Serial.print(byteSize, HEX);
Serial.print(", double : ");Serial.print(doubleSize,  HEX);
Serial.println();
//for (mask=0x00000001; mask > 0; mask <<= 1) { //iterate through bit mask
  for (shift =0; shift < 32; shift ++){
    bites=((bit32 << shift) & 0xFFFFFFFF);
            word0=((bites) & 0xFF);
            word1=(bites >> 8);
            word2=(bites >> 16);
            word3=(bites >> 24);
            Serial.print("Var : ");
            Serial.print(var, BIN);
            Serial.print(", Bites : ");
            Serial.println(bites, BIN); 
            //debug
            Serial.println(bites, HEX);
            Serial.print(word0, HEX); 
            Serial.print( " , ");
            Serial.print(word1, HEX); 
            Serial.print( " , ");
            Serial.print(word2, HEX); 
            Serial.print( " , ");
            Serial.print(word3, HEX); 
            Serial.print( " , ");
            Serial.print(word4, HEX); 
            Serial.print(" ! ");
            Serial.print(shift); 
            Serial.println(" $ ");
  }
}
/***********************************************************************************/
void grid_11_tst(){
  uint8_t RAM =0x00;
  unsigned long bit32 = 0x00000001;  // pay attention the int is 2 bytes in arduino uno, but is 4 bytes in arduino Duo! Check and take care with this!
  unsigned long bites= 0x00;
  unsigned long var =0x00;
  uint8_t shift = 0x00;
  uint8_t word0, word1, word2, word3, word4;
  bool flag = true;
  uint8_t data = 0x00;
  
 for( RAM=0; RAM < 16; RAM ++){
      digitalWrite(VFD_cs, LOW);
      delayMicroseconds(1);
          for (shift =0; shift < 32; shift ++){
            bites=((bit32 << shift) & 0xFFFFFFFF);
            word0=((bites) & 0xFF);
            word1=(bites >> 8);
            word2=(bites >> 16);
            word3=(bites >> 24);
          
                    send_byte_without_CS(0xFC);// Write to RAM chars of customer.
                    delayMicroseconds(1);
                    send_byte_without_CS(0x00 | RAM ); // Position o RAM address to write
                    delayMicroseconds(1);
                    send_byte_without_CS(word0);
                    send_byte_without_CS(word1);
                    send_byte_without_CS(word2);
                    send_byte_without_CS(word3);
                    send_byte_without_CS(0x00); // The last bits don't have symbols assigned.
          }
      digitalWrite(VFD_cs, HIGH);
      delayMicroseconds(1);
 } 
}
/*************************************************************************************/
void viewing_pixel(){
  unsigned int RAM=0;
  uint8_t tmp =0x00;
  uint8_t data=0;
  bool flag = true;
  send_arrayVector("              ", 14); // send space to all acitvated digits to clear VFD
  send_byte(0xE0);
  delayMicroseconds(5);
send_byte(0xF0 | 0x04); // Set to autoincrement
delayMicroseconds(10);
// I only see 14 symbols, but the grid have 24. All position 0 of tabel of symbol are empty, also the 5º of first byte.
    for( RAM=0; RAM < 16; RAM++){
          digitalWrite(VFD_cs, LOW);
          delayMicroseconds(10);
          send_byte_without_CS(0xE0); // Here I fix it to stay on grid eleven to see all symbols of this grid.
          send_byte_without_CS(0x90 + RAM); // Display the contents of 16 position of customer RAM
          digitalWrite(VFD_cs, HIGH);
          delay(500);
    } 
}
/*************************************************************************************/
void viewing(){
  unsigned int RAM=0;
  uint8_t tmp =0x00;
  uint8_t data=0;
  bool flag = true;
  send_arrayVector("              ", 14); // send space to all acitvated digits to clear VFD
  send_byte(0xE0);
  delayMicroseconds(5);
  for( RAM=0; RAM < 14; RAM++){
      digitalWrite(VFD_cs, LOW);
      delayMicroseconds(5);
                if (RAM > 0x14){  // 0x14 is grid number 14, on the VFD of 15 grids
                    if (flag== true){
                      flag = false;
                      //Serial.print(" Flag: ");Serial.print (flag);Serial.print (", RAM: ");Serial.println(RAM);
                       send_byte_without_CS(0xE0);
                delayMicroseconds(50);
                     }
                }
          send_byte_without_CS((0x41 + RAM));
          delayMicroseconds(5);
      digitalWrite(VFD_cs, HIGH);
      delay(250);  
    }
    delay(500);
send_byte(0xF0 | 0x05); // Set to autoincrement
delayMicroseconds(10);
// I only see 14 symbols, but the grid have 24. All position 0 of tabel of symbol are empty, also the 5º of first byte.
    for( RAM=0; RAM < 14; RAM++){
          digitalWrite(VFD_cs, LOW);
          delayMicroseconds(10);
          send_byte_without_CS(0xEA); // Here I fix it to stay on grid eleven to see all symbols of this grid.
          send_byte_without_CS(0x90 + RAM); // Display the contents of 16 position of customer RAM
          digitalWrite(VFD_cs, HIGH);
          delay(250);
    } 
}
/***********************************************************************/
void clean(){
send_byte(0xE0);
delayMicroseconds(5);
send_arrayVector("              ", 14); // send space to all acitvated digits to clear VFD
delayMicroseconds(5);
send_byte(0xE0);
delayMicroseconds(5);
}
/***********************************************************************/
void ports(){
uint8_t strLength = 0x00; // Used to get length of string.
byte arrStr[16];
//
strcpy(arrStr, "  ON/OFF 0&1  ");
strLength = strlen(arrStr);
strRevert(arrStr);
send_arrayVector(arrStr,strLength);
delay(1000);
clean();

strcpy(arrStr, "LED ON Port 0 ");
strLength = strlen(arrStr);
strRevert(arrStr);
send_arrayVector(arrStr,strLength);
send_byte(0xF8 | 0x01); // active port 0
delay(1000);
clean();

strcpy(arrStr, "LED ON Port 1 ");
strLength = strlen(arrStr);
strRevert(arrStr);
send_arrayVector(arrStr,strLength);
send_byte(0xF8 | 0x02); // active port 1 
delay(1000);
clean();

strcpy(arrStr, "  Both 0 & 1  ");
strLength = strlen(arrStr);
strRevert(arrStr);
send_arrayVector(arrStr,strLength);
send_byte(0xF8 | 0x03); // active port0 and port1
delay(1000);
clean();

strcpy(arrStr, "  0 & 1 OFF  ");
strLength = strlen(arrStr);
strRevert(arrStr);
send_arrayVector(arrStr,strLength);
send_byte(0xF8 | 0x00); // deactive port0 ant port1
delay(1000);
clean();
}
/*******************************************************************************/
void individualNumbers(){
//Note: Here we sent the position grid where we want write the char: 0xE0 grid "0", 0xE1 grid "1", 0xE2 grid "2" and so on!
digitalWrite(VFD_cs, LOW);
delayMicroseconds(5);
send_byte_without_CS(0xE0);// Display digit is set to the  first Digits(first digit is on the right side to this display)
delayMicroseconds(1);
send_byte_without_CS(0x30); // char 0
delayMicroseconds(1);
send_byte_without_CS(0x31); // char 1
delayMicroseconds(1);
send_byte_without_CS(0x32); // char 2
delayMicroseconds(1);
send_byte_without_CS(0x33); // char 3
delayMicroseconds(1);
send_byte_without_CS(0x34); // char 4
delayMicroseconds(1);
send_byte_without_CS(0x35); // char 5
delayMicroseconds(1);
send_byte_without_CS(0x36); // char 6
delayMicroseconds(1);
send_byte_without_CS(0x37); // char 7
delayMicroseconds(1);
send_byte_without_CS(0x38); // char 8
delayMicroseconds(1);
send_byte_without_CS(0x39); // char 9
delayMicroseconds(1);
digitalWrite(VFD_cs, HIGH);
delayMicroseconds(5);
}
void onAll(){
  /* next lines is only to blink all segments in the VFD */
  for(uint8_t i = 0; i < 4; i++){
    send_byte(0xF0 | 0x03);// All on
    delay(500);
    send_byte(0xF0 | 0x00);// All on
    delay(500);
  }
}
/***********************************************************************/
void setup() {
pinMode(7, OUTPUT);
pinMode(8, OUTPUT);
pinMode(9, OUTPUT);
pinMode(10, OUTPUT);
digitalWrite(10, HIGH);
 vfd_init();
delay(500);
Serial.begin(115200);
}
/**********************************************************************/
void loop() {
  uint8_t strLength = 0x00; // Used to get length of string.
  byte arrStr[16];
  
send_byte(0xE0); //Position grid to write (have 0xE0) 
send_byte(0xF0 | 0x05);// 0xF5 to setting autoincrement, no autoincrement 0xF4
delay(1);
send_byte(0xF0 | 0x01);// Normal operation or all on or all off
delay(1);

onAll(); // This function do VFD all Bright.

delayMicroseconds(5);
send_arrayVector("--------------", 14); // send space to all acitvated digits to clear VFD
delayMicroseconds(1);
delay(1000);
clean();

strcpy(arrStr, "HI FOLKS      ");
strLength = strlen(arrStr);
strRevert(arrStr);
send_arrayVector(arrStr,strLength);
delay(2000);
clean();
delayMicroseconds(5);
//
individualNumbers(); // Write to display the numbers from 0 to 9 but by single mode digit.
delay(1000);
clean();
//
ports();  // This function make activation and deactivation of port 0 and 1. Pins 17 & 18 of M66004SP
delay(1000);
clean();
//
viewing_pixel(); // Here turn on pixel by pixel from matrix 5*7( start pixel 0 finish pixel 16) is range of memory RAM customer defined.
delay(1000);
clean();
//
grid_11_fill_RAM(); // Here I fill os 16 positions of customer RAM with user bits to matrix 5*7.
delayMicroseconds(10);
viewing(); // This function show the 16 letters and the 16 bits
}
