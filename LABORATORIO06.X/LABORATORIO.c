/*
 * Universidad del Valle de Guatemala
 * Programación de Microcontroladores
 * Carlos Mauricio Molina López (#21253)
 * LABORATORIO06
 * Created on 24 de octubre de 2022, 10:58 AM
 */

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)


#include <xc.h>
#include <stdint.h>
#include <pic16f887.h>

#define _XTAL_FREQ 8000000
//******************************************************************************
// Variables 
//******************************************************************************
unsigned int ADC_Voltaje1; 
int loop; 
char option_selected;
int equivalent;

//******************************************************************************
// Prototipos de Funciones
//******************************************************************************
void setup(void); 
void setupADC(void);
void initUART(void);
void print(unsigned char *palabra);
void conversion(int voltaje);

//******************************************************************************
// Interrupción
//******************************************************************************
void __interrupt() isr (void){
    
    if (PIR1bits.ADIF){   //Interrupción del ADC cuando la lectura termina
        PORTBbits.RB7=1; 
        PIR1bits.ADIF=0; 
    
    PIR1bits.TXIF = 0;
    }
   
}
//******************************************************************************
// Código Principal 
//******************************************************************************

void main(void) {
    setup();
    setupADC();
    initUART();
            
    //Loop Principal
    while(1){
        print("\r1) Leer Potenciometro \r");
        print("2) Enviar ASCII \r"); 
                
                
        while(PIR1bits.RCIF == 0){
            ;
        }
        
        //--Lectura Canal AN1
        ADCON0bits.CHS = 0b0001;
        __delay_us(100);
        ADCON0bits.GO = 1;
        while (ADCON0bits.GO == 1){
            ;
        }
        ADC_Voltaje1 = ADRESH;
        conversion(ADC_Voltaje1);
        CCPR1L=equivalent; 
        __delay_us(100);
        
        //Selección en consola
        option_selected = RCREG;
        
        //Ejecución de elección
        if (option_selected == '1'){
            print("Valor del pot --> ");
            TXREG = CCPR1L;
        }
        else if (option_selected == '2'){
            print("Caracter ASCII a mostrar...");
            while(PIR1bits.RCIF == 0){
                ;
            }
            PORTB = RCREG; 
        }
        else {
            print("error\r");
        }
    }
        
        /*if (TXSTAbits.TRMT == 1){
            TXREG = CCPR1L;
        }
        __delay_ms(1000);*/
}

//******************************************************************************
//Funciones
//******************************************************************************

void setup(void){
    //Configuración de I/O 
    TRISB = 0;
    PORTB = 0; 
    
    //Configuración del Oscilador Interno
    OSCCONbits.IRCF = 0b111;        // 8MHz
    OSCCONbits.SCS = 1;             // Oscilador Interno
    
     //Configuración de las Interrupciones
    INTCONbits.GIE = 1;
    //INTCONbits.PEIE = 1;
    
    PIE1bits.ADIE = 1;              // Se habilita la interrupción del ADC
    INTCONbits.TMR0IE = 1;          // Se habilitan las interrupciones del TMR0
    
    PIR1bits.ADIF = 0;              // Flag de ADC en 0
}

void setupADC(void){
    //Puerto de Entrada
    TRISAbits.TRISA1 = 1;
    ANSELbits.ANS1 = 1; 
    
    //Módulo de ADC
    ADCON0bits.ADCS = 0b10;         // Fosc/32
    
    ADCON1bits.VCFG1 = 0;           // Voltaje de Referencia + - VSS
    ADCON1bits.VCFG0 = 0;           // Voltaje de Referencia - - VDD
    
    //Formato de Resultado 
    ADCON1bits.ADFM = 0;            // Justificado a la Izquierda
    
    //Canal
    ADCON0bits.CHS = 0b0001;        // Canal AN1
    
    //Habilitación de ADC
    ADCON0bits.ADON = 1; 
    
    //Delay (Ejemplo)
    __delay_us(100);
}

void initUART(void){
    
    SPBRG = 12;                     // Baud rate (8MHz/9600)
    TXSTAbits.SYNC = 0;             // Asíncrono 
    RCSTAbits.SPEN = 1;             // Se habilita el módulo UART
    TXSTAbits.TXEN = 1;             /* Transmisión habilitada; TXIF se enciende
                                     automaticamente.*/
    
    PIR1bits.TXIF = 0;              // Apagamos la bandera de transmisión
    
    RCSTAbits.CREN = 1;             // Habilitamos la recepción
    
    
}

void conversion(int voltaje){
    equivalent = (unsigned short) (48+( (float)(207)/(255) ) * (voltaje-0));
}

void print(unsigned char *palabra){
    
    while (*palabra != '\0'){
        while (TXIF != 1);
        TXREG = *palabra;
        *palabra++;
    }
   
}
