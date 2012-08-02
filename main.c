#include <msp430.h>
#include <stdint.h>
#include <legacymsp430.h>
#include "lcd16.h"

// variaveis globais
unsigned int dado;
int periodos=0;
int max1 = 0;
int max2 = 0;
int min1 = 0;
int min2 = 0;
int calibrando =0;
int toogle =1;
int bpm = 0;
int subindo=0;
int vales=0;
int meio =0;




int main(void) 
{
    WDTCTL = WDTPW  + WDTHOLD; // Para o WDT(Wath dog Timer)

    // calibrando o clock para trabalhar em 16MHz
    BCSCTL1 = CALBC1_16MHZ;  
    DCOCTL = CALDCO_16MHZ;
	
    // configurando conversor AD para trabalhar com tensao no pino 4
    ADC10CTL0 &= ~ENC;
	ADC10CTL1 = 4 << 12;
    ADC10CTL0 = ADC10ON + ENC + ADC10SHT_0;
	
    // configurando LED
    P1DIR |= BIT6;  // EN saida
    P1DIR |= BIT7;  // RS saida
    P2DIR = 0xFF; 
    P1OUT = 0x00;
	lcdinit();
    prints("Batimentos ");

    
    // Configurando interrupcao
    TACCR0 = 32000;  //   16 MHz / 32000 = 500 Hz

    // Usar clock TASSEL_2, no modo UP (MC_1).
    TACTL = TASSEL_2 | MC_1;
  
    // Reseta o timer no fim de contagem(OUTMOD_7) e habilita interrupcao (CCIE)
    TACCTL1 = OUTMOD_7 | CCIE;
   
   // LPM0 (Lower power mode) bota a cpu para dormir com interrupcao habilitada.
    __bis_SR_register(CPUOFF | GIE);

    return 0;
}

int batimentos(){
    int d1,d2;
    d1 = max1-min1;
    d2 = max2-min2;
    max2=max1;
    min2=min1;
    float r;
    r = d2/(float)d1;
    
    if (r>1.7){
        bpm = 60+(1/r)*27;
    }else{
        bpm= 0;
         }
     return bpm;
}

unsigned int lerConversorAD(){
    ADC10CTL0 |= ADC10SC;
	while(ADC10CTL1 & ADC10BUSY);
	return ADC10MEM;    
}


int calcula_freq(){
    float f; 
    f = vales/2*500/350;
    vales=0;
    return f;
}

interrupt(TIMER0_A1_VECTOR) ta1_isr(void)
{
    // Limpa flag de interrupcao 
    TACCTL1 &= ~CCIFG;
 
    periodos+=1;
    if (calibrando <2){
        if (periodos > 1500){ 
            calibrando=2; 
            meio = (max1-min1)/2.0 + min1;        
        }
    }else{
        if (periodos> 350) { // tempo de amostragem 350 interrupcoes.
            gotoXy(9,1); 
            integerToLcd(calcula_freq());
            prints("Hz");
            gotoXy(9,1);
            prints("  ");
            
            gotoXy(0,1);
            if (toogle ==1){
                integerToLcd(batimentos());
                if (bpm > 0){
                    prints(" S2");
                }else{
                    prints("   ");
                }

                toogle = 0;
                gotoXy(0,1);
                prints("  ");
            }else{
                if (batimentos()){
                    prints("       ");
                }
                toogle =1;
            }

            periodos = 0;
        }
    }


    dado = lerConversorAD(); 
    
    // Estado 0: estado necessario para inicializar o min1 
    if (calibrando ==0){ 
        min1=dado; 
        calibrando =1;
    }
    
    // Estado 1: calibrando a frequencia inicial
    if (calibrando ==1){
        if (dado > max1)max1=dado;
        if (dado < min1)min1=dado;
        
    }else{

        // Estado Final: Ja calibrou agora esta lendo
        if (dado > max2)max2=dado; 
        if (dado < min2)min2=dado;
        
        if ((subindo == 0) && (dado > meio)){
            subindo = 1; // agora esta no vale de cima.
            vales+=1;
        }else {
            if ((subindo == 1) && ( dado < meio)){
                subindo = 0; // agor a esta no vale de baixo.
                vales+=1;
            }
        }
    
    }
    
}
