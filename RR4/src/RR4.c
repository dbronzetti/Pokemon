/* Round Robin algorithm for CPU scheduling ( considering arrival time  and maintains a queue  as well)
Ref. Operating Systems by william stallings
For more programs from OS   => Coding Is Simplified
 */

#include<stdio.h>

struct process //prototipo estructura de proceso
	{
	char name;
	int at,bt,wt,tt,rt; //arrivalTime, burstTime, waitingTime, turnArroundTime, remainigTime
	int completed; //tiempo procesador en el cual finalizo
	float ntt;
	}p[10];

int n;
int q[10];  //cola en la que se van a ir subiendo y bajando procesos
int front=-1,rear=-1;

void enqueue(int i) //funcion encolar
	{
	if(rear==10)
		printf("overflow"); //si la cola tiene 10 procesos, overflow

	rear++;	//si no es el primero en la cola, sumar detras del ultimo
	q[rear]=i;

	if(front==-1) //si es el primero en la cola
		front=0;

	}

int dequeue()
	{
	if(front==-1)
		printf("underflow"); //si la cola esta vacia, underflow

	int temp=q[front];

	if(front==rear)
		front=rear=-1; //si en la cola hay solo 1 elemento
		else
			front++;

	return temp;
	} //me retorna el primer proceso que esta en la cola

int isInQueue(int i)
	{
	int k;
	for(k=front;k<=rear;k++)
		{
		if(q[k]==i)
		return 1;
		}
	return 0;
	}

void sortByArrival()
	{
	struct process temp;
	int i,j;
	for(i=0;i<n-1;i++)
	for(j=i+1;j<n;j++)
		{
		if(p[i].at>p[j].at)
			{
			temp=p[i];
			p[i]=p[j];
			p[j]=temp;
			}
		}
	}

void main()
	{
	int i,j,time=0,sum_bt=0,tq;
	char c;
	float avgwt=0; //average waitingTime

	// printf("Enter no of processes:");
	 //scanf("%d",&n);

	 n=4;
	/* for(i=0,c='A';i<n;i++,c++)
		 {
		 p[i].name=c;

		 printf("Enter the arrival time and burst time of process %c: \n",p[i].name);
		 scanf("%d%d",&p[i].at,&p[i].bt);

		 p[i].rt=p[i].bt;
		 p[i].completed=0;
		 sum_bt+=p[i].bt;   //sum_bt = sum_bt + P[i].bt

		} */
	 //le hardcodeo las estructuras
	 p[0].at=0;
	 p[0].bt=9;
	 p[0].name="A";
	 p[0].rt=p[0].bt;
	 p[0].completed=0;
	 sum_bt+=p[0].bt;

	 p[1].at=1;
	 p[1].bt=5;
	 p[1].name="B";
	 p[1].rt=p[1].bt;
	 p[1].completed=0;
	 sum_bt+=p[1].bt;

	 p[2].at=2;
	 p[2].bt=3;
	 p[2].name="C";
	 p[2].rt=p[2].bt;
	 p[2].completed=0;
	 sum_bt+=p[2].bt;

	 p[3].at=3;
	 p[3].bt=4;
	 p[3].name="D";
	 p[3].rt=p[3].bt;
	 p[3].completed=0;
	 sum_bt+=p[3].bt;

	//printf("Enter the time quantum:\n");
	//scanf("%d",&tq);
	tq=3;

	sortByArrival();	// ordenar procesos por orden de llegada
	enqueue(0);          // encolar el primer proceso

	//printf("Process execution order: "); //aca comienza GANTT

	for(time=p[0].at;time<sum_bt;)       // run until the total burst time reached
		{
		i=dequeue(); //saco de la cola el primer proceso

		if(p[i].rt<=tq)	/* para procesos cuyo remain es igual o menor al quantum */
			{

			time+=p[i].rt; // time= time + p[i].rt
			p[i].rt=0; //ya no le queda burst
			p[i].completed=1; //proceso completo

			printf(" %c ",p[i].name);

			p[i].wt=time-p[i].at-p[i].bt;
			p[i].tt=time-p[i].at;
			p[i].ntt=((float)p[i].tt/p[i].bt);

			for(j=0;j<n;j++)       /*enqueue the processes which have come                                         while scheduling */
				{
				if(p[j].at<=time && p[j].completed!=1 && isInQueue(j)!=1)
					{  enqueue(j); }
				}
			}
			else     // aca entrar procesos con BT mayores al quantum
				{
				time+=tq; //time = time + tq
				p[i].rt-=tq;

				printf(" %c ",p[i].name);

				for(j=0;j<n;j++)    /*first enqueue the processes which have come while scheduling */
					{
					if(p[j].at<=time && p[j].completed!=1&&i!=j&& isInQueue(j)!=1)
						 { enqueue(j); }
					}

				enqueue(i);   // encolar el proceso no finalizado

				}
		}

	//imprimir resultados
	printf("\nName\tArrival Time\tBurst Time\tWaiting Time\tTurnAround Time\t Normalized TT");
	for(i=0;i<n;i++)
		{
		avgwt+=p[i].wt; // avgwt = avgwt + P[i].wt
		printf("\n%c\t\t%d\t\t%d\t\t%d\t\t%d\t\t%f",p[i].name,p[i].at,p[i].bt,p[i].wt,p[i].tt,p[i].ntt);
		}

	printf("\nAverage waiting time:%f\n",avgwt/n);
}
