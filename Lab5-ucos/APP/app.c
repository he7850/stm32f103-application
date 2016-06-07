#include <includes.h>

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

#define STK_Size 100
int LED0_Task_STK[STK_Size];
int LED1_Task_STK[STK_Size];
int Task_STK[STK_Size];

#define MAX_TICS 100000
#define DHT11_OK 0
#define DHT11_NO_CONN 1
#define DHT11_CS_ERROR 2
#define DHT11_PORT GPIOB
#define DHT11_PIN GPIO_Pin_0

// delay function for 1us 
void Delay_us(int times){
    unsigned int i;
    for (i=0; i<times; i++){	// maybe 1.9us
    }
}
// delay function for 1ms
void Delay_ms(int times){
		unsigned int i, j;
    for (j=0; j<times; j++){
        for (i=0; i<0x3ff; i++){
        }
    }
    //OSTimeDly(OS_TICKS_PER_SEC / 1000 * times); //this function cannot be used 
}
// delay for almost 0.5 second
void Delay_long(int times){
    unsigned int i, j;
    for (j=0; j<times; j++){
        for (i=0; i<0x3ffff; i++){
        }
    }
}
// set value for DHT11 GPIO
void DHT11_Set(int state){
    BitAction s;
    if (state){
        s = Bit_SET;
    }else{
        s = Bit_RESET;
    }
    GPIO_WriteBit(DHT11_PORT, DHT11_PIN, s);
}
// set DHT11 GPIO to output mode
void DHT11_Pin_OUT(){
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
    DHT11_Set(1);
}
// set DHT11 GPIO to input mode
void DHT11_Pin_IN(){
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DHT11_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
    DHT11_Set(1);
}
// get data from DHT11 GPIO, following two statements are equivalent
uint8_t DHT11_Check(){
    return (DHT11_PORT->IDR & DHT11_PIN)> 0;
    //return GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN);
}
// wait DHT11GPIO to be @state, if overtime then error
void DHT11_Wait(int state, int place){
    int loopCnt = MAX_TICS;
    while (DHT11_Check() != state){
        if (loopCnt-- == 0){
            //ErrorState(1000 + state * 1000 + place);
        }
    }
}
int val = 10;
// read a byte from DHT11
uint8_t DHT11_Read_Byte(){
    int i, cnt;
    uint8_t data = 0;
    for (i=0; i<8; i++){
				cnt=0;
        data <<= 1;	
        DHT11_Wait(1, ++val);//current low level
        while (DHT11_Check() > 0){ //count time in high level
            Delay_us(1);
            cnt++;
        }
        data |= cnt > 5; //cnt>b'100000 means enough long time
    }
    return data;
}
// reset DHT11
void DHT11_Rst(){
    // stm32 output handshake signal
    DHT11_Pin_OUT();
    DHT11_Set(0);
    Delay_us(25000);
    DHT11_Set(1);
    Delay_us(40);
    DHT11_Set(0);
    // transform into input mode to read DHT11's handshake signal
    DHT11_Pin_IN();
}
// read data from DHT11
uint8_t DHT11_Read_Data(uint8_t *buf){
    int i;
    unsigned int cpu_sr;
    // ENTER CRITICAL TO CLOSE INTERRUPT
    OS_ENTER_CRITICAL();
    val = 10;
    DHT11_Rst(); // send handshake signal
    if (DHT11_Check() == 0){  // if replied
        DHT11_Wait(1, 2);  //wait low level
        DHT11_Wait(0, 3);	 //wait high level
        for (i=0; i<5; i++){  //handshake ends,read 40bits
            buf[i] = DHT11_Read_Byte();
        }

        // set GPIO to output mode
        DHT11_Pin_OUT();
        OS_EXIT_CRITICAL(); 

        // check sum
        if (buf[0] + buf[1] + buf[2] + buf[3] == buf[4]){
            return DHT11_OK;
        }else{
            return DHT11_CS_ERROR;
        }
    }else{
        // no reply
        OS_EXIT_CRITICAL(); 
        return DHT11_NO_CONN;
    }
}
// get humidity
uint8_t DHT11_Humidity(uint8_t *buf){
    return buf[0];
}
// get temperature
uint8_t DHT11_Temperature(uint8_t *buf){
    return buf[2];
}

