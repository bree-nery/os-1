#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "parser.h"
#include "openF.h"
#include "handler.h"

int verboseTrue = 0; 
int exitStatus = 0;

int status_list[100];
int statusCount = 0;

void add_status(int st)
{
    status_list[statusCount] = st; 
    statusCount++;
}
 
int parser(int argc, char** argv)
{
    int loop = 1;
    while (loop)
    {
        int a;  
        int fd;      

        static struct option long_options[] = 
        {
            {"rdonly", required_argument, 0, 'r'},
            {"wronly", required_argument, 0, 'w'},
            {"command", required_argument, 0, 'c'},
            {"verbose", no_argument, 0, 'v'},
            {0,0,0,0} 
        };

        int option_index = 0;
                
        a = getopt_long (argc, argv, "", long_options, &option_index);
        if (a == -1)
            break;
        
        switch(a)
        {
            case 'r': /* READ ONLY */
            { 
                if (optarg[0] == '-' && optarg[1] == '-')
                {
                    if (verboseTrue == 1)
                    {
                        printf("--wronly\n");
                    }
                    fprintf(stderr, "option '--wronly' requires an argument \n");
                    exitStatus = 1;
                   // optind--;
                    break;
                }

                if (verboseTrue == 1)
                    printf("--rdonly %s\n", optarg);
                
                if ((optind < argc && argv[optind][0] != '-'))
                {
                    exitStatus = 1;
                }  
                /*
                if ((optind < argc && optarg[2] != '-'))
                {
                    //printf("%d", optind);   
                    //printf("TACOS\n");
                    exitStatus = 1;
                }
                */
                fd = open_rdonly_f(optarg);
                handle_fd(fd); 
                break;
            }
            case 'w': /* WRITE ONLY */
            {
                
                if (optarg[0] == '-' && optarg[1] == '-')
                {
                    if (verboseTrue == 1)
                    {
                        printf("--wronly\n");
                    }
                    fprintf(stderr, "option '--wronly' requires an argument \n");
                    exitStatus = 1;
                   // optind--;
                    break;
                }
                

                if (verboseTrue == 1)
                {
                    printf("--wronly %s \n", optarg);
                }

                
                
                if ((optind < argc && argv[optind][0] != '-'))
                {
                    exitStatus = 1;
                }  
                
                
                fd = open_wronly_f(optarg);
                handle_fd(fd);
                break;
            }
            case 'c':
                // Bring optind back one to read arguments 
                optind--;
               
                // File Descriptor args
                int fdArgsLen = 0;
                int fdArgsCount = 0;
                int input = -1; 
                int output = -1;
                int error = -1; 

                // Command Args
                char* cmdArgs[100];
                int cmdArgsLen = 0;
                int cmdArgsIndex = 0; // Incrementer for cmdArgs
                int cmdArgsCount = 0;
                int cmdStatus;

                for (int i= 0; optind < argc; optind++)
                {
                    // Go through command args until we reach a new option
                    if(argv[optind][0] == '-' && argv[optind][1] == '-')
                    {
                        //printf("Discovered new option: %s \n",argv[optind]);
                        break;
                    }

                    // Process first three arguments
                    // Conditions: Must be a single digit
                    int temp; 
                    char* end;
                    if(i == 0 | i == 1 | i == 2)
                    {
                        fdArgsLen = strlen(argv[optind]);
                        if (fdArgsLen != 1) // Check that it is a single digit
                            fprintf(stderr,"Error! Argument is not a single character! \n");

                        temp = strtol(argv[optind], &end,0);
                        if(end == argv[optind]) // Not a digit
                            fprintf(stderr,"Error! Argument is not a digit! \n");

                        if(i == 0)  input = temp;
                        if(i == 1)  output = temp;
                        if(i == 2)  error = temp; 
                        fdArgsCount++;
                    }

                    // Parse remaining cmd arguments
                    else 
                    {
                        cmdArgsLen = strlen(argv[optind]);
                        cmdArgs[cmdArgsIndex] = malloc((cmdArgsLen+1)*sizeof(char));
                        strcpy(cmdArgs[cmdArgsIndex], argv[optind]);
                        cmdArgs[cmdArgsIndex][cmdArgsLen+1] = '\n';
                        cmdArgsIndex++;
                        cmdArgsCount++;
                    }

                    i++;
                }

                if(fdArgsCount != 3)
                    fprintf(stderr,"ERROR: Too little arguments \n");
                if(input == -1 | output == -1 | error == -1)
                {
                    fprintf(stderr,"ERROR: Command args incorrectly set \n");
                }

                // Append null pointer to cmdArgs
                cmdArgs[cmdArgsCount] = NULL;
                if( verboseTrue == 1)
                {
                    printf("--commmand");
                    for (int j = 0; j < cmdArgsCount; j++)
                    {
                        printf(" %s",cmdArgs[j]);
                    }
                    printf("\n");
                }
                cmdStatus = command(input,output,error,cmdArgs,cmdArgsCount);
                /*if (cmdStatus == -1) // FATAL ERROR - break parsing and return max exit status
                    loop = 0;
                */
                status_list[statusCount-1] = cmdStatus;

                break;
            case 'v':
                if (verboseTrue == 1)
                {
                    printf("--verbose \n");
                }
                verboseTrue = 1;
                break;
            default:
                abort();
            case '?':
                exitStatus = 1;
                break;
        }
    }

    // Return maximum exit status
    return exitStatus;    
}

