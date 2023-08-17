#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//CSC 322 Project 1 - Hyun Kim

#define MAX_LENGTH 127 //Maximum length of Boat Name
#define MAX_POINTERS 120 //Maximum number of pointers
#define SLIP_RATE 12.50 //Slip monthly rate per foot
#define LAND_RATE 14.00 //Land monthly rate per foot
#define TRAILOR_RATE 25.00 //Trailor monthly rate per foot
#define STORAGE_RATE 11.20 //Storage monthly rate per foot

//Define String type
typedef char String[MAX_LENGTH];

//Define enum about Boat Place Type
typedef enum { 
    slip,
    land,
    trailor,
    storage
} PlaceType;

//Define union about Boat ExtraInfo 
typedef union { 
    int SlipNumber;
    char BayLetter;
    String TrailorLicense;
    int StorageSpace;
} ExtraInfo;

//Define Boat struct
typedef struct { 
    String Name; 
    int Length;
    PlaceType BoatPlace;
    ExtraInfo TypeInfo; 
    float MoneyOwed; 
} Boat;

//Define an array of boat pointers 
typedef Boat* BoatManagement[MAX_POINTERS];

//Function prototyping
PlaceType StringToPlaceType(char* PlaceString);
char* PlaceToString(PlaceType Place);
int LoadCSVData(Boat** BoatDB, char* DBFile);
void* Malloc(size_t size);
int CompareBoat(const void * FirstBoat, const void * SecondBoat);
int AddBoat(Boat** BoatDB, int DBSize, char* BoatData);
int DisplayMenu(Boat** BoatDB, int DBSize);
char GetUserOption();
void PrintDatabase(Boat** BoatDB, int DBSize);
int FindBoat(Boat** BoatDB, int DBSize);
int RemoveBoat(Boat** BoatDB, int DBSize);
void MakePayment(Boat** BoatDB, int DBSize);
void UpdateMoneyOwed(Boat** BoatDB, int DBSize);
void SaveCSVData(Boat** BoatDB, int DBSize, char* FileName);
void FreeBoatMemory(Boat** BoatDB, int DBSize);

int main(int argc, char* argv[]) {

    BoatManagement BoatDatabase; //Database (array) of boat pointers 
    int DatabaseSize; //Size (number) of pointers actually pointing to a boat struct
    String DatabaseFile; //Name of CSV file containing boat data
    
    if(argc == 2) { //If command line argument provides boat data in CSV file
        
        //Assign file name from command line argument
        strcpy(DatabaseFile,argv[1]);
        
        //Load boat data from CSV file and update database size 
        DatabaseSize = LoadCSVData(BoatDatabase, DatabaseFile);
        
        //Display menu to the user and update database size based on user option
        DatabaseSize = DisplayMenu(BoatDatabase, DatabaseSize);
        
        //Save CSV Data into the same file where boat data was loaded from
        SaveCSVData(BoatDatabase, DatabaseSize, DatabaseFile);
        
        //Free allocated memory for all boat structs and print exit message
        FreeBoatMemory(BoatDatabase, DatabaseSize);
    }
    
    //If there are too less arguments, print error message and exit
    else if(argc < 2) { 
        perror("Failed to load csv file: Boat file not provided.\n");
        exit(EXIT_FAILURE);
    }
    
    else { //If there are too many arguments, print error message and exit
        perror("Failed to load csv file: Too many arguments.\n");
        exit(EXIT_FAILURE);
    }
    
    return(EXIT_SUCCESS);
}

//Function to convert String to PlaceType enum
PlaceType StringToPlaceType(char* PlaceString) {
    if(!strcasecmp(PlaceString,"slip")) {
        return(slip);
    }
    if(!strcasecmp(PlaceString,"land")) {
        return(land);
    }
    if(!strcasecmp(PlaceString,"trailor")) {
        return(trailor);
    }
    if(!strcasecmp(PlaceString,"storage")) {
        return(storage);
    }
    //Print error message and exit if place type is invalid 
    perror("ERROR: Invalid place type.");
    exit(EXIT_FAILURE);
}

//Function to convert PlaceType enum to String
char* PlaceToString(PlaceType Place) {
    switch(Place) {
        case slip:
            return("slip");
        case land:
            return("land");
        case trailor:
            return("trailor");
        case storage:
            return("storage");
        default: 
            //Print error message and exit if place type is invalid 
            perror("ERROR: Invalid place type.");
            exit(EXIT_FAILURE);
    }
}

