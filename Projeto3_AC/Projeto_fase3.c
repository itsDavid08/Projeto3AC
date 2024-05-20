#include <reg51.h>

//tempos dos timers 0 e 1

//Timer - 10ms -> (65536 (10000h) - 10000 (2710h)) = 55536 (D8F0h)
#define TempoH 0xD8
#define TempoL 0xF0
//




//ports para os segmentos dos displays D1 e D2 e as entradas das respostas

//segmentos do display D1
sbit seg1A = P1^0;
sbit seg1B = P1^1;
sbit seg1C = P1^2;
sbit seg1D = P1^3;
sbit seg1E = P1^4;
sbit seg1F = P1^5;
sbit seg1G = P1^6;
sbit seg1DP = P1^7;

//segmentos do display D2
sbit seg2A = P2^0;
sbit seg2B = P2^1;
sbit seg2C = P2^2;
sbit seg2D = P2^3;
sbit seg2E = P2^4;
sbit seg2F = P2^5;
sbit seg2G = P2^6;
sbit seg2DP = P2^7;

//Ports para as respostas
sbit resA = P3^4;
sbit resB = P3^5;
sbit resC = P3^6;
sbit resD = P3^7;
sbit resAnd = P3^3;

//declara��o de variaveis

unsigned char Segundos = 5; //variavel que armazena o numero a mostrar no display D1 (segundos)
unsigned char DecimasSegundos = 0; //variavel que armazena o numero a mostrar no display D2 (decimas de segundos)
unsigned int conta = 0;

unsigned char Start = 1; //se � um o timer1 decresce o temporizador, se n�o conta os segundos para alternar o display

unsigned char resposta = 0; //0-sem resposta, 1-A, 2-B, 3-C, 4-D

unsigned char GuardaSegundos = 0; //para guardar uma copia dos segundos 
unsigned char GuardaDecimasSegundos = 0; //para guardar uma copia das decimas



void Init(void); //fun��o para inicializar os timers e interrup��es
void mostraDisplay (unsigned char Unidade, unsigned char Decimas); //mostra os n�meros no display 

void main(void)
{
	Init();
	
	while(1) //ciclo infinito
	{
		resAnd = resA && resB && resC && resD;
		mostraDisplay (Segundos, DecimasSegundos);
	}
}

void Init(void)
{
	//configurar o registo IE
	EA = 1; //ativa interrup��es globais
	ET0 = 1; //ativa interrup��es do timer 0
	ET1 = 1; //ativa interrup��es do timer 1
	EX0 = 1; //ativa interrup��o externa 0
	EX1 = 0; //desativa interrup��o externa 1
	
	//configura��o das prioridades
	//prioridade por defeito
	IP = 0x00;
	
	//configura��o dos timers	
	//Timer0 - 10ms -> (65536 (10000h) - 10000 (2710h)) = 55536 (D8F0h)
	TH0 = TempoH;
	TL0 = TempoL;
	//Timer1 - 10ms -> (65536 (10000h) - 10000 (2710h)) = 55536 (D8F0h)
	TH1 = TempoH;
	TL1 = TempoL;
	
	//configura��o registo TMOD
	TMOD &= 0xF0; //limpa os 4 bits do timer 0 
	TMOD |= 0x01; //modo 1 do timer 0
	
	TMOD &= 0x0F; //limpa os 4 bits do timer 1 
	TMOD |= 0x10; //modo 1 do timer 1
	
	//configura��o registo TCON
	TR0 = 0; //timer 0 come�a desligado
	TR1 = 0; //timer 1 come�a desligado
	IT0 = 1; //interrup��o externa 0 ativa no falling edge
	IT1 = 1; //interrup��o externa 1 ativa no falling edge
}

//interrup��o externa 0 come�a a contagem do temporizador
//tambem inicializa o timer 0 para evitar o debounce do bot�o

