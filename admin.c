#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>



int main() {
    // Create shared memory segment for termination flag
    key_t key = ftok("hotelmanager.c", 1);
    if(key==-1)
    {
        perror("Error in ftok ");
        return 1;
    }
    //attach the shared memory segment to parent process
    int shmid = shmget(key, sizeof(int), IPC_CREAT | 0644);
    if (shmid==-1)
    {
        perror("Error in shmget");
        return 1;
    }
    void *shmPtr = shmat(shmid, NULL, 0);

   if(shmPtr == (void *) -1)
   {
     perror("Error in shmPtr in attaching the memory segment\n");
     return 1;
   }
   int *msgPtr=(int*)shmPtr;
   *msgPtr=0;
   
    char choice;
    while (1) {
        printf("Do you want to close the hotel? Enter Y for Yes and N for No: ");
        scanf(" %c", &choice);

        if (choice == 'Y' || choice == 'y') {
            *msgPtr=-1;
            
            if(shmdt(shmPtr)==-1)
            {
                perror("Error in shmdt in detaching the memory segment\n");
                return 1;
            }
            return 0;
        }
        else if (choice == 'N' || choice == 'n')
        {
            printf("%d", *msgPtr);
        }
        else
        {
            printf("Invalid input please enter Y or N   \n");
        }
    }

}