//Function to load CSV boat data and return updated database size
int LoadCSVData(Boat** BoatDB, char* DBFile) {
    
    //Open and read CSV file of boat data
    FILE *InputCSV = fopen(DBFile, "r"); 
    String BoatData; //Line of boat data in CSV format
    int DBSize = 0; //Initialize size to 0 since no boat pointers are used
    
    //Check file open error
    if(InputCSV == NULL) {
        perror("ERROR: Failed to open file.\n");
        exit(EXIT_FAILURE);
    }

    //Add boats based on each line of data read from file and update DBSize
    while(fgets(BoatData, sizeof(BoatData), InputCSV) != NULL) {
        DBSize = AddBoat(BoatDB, DBSize, BoatData);
    }
    
    //Close the file input stream and check file close error
    if(fclose(InputCSV) == EOF) {
        perror("ERROR: Failed to close file.");
        exit(EXIT_FAILURE);
    }
    
    return DBSize; //Return updated database size 
}

//Function to check malloc error and return pointer to malloced memory
void* Malloc(size_t size) {

    void* Memory;

    //Check malloc error and print error message if malloc fails
    if((Memory = malloc(size)) == NULL) {
        perror("ERROR: Failed to malloc.");
        exit(EXIT_FAILURE);
    }

    return Memory; //Return pointer of the malloced memory
}

//Function passed to qsort as a parameter to sort boats based on their name
int CompareBoat(const void * B1, const void * B2) {

    //Names of two boats for comparing and sorting boat pointers
    char* FirstBoat = (*(Boat**)B1)->Name; 
    char* SecondBoat = (*(Boat**)B2)->Name; 

    //Return the string comparison result of the two boats' names 
    return strcmp(FirstBoat, SecondBoat); 
}

//Function to create new boat struct and add its pointer to the boat database
int AddBoat(Boat** BoatDB, int DBSize, char* BoatData) {
    
    //Allocate memory for a new boat 
    Boat* NewBoat = (Boat*)Malloc(sizeof(Boat));
    
    //Separate CSV data using strtok and assign data to boat variables
    strcpy(NewBoat->Name, strtok(BoatData,",")); 
    NewBoat->Length = atoi(strtok(NULL, ",")); 
    NewBoat->BoatPlace = StringToPlaceType(strtok(NULL, ",")); 

    //Assign extra information of the boat based on its place type
    switch(NewBoat->BoatPlace) {
        case slip:
            NewBoat->TypeInfo.SlipNumber = atoi(strtok(NULL, ","));
            break;
        case land:
            NewBoat->TypeInfo.BayLetter = strtok(NULL, ",")[0];
            break;
        case trailor:
            strcpy(NewBoat->TypeInfo.TrailorLicense, strtok(NULL, ","));
            break;
        case storage:
            NewBoat->TypeInfo.StorageSpace = atoi(strtok(NULL, ","));
            break;
        default: //Print error message if the boat's place type is invalid
            perror("ERROR: Invalid place type.");
            printf("Failed to assign extra information for %s\n", NewBoat->Name);
    }

    NewBoat->MoneyOwed = atof(strtok(NULL, ",")); 
    BoatDB[DBSize] = NewBoat; //Store pointer to the new boat in the database
    DBSize++; //Increment DBSize as each new boat pointer is stored
    
    //Sort the boat pointers based on the boat names 
    qsort(BoatDB, DBSize, sizeof(NewBoat), CompareBoat);
 
    return DBSize; //Return updated DBSize
}

