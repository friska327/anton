//ds18b20

#define MATCH_ROM	0x55
#define READ_ROM	0x33
#define SKIP_ROM	0xCC
#define CONVERT_T	0x44
#define Read_Scratchpad 0xBE
#define Write_Scratchpad 0x4E
#define Copy_Scratchpad 0x48
#define RECALL_E2 0xB8
#define SEARCH_ROM 0xF0
#define ALARM_SEARCH 0xEC


struct ID_ds18b20
{
char ID [8];	
char num;	
char alarm;
char MSB;
char LSB;
char TH;
char TL;
char ctrl;
char disput_bit;	
};

char initial_ds18b20();

float read_T();

char Tree_search(struct ID_ds18b20 *pt);
char READ_ROM_DS(struct ID_ds18b20 *ID1);

void read_scr(struct ID_ds18b20 *ID_X);
void write_scr(struct ID_ds18b20 *ID_X);
void copy_scr(struct ID_ds18b20 *ID_X);
//void convert_T(char *addr);

void send_Bit(char tmp);
char recive_Bit();
void send_Byte (char tmp);
char read_Byte ();


void convert(); //broadcast mess

void  alarm_search();
char Compute_CRC8 (uint8_t* data, uint8_t length);
