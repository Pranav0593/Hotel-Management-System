#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//task-1-take inputs
//task-2-attach waiter process to shm
//task-3-check for valid order
//task-4-ask for retransmission
//task-5-calculate bill and return
//task-6 -termination

#define MAX_ITEMS 50
#define MENU_FILE "menu.txt"
#define READ_END 0
#define WRITE_END 1
#define SHM_SIZE 54

//INDEX OF ELEMENTS

#define WAITER 50
#define TOTAL_BILL 51
#define ORDER_ERROR 52
#define TERMINATE 53
#define MAX_SET_CUSTOMER 100

int main()
{   //int itemsRequested[MAX_ITEMS];
    int waiterID;

    printf("Enter Waiter ID: ");
    scanf("%d",&waiterID);

    int totalBill = 0;
   
    //Shared memory between waiter and manager to send the totalbills
    key_t key2 = ftok("waiter.c", waiterID);
    int shmid2 = shmget(key2,sizeof(int), IPC_CREAT | 0644);
   
   
    if (shmid2 == -1)
    {
        perror("Error creating shared memory");
        return 1;
    }
   
     int *shmPtr2=(int*)shmat(shmid2, NULL, 0);
     if(shmPtr2==(int*)-1)
     {
        perror("Error in shmPtr in atttaching the memory");
        return 1;
     }

     //Shared memory between waiter and manager to send the closing signal
    key_t key3 = ftok("waiter.c", waiterID+10);
    int shmid3 = shmget(key3,sizeof(int), IPC_CREAT | 0644);
   
   
   
   
    if (shmid3 == -1)
    {
        perror("Error creating shared memory");
        return 1;
    }
     int *shmPtr3=(int*)shmat(shmid3, NULL, 0);
     if(shmPtr3==(int*)-1)
     {
        perror("Error in shmPtr in atttaching the memory");
        return 1;
     }
     
     
     //table-waiter shm  
   
    key_t key1 = ftok("table.c", waiterID);
    int shmid1 = shmget(key1, SHM_SIZE*sizeof(int), IPC_CREAT | 0644); // 54 locations
   
   
     int *shmPtr1=(int *)shmat(shmid1, NULL, 0); //kernel will find the place for the shmptr in attaching the memory
     if(shmPtr1==(int*)-1)
     {
        perror("Error in shmPtr in atttaching the memory");
        return 1;
     }
   
    //getting data from file
    FILE *menuFile = fopen(MENU_FILE, "r");
    if (menuFile == NULL)
    {
        printf("Error opening menu file.\n");
        return 1;
    }

    // Count the number of items in the menu file
    int numItems = 0;
    char buffer[100];
    while (fgets(buffer, sizeof(buffer), menuFile) != NULL)
    {
        numItems++;
    }

    // returning the file pointer back to the start
    rewind(menuFile);

     // Read menu items
    char menu[numItems][50];
    int prices[numItems];
    for (int i = 0; i < numItems; i++)
    {
        fgets(buffer, sizeof(buffer), menuFile);
        sscanf(buffer, "%*d. %99[^0-9] %d INR", menu[i], &prices[i]);
    }

    fclose(menuFile);
   
   
    int *msgPtr=shmPtr1;
   
    do 
    {  
        while (shmPtr1[WAITER] != 1);

       

        for(int i=0;i<MAX_ITEMS;i++)
        {
            if(msgPtr[i]>0 && msgPtr[i]< numItems + 1)
            {
                totalBill+=prices[msgPtr[i]-1];
            }
            else if(msgPtr[i]<0 || msgPtr[i]>numItems)
            {
               
               totalBill=-1;
               break;
               
            }
        }

        //Updating the bill to manager if the order is valid

        if(totalBill!=-1)
        {
            *shmPtr2=totalBill;
            while(*shmPtr2 != -1);
        }
       
        shmPtr1[TOTAL_BILL] = totalBill;
        totalBill = 0;

        shmPtr1[TERMINATE] = *shmPtr3;

        shmPtr1[WAITER] = 0;
       
        while(shmPtr1[WAITER] != 1);


    }while(shmPtr1[TERMINATE] !=-1);
   
    *shmPtr2 = -2;  //INDICATION to table THAT TABLE HAS TERMINATED


    // detach
    if(shmdt(shmPtr2)==-1){
        perror("Error in detaching shmptr2");
        return 1;
    }
    if(shmdt(shmPtr3)==-1){
        perror("Error in detaching shmptr3");
        return 1;
    }

    if(shmdt(shmPtr1) ==-1){
        perror("Error in detaching shmptr1");
        return 1;
    }



    if(shmctl(shmid1, IPC_RMID, 0) ==-1){
        perror("Error is shmctl\n");
        return 1;
    }

    return 0;
}