// 7-segment configuration
void GPIO_Configuration(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_DeInit();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0
                                | GPIO_Pin_1
                                | GPIO_Pin_2
                                | GPIO_Pin_3
                                | GPIO_Pin_4
                                | GPIO_Pin_5
                                | GPIO_Pin_6
                                | GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

}
// select which number to light
void digit_select(int index){
    BitAction v[4];
    int i;
    for (i=0; i<4; i++){
        if (index == i){
            v[i] = Bit_SET;
        }else{
            v[i] = Bit_RESET;
        }
    }
    GPIO_WriteBit(GPIOA, GPIO_Pin_9, v[0]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_10, v[1]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_11, v[2]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_12, v[3]);
}
// display digit and point after digit_select()
void digit_show(int dight, int point){
    int segment, i, base;
    BitAction v[8];
    switch (dight){
        case 0:segment = 0xee; break; // 0b11101110  0 -> 7
        case 1:segment = 0x24; break; // 0b00100100
        case 2:segment = 0xba; break; // 0b10111010
        case 3:segment = 0xb6; break; // 0b10110110
        case 4:segment = 0x74; break; // 0b01110100
        case 5:segment = 0xd6; break; // 0b11010110
        case 6:segment = 0xde; break; // 0b11011110
        case 7:segment = 0xa4; break; // 0b10100100
        case 8:segment = 0xfe; break; // 0b11111110
        case 9:segment = 0xf6; break; // 0b11110110
        default:segment = 0xda; break; // 0b11011010 error state
    }
    segment |= point!= 0; // last bit is the fractional point
    base = 1 << 8;
    for (i=0; i<8; i++){
        base >>= 1;
        if ((segment & base )== 0){
            v[i] = Bit_SET;		// output of SET means light off
        }else{
            v[i] = Bit_RESET;	// output of RESET means light up
        }
    }
    GPIO_WriteBit(GPIOA, GPIO_Pin_0, v[0]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, v[1]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_2, v[2]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_3, v[3]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_4, v[4]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_5, v[5]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_6, v[6]);
    GPIO_WriteBit(GPIOA, GPIO_Pin_7, v[7]);
}
// use time-complex way to display one digit of @digit
void led_show(int digit){
    static int index = -1;
    int i;
    int base = 1000;
    index = (index + 1) % 4;	//next digital tube
    for (i=0; i<index; i++){	//display abcd: a->b->c->d
        base /= 10;
    }
    digit = (digit / base) % 10;
    digit_select(index);
    digit_show(digit, 0);
}
// display integer(<10000) for 1s 
void led_show_int(int integer){
		int i;
		for(i=0;i<172;i++){	//50 pass per second
			led_show(integer);
			Delay_ms(5);	//blink per 5ms
		}
}
// read tempterature from DHT11
int ledValue = 0;
void LED0_task(void* pdata){
    uint8_t buf[5];
    int state;
    memset(buf, 0, sizeof(buf));
    while (1){
        state = DHT11_Read_Data(buf);
        switch(state){
            case DHT11_CS_ERROR:
                ledValue = 9002;
                break;
            case DHT11_NO_CONN:
                ledValue = 9001;
                break;
            case DHT11_OK:
                ledValue = DHT11_Temperature(buf);
                break;
        }
				led_show_int(ledValue);
    }
}
void LED1_task(void* pdata){
    while(1){
			led_show_int(ledValue);
    }
}

int main() {
    GPIO_Configuration();
		LED0_task(NULL);	//get temperature from DHT11 and display
//		ledValue = 1000;
//		while(1){
//				ledValue--;
//				led_show_int(ledValue);	
//		}
}
