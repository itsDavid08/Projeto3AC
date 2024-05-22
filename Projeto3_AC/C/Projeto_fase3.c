#include <reg51.h>

//tempos dos timers 0 e 1

//Timer - 10ms -> (65536 (10000h) - 10000 (2710h)) = 55536 (D8F0h)
#define TempoH 0xD8
#define TempoL 0xF0
//




//atribuição dos pinos
//display D1
sbit D1A = P1^0;
sbit D1B = P1^1;
sbit D1C = P1^2;
sbit D1D = P1^3;
sbit D1E = P1^4;
sbit D1F = P1^5;
sbit D1G = P1^6;
sbit D1DP = P1^7;

//display D2
sbit D2A = P2^0;
sbit D2B = P2^1;
sbit D2C = P2^2;
sbit D2D = P2^3;
sbit D2E = P2^4;
sbit D2F = P2^5;
sbit D2G = P2^6;
sbit D2DP = P2^7;

//Botões de entrada
sbit bA = P3^4;
sbit bB = P3^5;
sbit bC = P3^6;
sbit bD = P3^7;
sbit bAnd = P3^3;



unsigned char Segundos = 5; //guarda os segundos a mostrar no D1
unsigned char DecimasSegundos = 0; //guarda as decimas de segundos mostrados no D2
unsigned int conta = 0;

unsigned char Start = 1; //variavel que indica se dá-se início à contagem decrescente

unsigned char resposta = 0; // interpreta a qual foi o botão presionado pelo utilizador 

unsigned char GuardaSegundos = 0; //guarda uma copia dos segundos 
unsigned char GuardaDecimasSegundos = 0; //guarda uma copia das decimas dos segundos



void Init(void); 
void mostraDisplay (unsigned char S, unsigned char D); 

void main(void)
{
	Init();
	
	while(1) //ciclo infinito
	{
		bAnd = bA && bB && bC && bD; //ISTO É TEMPORAL JÁ QUE SERÁ FEITO FISICAMENTE
		mostraDisplay (Segundos, DecimasSegundos);
	}
}

void Init(void)
{
	EA = 1; //ativar as interrupções
	ET0 = 1; //ativa interrupçõe do timer 0
	ET1 = 1; //ativa interrupções do timer 1
	EX0 = 1; //ativa interrupção externa 0
	EX1 = 0; //não ativa interrução externa 1
	
	

	IP = 0x00;//prioridades das interrupções
	

	//Timer0 - 10ms -> (65536 (10000h) - 10000 (2710h)) = 55536 (D8F0h)
	TH0 = TempoH;
	TL0 = TempoL;
	
	//Timer1 - 10ms -> (65536 (10000h) - 10000 (2710h)) = 55536 (D8F0h)
	TH1 = TempoH;
	TL1 = TempoL;
	

	TMOD &= 0xFF; //apaga o que estiver no TMOD
	TMOD |= 0x11; //Coloca os dois Timers no modo 1 (16 bits)

	TR0 = 0; //timer 0 desligado
	TR1 = 0; //timer 1 desligado
	
	//interrupções externas ativam quando o botão passa a ter valor lógico zero
	IT0 = 1; 
	IT1 = 1; 
}


void InterrupcaoExterna0 (void) interrupt 0  //foi presionado B1
{
	
	
	if (Start == 0) //caso estiver a contar a Zero, a contagem terá acabado e é preciso reiniciar a contagem
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
	else{  //caso contrario, inicia a contagem
		TR0 = 1; //timer 0 come�a a contar o tempo
		TR1 = 1; //timer 1 come�a a contar o tempo
		EX0 = 0; //desativa interrup��o externa 0
	}
}




void Interrupcao_Timer0(void) interrupt 1  //timer 0 usado para debounce de todos os botões
{
	TR0 = 0; //desativa o timer0
	
	//elimina o ruido do botão ao limpar as flags das interrupções externas
	IE0 = 0; 
	IE1 = 0;
	
	//ativa as interrupções externas
	EX0 = 1; 
	EX1 = 1;
	
	
	//faz reset ao timer 0
	//Timer 0 - 10ms -> (65536 (10000h) - 10000 (2710h)) = 55536 (D8F0h)
	TH0 = TempoH; 
	TL0 = TempoL;
}




