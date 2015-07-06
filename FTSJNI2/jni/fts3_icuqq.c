/*
**
** QQ ICU分词器
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unicode/ustring.h>
#include <unicode/ubrk.h>
#include <unicode/uregex.h>
#include <unicode/utf16.h>
#include <unicode/utf8.h>
#include <unicode/uchar.h>

typedef struct Node Node;

struct Node
{
  char* word;
  int start;
  int end;
  // 0 其他，1 汉字，2 英文，3 数字
  int isHan;
  struct Node* next;
};

static Node* pList = NULL;
static URegularExpression* reExp = NULL;

int initList(Node** pNode);
int insertLastList(Node **pNode, char* word, int start, int end, int isHan);
void mergeList(Node **pFirst, Node *pSecond);
void clearList(Node *pHead);
void printList(Node *pHead);

void initRegExp();
void closeRegExp();
int detectHan(char* text);

void printTextRange(UChar* str, int32_t start, int32_t end);
void printEachForward(UBreakIterator* boundary, UChar* str);
void mergeDoubleHan(Node *pHead);
void deletePunctuation(Node *pNode);

int icuOpen(char* zInput)
{
  int size = strlen(zInput) + 1;
  UChar stringToExamineOrig[size];
  UChar stringToExamine[size];
  u_uastrcpy(stringToExamineOrig, zInput);
  UErrorCode status = U_ZERO_ERROR;

  // 英文转小写
  long length = u_strToLower(stringToExamine, size, stringToExamineOrig, u_strlen(stringToExamineOrig), "en", &status);
  if (U_FAILURE(status) || stringToExamine[length] != 0)
  {
    printf("error in u_strToLower(English locale) = %ld error = %s\n", length, u_errorName(status));
    return status;
  }

  status = U_ZERO_ERROR;
  UBreakIterator* boundary = ubrk_open(UBRK_CHARACTER, "zh_CN", stringToExamine, u_strlen(stringToExamine), &status);
  if (U_FAILURE(status))
  {
    printf("ubrk_open error: %s\n", u_errorName(status));
    return status;
  }

  initList(&pList);
  initRegExp();

  // step 1: 按字符切分；
  printEachForward(boundary, stringToExamine);
  // step 2: 合并双汉字；
  // mergeDoubleHan(pList);
  // step 3: 剔除标点；
  deletePunctuation(pList);

  // printList(pList);

  closeRegExp();
  ubrk_close(boundary);

  return U_ZERO_ERROR;
}

int icuClose()
{
  clearList(pList);
  return U_ZERO_ERROR;
}

char* getSegmentedMsg(char* msg)
{
  icuOpen(msg);

  char* result = (char*) malloc(2048 * sizeof(char));
  if (result == NULL)
  {
    printf("malloc fail\n");
    exit(1);
  }
  memset(result, 0, 2048 * sizeof(char));

  Node* pHead = pList;
  if(NULL != pHead)
  {
    pHead = pHead->next;
    while(NULL != pHead)
    {
      strcat(result, pHead->word);
      strcat(result, " ");
      pHead = pHead->next;
    }
  }

  icuClose();

  return result;
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

void mergeList(Node **pFirst, Node *pSecond)
{
  if(NULL == pSecond)
  {
    return;
  }
  Node* pHead = *pFirst;
  Node* pTmp = pHead;

  while (pTmp->next != NULL)
  {
    pTmp = pTmp->next;
  }
  pTmp->next = pSecond;
  *pFirst = pHead;
}

void clearList(Node *pHead)
{
  if (pHead == NULL)
  {
    return;
  }

  Node *pNext;
  while(pHead != NULL)
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
      // printf("string[%2d..%2d] \"%s, isHan = %d, len = %ld\"\n", pHead->start, pHead->end, pHead->word, pHead->isHan, strlen(pHead->word));
      printf("string[%2d..%2d] \"%s, len = %d\"\n", pHead->start, pHead->end, pHead->word, strlen(pHead->word));
      pHead = pHead->next;
    }
  }
}

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

void printTextRange(UChar* str, int32_t start, int32_t end)
{
  char charBuf[1024];
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
  if (isHan == 0)
  {
    char ch = word[0];
    if (isalpha(ch))
    {
      isHan = 2;
    }
    else if (isdigit(ch))
    {
      isHan = 3;
    }
  }
  insertLastList(&pList, word, start, end, isHan);

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

void mergeDoubleHan(Node *pHead)
{
  if(NULL == pHead)
  {
    return;
  }

  Node* pDoubleList = NULL;
  pHead = pHead->next;
  UBool finded = FALSE;
  while(NULL != pHead && NULL != pHead->next)
  {
    Node* next = pHead->next;
    // check double han
    if (pHead->isHan == 1 && next->isHan == 1)
    {
      if (!finded)
      {
        finded = TRUE;

        // init
        pDoubleList = (Node *) malloc(sizeof(Node));
        if (pDoubleList == NULL)
        {
          printf("malloc fail\n");
          exit(1);
        }

        memset(pDoubleList, 0, sizeof(Node));
        int size = strlen(pHead->word) + strlen(next->word) + 1;
        pDoubleList->word = malloc(size);
        memset(pDoubleList->word, 0, size);
        strcat(pDoubleList->word, pHead->word);
        strcat(pDoubleList->word, next->word);
        pDoubleList->start = pHead->start;
        pDoubleList->end = next->end;
        pDoubleList->isHan = 1;
      }
      else
      {
        int size = strlen(pHead->word) + strlen(next->word) + 1;
        char* word = malloc(size);
        memset(word, 0, size);
        strcat(word, pHead->word);
        strcat(word, next->word);
        int start = pHead->start;
        int end = next->end;
        int isHan = 1;

        insertLastList(&pDoubleList, word, start, end, isHan);
      }
    }

    pHead = pHead->next;
  }

  if (finded)
  {
    mergeList(&pList, pDoubleList);
  }
}

void deletePunctuation(Node *pNode)
{
  Node* pHead = pNode;
  Node* p = pHead;
  Node* q = p->next;

  while (q != NULL)
  {
    if (q->isHan == 0)
    {
      p->next = q->next;
      free(q->word);
      free(q);
      q = p->next;
    }
    else
    {
      p = q;
      q = p->next;
    }
  }
}
