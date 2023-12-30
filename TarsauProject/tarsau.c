#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>


#define MAX_FILENAME_LENGTH 256

typedef long int f_size;

typedef struct {
    char filename[MAX_FILENAME_LENGTH];
    char permissions[11]; 
    f_size size;
} FileInfo;
void createDirectory(const char *dirName){
    //create directory with permissions
 int result = mkdir(dirName, S_IRWXU | S_IRWXG | S_IRWXO);
   

}
//get file info using stat
void getfileInfo(const char *fileName, FileInfo *file) {
    struct stat fileStat;

    if (stat(fileName, &fileStat) == -1) {
        perror("Dosya bilgisi alınamadı");
        exit(1);
    }

    strcpy(file->filename, fileName);

  
    snprintf(file->permissions, sizeof(file->permissions), "%o", fileStat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));

    file->size = fileStat.st_size;
}

void mergeFiles(int nFiles, char *files[], char *outputName){
       f_size totalSize = 0;
      //creates an output file
        FILE *sau = fopen(outputName, "w");
       for (int i = 2; i < nFiles -2; i++) { 
        FileInfo file;
        getfileInfo(files[i], &file);
        totalSize += file.size;
    }

      if (!sau) {
        perror("Cannot open input");
    	}
	//save total size in 10 bytes
    fprintf(sau, "%010ld", totalSize);
    //get all file info
    for (int i = 2; i < nFiles -2; i++) {
        FileInfo file;
        getfileInfo(files[i], &file);

         fprintf(sau, "|%s, %s, %ld", file.filename, file.permissions, file.size);
 
    }
    printf("|");
    fprintf(sau,"|");
    //file content has been written here
    for (int i = 2; i < nFiles - 2; i++) { 
     	FileInfo file;
     	FILE *inputFile = fopen(files[i], "r");
        getfileInfo(files[i], &file);
        int fileSize = file.size;
	char* buffer = (char *)malloc(fileSize + 1);
	fread(buffer, 1, fileSize, inputFile);
	strcat(buffer, "\n");
        fwrite(buffer, 1, fileSize, sau);

        fclose(inputFile);
        free(buffer);
    }

       fclose(sau);
}


void extractFiles(char *archName, char *dirName) {
    createDirectory(dirName);
    
    FILE *sau = fopen(archName, "r");

    if (!sau) {
        perror("archive file cannot be open");
        return;
    }
//calculate the size of archive file
    fseek(sau, 0, SEEK_END);
    long fileSize = ftell(sau);
    fseek(sau, 0, SEEK_SET);
//allocate memory to read archive file
    char *buffer = (char *)malloc(fileSize + 1);

    if (!buffer) {
        perror("Memory allocation error");
        fclose(sau);
        return;
    }

    fread(buffer, 1, fileSize, sau);
    buffer[fileSize] = '\0';

    const char delimiters[] = "|,";

    char *token = strtok(buffer, delimiters);

    FileInfo fileInfo;
    FileInfo *fileInfoArray = NULL;  //dosya bilgilerinin array e kaydedilmesi
    int fileInfoCount = 0;
    int totalFileSize = 0;
    //file informations were read by using token according to '|' character
    while (token != NULL) {
        if (strstr(token, ".txt") != NULL) {
            strcpy(fileInfo.filename, token);

            token = strtok(NULL, delimiters);
            strcpy(fileInfo.permissions, token);

            token = strtok(NULL, delimiters);
            if (token != NULL && atoi(token) != 0) {
                fileInfo.size = atoi(token);
                totalFileSize += fileInfo.size;

                char *fileContent = (char *)malloc(fileInfo.size);
                if (!fileContent) {
                    perror("Memory allocation error");
                    fclose(sau);
                    free(buffer);
                    return;
                }

                fread(fileContent, 1, fileInfo.size, sau);

                

                free(fileContent);
            }

            
            fileInfoArray = realloc(fileInfoArray, (fileInfoCount + 1) * sizeof(FileInfo));
            fileInfoArray[fileInfoCount++] = fileInfo;
        }

        token = strtok(NULL, delimiters);
    }

//set size of each file inside archive
    int filesizes = fileSize;
    for(int i = fileInfoCount - 1; i >=0; --i){
    	filesizes += fileInfoArray[i].size;
    }
    

    int next = fileSize;

    for (int i = fileInfoCount - 1; i >= 0; i--) {
// directory/filename was created
  	char *archDir = (char *)malloc(strlen(fileInfoArray[i].filename) + strlen(dirName) + strlen(archName) + 1);

    sprintf(archDir, "%s/%s", dirName, fileInfoArray[i].filename);
    // Open destination file
    FILE *destFile = fopen(archDir, "wb");
	
	int incrs = next - fileInfoArray[i].size;

        for(int j = incrs; j < next; j++){
            fseek(sau, j, SEEK_SET);

            fputc(fgetc(sau), destFile);
        }
        next = incrs;
        fclose(destFile);
            free(archDir);
    }
    for(int i = fileInfoCount - 1; i >=0; --i){
    	printf("%s, ", fileInfoArray[i].filename);
    }
    printf("files opened in the %s directory", dirName );
  
    free(fileInfoArray);
    free(buffer);

    fclose(sau);
}



int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s file1 [file2 ...]\n", argv[0]);
        return 1;
    }

// tarsau -b txt1.txt txt2.txt -o tar.sau
if(strcmp(argv[1], "-b") == 0) {
    char *outputName = argv[argc-1]; 
    mergeFiles(argc, argv, outputName);
}
// tarsau -a tar.sau d1
else if(strcmp(argv[1], "-a") == 0){

 char *outputName = argv[argc-2]; 
 char *dirName = argv[argc-1];
       extractFiles(outputName, dirName);

}


    return 0;
}