void InterrupcaoExterna2 (void) interrupt 2 //foi presionado bA ou bB ou bC ou bD
{
	//TR1 = 1; //timer 1 come�a a contar o tempo
	//começa o debouce dos botões
	TR0 = 1; 
	EX1 = 0; 
	
	if (bA==0)
	{
		resposta= 'a'; 
	}
	
	if (bB==0)
	{
		resposta= 'b';
	}
	
	if (bC==0)
	{
		resposta= 'c';
	}
	
	if (bD==0)
	{
		resposta= 'd';
	}
	
	Start =0; //Start guarda a 0 já que para a contagem decresciva 
	GuardaSegundos = Segundos; //guarda uma copia dos segundos
	GuardaDecimasSegundos = DecimasSegundos; //guarda uma copia das decimas
}


void Interrupcao_Timer1 (void) interrupt 3
{
	if (Start==1) // se esta ativa a contagem decresciva
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
		
	
		if (Segundos == 0 && DecimasSegundos == 0) //caso os segundos e decimas de segundos chegarem a zero significa que não foi precionado nenhum botão de resposta 
		{
			GuardaSegundos = Segundos; //guarda uma copia dos segundos
			GuardaDecimasSegundos = DecimasSegundos; //guarda uma copia das decimas
			Start = 0;
			conta = 0;
		}
	}
	else // caso tenha acabado a contagem decresciva 
	{
		if (conta >= 100) //por cada segundo
		{
			if (Segundos == 10) //se os segundos(display1) mostrar um traço
			{
				Segundos = GuardaSegundos; //mostra os segundos 
				DecimasSegundos = GuardaDecimasSegundos; //mostra as decimas 
			}
			else //caso contrario
			{
				Segundos = 10; //o display 1 mostra um traço
				switch (resposta) //switch que interpreta a resposta introduzida
				{
					case 'a':
						DecimasSegundos = 11; //mostra A
						break;
					case 'b':
						DecimasSegundos = 12; //mostra B
						break;
					case 'c':
						DecimasSegundos = 13; //mostra C
						break;
					case 'd':
						DecimasSegundos = 14; //mostra D
						break;
					default:
						DecimasSegundos = 10; //mostra um traço
				}
			}
			conta = 0; //reinicia o contador
		}
		else //caso ainda não for contado nenhum segundo
		{
			++conta;
		}
	}
	
	
	//reinicia o timer1
	//Timer1 - 10ms -> (65536 (10000h) - 10000 (2710h)) = 55536 (D8F0h)
		TH1 = TempoH; 
		TL1 = TempoL;
	
}

//Fun��o para mostrar os caracteres no display

void mostraDisplay(unsigned char S, unsigned char D)
{
	
	//tabela com os valores a mostrar no display
	code unsigned display [26][7] = {  //é preciso atribuir ainda o valor para DP

		{0, 0, 0, 0, 0, 0, 1},//0
		{1, 0, 0, 1, 1, 1, 1},//1
		{0, 0, 1, 0, 0, 1, 0},//2
		{0, 0, 0, 0, 1, 1, 0},//3
		{1, 0, 0, 1, 1, 0, 0},//4
		{0, 1, 0, 0, 1, 0, 0},//5
		{0, 1, 0, 0, 0, 0, 0},//6
		{0, 0, 0, 1, 1, 1, 1},//7
		{0, 0, 0, 0, 0, 0, 0},//8
		{0, 0, 0, 0, 1, 0, 0},//9
		{1, 1, 1, 1, 1, 1, 0},//-
		{0, 0, 0, 1, 0, 0, 0},//A
		{1, 1, 0, 0, 0, 0, 0},//B
		{0, 1, 1, 0, 0, 0, 1},//C
		{1, 0, 0, 0, 0, 1, 0},//D
	};
	
	//mostra o display com 
	D1A = display[S][0];
	D1B = display[S][1];
	D1C = display[S][2];
	D1D = display[S][3];
	D1E = display[S][4];
	D1F = display[S][5];
	D1G = display[S][6];
	D1DP = 0; 							//mostra . no display
	
	D2A = display[D][0];
	D2B = display[D][1];
	D2C = display[D][2];
	D2D = display[D][3];
	D2E = display[D][4];
	D2F = display[D][5];
	D2G = display[D][6];
	D2DP = 1;								// não mostra o . no display
}
