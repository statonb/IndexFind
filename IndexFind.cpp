#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <getopt.h>
#include <iostream>
#include <string>
#include <map>

typedef struct
{
    std::string                 ticker;
    std::string                 name;
    int                         position;
}   stock_t;

typedef struct fn
{
    std::string                 fileName;
    std::map<std::string, int>  theMap;
    struct fn                   *pNext;
}   fileNode_t;

fileNode_t          *pHeadNode;
fileNode_t          *pWorkingNode;

int fileIngest(const char *fileName)
{
    char *cp;
    int value = 0;
    char lineBuf[256];
    char tempFilename[256];
    FILE *fp = fopen(fileName, "r");


    if ((FILE *)(NULL) == fp)
    {
        return 1;
    }

    strcpy(tempFilename, fileName);
    cp = strrchr(tempFilename, '.');
    if ((char *)(NULL) != cp)
    {
        *cp = '\0';
    }
    cp = strrchr(tempFilename, '/');
    if ((char *)(NULL) == cp)
    {
        cp = tempFilename;
    }
    else
    {
        cp++;
    }
    pWorkingNode->fileName = (std::string)cp;

    while (fgets(lineBuf, sizeof(lineBuf), fp))
    {
        cp = strchr(lineBuf, ',');
        if ((char *)(NULL) == cp)
        {
            fclose(fp);
            return 2;
        }
        *cp = '\0';
        pWorkingNode->theMap[(std::string)lineBuf] = ++value;
    }
    fclose(fp);
    pWorkingNode->pNext = new fileNode_t;
    pWorkingNode = pWorkingNode->pNext;
    pWorkingNode->pNext = (fileNode_t *)(NULL);

    return 0;
}

int processList(const char *fileName)
{
    char lineBuf[256];
    char *cp;
    int retVal = 0;
    FILE *fp = fopen(fileName, "r");

    if ((FILE *)(NULL) == fp)
    {
        return 1;
    }

    while   (   (fgets(lineBuf, sizeof(lineBuf), fp))
             && (0 == retVal)
            )
    {
        cp = strrchr(lineBuf, '\n');
        if ((char *)(NULL) != cp)
        {
            *cp = '\0';
        }
        retVal = fileIngest(lineBuf);
    }
    fclose(fp);
    return retVal;
}

void usage(void)
{
    printf("\nUsage Error\n");
}

int main(int argc, char *argv[])
{
    int                                     opt;
    bool                                    fileFound = false;
    bool                                    usageError = false;
    bool                                    quietFlag = false;
    std::map<std::string, int>::iterator    it;
    std::string                             stockSymbol = "";

    pHeadNode = new fileNode_t;
    pHeadNode->pNext = (fileNode_t *)(NULL);
    pWorkingNode = pHeadNode;

    struct option longOptions[] =
    {
        {"input",           required_argument,  0,      'i'}
        ,{"list",           required_argument,  0,      'l'}
        ,{"stock",          required_argument,  0,      's'}
        ,{"quiet",          no_argument,        0,      'q'}
        ,{0,0,0,0}
    };

    while (!usageError)
    {
        int optionIndex = 0;
        opt = getopt_long(argc, argv, "i:l:s:qh?", longOptions, &optionIndex);

        if (-1 == opt) break;

        switch (opt)
        {
        case 'i':
            if (0 == fileIngest(optarg))
            {
                fileFound = true;
            }
            else
            {
                usageError = true;
            }
            break;
        case 'l':
            if (0 == processList(optarg))
            {
                fileFound = true;
            }
            else
            {
                usageError = true;
            }
            break;
        case 's':
            stockSymbol = (std::string)optarg;
            break;
        case 'q':
            quietFlag = true;
            break;
        case 'h':
        case '?':
        default:
            usageError = true;
            break;
        }
    }

    // fileFound = (0 == fileIngest((const char *)("/mnt/omv1share1/Workspace/IndexFind/SPMidCap.csv")));
    // fileFound = (0 == processList((const char *)("/mnt/omv1share1/Workspace/IndexFind/MidCap.lst")));

    if  (   ("" == stockSymbol)
         || (true == usageError)
         || (false == fileFound)
        )
    {
        usage();
        return 1;
    }

    if (false == quietFlag)
    {
        pWorkingNode = pHeadNode;
        // If this node has been populated then the next node will be non-null
        while ((fileNode_t *)(NULL) != pWorkingNode->pNext)
        {
            std::cout << pWorkingNode->fileName << " has " << pWorkingNode->theMap.size() << " entries\n";
            pWorkingNode = pWorkingNode->pNext;
        }
    }

    pWorkingNode = pHeadNode;
    std::cout << stockSymbol << "\n";
    while ((fileNode_t *)(NULL) != pWorkingNode->pNext)
    {
        std::cout << '\t' << pWorkingNode->fileName << ": ";
        it = pWorkingNode->theMap.find(stockSymbol);
        if (it != pWorkingNode->theMap.end())
        {
            std::cout << it->second << '\n';
        }
        else
        {
            std::cout << "Not Found\n";
        }
        pWorkingNode = pWorkingNode->pNext;
    }

    return 0;
}
