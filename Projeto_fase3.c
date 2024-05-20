#include <reg51.h>

//tempos dos timers 0 e 1

//Timer0 - 5ms -> (65536 (10000h) - 5000 (1388h)) = 60536 (EC78h)
#define Tempo0H 0xEC
#define Tempo0L 0x78
//

//Timer1 - 50ms -> (65536 (10000h) - 50000 (C350h)) = 15536 (3CB0h)
#define Tempo1H 0x3C
#define Tempo1L 0xB0

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


//declara��o de variaveis

unsigned char DisplaySegundos = 5; //variavel que armazena o numero a mostrar no display D1 (segundos)
unsigned char DisplayDecimas = 0; //variavel que armazena o numero a mostrar no display D2 (decimas de segundos)

unsigned char MetadeDecima = 0; //timer 0 s� conta 50ms, cada dois ciclos de timer 0 passam 100ms, a variavel conta se ja passou dois ciclos

unsigned char Contando = 1; //se � um o timer1 decresce o temporizador, se n�o conta os segundos para alternar o display

unsigned char resposta = 0; //0-sem resposta, 1-A, 2-B, 3-C, 4-D

unsigned char CopiaDisplaySegundos = 0; //para guardar uma copia dos segundos 
unsigned char CopiaDisplayDecimas = 0; //para guardar uma copia das decimas

unsigned char Umseg = 0; //para contar um segundo no momento de alternar o display
//declara��o de fun��es

void Init(void); //fun��o para inicializar os timers e interrup��es
void display (unsigned char Unidade, unsigned char Decimas); //mostra os n�meros no display 

void main(void)
{
	Init();
	
	while(1) //ciclo infinito
	{
		display (DisplaySegundos, DisplayDecimas);
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
	//Timer0 - 5ms -> (65536 (10000h) - 5000 (1388h)) = 60536 (EC78h)
	TH0 = Tempo0H;
	TL0 = Tempo0L;
	//Timer1 - 50ms -> (65536 (10000h) - 50000 (C350h)) = 15536 (3CB0h)
	TH1 = Tempo1H;
	TL1 = Tempo1L;
	
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
	TR0 = 1; //timer 0 come�a a contar o tempo
	TR1 = 1; //timer 1 come�a a contar o tempo
	EX0 = 0; //desativa interrup��o externa 0
	
	if (Contando == 0) //se contando � 0, quere dizer que estava a mostrar a resposta, ent�o o programa inicia outra vez na interrup��o
	{
		Contando = 1; //contando volta a ser 1
		resposta = 0; //a resposta anterior � apagada
		DisplaySegundos = 5;
		DisplayDecimas = 0;
		//Timer0 - 5ms -> (65536 (10000h) - 5000 (1388h)) = 60536 (EC78h)
		TH0 = Tempo0H;
		TL0 = Tempo0L;
		//Timer1 - 50ms -> (65536 (10000h) - 50000 (C350h)) = 15536 (3CB0h)
		TH1 = Tempo1H;
		TL1 = Tempo1L;
		TR0 = 0; //timer 0 come�a desligado
		TR1 = 0; //timer 1 come�a desligado
		EX0 = 1; //ativa interrup��o 0
		EX1 = 0; //desativa interrup��o 1
	}
}

//timer 0 usado para evitar o debounce do bot�o, tempo de 5ms

void Timer0_ISR(void) interrupt 1
{
	//Timer0 conta 5ms de debounce
	TR0 = 0; //timer 0 p�ra de contar o tempo
	IE0 = 0; //limpa a flag se P3.2 foi pressionado (ru�do bot�o)
	EX0 = 1; //ativa interrup��o externa 0
	
	//Timer 0 - 5ms ->(65536(10000h) - 5000(1388h)) = 60536 (EC78h)
	TH0 = Tempo0H; //faz reset ao timer 0
	TL0 = Tempo0L;
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
	Contando=0; //contando passa a ser zero para n�o decrementar o temporizador 
	CopiaDisplaySegundos = DisplaySegundos; //guarda uma copia do tempo restante (segundos)
	CopiaDisplayDecimas = DisplayDecimas; //guarda uma copia do tempo restante (decimas)
}

//timer 1 � usado para contar os 5 segundos do temporizador e o tempo de 1 segundo para mostrar alternadamente a resposta e o tempo

void Timer1_ISR(void) interrupt 3
{
	EX1 = 1; //ativa interrup��o externa 1 para dar a resposta
	if (Contando==1)
	{
		if(MetadeDecima >= 1)  //se ja contou dois ciclos (50ms*2 = 100ms) decrementa o numero mostrado nas decimas
		{
			if(DisplayDecimas <= 0)   //se o numero nas decimas � zero, decrementa o segundo e a decima passa a 9
			{
				--DisplaySegundos;
				DisplayDecimas = 9;
			}
			else  //se n�o, simplesmente decrementa o n�mero mostrado nas decimas 
			{
				--DisplayDecimas;
			}
			MetadeDecima = 0; //volta a contar dois ciclos de 50ms
		}
		else
		{
			++MetadeDecima; //se s� contou 1 ciclo de 50ms, incrementa a variavel para saber que no seguinte ciclo j� passou 100ms 
		}
	
		//Timer1 - 50ms -> (65536 (10000h) - 50000 (C350h)) = 15536 (3CB0h)
		TH1 = Tempo1H; //faz reset ao timer 1
		TL1 = Tempo1L;
	
		if (DisplaySegundos == 0 && DisplayDecimas == 0) //caso o tempo acabar e n�o haber resposta 
		{
			DisplaySegundos = 0; //define o tempo a 0 para n�o haver erros no display 
			DisplayDecimas = 0;
			CopiaDisplaySegundos = DisplaySegundos; //guarda uma copia do tempo restante (segundos)
			CopiaDisplayDecimas = DisplayDecimas; //guarda uma copia do tempo restante (decimas)
			Contando = 0;
		}
	}
	else
	{
		if (Umseg >= 14) //se ja passou um segundo, alterna os displays
		{
			if (DisplaySegundos == 10) //se o display D1 mostra um tra�o, quere dizer que esta a mostrar a resposta
			{
				DisplaySegundos = CopiaDisplaySegundos; //volta a mostrar os segundos restantes
				DisplayDecimas = CopiaDisplayDecimas; //volta a mostrar as decimas restantes
			}
			else //se n�o, esta a mostrar o tempo
			{
				DisplaySegundos = 10; //mostra um tra�o no display dos segundos
				switch (resposta) //dependendo da resposta mostra no display D2
				{
					case 0:
						DisplayDecimas = 10; //sem resposta, mostra um tra�o
						break;
					case 1:
						DisplayDecimas = 11; //Resposta 1 mostra A
						break;
					case 2:
						DisplayDecimas = 12; //Resposta 2 mostra B
						break;
					case 3:
						DisplayDecimas = 13; //Resposta 3 mostra C
						break;
					case 4:
						DisplayDecimas = 14; //Resposta 4 mostra D
						break;
					default:
						DisplayDecimas = 10; //caso default mostra um tra�o
				}
			}
			Umseg = 0; //faz reset ao contador de 1 segundo
		}
		else
		{
			++Umseg; //incrementa o contador de 1 segundo
		}
	}
}

//Fun��o para mostrar os caracteres no display

void display(unsigned char Unidade, unsigned char Decimas)
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