void External0_ISR(void) interrupt 0
{
	
	
	if (Start == 0) //se contando � 0, quere dizer que estava a mostrar a resposta, ent�o o programa inicia outra vez na interrup��o
	{
		Start = 1; //contando volta a ser 1
		resposta = 0; //a resposta anterior � apagada
		Segundos = 5;
		DecimasSegundos = 0;
		//Timer0 - 10ms -> (65536 (10000h) - 10000 (2710h)) = 55536 (D8F0h)
		TH0 = TempoH;
		TL0 = TempoL;
		//Timer1 - 10ms -> (65536 (10000h) - 10000 (2710h)) = 55536 (D8F0h)
		TH1 = TempoH;
		TL1 = TempoL;
		TR0 = 1; //timer 0 come�a desligado
		TR1 = 0; //timer 1 come�a desligado
		EX0 = 0; //ativa interrup��o 0
		EX1 = 0; //desativa interrup��o 1
		conta = 0;
	}
	else{
		TR0 = 1; //timer 0 come�a a contar o tempo
		TR1 = 1; //timer 1 come�a a contar o tempo
		EX0 = 0; //desativa interrup��o externa 0
	}
}



//timer 0 usado para evitar o debounce do bot�o, tempo de 10ms
void Timer0_ISR(void) interrupt 1
{
	//Timer0 conta 10ms de debounce
	TR0 = 0; //timer 0 p�ra de contar o tempo
	IE0 = 0; //limpa a flag se P3.2 foi pressionado (ru�do bot�o)
	IE1 = 0;
	EX0 = 1; //ativa interrup��o externa 0
	EX1 = 1;
	
	//Timer 0 - 10ms -> (65536 (10000h) - 10000 (2710h)) = 55536 (D8F0h)
	TH0 = TempoH; //faz reset ao timer 0
	TL0 = TempoL;
}



//interrup��o externa 1 � usada para escolher a op��o
//tambem inicializa o timer 0 para evitar o debounce do bot�o

void External1_ISR(void) interrupt 2
{
	TR0 = 1; //timer 0 come�a a contar o tempo
	TR1 = 1; //timer 1 come�a a contar o tempo
	EX1 = 0; //desativa interrup��o externa 1
	
	if (resA==0)
	{
		resposta=1; //se foi selecionado A, resposta � igual a 1
	}
	
	if (resB==0)
	{
		resposta=2; //se foi selecionado B, resposta � igual a 2
	}
	
	if (resC==0)
	{
		resposta=3; //se foi selecionado C, resposta � igual a 3
	}
	
	if (resD==0)
	{
		resposta=4; //se foi selecionado D, resposta � igual a 4
	}
	Start =0; //contando passa a ser zero para n�o decrementar o temporizador 
	GuardaSegundos = Segundos; //guarda uma copia do tempo restante (segundos)
	GuardaDecimasSegundos = DecimasSegundos; //guarda uma copia do tempo restante (decimas)
}

//timer 1 � usado para contar os 5 segundos do temporizador e o tempo de 1 segundo para mostrar alternadamente a resposta e o tempo

