#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int noCustomers(int *ptrArray[], int arrLen){
    int sum = 0;
    for(int i = 0; i < arrLen; i++){
        int *ptrVal = (int *)ptrArray[i];
        sum = sum + *ptrVal;
    }
    return sum;
}

int main() {
   
    int no_of_tables;

    printf("Enter the Total Number of Tables at the Hotel:");
    scanf("%d",&no_of_tables);

    int sum = 0;
   
    int * shmptrArray[no_of_tables];
    int * flgshmptrArray[no_of_tables];
    int shmidArr[no_of_tables];
    int flgshmidArr[no_of_tables];
   
    for(int i = 0; i < no_of_tables; i++){
        key_t key = ftok("waiter.c", i+1);
        shmidArr[i] = shmget(key, sizeof(int), IPC_CREAT | 0644);
        shmptrArray[i] = (int*)shmat(shmidArr[i], NULL, 0);
        *(shmptrArray[i]) = -1;
        key_t flgkey = ftok("waiter.c", i+11);
        flgshmidArr[i] = shmget(flgkey, sizeof(int), IPC_CREAT | 0644);
        flgshmptrArray[i] = (int *)shmat(flgshmidArr[i], NULL, 0);
        *(flgshmptrArray[i]) = 0;
    }
   
    key_t adminkey = ftok("hotelmanager.c", 1);
    int adminshmid = shmget(adminkey, sizeof(int), IPC_CREAT | 0644);
    int *admin_terminate = (int*)shmat(adminshmid, NULL, 0);
   
    FILE *earningsFile;
    earningsFile = fopen("expencefile.txt", "w+");
   
    // printf("Enter the Total Number of Tables at the Hotel:");
    // scanf("%d",&no_of_tables);
   
    do {
        for(int i = 0; i < no_of_tables; i++){
            int *shmPtrAdd = (int *)shmptrArray[i];
            if(*shmPtrAdd==-1||*shmPtrAdd==-2){
               *flgshmptrArray[i] = *admin_terminate;
            }
            else{
                fprintf(earningsFile, "Earning from Table %d: %d INR\n", i+1,*shmPtrAdd);
                sum = sum + *shmPtrAdd;
                *flgshmptrArray[i] = *admin_terminate;
                *shmPtrAdd=-1;
            }
        }   
    }while(!(*admin_terminate==-1 && noCustomers(shmptrArray, no_of_tables)==-2*no_of_tables));

   
    for(int i = 0; i < no_of_tables; i++){
      if(shmdt(shmptrArray[i]) ==-1){
          perror("Error in detaching shmptr1");
          return 1;
      }
      if(shmdt(flgshmptrArray[i]) ==-1){
          perror("Error in detaching shmptr1");
          return 1;
      }
    }
   
    for(int i = 0; i < no_of_tables; i++){
      if(shmctl(shmidArr[i], IPC_RMID, 0) ==-1){
        perror("Error is shmidctl\n");
        return 1;
      }
      if(shmctl(flgshmidArr[i], IPC_RMID, 0) ==-1){
        perror("Error is flagctl\n");
        return 1;
      }
    }
   
    if(shmdt(admin_terminate) ==-1){
          perror("Error in detaching shmptr1");
          return 1;
      }
   
    if(shmctl(adminshmid, IPC_RMID, 0) ==-1){
        perror("Error is adminctl\n");
        return 1;
      }
   
    fprintf(earningsFile, "Total Earnings of Hotel: %d INR\n", sum);
    fprintf(earningsFile, "Total Wages of Waiters: %f INR\n", 0.4*sum);
    fprintf(earningsFile, "Total Profit: %f INR\n", 0.6*sum);
   
    printf("Total Earnings of Hotel: %d INR\n", sum);
    printf("Total Wages of Waiters: %f INR\n", 0.4*sum);
    printf("Total Profit: %f INR\n", 0.6*sum);
    printf("Thank you for visiting the Hotel!\n");
 
   
    return 0;
   
}
