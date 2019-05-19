#include <avr/io.h>
#include<util/delay.h>
#include <avr/interrupt.h>
#include <LiquidCrystal.h>
#include<stdint.h>
#define SET_BIT(PORT,BIT) PORT|=(1<<BIT)
#define CLR_BIT(PORT,BIT) PORT&=~(1<<BIT)
volatile int flag=1;
volatile int flag1=1;
volatile int y1=0;
volatile int z1=0;
volatile int mode=0;
int cabintemp=0;
int usertemp=0;
LiquidCrystal lcd(12, 11, 9, 8, 4, 1);
volatile int16_t x=00,espeed=0,ctemp=00,y=0;
int count=0,counter=0;
void port_initilization()
{
  SET_BIT(DDRB,PB0);//digital port initilization
  SET_BIT(DDRB,PB1);
  CLR_BIT(PORTB,PORTB0);
  CLR_BIT(PORTB,PORTB1);
  CLR_BIT(PINC,PINC0);//analog port initilizations
  CLR_BIT(PINC,PINC1);
  lcd.begin(16, 2);//lcd basic setup
  //HVAC
  DDRD&=~(1<<PD2); //POWER switch pd2 input interrupt 0
  DDRD|=(1<<PD0); //led pd0 power led output
  DDRD&=~(1<<PD3); //mode switch pd3 interrupt 1 input
  DDRD|=(1<<PD5);//led pd5 ac
  DDRD|=(1<<PD6);//led pd6 heater
  DDRD|=(1<<PD7);//led pd7 blower
  DDRC&=~(1<<PC0);//adc0 PC0
  DDRC&=~(1<<PC2);//adc1 PC1
  //MIRROR
    DDRB &=~(1<<PB2);
  	DDRB &=~(1<<PB5);
    DDRC |=(1<<PC5);
    DDRC |=(1<<PC4);
    DDRC |=(1<<PC3);
  	DDRD &=~(1<<PD7);
    //DDRC&=~(1<<PC3);
  	//DDRC&=~(1<<PC4);
  	DDRB|=(1<<PB3);
     
}
//this temperature reads temperature from the sensors
void read_temp()
{
  x=adc_read0();
 
  ADCSRA&=0X00;
  espeed=speed_map(x,0,1024,0,360);
  y=adc_read1();
  ADCSRA&=0X00;
    ctemp=temp_map(y,1,255,0,30);
    cabintemp=ctemp;
    z1=adc_read2();
    ADCSRA&=0X00;
    usertemp=temp_map(z1,1,255,0,30);
}
//this function displays temperature on lcd
void display_temp()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("speed");
    lcd.setCursor(6,0);
    lcd.print(espeed);
    lcd.setCursor(10,0);
    lcd.print("kmph");
    lcd.setCursor(0, 1);
    lcd.print("temp");
    lcd.setCursor(8,1);
    lcd.print(ctemp);
    lcd.setCursor(11,1);
    lcd.print("C");
}
//analog read functions for analog temperaure functions
uint16_t adc_read0()
{ADMUX&=0X00;
 ADMUX|=(1<<REFS0);//ADC CHANNEL 0 WITH AVCC AS REFERENCE
 ADCSRA|=(1<<ADEN);//ENABLE ADC
 ADCSRA|=(1<<ADSC);
 ADC=0;
while((ADCSRA)&(1<<ADSC));//START CONVERSION
return(ADC);
}
uint16_t adc_read1()
{
 ADMUX&=0X00; 
 ADMUX|=(1<<MUX0);
 ADMUX|=(1<<REFS0);//ADC CHANNEL 0 WITH AVCC AS REFERENCE
 ADCSRA|=(1<<ADEN);//ENABLE ADC
 ADCSRA|=(1<<ADSC);
 ADC=0;
while((ADCSRA)&(1<<ADSC));//START CONVERSION
return(ADC);
}
uint16_t adc_read2()
{
 ADMUX&=0X00; 
 ADMUX|=(1<<MUX1);
 ADMUX|=(1<<REFS0);//ADC CHANNEL 0 WITH AVCC AS REFERENCE
 ADCSRA|=(1<<ADEN);//ENABLE ADC
 ADCSRA|=(1<<ADSC);
 ADC=0;
while((ADCSRA)&(1<<ADSC));//START CONVERSION
return(ADC);
}
//this function maps the analog read value to actual temperature 
long temp_map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
long speed_map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void interrupt0()
{
DDRD&=~(1<<2);//interrupt switch power pd2
PORTD|=(1<<2);
EICRA|=(1<<ISC00);
EIMSK|=(1<<INT0);
sei();
}
ISR(INT0_vect)
{
      if(!(PIND&(1<<PD2)))
      flag=1;//mode 0 cabin vs user
       //mode=1;
      else
      flag=0;//mode 1 cabin vs 22
       //mode=0;
}

void interrupt1()
{
  DDRD&=~(1<<3);//interrupt switch mode pd3
  PORTD|=(1<<3);
  EICRA|=(1<<ISC10);
  EIMSK|=(1<<INT1);
  sei();
}