void Timer1_ISR(void) interrupt 3
{
	if (Start==1)
	{
		
		if (conta >= 10){
			
			if(DecimasSegundos == 0)
			{
				DecimasSegundos = 9;
				--Segundos;
			}
			else{
				--DecimasSegundos;
			}
			conta = 0;
		}
		else{
		conta++;
		}
		
	
		if (Segundos == 0 && DecimasSegundos == 0) //caso o tempo acabar e n�o haber resposta 
		{
			GuardaSegundos = Segundos; //guarda uma copia do tempo restante (segundos)
			GuardaDecimasSegundos = DecimasSegundos; //guarda uma copia do tempo restante (decimas)
			Start = 0;
			conta = 0;
		}
	}
	else
	{
		if (conta >= 100) //se ja passou um segundo, alterna os displays
		{
			if (Segundos == 10) //se o display D1 mostra um traço, quere dizer que esta a mostrar a resposta
			{
				Segundos = GuardaSegundos; //volta a mostrar os segundos restantes
				DecimasSegundos = GuardaDecimasSegundos; //volta a mostrar as decimas restantes
			}
			else //se n�o, esta a mostrar o tempo
			{
				Segundos = 10; //mostra um tra�o no display dos segundos
				switch (resposta) //dependendo da resposta mostra no display D2
				{
					case 1:
						DecimasSegundos = 11; //Resposta 1 mostra A
						break;
					case 2:
						DecimasSegundos = 12; //Resposta 2 mostra B
						break;
					case 3:
						DecimasSegundos = 13; //Resposta 3 mostra C
						break;
					case 4:
						DecimasSegundos = 14; //Resposta 4 mostra D
						break;
					default:
						DecimasSegundos = 10; //caso default mostra um tra�o
				}
			}
			conta = 0; //faz reset ao contador de 1 segundo
		}
		else
		{
			++conta; //incrementa o contador de 1 segundo
		}
	}
	
	//Timer1 - 10ms -> (65536 (10000h) - 10000 (2710h)) = 55536 (D8F0h)
		TH1 = TempoH; //faz reset ao timer 1
		TL1 = TempoL;
	
}

//Fun��o para mostrar os caracteres no display

void mostraDisplay(unsigned char Unidade, unsigned char Decimas)
{
	
	//n�meros das unidades
	code unsigned segments1 [11][8] = {
		{0, 0, 0, 0, 0, 0, 1, 0},//0.
		{1, 0, 0, 1, 1, 1, 1, 0},//1.
		{0, 0, 1, 0, 0, 1, 0, 0},//2.
		{0, 0, 0, 0, 1, 1, 0, 0},//3.
		{1, 0, 0, 1, 1, 0, 0, 0},//4.
		{0, 1, 0, 0, 1, 0, 0, 0},//5.
		{0, 1, 0, 0, 0, 0, 0, 0},//6.
		{0, 0, 0, 1, 1, 1, 1, 0},//7.
		{0, 0, 0, 0, 0, 0, 0, 0},//8.
		{0, 0, 0, 0, 1, 0, 0, 0},//9.
		{1, 1, 1, 1, 1, 1, 0, 0}, //-.
	};
	
	//numeros das d�cimas
	code unsigned segments2 [15][8] = {
		{0, 0, 0, 0, 0, 0, 1, 1},//0
		{1, 0, 0, 1, 1, 1, 1, 1},//1
		{0, 0, 1, 0, 0, 1, 0, 1},//2
		{0, 0, 0, 0, 1, 1, 0, 1},//3
		{1, 0, 0, 1, 1, 0, 0, 1},//4
		{0, 1, 0, 0, 1, 0, 0, 1},//5
		{0, 1, 0, 0, 0, 0, 0, 1},//6
		{0, 0, 0, 1, 1, 1, 1, 1},//7
		{0, 0, 0, 0, 0, 0, 0, 1},//8
		{0, 0, 0, 0, 1, 0, 0, 1},//9
		{1, 1, 1, 1, 1, 1, 0, 1},//-
		{0, 0, 0, 1, 0, 0, 0, 1},//A
		{1, 1, 0, 0, 0, 0, 0, 1},//B
		{0, 1, 1, 0, 0, 0, 1, 1},//C
		{1, 0, 0, 0, 0, 1, 0, 1},//D
	};
	
	seg1A = segments1[Unidade][0];
	seg1B = segments1[Unidade][1];
	seg1C = segments1[Unidade][2];
	seg1D = segments1[Unidade][3];
	seg1E = segments1[Unidade][4];
	seg1F = segments1[Unidade][5];
	seg1G = segments1[Unidade][6];
	seg1DP = segments1[Unidade][7];
	
	seg2A = segments2[Decimas][0];
	seg2B = segments2[Decimas][1];
	seg2C = segments2[Decimas][2];
	seg2D = segments2[Decimas][3];
	seg2E = segments2[Decimas][4];
	seg2F = segments2[Decimas][5];
	seg2G = segments2[Decimas][6];
	seg2DP = segments2[Decimas][7];
}
