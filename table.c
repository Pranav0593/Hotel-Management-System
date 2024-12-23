#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_CUSTOMERS 5
#define MAX_ITEMS 50
#define MENU_FILE "menu.txt"
#define READ_END 0
#define WRITE_END 1
#define SHM_SIZE 54
#define WAITER 50
#define TOTAL_BILL 51
#define ORDER_ERROR 52
#define TERMINATE 53

void displayMenu(char menu[][50], int prices[], int numItems) {
    printf("Menu:\n");
    for (int i = 0; i < numItems; i++)
    {
        printf("%d. %s %d INR\n", i + 1, menu[i], prices[i]);
    }
}

int main() {
    int tableNumber, numberOfCustomers;
    int fd[MAX_CUSTOMERS][2];    //file descriptors
   

    printf("Enter Table Number: ");
    scanf("%d", &tableNumber);

     //creating shared memory corresponding to communicate the order to waiter
        key_t key = ftok("table.c", tableNumber);
        int shmid = shmget(key, sizeof(int)*54, IPC_CREAT | 0666);
   	
        if (shmid == -1)
        {
            perror("Error creating shared memory");
            return 1;
        }
       
        void *tablePointer=shmat(shmid, NULL, 0); //kernel will find the place for the shmptr in attaching the memory
        int *shmPtr=(int*)tablePointer;   //type casting to int
        if(shmPtr==(int*)-1)
        {
            perror("Error in shmPtr in atttaching the memory");
            return 1;
        }
	
	//initializing the shared memory with zeros
        for(int i=0;i<SHM_SIZE;i++)
        {
            shmPtr[i]=0;
        }
        shmPtr[ORDER_ERROR]=-1;
        shmPtr[TERMINATE]=0;
   
   //gathering data from the file into respective arrays
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
            sscanf(buffer, "%*d. %49[^0-9] %d INR", menu[i], &prices[i]);
        }

        fclose(menuFile);
 

    while(shmPtr[TERMINATE]==0)
    {
        printf("Enter Number of Customers at Table (maximum no. of customers can be 5): \n");
        scanf("%d", &numberOfCustomers);
        if (numberOfCustomers ==-1)
            {  
                shmPtr[TERMINATE]=-1;
                printf("No more customers\n");
                break;
               
            }
        while (shmPtr[ORDER_ERROR]==-1)
        {
            pid_t pid;
            // Create pipes and fork customer processes
     
            for (int i = 0; i < numberOfCustomers; i++)
            {  
                if (pipe(fd[i]) == -1)
                {
                    perror("Error creating pipe for customer process");
                    return 1;
                }

                pid= fork();
           
                if (pid < 0)
                {
                    perror("Error forking customer process");
                    return 1;
                }
                // Child process (customer)
                else if (pid == 0)
                {          
                    close(fd[i][READ_END]); // Close read end of the pipe
               
                    int menuItem;
                    int orderArray[10];
                    for(int i=0; i<10; i++)
                    {
                        orderArray[i]=0;
                    }
                    int orderArrayIndex=0;
                    int countOrders=0;
                    displayMenu(menu, prices, numItems);
                    printf("Enter the serial number(s) of the item(s) to order from the menu. Enter -1 when done: \n");

                    while (1)
                    {

                        if (orderArrayIndex== 10)
                        {
                            break;
                        }
                       
                        scanf("%d", &menuItem);

                        if (menuItem == -1)
                        {
                            break;
                        }

                       

                        else
                        {
                        orderArray[orderArrayIndex]=menuItem;
                        orderArrayIndex ++;
                        countOrders ++;                  
                        }
                    }
                   
                    // Send order to table process through pipe
                    write(fd[i][WRITE_END], orderArray, 10*sizeof(int));

               
                    close(fd[i][WRITE_END]); // Close write end of the pipe
                    return 0;
                }
                // Parent process (table)  
                else
                {
                    wait(NULL); //waits for all child programs to terminate
                    close(fd[i][WRITE_END]); // Close write end of the pipe
               
                }

            }
           
                int reciveArr[10];
                int totalorders[MAX_ITEMS];
               
                for(int i=0;i<MAX_ITEMS;i++)
                {
                    totalorders[i]=0;
                }    
                int totalorderIndex=0;
           
            //size of reciveArr is 10 bcz max orders a customer can orde is 10
                for(int i=0;i< numberOfCustomers;i++)
                {
                    for(int k=0; k<10; k++)
                    {
                        reciveArr[k]=0;
                    }
                    read(fd[i][READ_END],reciveArr,10*sizeof(int)); //reading from the pipe
               
               
                    for(int j=0;j<10;j++)
                    {
                        totalorders[totalorderIndex]=reciveArr[j];
                        totalorderIndex ++;
                    }
               
                }
               
                //closing the read end of the pipe at the parent process
                for(int i=0;i<numberOfCustomers;i++)
                    close(fd[i][READ_END]);

                for(int i=0;i<MAX_ITEMS;i++)
                {
                    shmPtr[i]=totalorders[i];
                }

                shmPtr[WAITER]=1;

                while (shmPtr[WAITER]==1);

                if(shmPtr[TOTAL_BILL]>=0)
                {
                    printf("THE BILL AMOUNT FOR TABLE %d : %d\n",tableNumber,shmPtr[TOTAL_BILL]);
                    for(int i=0;i<=ORDER_ERROR;i++)
                    {
                        shmPtr[i]=0;
                    }
                   
                }
                else
                {
                    for(int i=0;i<=TOTAL_BILL;i++)
                    {
                        shmPtr[i]=0;
                    }
		   
                    shmPtr[TERMINATE]=0;
                    printf("Wrong order chosen give the order again \n");

                }
        }

        if(shmPtr[TERMINATE]==-1)
        {
            break;
        }        
       
        else
         {
            
                shmPtr[ORDER_ERROR]=-1;
            
    	}
   }
    shmPtr[WAITER]=1;


       
   
    //detach from the shared memory segment  
    shmdt(shmPtr);
   
    return 0;
}
