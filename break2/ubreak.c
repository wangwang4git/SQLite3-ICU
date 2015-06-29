/*
*******************************************************************************
*
*   中文分词
*   分词规则：按字切分；合并双汉字；剔除标点；
*
*******************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unicode/ustring.h>
#include <unicode/ubrk.h>

typedef struct Node
{
  char* word;
  int start;
  int end;
  struct Node* next;
} Node;

static char* input;
static Node* pList = NULL;

void processOptions(int argc, char** argv);
void printUsage();

int initList(Node** pNode);
int insertLastList(Node **pNode, char* word, int start, int end);
void clearList(Node *pHead);
void printList(Node *pHead);

void printTextRange(UChar* str, int32_t start, int32_t end);
void printEachForward( UBreakIterator* boundary, UChar* str);

int main(int argc, char** argv)
{
  processOptions(argc, argv);

  printf("Boundary Analysis\n"
    "-------------------\n\n");
  printf("Examining: %s\n", input);

  UBreakIterator* boundary;
  UErrorCode status = U_ZERO_ERROR;
  UChar stringToExamine[strlen(input) + 1]; 

  initList(&pList);
  u_uastrcpy(stringToExamine, input);
  boundary = ubrk_open(UBRK_WORD, "zh_CN", stringToExamine, u_strlen(stringToExamine), &status);
  if (U_FAILURE(status))
  {
    printf("ubrk_open error: %s\n", u_errorName(status));
    exit(1);
  }

  printf("\n----- Word Boundaries: -----------\n"); 
  printEachForward(boundary, stringToExamine);

  printList(pList);

  ubrk_close(boundary);
  // clearList(pList);

  printf("\nEnd of boundary analysis\n");

  return 0;
}

void processOptions(int argc, char** argv)
{
  // for test
  if (argc == 1)
  {
    input = "QQ智能终端月活跃账户达到6.03亿，比去年同期增长23%。";
    return;
  }

  int optInd;
  UBool doUsage = FALSE;
  char* arg;

  for (optInd = 1; optInd < argc; ++optInd)
  {
    arg = argv[optInd];

    if (strcmp(arg, "--help") == 0)
    {
        doUsage = TRUE;
    }
    /* POSIX.1 says all arguments after -- are not options */
    else if (strcmp(arg, "--") == 0)
    {
        /* skip the -- */
        ++optInd;
        break;
    }
    /* unrecognized option */
    else if (strncmp(arg, "-", strlen("-")) == 0)
    {
        printf("ubreak: invalid option -- %s\n", arg + 1);
        doUsage = TRUE;
    }
    else
    {
        break;
    }
  }

  if (doUsage)
  {
    printUsage();
    exit(0);
  }

  int  remainingArgs = argc - optInd;
  if (remainingArgs < 1)
  {
    printf("ubreak:  string are missing.\n");
    printUsage();
    exit(1);
  }

  if (remainingArgs > 1)
  {
    // More than one string to be processed.
    // pass
  }

  input = argv[optInd];
}

void printUsage()
{
  printf("ubreak [options] string...\n"
    "\t--help\t\tdisplay this help and exit\n"
    "\t--\t\tstop further option processing\n"
    );
  exit(0);
}

// 带头节点链表
int initList(Node** pNode)
{
  *pNode = (Node *) malloc(sizeof(Node));
  if (*pNode == NULL)
  {
    printf("malloc fail\n");
    exit(1);
  }

  memset(*pNode, 0, sizeof(Node));

  return 0;
}

int insertLastList(Node **pNode, char* word, int start, int end)
{
  Node* pHead = *pNode;
  Node* pTmp = pHead;
  Node* pInsert;

  pInsert = (Node *) malloc(sizeof(Node));
  if (pInsert == NULL)
  {
    printf("malloc fail\n");
    exit(1);
  }
  memset(pInsert, 0, sizeof(Node));
  pInsert->word = word;
  pInsert->start = start;
  pInsert->end = end;

  while (pTmp->next != NULL)
  {
    pTmp = pTmp->next;
  }
  pTmp->next = pInsert;
  *pNode = pHead;

  return 0;
}

void clearList(Node *pHead)
{
  if (pHead == NULL)
  {
    return;
  }

  Node *pNext; 
  while(pHead->next != NULL)
  {
    pNext = pHead->next;
    free(pHead->word);
    free(pHead);
    pHead = pNext;
  }
}

void printList(Node *pHead)
{
  if(NULL == pHead)
  {
    printf("list null\n");
  }
  else
  {
    pHead = pHead->next;
    while(NULL != pHead)
    {
      printf("string[%2d..%2d] \"%s\"\n", pHead->start, pHead->end, pHead->word); 
      pHead = pHead->next;
    }
  }
}

void printTextRange(UChar* str, int32_t start, int32_t end)
{
  char charBuf[1000];
  UChar savedEndChar;

  savedEndChar = str[end];
  str[end] = 0;
  u_austrncpy(charBuf, str + start, sizeof(charBuf) - 1);
  charBuf[sizeof(charBuf) - 1] = 0;
  // printf("string[%2d..%2d] \"%s\"\n", start, end - 1, charBuf); 

  char* word = malloc(strlen(charBuf) + 1);
  if (word == NULL)
  {
    printf("malloc fail\n");
    exit(1);
  }
  memset(word, 0, strlen(charBuf) + 1);
  strcpy(word, charBuf);
  insertLastList(&pList, word, start, end - 1);

  str[end] = savedEndChar;
}

void printEachForward( UBreakIterator* boundary, UChar* str)
{
  int32_t end;
  int32_t start = ubrk_first(boundary);
  for (end = ubrk_next(boundary); end != UBRK_DONE; start = end, end = ubrk_next(boundary))
  {
      printTextRange(str, start, end );
  }
}