//Function to display menu and return updated database size based on user option
int DisplayMenu(Boat** BoatDB, int DBSize) { 

    char UserOption; 
    String BoatData; //Line of boat data in CSV format
        
    //Display welcome message to the user
    printf("Welcome to the Boat Management System\n");
    printf("-------------------------------------\n");

    do {

        UserOption = GetUserOption(); //Receive user option for menu

        if (UserOption == 'I') {
            PrintDatabase(BoatDB, DBSize); //Print boat inventory
        }

        if (UserOption == 'A') {
            printf("Please enter the new boat CSV data\t\t\t: ");
            fgets(BoatData, sizeof(BoatData), stdin);
            //Add a boat based on its CSV data and update database size
            DBSize = AddBoat(BoatDB, DBSize, BoatData);
        }

        if (UserOption == 'R') {
            //Remove a boat and update database size
            DBSize = RemoveBoat(BoatDB, DBSize);
        }

        if (UserOption == 'P') {
            MakePayment(BoatDB, DBSize); //Make payment for a boat
        }
            
        if (UserOption == 'M') {
            //Update the monthly amount of money owed for all boats
            UpdateMoneyOwed(BoatDB, DBSize);
        }

    } while (UserOption != 'X'); //Terminate if user option is X

    return DBSize; //Return updated database size
}