ISR(INT1_vect)
{
    if(!(PIND&(1<<PD3)))
    {flag1=1;//mode 0 cabin vs user
     mode=1;}
    else
    {flag1=0;//mode 1 cabin vs 22
     mode=0;}
}

void acon()
{
  PORTD|=(1<<PD5);
}
void acoff()
{ PORTD&=~(1<<PD5);
}
void heateron()
{
  PORTD|=(1<<PD6);
}
void heateroff()
{
   PORTD&=~(1<<PD6);
}

void manualset(uint16_t cabintemp,uint16_t usertemp)
{
if(cabintemp>usertemp)// 0=Cabin 1=user
  {
          acon();
           lcd.setCursor(0, 1);
           lcd.print("AC ON");
          _delay_ms(100);
           acoff();
           
          heateroff();

  }
else
  if(cabintemp<usertemp)
  {       acoff();
          heateron();
          lcd.setCursor(0, 1);
          lcd.print("heat ON");
         _delay_ms(100);
          heateroff();
          
  }
  else
  {
    acoff();
    heateroff();
  
  }

    
}


int power_led_on()
{

  PORTD|=(1<<PD0);

}

int power_led_off()
{

    PORTD&=~(1<<PD0);
}

//timer initilizations and delay functions
void timer2_init()
{
TCCR2A|=0X00;
TCNT2=0X00;
TCCR2B|=((1<<CS20)|(1<<CS22));
TCCR2B&=~(1<<CS21);
TIMSK2|=(1<<0);
}

ISR(TIMER2_OVF_vect)
{
count++;
  if(count<=(10))
  {
  count=0;
    counter++;
  }

}
void _delay100_ms()
{
counter=0;
  TCNT2=0X00;
  TIMSK2|=(1<<0);
  while(counter<6);
  TIMSK2&=~(1<<0);
 
}

void mirror()
{
 
if(!(PINB&(1<<PB5)) && !(PINB&(1<<PB2))) //  mirror up
  {
     PORTC|=(1<<PC3);//MOTOR ACLK
     PORTC&=~(1<<PC4);
     PORTC|=(1<<PC5);//LED
     lcd.setCursor(0, 1);
     lcd.print("up");
  }
  else if(!(PINB&(1<<PB5)) && PINB&(1<<PB2)) // mirror down
  {
     PORTC|=(1<<PC4);//MOTOR CLK
     PORTC&=~(1<<PC3);
     PORTC|=(1<<PC5);//LED
     
  }
  else if(PINB&(1<<PB5) && (!(PINB&(1<<PB2)))) // mirror right
  {
     PORTC|=(1<<PC3);//MOTOR ACLK
     PORTC&=~(1<<PC4);
     PORTC&=~(1<<PC5);//LED
    
  }
  else if(PINB&(1<<PB5) && PINB&(1<<PB2)) // mirror left
  {
     PORTC|=(1<<PC4);//MOTOR CLK
     PORTC&=~(1<<PC3);
     PORTC&=~(1<<PC5);//LED
   
  }
  else
  {
     PORTC&=~(1<<PC3);
     PORTC&=~(1<<PC4);
     PORTC&=~(1<<PC5);//LED
  }
}

void seat()
{ 
if(PIND&(1<<PC3))
  {
     PORTC|=(1<<PC3);//MOTOR ACLK
     PORTC&=~(1<<PC4);
     PORTC&=~(1<<PC5);//LED
    
  }
  else 
  {
    PORTD|=(1<<PD7);
    PORTC &=~(1<<PC5);  
    
  }
  if(PINC&(1<<PC4))
  {
    PORTB|=(1<<PB5);
    PORTB &=~(1<<PB2);
    
  }
  else 
  {
    PORTB|=(1<<PB2);
    PORTB &=~(1<<PB5);
   
    
  }
}  
//main function
int main() 
{ sei();
  interrupt0();
  interrupt1();
  port_initilization();
  timer2_init();
   
  while(1)
  { 
    
      
     if(flag==1)
     {
       
        read_temp();
        display_temp();
        power_led_on();
   
      
  
       if(flag1==1)// manual set of temperature is on
         //mode 0 cabin vs user
       { 
         manualset(cabintemp,usertemp);
          if((PIND&(1<<PD7)))
          {
            mirror();
          }    
          else
           {
           PORTC&=~(1<<PC3);
           PORTC&=~(1<<PC4);
           PORTC&=~(1<<PC5);//LED
           }


       }
       else//manual set temp is off
         //mode 1 cabin vs 22
       {
         
         usertemp=22;
         manualset(cabintemp,usertemp);
          if((PIND&(1<<PD7)))
          {
            mirror();
          }    
          else
           {
           PORTC&=~(1<<PC3);
           PORTC&=~(1<<PC4);
           PORTC&=~(1<<PC5);//LED
           }


       }


     }
     else
      {
       power_led_off();
       acoff();
       heateroff();
       lcd.clear();
       PORTC&=~(1<<PC3);
       PORTC&=~(1<<PC4);
       PORTC&=~(1<<PC5);
      }
   
    }
 
}
