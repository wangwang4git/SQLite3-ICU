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
#include <unicode/ustring.h>
#include <unicode/ubrk.h>
#include <unicode/uregex.h>

typedef struct Node
{
  char* word;
  int start;
  int end;
  int isHan;
  struct Node* next;
} Node;

static char* input = NULL;
static Node* pList = NULL;
static URegularExpression* reExp = NULL;

void processOptions(int argc, char** argv);
void printUsage();

int initList(Node** pNode);
int insertLastList(Node **pNode, char* word, int start, int end, int isHan);
void clearList(Node *pHead);
void printList(Node *pHead);

void printTextRange(UChar* str, int32_t start, int32_t end);
void printEachForward(UBreakIterator* boundary, UChar* str);

void initRegExp()
{
  char* pattern = "\\p{script=han}{1}";

  UErrorCode status = U_ZERO_ERROR;
  UParseError parseErr;
  reExp = uregex_openC(pattern, 0, &parseErr, &status);
  if (U_FAILURE(status))
  {
      printf("error in pattern: \"%s\" at position %d\n", u_errorName(status), parseErr.offset);
      exit(-1);
  }
}

void closeRegExp()
{
  if (reExp != NULL)
  {
    uregex_close(reExp);
  }
}

// 中文检测
int detectHan(char* text)
{
    if (text == NULL)
    {
      return 0;
    }

    UChar stringToExamine[strlen(text) + 1];
    u_uastrcpy(stringToExamine, text);

    UErrorCode status = U_ZERO_ERROR;
    uregex_setText(reExp, stringToExamine, u_strlen(stringToExamine), &status);
    if (U_FAILURE(status))
    {
        printf("uregex_setUText error: %s\n", u_errorName(status));
        exit(1);
    }

    status = U_ZERO_ERROR;
    UBool doMatch = uregex_matches64(reExp, 0, &status);
    if (U_FAILURE(status))
    {
        printf("uregex_matches64 error: %s\n", u_errorName(status));
        exit(1);
    }

    return doMatch == TRUE ? 1 : 0;
}

int main(int argc, char** argv)
{
  processOptions(argc, argv);

  printf("Boundary Analysis\n"
    "-------------------\n\n");
  printf("Examining: %s\n", input);

  UBreakIterator* boundary;
  UErrorCode status = U_ZERO_ERROR;
  UChar stringToExamine[strlen(input) + 1];

  initRegExp();
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
  clearList(pList);
  closeRegExp();

  printf("\nEnd of boundary analysis\n");

  return 0;
}

void processOptions(int argc, char** argv)
{
  // for test
  if (argc == 1)
  {
    // 目前WeChat的活跃用户数有4,000,000,000左右。
    // QQ智能终端月活跃账户达到6.03亿，比去年同期增长23%。
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

int insertLastList(Node **pNode, char* word, int start, int end, int isHan)
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
  pInsert->isHan = isHan;

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
      printf("string[%2d..%2d] \"%s, isHan = %d, len = %ld\"\n", pHead->start, pHead->end, pHead->word, pHead->isHan, strlen(pHead->word));
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
  int isHan = detectHan(word);
  insertLastList(&pList, word, start, end - 1, isHan);

  str[end] = savedEndChar;
}

void printEachForward(UBreakIterator* boundary, UChar* str)
{
  int32_t end;
  int32_t start = ubrk_first(boundary);
  for (end = ubrk_next(boundary); end != UBRK_DONE; start = end, end = ubrk_next(boundary))
  {
      printTextRange(str, start, end );
  }
}