//Function to get and return user option
char GetUserOption() {

    String Input; //User input
    char Option; //User option (first char of user input)

    printf("\n(I)ventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
    fgets(Input, sizeof(Input),stdin); //Get user input
    Option = toupper(Input[0]); //Make first char of user input to uppercase

    //Check size and first char of input to detect if user input is invalid
    while(strlen(Input) > 2 || !(Option == 'I' || Option == 'A' || Option == 'R'|| 
            Option == 'P' || Option == 'M' || Option == 'X')) {
        
        printf("Invalid option %s", Input);
        printf("\n(I)ventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        fgets(Input, sizeof(Input),stdin);
        Option = toupper(Input[0]);
    }

    return Option; //Return user option
}

//Function to print all boats in the database
void PrintDatabase(Boat** BoatDB, int DBSize) {
    
    int Index;
    for(Index = 0; Index < DBSize; Index++) {
        printf("%-20s %d'", BoatDB[Index]->Name, BoatDB[Index]->Length);
        printf("%8s", PlaceToString(BoatDB[Index]->BoatPlace));
        
        //Print extra information of a boat based on its place type
        switch(BoatDB[Index]->BoatPlace) {
            case slip:
                printf("   # %2d", BoatDB[Index]->TypeInfo.SlipNumber);
                break;
            case land:
                printf("%7c", BoatDB[Index]->TypeInfo.BayLetter);
                break;
            case trailor:
                printf("%7s", BoatDB[Index]->TypeInfo.TrailorLicense);
                break;
            case storage:
                printf("   # %2d", BoatDB[Index]->TypeInfo.StorageSpace);
                break;
            default: //Print error message if the boat's place type is invalid 
                perror("ERROR: Invalid place type."); 
                printf("Failed to locate extra info. of %s\n", BoatDB[Index]->Name);
        }   

        printf("   Owes $%7.2f\n", BoatDB[Index]->MoneyOwed);
    }
}

//Function to find the index of the boat pointer in the database
int FindBoat(Boat** BoatDB, int DBSize) {
    
    String BoatToFind; //Name of the boat to find
    int Index;
    
    printf("Please enter the boat name\t\t\t\t: ");
    //Get boat name and remove the new line at the end
    fgets(BoatToFind, sizeof(BoatToFind), stdin); 
    BoatToFind[strlen(BoatToFind)-1] = '\0'; 

    //Loop to find the index of the boat pointer by boat name
    for(Index = 0; Index < DBSize; Index++) {
        //If the name matches regardless of case
        if(strcasecmp(BoatDB[Index]->Name, BoatToFind) == 0) {
            return Index; //Return the index of the boat pointer 
        }
    }
    
    printf("No boat with that name\n");
    return -1; //Return -1 if the boat is not found
}

//Function to remove boat and return updated database size
int RemoveBoat(Boat** BoatDB, int DBSize) {

    //TempBoat used to store the boat pointer to remove
    Boat* TempBoat;
    int BoatIndex = FindBoat(BoatDB, DBSize);    

    if(BoatIndex != -1) { //If the boat is found
        
        //Store the boat pointer to remove in TempBoat
        TempBoat = BoatDB[BoatIndex]; 
        BoatDB[BoatIndex] = NULL;

        //Remove the boat pointer and free allocated memory
        free(TempBoat); 

        //Shift boat pointers to the left to fill in empty spot
        while(BoatIndex < DBSize) {
            BoatDB[BoatIndex] = BoatDB[BoatIndex + 1];
            BoatIndex++;
        }

        DBSize--; //Decrement database size
    }
    
    return DBSize; //Return updated database size
}

//Function to make payment on a boat
void MakePayment(Boat** BoatDB, int DBSize) {
    
    float Payment;
    int BoatIndex = FindBoat(BoatDB, DBSize);
    
    if(BoatIndex != -1) { //If the boat is found
        printf("Please enter the amount to be paid\t\t\t: ");
        scanf("%f", &Payment);
        getchar(); //Get and discard the new line character in the buffer
        
        //Check if payment exceeds the money owed for the boat
        if((BoatDB[BoatIndex]->MoneyOwed - Payment) >= 0) {
            //Update money owed for the boat
            BoatDB[BoatIndex]->MoneyOwed -= Payment; 
        }
        
        else {  
            printf("That is more than the amount owed, $%.2f\n",
                    BoatDB[BoatIndex]->MoneyOwed);
        }
    }
}

//Function to update the monthly amount of money owed for all boats
void UpdateMoneyOwed(Boat** BoatDB, int DBSize) {
    
    int Index;
    for(Index = 0; Index < DBSize; Index++) {
        //Update amount of money owed for a boat based on its place type
        switch (BoatDB[Index]->BoatPlace) {
            case slip:
                BoatDB[Index]->MoneyOwed += BoatDB[Index]->Length * SLIP_RATE;
                break;
            case land:
                BoatDB[Index]->MoneyOwed += BoatDB[Index]->Length * LAND_RATE;
                break;
            case trailor:
                BoatDB[Index]->MoneyOwed += BoatDB[Index]->Length * TRAILOR_RATE;
                break;
            case storage:
                BoatDB[Index]->MoneyOwed += BoatDB[Index]->Length * STORAGE_RATE;
                break;
            default: //Print error message if place type is invalid 
                perror("ERROR: Invalid place type.");
                printf("Failed to update monthly payment for %s\n", BoatDB[Index]->Name);
        }
    }
}

//Function to record and save data into the same CSV file used to load data
void SaveCSVData(Boat** BoatDB, int DBSize, char* DBFile) {

    //Open and write to CSV file of boat data
    FILE *OutputCSV = fopen(DBFile, "w"); 
    int Index;
        
    //Check file open error
    if(OutputCSV == NULL) {
        perror("ERROR: Failed to open file.\n");
        exit(EXIT_FAILURE);
    }
    
    //Write data of all boats to the CSV file in CSV format
    for(Index = 0; Index < DBSize; Index++) {  
        fprintf(OutputCSV,"%s,%d,", BoatDB[Index]->Name, BoatDB[Index]->Length);
        fprintf(OutputCSV, "%s,", PlaceToString(BoatDB[Index]->BoatPlace));

        //Write extra information of a boat based on its place type
        switch(BoatDB[Index]->BoatPlace) {
            case slip:
                fprintf(OutputCSV, "%d,", BoatDB[Index]->TypeInfo.SlipNumber);
                break;
            case land:
                fprintf(OutputCSV, "%c,", BoatDB[Index]->TypeInfo.BayLetter);
                break;
            case trailor:
                fprintf(OutputCSV, "%s,", BoatDB[Index]->TypeInfo.TrailorLicense);
                break;
            case storage:
                 fprintf(OutputCSV, "%d,", BoatDB[Index]->TypeInfo.StorageSpace);
                break;
            default: //Print error message if the boat's place type is invalid 
                perror("ERROR: Invalid place type.");
                printf("Failed to locate extra info. about %s\n", BoatDB[Index]->Name);
                fprintf(OutputCSV, "NULL, "); //Write NULL in the CSV file 
        }

        fprintf(OutputCSV, "%.2f\n", BoatDB[Index]->MoneyOwed);
    }
    
    //Close the file output stream and check file close error
    if(fclose(OutputCSV) == EOF) {
        perror("ERROR: Failed to close file.");
        exit(EXIT_FAILURE);
    }
}

//Function to free memory of all boats and display exit message
void FreeBoatMemory(Boat** BoatDB, int DBSize) {

    int Index;

    //Loop to free memory allocated to all boats
    for(Index = 0; Index < DBSize; Index++) {
        free(BoatDB[Index]);
    } 

    //Display exit message after freeing all allocated memory
    printf("\nExiting the Boat Management System\n");
}
